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
            vetRandom[vetIndex].chave   = (rand_u32()%100)/1000.0;
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

    randKey.ordenaVetDecod();

    double tempoS = tempoSaidaSat;
    if(tempoS <= 0.0)
        tempoS = instancia.vetTempoSaida[sat.sateliteId];

    // Reseta o satelite
    for(int ev=0; ev < instancia.numEv; ++ev)
    {
        sat.vetEvRoute[ev].routeSize = 2;
        sat.vetEvRoute[ev].route[1].cliente = sat.vetEvRoute[ev].route[0].cliente = sat.sateliteId;
        sat.vetEvRoute[ev].route[0].tempoSaida = instancia.vetTempoSaida[sat.sateliteId];
    }


    // Seta 0 para o cliente que deve ser atendido por sat e 1 caso contrario
    vector<int> vetClieAtend(vetSatAtendCliente);

    for(int &it : vetClieAtend)
    {
        if(it == sat.sateliteId)
            it = 0;
        else
            it = 1;
    }


    int posAnt     = 0;     // Guarda a posicao mais a esquerda que nao foi utilizada
    int pos        = 0;     // Guarda a posicao atual
    int ev         = 0;
    int clienteAnt = sat.sateliteId;
    static std::vector<bool> vetBackTrack(instancia.numNos*2);


    // Cada iteracao do do-while um novo veiculo eh gerado
    // Inicio do while(!atendeTodosOsClientes))
    do
    {
        std::fill(vetBackTrack.begin()+posAnt, vetBackTrack.begin()+randKey.vetDecod.size(), true);

        pos = posAnt;
        bool clienteInv = true;

        while(clienteInv)
        {
            // Encontra um cliente que respeita a capacidade do ev
            while((sat.vetEvRoute[ev].demanda + instancia.getDemand(randKey.vetDecod[pos].cliente)) > instancia.getEvCap(ev) ||
                  !vetBackTrack[pos] || vetClieAtend[pos])
            {
                pos += 1;

                if(pos == randKey.vetDecod.size())
                {
                    pos = -1;
                    clienteInv = false;
                    break;

                }

            }

            const int cliente = randKey.vetDecod[pos].cliente;
            if(instancia.isRechargingStation(cliente) && sat.vetEvRoute[ev].getUtilizacaoRecarga(cliente) == instancia.numUtilEstacao)
            {
               pos += 1;


               if(pos == randKey.vetDecod.size())
               {
                   clienteInv = false;
                   pos = -1;
                   break;
               }
            }

            // Verifica se eh possivel chegar ate pos
            if(pos != -1)
            {
                const int cliente = randKey.vetDecod[pos].cliente;

                int routeSize = sat.vetEvRoute[ev].routeSize;
                const EvNo &evNo = sat.vetEvRoute[ev].route[routeSize-1];
                const double dist = instancia.getDistance(evNo.cliente, cliente);

                double tempoChegada = evNo.tempoSaida + dist;
                double tempoSaida   = tempoChegada + dist;
                double bateria      = evNo.bateriaRestante - dist;
                const double iniTW  = instancia.getInicioJanelaTempoCliente(cliente);
                const double fimTW  = instancia.getFimJanelaTempoCliente(cliente);

                if(tempoChegada < iniTW)
                    tempoChegada = iniTW;

                // Verifica viabilidade da janela de tempo
                if(!((tempoChegada <= fimTW) || (abs(tempoChegada-fimTW) <= TOLERANCIA_JANELA_TEMPO)))
                {
                    vetBackTrack[pos] = false;
                    pos = posAnt;
                    clienteInv = true;
                    continue;
                }

                // Verifica viabilidade da bateria
                if(bateria < -TOLERANCIA_BATERIA)
                {

                    //vetBackTrack[pos] = false;
                    pos = posAnt;
                    clienteInv = true;
                    continue;
                }

                if(instancia.isRechargingStation(cliente))
                {

                    double dif = instancia.getEvBattery(ev) - bateria;
                    tempoSaida += dif*instancia.vectVeiculo[ev].taxaRecarga;
                    bateria = instancia.getEvBattery(ev);
                }
                else
                {
                    // Verifica se eh possivel voltar ao sat ou a uma estacao

                }

                // Escreve em sat


            }

        } // End while(clienteInv)

        if(pos != -1)
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