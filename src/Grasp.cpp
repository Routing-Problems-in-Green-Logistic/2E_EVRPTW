/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    02/06/22
 *  Arquivo: Grasp.cpp
 * ****************************************
 * ****************************************/

#include "Grasp.h"
#include "greedyAlgorithm.h"
#include "Auxiliary.h"

using namespace GreedyAlgNS;
using namespace NameS_Grasp;

Solucao* NameS_Grasp::grasp(Instance &instance, const int numIte, const float alfa, const float beta, Estatisticas &estat)
{
    Solucao *solBest = new Solucao(instance);
    solBest->distancia = DOUBLE_MAX;

    estat.numSol = 0.0;
    estat.numIte = numIte;
    estat.distAcum = 0.0;
    estat.erro = "";
    bool gabi = false;

    for(int i=0; i < numIte; ++i)
    {
        Solucao sol(instance);


        if(estat.numSol == 0 && i == 110)
        {
            gabi = true;

        }

        if(gabi)


        construtivo(sol, instance, alfa, beta);

        if(sol.viavel)
        {

            if(!sol.checkSolution(estat.erro, instance))
            {
                cout<<"\n\nSOLUCAO:\n\n";
                sol.print(instance);

                cout << estat.erro<< "\n****************************************************************************************\n\n";
                delete solBest;
                return nullptr;
            }
            else
            {

                estat.numSol += 1;
                estat.distAcum += sol.distancia;

                if(sol.distancia < solBest->distancia)
                {
                    solBest->copia(sol);
                }
            }
        }

    }

    return solBest;

}