/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/07/22
 *  Arquivo: Aco.cpp
 * ****************************************
 * ****************************************/

#include "Aco.h"
#include "Instance.h"
#include "../greedyAlgorithm.h"
#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

using namespace std;
using namespace boost::numeric;

void N_Aco::aco(Instance &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, const vector<int8_t> &clientes, const int sateliteId, Satelite &satBest)
{

    Solucao solucao(instance);
    bool construtivo = GreedyAlgNS::secondEchelonGreedy(solucao, instance, acoPar.alfaConst);

    if(!construtivo)
    {
        for (int i = 0; i < acoPar.numItMaxHeur; ++i)
        {
            solucao = Solucao(instance);
            construtivo = GreedyAlgNS::secondEchelonGreedy(solucao, instance, acoPar.alfaConst);

            if(construtivo)
                break;
        }
    }

    ublas::matrix<double> matFeromonio(instance.numNos, instance.numNos, acoPar.feromonioInicial);
    Ant antBest(instance, sateliteId, true);
    vector<Proximo> vetProximo(1+instance.numClients+instance.numRechargingS);

    for(int iteracoes = 0; iteracoes < acoPar.numIteracoes; ++iteracoes)
    {

        std::vector<Ant> vetAnt(acoPar.numAnts, Ant(instance, sateliteId));

        for(Ant &ant:vetAnt)
        {
            int rotaEv = -1;

            while(existeClienteNaoVisitado(ant, instance))
            {
                // Cria uma nova rota
                rotaEv += 1;

                if(rotaEv < instance.getN_Evs())
                {
                    EvRoute &evRoute = ant.satelite.vetEvRoute[rotaEv];
                    evRoute.routeSize = 1;
                    int pos = 0;
                    double bat = instance.getEvBattery(evRoute.idRota);

                    do
                    {
                        int proxVetProximo = 0;
                        if(instance.isRechargingStation(evRoute[pos].cliente))
                            bat = instance.getEvBattery(evRoute.idRota);

                        // satelite
                        if(clienteJValido(instance, evRoute[pos].cliente, sateliteId, bat, ant.vetNosAtend, sateliteId))
                        {
                            vetProximo[proxVetProximo].atualiza(sateliteId, instance.getDistance(evRoute[pos].cliente, sateliteId),
                                                                matFeromonio(evRoute[pos].cliente, sateliteId), acoPar);
                            proxVetProximo += 1;
                        }

                        // estacoes e EV
                        //for(int )

                    }while(evRoute[pos].cliente != sateliteId);


                }
                else
                    break;

            }

            if(!existeClienteNaoVisitado(ant, instance))
                ant.viavel = true;
        }
    }

}

bool N_Aco::clienteJValido(Instance &instancia, const int i, const int j, const double bat, const vector<int8_t> &vetNosAtend, const int sat)
{
    if(i == j)
        return false;

    const double dist_i_j = instancia.getDistance(i,j);
    double batTemp = bat - dist_i_j;


    if((bat-dist_i_j) >= -TOLERANCIA_BATERIA)
    {

        // Verifica se j eh estacao de recarga
        if(instancia.isRechargingStation(j))
            return true;

        // Verifica se eh possivel retornar ao satelite
        if((batTemp-instancia.getDistance(j, sat)) >= -TOLERANCIA_BATERIA)
            return true;

        // Percorre as estacoes de recarga
        for(int es=instancia.getFirstRechargingSIndex(); es <= instancia.getEndRechargingSIndex(); ++es)
        {
            if(vetNosAtend[es] < instancia.numUtilEstacao)
            {
                if((batTemp - instancia.getDistance(j, es)) >= -TOLERANCIA_BATERIA)
                    return true;
            }
        }

        // Nao eh possivel retornar ao deposito ou chegar no satelite
        return false;
    }
    else
        return false;

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


