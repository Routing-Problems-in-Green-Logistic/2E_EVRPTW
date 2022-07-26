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

    ublas::matrix<double> matFeromonio(instance.numNos, instance.numNos, 1.0/solucao.distancia);
    Ant antBest(instance, sateliteId);

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


