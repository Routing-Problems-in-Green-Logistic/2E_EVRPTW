//
// Created by igor on 23/01/2022.
//

#include <chrono>
#include "ViabilizadorRotaEv.h"
#include "Auxiliary.h"

using namespace NameViabRotaEv;
using namespace NS_Auxiliary;

#if TEMPO_FUNC_VIABILIZA_ROTA_EV
    double NameViabRotaEv::global_tempo = 0.0;
#endif

/**
 *
 * Tenta viabilizar a rota inserindo um estacao de recarga em uma posicao da rota
 *
 * @param evRoute               Eh alterado durante a execucao
 * @param pos
 * @param instance
 * @param best
 * @param insercaoEstacao
 * @return
 */



bool NameViabRotaEv::viabilizaRotaEv(EvRoute &evRoute, const Instance &instance, const bool best, NameViabRotaEv::InsercaoEstacao &insercaoEstacao)
{


#if TEMPO_FUNC_VIABILIZA_ROTA_EV

    static auto start = std::chrono::high_resolution_clock::now();
    static auto end   = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> duracao = end - start;

    start = std::chrono::high_resolution_clock::now();


    end   = std::chrono::high_resolution_clock::now();
    duracao = end - start;
    global_tempo += duracao.count();

#endif

    /* *************************************************************************************************************
     * *************************************************************************************************************
     * Tenta viabilizar a rota inserindo um estacao de recarga em uma posicao da rota
     *
     * evRoute nao eh uma rota valida, somente o cliente eh valido.
     * *************************************************************************************************************
     * *************************************************************************************************************
     */


    //PRINT_DEBUG("", "Rota: ");
    //for(int i=0; i < tamRoute; ++i)
    //    cout<<route[i]<<" ";

    //cout<<"\n\n";

//    std::vector<PosicaoEstacao> vectorPosicaoEstacao(instance.getN_RechargingS());
//    int proxVectorPosEst = 0;

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

    insercaoEstacao.distanciaRota = DOUBLE_MAX;
    insercaoEstacao.estacao = -1;
    insercaoEstacao.pos = -1;


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




double NameViabRotaEv::testaRota(EvRoute &evRoute, const int tamRoute, const Instance &instance, const bool escrita, const double tempoSaidaSat)
{

    double bateriaRestante = instance.getEvBattery(evRoute.idRota);
    double distanciaRota = 0.0;

    if(escrita)
    {
        evRoute[0].tempoSaida = tempoSaidaSat;
        evRoute[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
    }

    double tempo = tempoSaidaSat;

    for(int i=0; i < (tamRoute-1); ++i)
    {
        double dist = instance.getDistance(evRoute[i].cliente, evRoute[i+1].cliente);
        bateriaRestante -= instance.getEvTaxaConsumo(evRoute.idRota)*dist;
        distanciaRota += dist;
        tempo += dist;
        const ClienteInst &clienteInstProx = instance.vectCliente[evRoute[i+1].cliente];


        if(tempo < clienteInstProx.inicioJanelaTempo)
            tempo = clienteInstProx.inicioJanelaTempo;


        if(!((tempo <= clienteInstProx.fimJanelaTempo) || (abs(tempo-clienteInstProx.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)))
        {
            if(escrita)
            {

                cout << "JANELA DE TEMPO\n";
                cout<<"cliente: "<<evRoute[i+1].cliente<<"\n";
                cout<<"tempo chegada: "<<tempo<<"\nfinal janela de tempo: "<<clienteInstProx.fimJanelaTempo<<"\n";

            }

            //cout<<"Inviavel, janela de tempo i: "<<i<<" \n";
            //cout<<"\tFim Janela: "<<clienteInstProx.fimJanelaTempo<<"; tempo chegada: "<<tempo<<"\n";

            //evRoute.print(instance, true);

            return -1.0;
        }


        if(escrita)
            evRoute[i+1].tempoCheg  = tempo;

        tempo += clienteInstProx.tempoServico;

        if(bateriaRestante < -TOLERANCIA_BATERIA)
        {
            if(escrita)
                cout<<"BATERIA!\n";

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
