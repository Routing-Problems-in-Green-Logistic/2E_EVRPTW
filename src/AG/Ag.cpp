/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    01/09/22
 *  Arquivo: ag.cpp
 * ****************************************
 * ****************************************/

#include "Ag.h"
#include "../mersenne-twister.h"

NS_Ag::RandomKey::RandomKey(Instance &instancia)
{

    int tam   = instancia.numClients + instancia.numRechargingS*instancia.numUtilEstacao*instancia.numEv;
    vetRandom = std::vector<RandomNo>(tam);
    vetDecod  = std::vector<RandomNo>(tam);

    int vetIndex = 0;

    for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
    {

        for(int quant = 0; quant < instancia.numUtilEstacao*instancia.numEv; ++quant, ++vetIndex)
        {
            vetRandom[vetIndex].cliente = rs;
            vetRandom[vetIndex].chave   = (rand_u32()%1000)/1000.0;
        }

    }

    for(int clie=instancia.getFirstClientIndex(); clie <= instancia.getEndClientIndex(); ++clie, ++vetIndex)
    {
        vetRandom[vetIndex].cliente = clie;
        vetRandom[vetIndex].chave   = (rand_u32()%1000)/1000.0;
    }

}

// Para cada cliente(RS, 2°, 3° RS, clientes) gera um valor aleatorio para chave [0, 1)
void NS_Ag::RandomKey::geraRandoKey()
{
    for(auto it:vetRandom)
        it.chave = (rand_u32()%1000)/1000.0;
}

void NS_Ag::RandomKey::ordenaVetDecod()
{
    std::copy(vetRandom.begin(), vetRandom.end(), vetDecod.begin());
    std::sort(vetDecod.begin(), vetDecod.end());

}

void NS_Ag::RandomKey::printVetDecod(string &str)
{
    for(auto clie:vetDecod)
        str += to_string(clie.cliente)+"\t";

    str += "\n";

    for(auto clie:vetDecod)
        str += to_string(clie.chave)+"\t";

    str += "\n";
}

bool NS_Ag::atendeTodosOsClientes(const std::vector<int> &vetClienteAtend)
{
    for(const int &it:vetClienteAtend)
    {
        if(it == 0)
            return false;
    }

    return true;
}

/** ******************************************************************************
 ** ******************************************************************************
 **
 ** @param instancia
 ** @param randKey                  Chama ordenaVetDecod(); modificado
 ** @param sat                      Guarda a solucao para o satelite; modificado
 ** @param vetSatAtendCliente       Indica o sat para cada cliente; nao modificado
 ** @param tempoSaidaSat            Tempo de saida para sat, se <= 0: tempoSaida=vetTempoSaida[sat]; nao modificado
 **
 ** ******************************************************************************
 ** ******************************************************************************/

