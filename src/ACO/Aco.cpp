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

    double dist  = 0.0;
    int numArcos = 0;

    for(int i=0; i < instance.numNos; ++i)
    {
        if(instance.isClient(i) || i == sateliteId)
        {
            if(i != sateliteId)
            {
                dist += instance.getDistance(i, sateliteId);
                dist += instance.getDistance(sateliteId, i);

                numArcos += 1;
            }

            for(int j=instance.getFirstClientIndex(); j <= instance.getEndClientIndex(); ++j)
            {
                dist += 2.0*instance.getDistance(i, j);
                numArcos += 2;
            }


        }
    }

    Solucao solucao(instance);
    bool construtivo = GreedyAlgNS::secondEchelonGreedy(solucao, instance, acoPar.alfaConst);
    ublas::matrix<double> matFeromonio(instance.numNos, instance.numNos, instance.numNos/dist);

    for(int iteracoes = 0; iteracoes < acoPar.numIteracoes; ++iteracoes)
    {

        std::vector<Ant> vetAnt(acoPar.numAnts, Ant(instance, sateliteId));
    }
}


void N_Aco::atualizaFeromonio(ublas::matrix<double> &matFeromonio, Satelite &satelite)
{
    double dist = getDistSat(satelite);
}

void N_Aco::evaporaFeromonio(ublas::matrix<double> &matFeromonio, const int iInic, const int iFim, const  int jInic, const int jFim)
{

}


