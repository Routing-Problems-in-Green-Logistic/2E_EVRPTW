//
// Created by ****** on 23/01/2022.
//

#include <chrono>
#include "ViabilizadorRotaEv.h"
#include "Auxiliary.h"

using namespace NS_viabRotaEv;
using namespace NS_Auxiliary;

static int num_G = 0;

#if TEMPO_FUNC_VIABILIZA_ROTA_EV
    double NS_viabRotaEv::global_tempo = 0.0;
#endif

/**
 *
 * Tenta viabilizar a rota inserindo um estacao de recarga em uma posicao da rota
 *
 * @param evRoute               Eh alterado durante a execucao
 * @param pos
 * @param instance
 * @param best
 * @param insercaoEstacao       Guarda o custo da rota viabilizada quando a func retorna true
 * @return
 */

/*
 *      Rota: 0 1 2 3 4 0       Dist 50
 * Nova Rota: 0 1 2 3 4 5 0     Dist 60         **best
 * Incr: 10
 *
 * Nova Rota2 0 1 2 3 5 4 0   Dist 54
 * Incr: 4
 *
 * Dif: 10 - 4: 6
 *
 * Custo de insercao da estacao tem que ser menor que 6
 *
 */


bool NS_viabRotaEv::viabilizaRotaEv(EvRoute &evRoute, Instancia &instance, const bool best,
                                    NS_viabRotaEv::InsercaoEstacao &insercaoEstacao, double custoInserMax,
                                    const bool construtivo, const double tempoSaidaSat,
                                    Vector<int> *vetInviabilidade)
{


#if TEMPO_FUNC_VIABILIZA_ROTA_EV

    static auto start = std::chrono::high_resolution_clock::now();
    static auto end   = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> duracao = end - start;

    start = std::chrono::high_resolution_clock::now();


    //end   = std::chrono::high_resolution_clock::now();
    //duracao = end - start;
    //global_tempo += duracao.count();

#endif

    /* *************************************************************************************************************
     * *************************************************************************************************************
     * Tenta viabilizar a rota inserindo um estacao de recarga em uma posicao da rota
     *
     * evRoute nao eh uma rota valida, somente o cliente eh valido.
     * *************************************************************************************************************
     * *************************************************************************************************************
     */

    double aux = 0.0;

/*    PRINT_DEBUG("", "");
    evRoute.print(instance, true);*/

/*    num_G += 1;
    PRINT_DEBUG("", "");
    cout<<"num: "<<num_G<<"\n\n";*/

    static EvRoute evRouteSta(-1, -1, evRoute.routeSizeMax, instance);
    evRouteSta.copiaCliente(evRoute);

    //evRouteSta.print(instance, true);

    bool existeEstacao = false;

    // Verifica se todas as estacoes estao sendo utilizadas
    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {
        if(evRoute.getUtilizacaoRecarga(i) < evRoute.numMaxUtilizacao)
        {
            existeEstacao = true;
            break;
        }
    }

    if(!existeEstacao)
    {

        if(vetInviabilidade)
            (*vetInviabilidade)[Inv_nao_ev_rs] += 1;

        return false;
    }

    // verifica se a rota atende as janelas de tempo
    if(testaRotaTempo(evRouteSta, evRouteSta.routeSize, instance, true, tempoSaidaSat, 0) <= 0.0)
    {
        //cout<<"Rota eh invalida pela janela de tempo\n\n";


        if(vetInviabilidade)
            (*vetInviabilidade)[Inv_tw] += 1;

        return false;
    }

/*    PRINT_DEBUG("", "");
    string str;
    evRouteSta.print(str, instance, false);
    cout<<"evRouteSta: "<<str<<"\n";
    str = "";
    evRoute.print(str, instance, false);
    cout<<"evRoute: "<<str<<"\n\n";*/


    // Encontra uma posicao inicial e final tal que ate inicio eh viavel e do final ate o fim da rota eh viavel (pode nao ser possivel)
    // posInv possui a posicao em que a primeira inviabilidade ocorreu
    int inicio  = 0;
    int fim     = evRoute.routeSize-2;          // = ultimo cliente antes do satelite
    int posInv  = -1;

    for(int i=1; i < evRouteSta.routeSize; ++i)
    {
        if(evRouteSta[i].bateriaRestante < -TOLERANCIA_BATERIA)
        {
            posInv = i;
            break;
        }


        if(instance.isRechargingStation(evRouteSta[i].cliente))
            inicio = i;

    }

    if(construtivo)
    {
        for(int i=evRouteSta.routeSize-1; i >= 0 && i > posInv; --i)
        {
            if(instance.isRechargingStation(evRouteSta[i].cliente))
                fim = i;
        }
    }

    //cout<<"inicio: "<<inicio<<"\nposInv: "<<posInv<<"\nfim: "<<fim<<"\n";

    insercaoEstacao.distanciaRota = DOUBLE_MAX;
    insercaoEstacao.estacao = -1;
    insercaoEstacao.pos = -1;

    // Percorre a rota inserindo em cada arco (i,j) a estacao mais proxima
    // Criterio: viabilidade e (depois custo)
    for(int i=inicio; i < posInv ; ++i)
    {

        const auto  *const vet = instance.getEstacoes(evRouteSta[i].cliente, evRouteSta[i+1].cliente);

        if(vet == nullptr)
        {
            cout<<"i: "<<evRouteSta[i].cliente<<"; j: "<<evRouteSta[i+1].cliente<<"\n\n";
            string strRota;
            evRoute.print(strRota, instance, true);
            cout<<"ROTA: "<<strRota<<"\n\n";

            PRINT_DEBUG("", "vetor NULL");
            throw "NULL";
        }

        for(int est=0; est < instance.numEstacoesPorArco; ++est)
        {
            if(vet[est] != -1)
            {
                if(evRoute.getUtilizacaoRecarga(vet[est]) + 1 <= instance.numUtilEstacao)
                {


                    const int estacao = vet[est];
                    const double distInser = -instance.getDistance(evRouteSta[i].cliente, evRouteSta[i + 1].cliente) +
                                             instance.getDistance(evRouteSta[i].cliente, estacao) +
                                             instance.getDistance(estacao, evRouteSta[i + 1].cliente);

                    if(distInser < custoInserMax)
                    {
                        shiftVectorClienteDir(evRouteSta.route, i + 1, 1, evRouteSta.routeSize);
                        evRouteSta.routeSize += 1;

                        evRouteSta[i+1].cliente = estacao;

                        /*
                        string str;
                        evRouteSta.print(str, instance, true);
                        cout<<"\tTestando rota: "<<str<<"\n";
                        */

                        string strRotaBt;

                        //const double distTemp  = testaRota(evRouteSta, evRouteSta.routeSize, instance, false, tempoSaidaSat, 0, &strRotaBt);
                        const double dist = testaRota(evRouteSta, evRouteSta.routeSize, instance, false, tempoSaidaSat,
                                                      i, nullptr, vetInviabilidade);

                        string str;
                        evRouteSta.print(str, instance, true);
                        //cout<<"Testando Viabilidade rota: "<<str<<"\nDist: "<<dist<<"  "<<distTemp<<"\nRota Bateria: "<<strRotaBt<<"\n\n";


/*                    auto negativo = [](double dist){return dist<0;};

                    if(negativo(dist) && !negativo(distI) || !negativo(dist) && negativo(distI))
                    {
                        PRINT_DEBUG("", "");
                        cout<<"dist: "<<dist<<"\ndistI: "<<distI<<"\n\n";

                        cout<<"evRouteSta: "<<evRouteSta.route[i]<<"\nevRoute: "<<evRoute.route[i]<<"\n\n";



                        throw "ERRO";
                    }
                    else
                    {
                        if(abs(dist-distI) > 1e-3)
                        {

                            PRINT_DEBUG("", "");
                            cout<<"dist: "<<dist<<"\ndistI: "<<distI<<"\n\n";
                            throw "ERRO";
                        }
                    }
*/

                        shiftVectorClienteEsq(evRouteSta.route, i + 1, evRouteSta.routeSize);
                        evRouteSta.routeSize -= 1;

                        if(dist > 0 && (dist - evRoute.distancia) < custoInserMax)
                        {
                            //cout<<"insercao estacao\n\n";

                            insercaoEstacao.pos = i;
                            insercaoEstacao.estacao = estacao;
                            insercaoEstacao.distanciaRota = dist;

                            if(!best)
                                return true;

                            custoInserMax = dist - evRoute.distancia;
                            break;

                        }

                    }

                }
            }
            else
                break;
        }

    }


#if TEMPO_FUNC_VIABILIZA_ROTA_EV
    end   = std::chrono::high_resolution_clock::now();
    duracao = end - start;
    global_tempo += duracao.count();
#endif

    if(insercaoEstacao.distanciaRota < DOUBLE_MAX)
    {
        return true;
    }
    else
        return false;

}