void NS_Ag::decodificaSol(Instance &instancia, RandomKey &randKey, Satelite &sat, const vector<int> &vetSatAtendCliente, const double tempoSaidaSat)
{

    /* *************************************************************************************************************************************
     * *************************************************************************************************************************************
     *
     * DECODIFICACAO DA SOLUCAO:
     *
     * Sequencia: 2 3 4 5 6 7 ...
     *
     * posAnt: posicao mais a esq que nao foi utilizada
     * pos:    posicao atual
     *
     * Inicio veiculo: sat
     *
     * Tenta adicionar o arco: (sat 2):
     *
     *      .Verificacoes:
     *
     *          Bat:    é possivel chegar em 2 com a bat atual?
     *          carga:  Carga do veiculo + carga(2) <= capacidade(veicEv)?              (false) => (backTrack(2) <- false)
     *          TW:     t_sai(sat) + t(sat 2) <= tw_fim(2)?                             (false) => (backTrack(2) <- false)
     *
     *          Viabilidade de (2 sat) e (2 rs), se nao for possivel, 2 eh descartado   (backTrack(2) <- true)
     *
     *
     *      .Se o arco (sat, 2) eh adicionado a sol e 2 == rs:
     *
     *          pos deve retroceder para ??
     *
     *      .Caso contrario:
     *
     *          pos nao retrocede!
     *
     *
     *      .vet backTrack:
     *
     *          Indica se a posicao deve ser testada novamente, no mesmo veic. Ex: Se nao eh possivel chergar em um cliente por conta da bat,
     *              se o veicEv eh recaregado, essa pos pode se tornar viavel;
     *
     *          Quando recarregar a bat, backTrack <- true para todas as possicoes
     *
     *      .pos so retrocede para posAnt quando atinge uma rs ou comeca um novo veiculo
     *
     *
     * *************************************************************************************************************************************
     * *************************************************************************************************************************************/

    randKey.ordenaVetDecod();

    double tempoS = tempoSaidaSat;
    if(tempoS <= 0.0)
        tempoS = instancia.vetTempoSaida[sat.sateliteId];

    // Reseta o satelite
    for(int ev=0; ev < instancia.numEv; ++ev)
    {
        sat.vetEvRoute[ev].routeSize = 1;
        sat.vetEvRoute[ev].route[1].cliente = sat.vetEvRoute[ev].route[0].cliente = sat.sateliteId;
        sat.vetEvRoute[ev].route[0].tempoSaida = tempoS;
    }


    // Seta 0 para o cliente que deve ser atendido por sat e 1 caso contrario. Rs nao sao marcadas nesse vetor
    vector<int> vetClieAtend(vetSatAtendCliente);

    for(int &it : vetClieAtend)
    {
        if(it == sat.sateliteId)
            it = 0;
        else
            it = 1;
    }


    int posAnt     = 0;                                              // Guarda a posicao mais a esquerda que nao foi utilizada
    int pos        = 0;                                              // Guarda a posicao atual
    int ev         = 0;
    int clienteAnt = sat.sateliteId;
    static std::vector<bool> vetBackTrack(instancia.numNos*2);       // Indica se a posicao deve ser testada novamente, no mesmo veic

     // Incrementa posAnt se vetDecod eh invalido
    auto avancaPosAnt = [&]()
    {

        while(randKey.vetDecod[posAnt].chave < 0.0)
            posAnt += 1;
    };


    // Cada iteracao do do-while um novo veiculo eh gerado
    // Inicio do while(!atendeTodosOsClientes())
    do
    {
        std::fill(vetBackTrack.begin()+posAnt, vetBackTrack.begin()+randKey.vetDecod.size(), true);

        avancaPosAnt();

        pos = posAnt;
        bool naoSat = true;

        while(naoSat)
        {
            // Encontra um cliente que respeita a capacidade do ev, nao foi atendido e eh diferente do cliente anterior
            while(!((sat.vetEvRoute[ev].demanda + instancia.getDemand(randKey.vetDecod[pos].cliente)) <= instancia.getEvCap(ev) &&
                  vetBackTrack.at(pos) && vetClieAtend[randKey.vetDecod[pos].cliente] == 0 && randKey.vetDecod[pos].cliente != clienteAnt &&
                  randKey.vetDecod[pos].chave >= 0.0))
            {
                if((sat.vetEvRoute[ev].demanda + instancia.getDemand(randKey.vetDecod[pos].cliente)) > instancia.getEvCap(ev))
                    vetBackTrack[pos] = false;

                pos += 1;

                // Verifica se chegou ao final
                if(pos == randKey.vetDecod.size())
                {
                    pos = -1;
                    naoSat = false;
                    break;

                }
            }

            if(naoSat && pos != -1)
            {
                const int cliente = randKey.vetDecod[pos].cliente;

                // Verifica a utilizacao max da rs
                if(instancia.isRechargingStation(cliente) && sat.vetEvRoute[ev].getUtilizacaoRecarga(cliente) == instancia.numUtilEstacao)
                {
                    naoSat = true;
                    continue;
                }

                int routeSize = sat.vetEvRoute[ev].routeSize;
                const EvNo &evNo = sat.vetEvRoute[ev].route[routeSize-1];
                const double dist = instancia.getDistance(evNo.cliente, cliente);

                double tempoChegada = evNo.tempoSaida + dist;
                double tempoSaida = tempoChegada + instancia.vectCliente[cliente].tempoServico;
                double bateria = evNo.bateriaRestante - dist;
                const double iniTW = instancia.getInicioJanelaTempoCliente(cliente);
                const double fimTW = instancia.getFimJanelaTempoCliente(cliente);

                if(tempoChegada < iniTW)
                    tempoChegada = iniTW;

                // Verifica viabilidade da janela de tempo
                if(!((tempoChegada <= fimTW) || (abs(tempoChegada - fimTW) <= TOLERANCIA_JANELA_TEMPO)))
                {
                    vetBackTrack[pos] = false;
                    pos = posAnt;
                    naoSat = true;
                    continue;
                }

                // Verifica viabilidade da bateria
                if(bateria < -TOLERANCIA_BATERIA)
                {

                    //vetBackTrack[pos] = false;
                    pos = posAnt;
                    naoSat = true;
                    continue;
                }

                bool voltaSatRS = true;

                if(instancia.isRechargingStation(cliente))
                {

                    double dif = instancia.getEvBattery(ev) - bateria;
                    tempoSaida += dif * instancia.vectVeiculo[ev].taxaRecarga;
                    bateria = instancia.getEvBattery(ev);

                    //pos = posAnt;

                } else
                {
                    // Verifica se eh possivel voltar ao sat
                    double batTemp = bateria + instancia.getDistance(cliente, sat.sateliteId);
                    if(batTemp < -TOLERANCIA_BATERIA)
                    {

                        // Verifica se eh possivel chegar a uma rs
                        int posTemp = pos+1;                       // Pode existir uma rs antes de pos??

                        bool cond = randKey.vetDecod[posTemp].chave >= 0 &&
                                    (instancia.isRechargingStation(randKey.vetDecod[posTemp].cliente) &&
                                     sat.vetEvRoute[ev].getUtilizacaoRecarga(randKey.vetDecod[posTemp].cliente) <
                                     instancia.numUtilEstacao);

                        while(!cond)
                        {
                            posTemp += 1;
                            if(posTemp == randKey.vetDecod.size() - 1)
                                break;

                            cond = randKey.vetDecod[posTemp].chave >= 0 &&
                                   (instancia.isRechargingStation(randKey.vetDecod[posTemp].cliente) &&
                                    sat.vetEvRoute[ev].getUtilizacaoRecarga(randKey.vetDecod[posTemp].cliente) <
                                    instancia.numUtilEstacao);

                        }

                        if(cond)
                        {
                            // cliente eh valido
                        }
                        else
                        {
                            // cliente eh invalido
                            pos += 1;
                            naoSat = true;
                            continue;
                        }

                    }

                }

                if(pos >= 0)
                {
                    randKey.vetDecod[pos].chave = -1.0;

                    if(instancia.isRechargingStation(cliente))
                    {
                        avancaPosAnt();
                        pos = posAnt;
                    }
                    else
                        pos += 1;

                    // Escreve em sat
                    EvRoute &evRoute = sat.vetEvRoute[ev];
                    double dem = instancia.getDemand(cliente);
                    sat.demanda += dem;
                    evRoute.demanda += dem;

                    const int posEvRoute = evRoute.routeSize;
                    evRoute[posEvRoute].cliente = cliente;
                    evRoute[posEvRoute].tempoCheg = tempoChegada;
                    evRoute[posEvRoute].tempoSaida = tempoSaida;
                    evRoute[posEvRoute].bateriaRestante = bateria;
                    evRoute.routeSize += 1;

                    evRoute.distancia += dist;
                    sat.distancia += dist;
                }



            } // End if(naoSat)


        } // End while(naoSat)

        // Se nao existe cliente, volta para o sat
        if(!naoSat)
        {

        }
        else
        {

        }

        ev += 1;
        if(ev >= instancia.numEv)
            break;

    }while(!atendeTodosOsClientes(vetClieAtend));



}