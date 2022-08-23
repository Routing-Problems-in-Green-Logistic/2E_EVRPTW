/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/07/22
 *  Arquivo: Aco.cpp
 *
 *
 * ****************************************
 * ****************************************/

#include "Aco.h"
#include "Instance.h"
#include "../greedyAlgorithm.h"
#include "../mersenne-twister.h"
#include "../greedyAlgorithm.h"
#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/format.hpp>

#define PRINT_0 FALSE
#define PRINT_GRAPS TRUE

using namespace std;
using namespace boost::numeric;

/* ******************************************************************************************************
 * ******************************************************************************************************
 * @param instance
 * @param acoPar
 * @param acoEst
 * @param sateliteId
 * @param satBest
 * @param vetSatAtendCliente
 * ******************************************************************************************************
 * ******************************************************************************************************
 */
bool N_Aco::aco(Instance &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, int sateliteId, Satelite &satBest,
                const vector<int> &vetSatAtendCliente, Parametros &param, NameS_Grasp::Estatisticas &est)
{

    if(instance.numSats > 1)
    {
cout<<"NUM de SATs EH > 1\n\n";
        return false;
    }


    Solucao *solGrasp = NameS_Grasp::grasp(instance, param, est, true);

#if PRINT_GRAPS == TRUE
    if(solGrasp)
    {
        if(solGrasp->viavel)
        {
cout << "sol grasp viavel: " << solGrasp->viavel << "\ndist: " << solGrasp->distancia << "\n";
cout<<"sat: "<<solGrasp->satelites[sateliteId].distancia<<"\n";

        }
        else
cout<<"sol grasp inviavel\n";

    }
#endif


    ublas::matrix<double> matFeromonio(instance.numNos, instance.numNos, 0.01);
    ublas::matrix<double> matIncrementoFerm(instance.numNos, instance.numNos, 0.0);

    //matFeromonio += matIncrementoFerm;

    // Inicializa o feromonio com a sol do grasp:

    auto satartFerom = [&]()
    {
        //const double feromInicial = 1.0/solGrasp->satelites[sateliteId].distancia;
        double min = DOUBLE_INF;
        //cout<<"ferom ini: "<<feromInicial<<"\n";

        for(const EvRoute &evRoute:solGrasp->satelites[sateliteId].vetEvRoute)
        {
            if(evRoute.routeSize > 2)
            {
                double inc = 1.0/evRoute.distancia;
                if(inc < min)
                    min = inc;

                for(int i=0; i < (evRoute.routeSize-1); ++i)
                {
                    //matFeromonio(evRoute.route[i].cliente, evRoute.route[i+1].cliente) = 1.0/instance.getDistance(evRoute.route[i].cliente, evRoute.route[i+1].cliente);
                    matFeromonio(evRoute.route[i].cliente, evRoute.route[i+1].cliente) += inc;
                }
            }
        }

    };

    satartFerom();

    auto printMatFerom = [&](int it)
    {
        cout<<"\nMAT DE FEROMONIO NA ITERACAO "<<it<<"\n\n";

        for(int i=0; i < instance.numNos; ++i)
        {
            for(int j =0; j < instance.numNos; ++j)
            {
                cout<<str(boost::format("%.5f ") % matFeromonio(i,j));
            }

            cout<<"\n";
        }
    };

    printMatFerom(-1);

    Ant antBest(instance, sateliteId, true);
    vector<Proximo> vetProximo(1+instance.numClients+instance.numRechargingS);
    vector<Proximo> vetProximoAux(1+instance.numClients+instance.numRechargingS);

    for(int iteracao = 0; iteracao < acoPar.numIteracoes; ++iteracao)
    {

        Ant antBestIt(instance, sateliteId, true);
        int numAntsViaveisIt = 0;

#if PRINT_0 == TRUE
cout<<"ITE: "<<iteracoes<<"\n";
#endif
        std::vector<Ant> vetAnt(acoPar.numAnts, Ant(instance, sateliteId));

        int antCount = 0;
        for(Ant &ant:vetAnt)
        {


#if PRINT_0 == TRUE
cout<<"ant: "<<antCount<<"\n\n";
#endif
            antCount += 1;

            int rotaEv = -1;

            while(existeClienteNaoVisitado(ant, instance))
            {
                // Cria uma nova rota
                rotaEv += 1;

                if(rotaEv < instance.getN_Evs())
                {

#if PRINT_0 == TRUE
cout<<"\tROTA: "<<rotaEv<<"\n";
#endif
                    EvRoute &evRoute = ant.satelite.vetEvRoute[rotaEv];
                    evRoute[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
                    evRoute[0].tempoSaida = instance.getDistance(0, sateliteId);
                    evRoute.routeSize = 1;

                    int pos             = 0;
                    double multSoma     = 0.0;
                    double multMax      = -1.0;
                    int multMaxIndice   = -1;
                    int proxVetProximo  = 0;
                    bool existeCliente  = false;

                    auto atualizaVetProx = [&](int clienteJ)
                    {
                        double temp = vetProximo[proxVetProximo].atualiza(clienteJ, instance.getDistance(evRoute[pos].cliente, clienteJ),
                                                                          matFeromonio(evRoute[pos].cliente, clienteJ), acoPar);

#if PRINT_0 == TRUE
cout<<"\t\t\tferom_X_dist: "<<temp<<"\n";
#endif

                        if(temp > multMax)
                        {
                            multMax         = temp;
                            multMaxIndice   = proxVetProximo;
                        }

                        multSoma += temp;
                        proxVetProximo += 1;

                        if(instance.isClient(clienteJ))
                            existeCliente = true;

                    };


#if PRINT_0 == TRUE
cout<<"\t\t";
#endif
                    // Escolhe a proxima aresta
                    do
                    {
                        existeCliente = false;

                        const int clienteI       = evRoute[pos].cliente;
                        const double tempoSaidaI = evRoute[pos].tempoSaida;


#if PRINT_0 == TRUE
cout<<clienteI<<"\n";
#endif
                        proxVetProximo = 0;
                        multMax = -1.0;
                        multMaxIndice = 0;

                        bool voltaSat = false;
                        bool rsConsecutivos2 = false;
                        bool clienteViavel = false;

                        if(pos >= 2)
                        {
                            if(instance.isRechargingStation(evRoute[pos].cliente))// && instance.isRechargingStation(evRoute[pos-1].cliente))
                               rsConsecutivos2 = true;
                        }


                        // satelite
                        if(clienteJValido(instance, clienteI, sateliteId, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                        {
                            //atualizaVetProx(sateliteId);
                            voltaSat = true;
                        }



                        // Clientes
                       for(int j=instance.getFirstClientIndex(); j <= instance.getEndClientIndex(); ++j)
                       {
                           if(vetSatAtendCliente[j] == sateliteId)
                           {
                               // Verifica Capacidade de carga do EV
                               if(j != clienteI && (evRoute.demanda + instance.getDemand(j)) <= instance.getEvCap(rotaEv))
                               {
                                   if(clienteJValido(instance, clienteI, j, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                                   {
                                       atualizaVetProx(j);
                                       clienteViavel = true;
                                   }
                               }
                               else
                               {

#if PRINT_0 == TRUE
                                   if(j != clienteI)
                                   {

cout<<j<<": cap\n";
                                   }
#endif

                               }
                           }
                       }



#if PRINT_0 == TRUE
cout<<"\n\t\tapos clientes: tam: "<<proxVetProximo<<"\n";
#endif

                       if(!rsConsecutivos2 && !clienteViavel)
                       {
                           bool voltaDep = false;

                           // Se nao existe cliente entao, proxVetProximo eh igual a 1

                           // Estacoes de Recarga
                           for(int j = instance.getFirstRS_index(); j <= instance.getEndRS_index(); ++j)
                           {
                               if(clienteJValido(instance, clienteI, j, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                               {

                                   if(!voltaDep && instance.vetVoltaRS_sat[instance.getIndiceVetVoltaRS_sat(sateliteId, j)]==1)
                                   {
                                       //voltaDep = true;
                                       //proxVetProximo = 0;
                                       atualizaVetProx(j);
                                   }
                                   else if(!voltaDep)
                                       atualizaVetProx(j);
                               }
                           }
                       }

                       if(proxVetProximo == 0 && voltaSat)
                       {
                           atualizaVetProx(sateliteId);
                       }


#if PRINT_0 == TRUE
cout<<"\n\t\ttam: "<<proxVetProximo<<"\n";
#endif

                       if(proxVetProximo >= 1)
                       {
                           int proxClienteInd = 0;
                           static const int q0 = int(acoPar.q0*100);

                           std::sort(vetProximo.begin(), vetProximo.begin()+proxVetProximo);


#if PRINT_0 == TRUE
                           for(int i=0; i < proxVetProximo; ++i)
                           {
cout<<"\t\t\t"<<vetProximo[i].cliente<<"("<<vetProximo[i].ferom_x_dist<<") "<<i<<"\n";
                           }
#endif

                           if((rand_u32()%101) <= q0)
                           {
                                proxClienteInd = multMaxIndice;
                           }
                           else
                           {


                               double prob = 0.0;
                               int numRand = rand_u32()%100;
                               int i = rand_u32()%proxVetProximo;
                               const int fist = i;

                               //for(int i=0; i < proxVetProximo; ++i)
                               do
                               {
                                   prob += (vetProximo[i].ferom_x_dist / multSoma) * 100;
                                   proxClienteInd = i;

                                   if(numRand <= int(prob) || proxVetProximo == 1)
                                       break;

                                   i = (i+1)%proxVetProximo;

                               }while(i!=fist);

                           }


#if PRINT_0 == TRUE
cout<<"proxClienteInd: "<<proxClienteInd<<"; vet tam: "<<proxVetProximo<<"\n";
#endif

                           // Cliente j foi escolhido


                           // Atualiza evRoute
                           atualizaClienteJ(evRoute, pos, vetProximo[proxClienteInd].cliente, instance, ant);
                           pos += 1;


#if PRINT_0 == TRUE
cout<<"Cliente escolhido: "<<vetProximo[proxClienteInd].cliente<<"\n";
string strRota;
evRoute.print(strRota, instance, true);
cout<<"ROTA: "<<strRota<<"\n\n";
cout<<"*************************************************\n\n";
#endif

                       }
                       else
                       {
                           ant.viavel = false;
                           break;
                       }


                    }while(evRoute[pos].cliente != sateliteId);


#if PRINT_0 == TRUE
                    if(evRoute[pos].cliente == sateliteId && pos > 0)
                        cout<<sateliteId<<"\n";
cout<<"\n\n";
#endif
                }
                else
                    break;

            } // Fim while(existeClienteNaoVisitado)

            acoEst.nAntGeradas += 1;

            // Verifica se ant eh viavel
            if(!existeClienteNaoVisitado(ant, instance))
            {



                //GreedyAlgNS::firstEchelonGreedy();

                ant.viavel = true;
                acoEst.nAntViaveis += 1;
                acoEst.sumDistAntsViaveis += ant.satelite.distancia;

                numAntsViaveisIt += 1;

                if(ant < antBestIt)//antBest.satelite.distancia)
                {
                    antBestIt.copia(ant);
                }


#if PRINT_0 == TRUE
                cout<<"ANT VIAVEL\n";
#endif
                //delete solGrasp;
                //return;
            }


#if PRINT_0 == TRUE
cout<<"\n*************************************************\n\n";
string strSat;
ant.satelite.print(strSat, instance);
cout<<strSat<<"\n\n*************************************************\n\n";
#endif


        } // Fim for ANTs

        acoEst.mediaAntsViaveisPorIt += double(numAntsViaveisIt);

#if PRINT_0 == TRUE
cout<<"*******************\n\n";
#endif

        // Evapora e atualiza o feromonio com a melhor formiga
        if(antBestIt.viavel)
        {
            cout<<"best: "<<antBestIt.satelite.distancia<<"\n\n";

            double feromMax = 1.0/antBestIt.satelite.distancia;
            atualizaFeromonio(matFeromonio, instance, acoPar, antBestIt, 0.1*feromMax, feromMax);
            printMatFerom(iteracao);

            if(antBestIt < antBest)
            {
                antBest.copia(antBestIt);
                acoEst.ultimaAtualisacaoIt = iteracao;
            }

        }

    } // Fim for iteracoes ACO

    satBest.copia(antBest.satelite);

    string erro;

    if(antBest.viavel)
    {
        acoEst.distBestAnt = antBest.satelite.distancia;
        acoEst.sumDistAntsViaveis /= double(acoEst.nAntViaveis);
        acoEst.mediaAntsViaveisPorIt /= double(acoPar.numIteracoes);

        if(satBest.checkSatellite(erro, instance))
        {

#if PRINT_0 == TRUE

cout<<"\n\nANT BEST VIAVEL\n";
cout<<"DIST "<<satBest.distancia<<"\n\n";

string satStr;
satBest.print(satStr, instance);
cout<<satStr;

#endif

            delete solGrasp;
            return true;

        }
        else
        {
            PRINT_DEBUG("\n\n", "ANT BEST EH INVIAVEL!!");
            cout<<"ERRO: "<<erro<<"\n\n\n";
            throw "ERRO";
        }
    }
    else
    {

cout<<"cliente0 id: \t"<<instance.getFirstClientIndex()<<"\n";
cout<<"clienteUltimo: \t"<<instance.getEndClientIndex()<<"\n";
cout<<"ANT BEST EH INVIAVEL\n";

    }

    delete solGrasp;
    return false;
}

void N_Aco::atualizaClienteJ(EvRoute &evRoute, const int pos, const int clienteJ, Instance &instance, Ant &ant)
{

    const double dist_i_j = instance.getDistance(evRoute[pos].cliente, clienteJ);
    evRoute[pos+1].cliente = clienteJ;
    evRoute.routeSize = pos+1;
    evRoute[pos+1].tempoCheg = evRoute[pos].tempoSaida+dist_i_j;

    if(instance.isClient(clienteJ))
    {
        if(evRoute[pos+1].tempoCheg < instance.vectCliente[clienteJ].inicioJanelaTempo)
            evRoute[pos+1].tempoCheg = instance.vectCliente[clienteJ].inicioJanelaTempo;
    }

    double bat = evRoute[pos].bateriaRestante-dist_i_j;


    if(bat < -TOLERANCIA_BATERIA)
    {
        string strRota;
        evRoute.print(strRota, instance, false);
        PRINT_DEBUG("", "ERRO, Deveria ser possivel chegar no cliente "<<clienteJ<<"; bat: "<<bat<<"\nRota: "<<strRota);
        cout<<"pos: "<<pos<<"\n";
        throw "ERRO";
    }

    if(instance.isRechargingStation(clienteJ))
    {
        double dif = instance.getEvBattery(evRoute.idRota) - (evRoute[pos].bateriaRestante-dist_i_j);
        evRoute[pos+1].tempoSaida = evRoute[pos+1].tempoCheg+instance.vectVeiculo[evRoute.idRota].taxaRecarga*dif;
        bat = instance.getEvBattery(evRoute.idRota);
    }
    else
    {
        evRoute[pos+1].tempoSaida = evRoute[pos+1].tempoCheg+instance.vectCliente[clienteJ].tempoServico;
    }


    evRoute[pos+1].bateriaRestante = bat;
    evRoute.distancia += dist_i_j;
    evRoute.demanda += instance.vectCliente[clienteJ].demanda;
    evRoute.routeSize += 1;

    ant.satelite.distancia += dist_i_j;
    ant.satelite.demanda += instance.vectCliente[clienteJ].demanda;
    ant.vetNosAtend[evRoute[pos+1].cliente] += 1;

}

/**
 * @param instancia
 * @param i
 * @param j
 * @param bat
 * @param vetNosAtend
 * @param sat
 * @param tempoSaidaI
 * @return
 */
bool N_Aco::clienteJValido(Instance &instancia, const int i, const int j, const double bat, const vector<int8_t> &vetNosAtend, const int sat, const double tempoSaidaI)
{

    // TEMPO ??


    if(i == j || (vetNosAtend[j] == 1 && !instancia.isSatelite(j) && !instancia.isRechargingStation(j)))
    {
#if PRINT_0 == TRUE
        if(i!=j)
            cout<<j<<": atend\n";
        else
            cout<<j<<" i==j\n";
#endif
        return false;
    }

    if(instancia.isRechargingStation(j) && vetNosAtend[j] == instancia.numUtilEstacao)
    {

#if PRINT_0 == TRUE
cout<<j<<": RS\n";
#endif
        return false;
    }

    const double dist_i_j = instancia.getDistance(i,j);

    if(dist_i_j == 0.0 && !instancia.isSatelite(j))
    {


#if PRINT_0 == TRUE
cout<<j<<": dist = 0\n";
#endif
        return false;
    }

    double batTemp = bat - dist_i_j;

    if(instancia.isClient(j))
    {
        double tempoChegadaJ = tempoSaidaI + dist_i_j;

        if(!((tempoChegadaJ <= instancia.vectCliente[j].fimJanelaTempo) || (abs(tempoChegadaJ-instancia.vectCliente[j].fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)))
        {

#if PRINT_0 == TRUE
cout<<j<<": janela de tempo: TC("<<tempoChegadaJ<<") TW("<<instancia.vectCliente[j].fimJanelaTempo<<")\n";
#endif
            return false;
        }

    }


    if((bat-dist_i_j) >= -TOLERANCIA_BATERIA)
    {

        // Verifica se j eh estacao de recarga
        if(instancia.isRechargingStation(j))
        {
            if(vetNosAtend[j] < instancia.numUtilEstacao)
            {

#if PRINT_0 == TRUE
cout<<j<<": ok\n";
#endif
                return true;
            }
            else
                return false;
        }

        // Verifica se eh possivel retornar ao satelite
        if(j == sat || (batTemp-instancia.getDistance(j, sat)) >= -TOLERANCIA_BATERIA)
        {

#if PRINT_0 == TRUE
            cout<<j<<": ok\n";
#endif
            return true;
        }

        // Percorre as estacoes de recarga
        for(int es= instancia.getFirstRS_index(); es <= instancia.getEndRS_index(); ++es)
        {
            if(j != es && vetNosAtend[es] < instancia.numUtilEstacao)
            {
                if((batTemp - instancia.getDistance(j, es)) >= -TOLERANCIA_BATERIA)
                {

#if PRINT_0 == TRUE
                    cout<<j<<": ok\n";
#endif
                    return true;
                }
            }
        }

        // Nao eh possivel retornar ao deposito ou chegar no satelite
        return false;
    }
    else
    {

#if PRINT_0 == TRUE
        cout<<j<<" bat\n";
#endif
        return false;
    }

}


void N_Aco::atualizaFeromonio(ublas::matrix<double> &matFeromonio, Instance &instancia, const AcoParametros &acoParam, const Ant &antBest, const double feromMin, const double feromMax)
{
    double inc = 1.0/getDistSat(antBest.satelite);
    evaporaFeromonio(matFeromonio, {antBest.satelite.sateliteId}, instancia, acoParam, feromMin);
    cout<<"inc: "<<inc<<"\n";

    // Percorre a solucao para add feromonio 1/dist
    for(const EvRoute &evRoute:antBest.satelite.vetEvRoute)
    {
        if(evRoute.routeSize > 2)
        {
            for(int i=0; i < (evRoute.routeSize-1); ++i)
            {
                const double feromTemp = matFeromonio(evRoute.route[i].cliente, evRoute.route[i+1].cliente);
                matFeromonio(evRoute.route[i].cliente, evRoute.route[i+1].cliente) = min((feromTemp+inc), feromMax);
            }
        }
    }
}

void N_Aco::evaporaFeromonio(ublas::matrix<double> &matFeromonio, const vector<int> &vetSat, Instance &instancia, const AcoParametros &acoParam, const double feromMin)
{

    static const double ro_1 = acoParam.ro;

cout<<"ferm min: "<<feromMin<<"\n\n";

    // Atualizacao das arestas (i,j), i,j clientes
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        for(int j=instancia.getFirstClientIndex(); j <= instancia.getEndClientIndex(); ++j)
        {
            if(i==j)
                continue;

            matFeromonio(i,j) = max(matFeromonio(i,j)*ro_1, feromMin);
        }
    }

    // Atualizacoes arestas (i,j), i sat e j cliente
    for(const int s:vetSat)
    {
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
            matFeromonio(s, i) = max(matFeromonio(s, i)*ro_1, feromMin);
    }

    // Atualizacoes arestas (i,j), i cliente e j cliente
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        for(const int s:vetSat)
            matFeromonio(i, s) = max(matFeromonio(i, s)*ro_1, feromMin);
    }

}