/* ***********************************************************************************
 * Verifica se a rota atende a somente a janela de tempo, desconsiderando a bateria
 * Se @posIni > 0, entao, considera-se que a rota esta correta ate @posIni
 * *********************************************************************************** */
double NS_viabRotaEv::testaRotaTempo(EvRoute &evRoute, const int tamRoute, const Instancia &instance, const bool escrita, const double tempoSaidaSat, const int posIni)
{

    double bateriaRestante = instance.getEvBattery(evRoute.idRota);
    double distanciaRota = 0.0;
    double tempo = 0.0;

    if(posIni == 0)
        tempo = tempoSaidaSat;
    else
    {

        for(int i=0; (i+1) <= posIni; ++i)
        {
            distanciaRota += instance.getDistance(evRoute[i].cliente, evRoute[i+1].cliente);
        }

        tempo = evRoute[posIni].tempoSaida;
        bateriaRestante = evRoute[posIni].bateriaRestante;
    }

    for(int i=posIni; i < (tamRoute-1); ++i)
    {
        double dist = instance.getDistance(evRoute[i].cliente, evRoute[i+1].cliente);
        bateriaRestante -= instance.getEvTaxaConsumo(evRoute.idRota)*dist;
        distanciaRota += dist;
        tempo += dist;
        const ClienteInst &clienteInstProx = instance.vectCliente[evRoute[i+1].cliente];


        if(tempo < clienteInstProx.inicioJanelaTempo)
            tempo = clienteInstProx.inicioJanelaTempo;


        if(!((tempo <= clienteInstProx.fimJanelaTempo) || (abs(tempo-clienteInstProx.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)))
            return -1.0;



        if(escrita)
            evRoute[i+1].tempoCheg  = tempo;

        tempo += clienteInstProx.tempoServico;

/*        if(bateriaRestante < -TOLERANCIA_BATERIA)
        {
            if(escrita)
            {
                cout << "BATERIA!\n"<<bateriaRestante<<"\ncliente: "<<evRoute[i+1].cliente<<"\n\n";

            }

            //cout<<"Inviavel: \n";
            //evRoute.print(instance, true);
            return -1.0;
        }*/

        if(instance.isRechargingStation(evRoute[i+1].cliente))
        {
            tempo += (instance.getEvBattery(evRoute.idRota) - bateriaRestante) *  instance.vectVeiculo[evRoute.idRota].taxaRecarga;

            if(escrita)
            {
                if(bateriaRestante < -TOLERANCIA_BATERIA)
                    evRoute[i + 1].bateriaRestante = bateriaRestante;
                else
                    evRoute[i + 1].bateriaRestante = instance.getEvBattery(evRoute.idRota);
            }

            bateriaRestante = instance.getEvBattery(evRoute.idRota);
        }
        else if(escrita)
            evRoute[i+1].bateriaRestante = bateriaRestante;



        if(escrita)
        {
            evRoute[i+1].tempoSaida = tempo;
        }

    }

    if(escrita)
        evRoute.atualizaParametrosRota(instance);

    return distanciaRota;
}



/* ***********************************************************************************
 * Verifica se a rota atende as restricoes de bateria e tempo
 * Se @posIni > 0, entao, considera-se que a rota esta correta ate @posIni
 * *********************************************************************************** */
double NS_viabRotaEv::testaRota(EvRoute &evRoute, const int tamRoute, const Instancia &instance, const bool escrita,
                                const double tempoSaidaSat, const int posIni, string *rotaBtDebug,
                                Vector<int> *vetInviabilidade)
{

    double bateriaRestante = 0.0;
    double distanciaRota = 0.0;
    double tempo = 0.0;

    if(posIni > 0)
    {

        for(int i=0; (i+1) <= posIni; ++i)
        {
            distanciaRota += instance.getDistance(evRoute[i].cliente, evRoute[i+1].cliente);
        }

        tempo = evRoute[posIni].tempoSaida;
        bateriaRestante = evRoute[posIni].bateriaRestante;

    }
    else
    {

        bateriaRestante = instance.getEvBattery(evRoute.idRota);
        distanciaRota = 0.0;
        tempo = tempoSaidaSat;
    }

    if(escrita && posIni==0)
    {
        evRoute[0].tempoSaida = tempoSaidaSat;
        evRoute[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
        bateriaRestante = evRoute[0].bateriaRestante;
    }

    if(rotaBtDebug)
        (*rotaBtDebug) = to_string(evRoute[posIni].cliente) + "("+ to_string(bateriaRestante)+") ";

    for(int i=posIni; i < (tamRoute-1); ++i)
    {
        double dist = instance.getDistance(evRoute[i].cliente, evRoute[i+1].cliente);
        bateriaRestante -= instance.getEvTaxaConsumo(evRoute.idRota)*dist;

        distanciaRota += dist;
        tempo += dist;
        const ClienteInst &clienteInstProx = instance.vectCliente[evRoute[i+1].cliente];

        if(tempo < clienteInstProx.inicioJanelaTempo)
        {
            tempo = clienteInstProx.inicioJanelaTempo;
        }

        //if(rotaBtDebug)
        //    (*rotaBtDebug) = to_string(evRoute[i+1].cliente) + "("+ to_string(bateriaRestante)+", "+ to_string(tempo) +") ";

        if(!((tempo <= clienteInstProx.fimJanelaTempo) || (abs(tempo-clienteInstProx.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)))
        {
            if(escrita && rotaBtDebug)
            {

                (*rotaBtDebug) += "JANELA DE TEMPO\n";
                (*rotaBtDebug) += "cliente: "+ to_string(evRoute[i+1].cliente)+"\n";
                (*rotaBtDebug) += "tempo chegada: " + to_string(tempo)+"\nfinal janela de tempo: "+
                                  to_string(clienteInstProx.fimJanelaTempo)+"\n\n";
                string strRota;
                evRoute.print(strRota, instance, false);
                (*rotaBtDebug) += "Rota:\n"+strRota+"\n\n";

            }

            //cout<<"Inviavel, janela de tempo i: "<<i<<" \n";
            //cout<<"\tFim Janela: "<<clienteInstProx.fimJanelaTempo<<"; tempo chegada: "<<tempo<<"\n";

            //evRoute.print(instance, true);

            if(vetInviabilidade)
                (*vetInviabilidade)[Inv_tw] += 1;

            return -1.0;
        }


        if(escrita)
        {
/*            cout<<"\tTEMPO CHEGADA("<<evRoute[i+1].cliente<<"): "<<tempo<<"; DIST("<<evRoute[i].cliente<<"-"<<evRoute[i+1].cliente<<"): "<<dist<<" \n";
            cout<<"\tTEMPO SAIDA("<<evRoute[i].cliente<<"): "<<evRoute[i].tempoSaida<<"\n";*/
            evRoute[i + 1].tempoCheg = tempo;

        }

        tempo += clienteInstProx.tempoServico;

        if(bateriaRestante < -TOLERANCIA_BATERIA)
        {
            if(escrita && rotaBtDebug)
            {
                (*rotaBtDebug) += "BATERIA!\n" + to_string(bateriaRestante) + "\ncliente: " + to_string(evRoute[i+1].cliente)+"\n\n";

            }


            if(vetInviabilidade)
                (*vetInviabilidade)[Inv_bat] += 1;

            //cout<<"Inviavel: \n";
            //evRoute.print(instance, true);
            return -1.0;
        }

        if(instance.isRechargingStation(evRoute[i+1].cliente))
        {
            tempo += (instance.getEvBattery(evRoute.idRota) - bateriaRestante) *  instance.vectVeiculo[evRoute.idRota].taxaRecarga;
            bateriaRestante = instance.getEvBattery(evRoute.idRota);
        }


        if(escrita)
        {
            evRoute[i + 1].tempoSaida = tempo;
            evRoute[i + 1].bateriaRestante = bateriaRestante;
        }

    }

    if(escrita)
        evRoute.atualizaParametrosRota(instance);




/*    if(escrita)
        cout<<"OK\n\tDISTANCIA: "<<distanciaRota<<"\n";*/
    return distanciaRota;

}
/*
 *
    // Percorre as estacoes de recarga
    for(int est = instance.getFirstRechargingSIndex(); est <= instance.getEndRechargingSIndex(); ++est)
    {

        // Verifica se eh possivel utilizar a estacao
        if(evRoute.getUtilizacaoRecarga(est) < evRoute.numMaxUtilizacao)
        {

            shiftVectorClienteDir(evRoute.route, 1, 1, evRoute.routeSize);
            evRoute.routeSize += 1;

            // Percorre a rota tentando inserir a estacao
            for(int i = 0; (i + 2) < (evRoute.routeSize); ++i)
            {
                evRoute[i + 1].cliente = est;

                // Verifica se as possicoes adjacentes sao diferentes da estacao
                if((evRoute[i].cliente != est) && (evRoute[i + 2].cliente != est))
                {
                    double dist = testaRota(evRoute, evRoute.routeSize, instance, false, evRoute.route[0].tempoSaida);

                    if(dist > 0.0 && dist < insercaoEstacao.distanciaRota)
                    {
                        insercaoEstacao.distanciaRota = dist;
                        insercaoEstacao.pos = i;
                        insercaoEstacao.estacao = est;

                        if(!best)
                        {

#if TEMPO_FUNC_VIABILIZA_ROTA_EV
                            end   = std::chrono::high_resolution_clock::now();
                            duracao = end - start;
                            global_tempo += duracao.count();
#endif
                            return true;
                        }
                    }

                }

                evRoute[i + 1].cliente = evRoute[i + 2].cliente;

            }

            evRoute.route[evRoute.routeSize - 2].cliente = evRoute.route[evRoute.routeSize - 1].cliente;
            evRoute.routeSize -= 1;
        }

    }
 */