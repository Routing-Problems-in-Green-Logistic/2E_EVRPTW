/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/07/22
 *  Arquivo: Aco.cpp
 *
 *  ERRO PROVAVELMENTE NA FUNC.: atualizaClienteJ
 *
 * ****************************************
 * ****************************************/

#include "Aco.h"
#include "Instance.h"
#include "../greedyAlgorithm.h"
#include "../mersenne-twister.h"
#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>


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
void N_Aco::aco(Instance &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, int sateliteId, Satelite &satBest, const vector<int> &vetSatAtendCliente)
{

    /*
    Solucao solucao(instance);
    bool construtivo = GreedyAlgNS::secondEchelonGreedy(solucao, instance, acoPar.alfaConst, 0);

    if(!construtivo)
    {
        for (int i = 0; i < acoPar.numItMaxHeur; ++i)
        {
            solucao = Solucao(instance);
            construtivo = GreedyAlgNS::secondEchelonGreedy(solucao, instance, acoPar.alfaConst, 0);

            if(construtivo)
                break;
        }
    }*/

    ublas::matrix<double> matFeromonio(instance.numNos, instance.numNos, acoPar.feromonioInicial);
    Ant antBest(instance, sateliteId, true);
    vector<Proximo> vetProximo(1+instance.numClients+instance.numRechargingS);
    vector<Proximo> vetProximoAux(1+instance.numClients+instance.numRechargingS);

    for(int iteracoes = 0; iteracoes < acoPar.numIteracoes; ++iteracoes)
    {

cout<<"ITE: "<<iteracoes<<"\n";
        std::vector<Ant> vetAnt(acoPar.numAnts, Ant(instance, sateliteId));

        int antCount = 0;
        for(Ant &ant:vetAnt)
        {
cout<<"ant: "<<antCount<<"\n\n";
            antCount += 1;

            int rotaEv = -1;

            while(existeClienteNaoVisitado(ant, instance))
            {
                // Cria uma nova rota
                rotaEv += 1;

                if(rotaEv < instance.getN_Evs())
                {

cout<<"\tROTA: "<<rotaEv<<"\n";
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
cout<<"\t\t\tferom_X_dist: "<<temp<<"\n";

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

cout<<"\t\t";

                    // Escolhe a proxima aresta
                    do
                    {
                        existeCliente = false;

                        const int clienteI       = evRoute[pos].cliente;
                        const double tempoSaidaI = evRoute[pos].tempoSaida;
cout<<clienteI<<" ";

                        proxVetProximo = 0;
                        multMax = -1.0;
                        multMaxIndice = 0;

                        // satelite
                        if(clienteJValido(instance, clienteI, sateliteId, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                            atualizaVetProx(sateliteId);

                        // Clientes
                       for(int j=instance.getFirstClientIndex(); j < instance.getEndClientIndex(); ++j)
                       {
                           if(vetSatAtendCliente[j] == sateliteId)
                           {
                               // Verifica Capacidade de carga do EV
                               if(j != clienteI && (evRoute.demanda + instance.getDemand(j)) <= instance.getEvCap(rotaEv))
                               {
                                   if(clienteJValido(instance, clienteI, j, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                                       atualizaVetProx(j);
                               }
                           }
                       }

                       // Estacoes de Recarga
                       for(int j=instance.getFirstRechargingSIndex(); j <= instance.getEndRechargingSIndex(); ++j)
                       {
                           if(clienteJValido(instance, clienteI, j, evRoute[pos].bateriaRestante, ant.vetNosAtend, sateliteId, tempoSaidaI))
                               atualizaVetProx(j);
                       }

cout<<"\n\t\ttam: "<<proxVetProximo<<"\n";

                       if(proxVetProximo >= 1)
                       {
                           int proxClienteInd = 0;
                           static const int q0 = int(acoPar.q0*100);

                           std::sort(vetProximo.begin(), vetProximo.begin()+proxVetProximo);

                           // Retira estacao de recarga se eh existe cliente
                           if(existeCliente)
                           {

                               multMax = -1.0;
                               multMaxIndice = 0;
                               int j=0;

                               for(int i=0; i < proxVetProximo; ++i)
                               {
                                   if(instance.isClient(vetProximo[i].cliente))
                                   {
                                       vetProximoAux[j] = vetProximo[i];
                                       if(vetProximoAux[j].ferom_x_dist > multMax)
                                       {
                                           multMax = vetProximoAux[j].ferom_x_dist;
                                           multMaxIndice = j;
                                       }

                                       j += 1;

                                   }
                               }

                               std::copy(vetProximoAux.begin(), vetProximoAux.begin()+j, vetProximo.begin());
                               proxVetProximo = j;
                           }



                           for(int i=0; i < proxVetProximo; ++i)
                           {
cout<<"\t\t\t"<<vetProximo[i].cliente<<"("<<vetProximo[i].ferom_x_dist<<") "<<i<<"\n";
                           }

                           if((rand_u32()%101) <= q0)
                           {
                                proxClienteInd = multMaxIndice;
                           }
                           else
                           {


                               double prob = 0.0;
                               int numRand = rand_u32()%100;
                               for(int i=0; i < proxVetProximo; ++i)
                               {
                                   prob += (vetProximo[i].ferom_x_dist / multSoma) * 100;
                                   proxClienteInd = i;

                                   if(numRand <= int(prob))
                                       break;
                               }
                           }

cout<<"proxClienteInd: "<<proxClienteInd<<"; vet tam: "<<proxVetProximo<<"\n";

                           // Cliente j foi escolhido


                           // Atualiza evRoute
                           atualizaClienteJ(evRoute, pos, vetProximo[proxClienteInd].cliente, instance, ant);
                           pos += 1;

cout<<"Cliente escolhido: "<<vetProximo[proxClienteInd].cliente<<"\n";
string strRota;
evRoute.print(strRota, instance, true);
cout<<"ROTA: "<<strRota<<"\n\n";
cout<<"*************************************************\n\n";

                       }
                       else
                       {
                           ant.viavel = false;
                           break;
                       }


                    }while(evRoute[pos].cliente != sateliteId);

                    if(evRoute[pos].cliente == sateliteId && pos > 0)
 cout<<sateliteId<<"\n";

cout<<"\n\n";

                }
                else
                    break;
            }

            if(!existeClienteNaoVisitado(ant, instance))
            {
                ant.viavel = true;

                if(ant.satelite.distancia < antBest.satelite.distancia)
                    antBest.copia(ant);
            }

cout<<"\n*************************************************\n\n";
            string strSat;
            ant.satelite.print(strSat, instance);

cout<<strSat<<"\n\n*************************************************\n\n";


        }

cout<<"*******************\n\n";

        // Evapora e atualiza o feromonio com a melhor formiga
        if(antBest.viavel)
        {
            double feromMax = 1.0/antBest.satelite.distancia;
            atualizaFeromonio(matFeromonio, instance, acoPar, antBest, 0.1*feromMax, feromMax);
        }
    }

    satBest.copia(antBest.satelite);


    string erro;

    if(antBest.viavel)
    {
        if(satBest.checkSatellite(erro, instance))
        {

cout <<"ANT BEST VIAVEL\n";
            string satStr;
            satBest.print(satStr, instance);

cout<<satStr;

        }
    }
    else
    {
cout<<"cliente0 id: "<<instance.getFirstClientIndex()<<"\n";
cout<<"ANT BEST EH INVIAVEL\n";
    }
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

    if(i == j || vetNosAtend[j] >= 1)
        return false;

cout<<"clienteJValido\n";

    const double dist_i_j = instancia.getDistance(i,j);

    if(dist_i_j == 0.0)
        return false;

    double batTemp = bat - dist_i_j;

    if(instancia.isClient(j))
    {
        double tempoChegadaJ = tempoSaidaI + dist_i_j;

        if(!((tempoChegadaJ <= instancia.vectCliente[j].fimJanelaTempo) || (abs(tempoChegadaJ-instancia.vectCliente[j].fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)))
            return false;

    }


    if((bat-dist_i_j) >= -TOLERANCIA_BATERIA)
    {

        // Verifica se j eh estacao de recarga
        if(instancia.isRechargingStation(j))
        {
            if(vetNosAtend[j] < instancia.numUtilEstacao)
                return true;
            else
                return false;
        }

        // Verifica se eh possivel retornar ao satelite
        if(j != sat && (batTemp-instancia.getDistance(j, sat)) >= -TOLERANCIA_BATERIA)
            return true;

        // Percorre as estacoes de recarga
        for(int es=instancia.getFirstRechargingSIndex(); es <= instancia.getEndRechargingSIndex(); ++es)
        {
            if(j != es && vetNosAtend[es] < instancia.numUtilEstacao)
            {
                if((batTemp - instancia.getDistance(j, es)) >= -TOLERANCIA_BATERIA)
                    return true;
            }
        }

        // Nao eh possivel retornar ao deposito ou chegar no satelite
        return false;
    }
    else
    {

        return false;
    }

}


void N_Aco::atualizaFeromonio(ublas::matrix<double> &matFeromonio, Instance &instancia, const AcoParametros &acoParam, const Ant &antBest, const double feromMin, const double feromMax)
{
    double inc = 1.0/getDistSat(antBest.satelite);
    evaporaFeromonio(matFeromonio, {antBest.satelite.sateliteId}, instancia, acoParam, feromMin);

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

    static const double ro_1 = 1.0 - acoParam.ro;

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