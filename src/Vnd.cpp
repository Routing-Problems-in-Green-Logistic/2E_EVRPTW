

#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;


void NS_vnd::rvnd(Solucao &solution, Instance &instance)
{

    EvRoute evRouteAux(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);
    while(mvEvSwapIntraRota(solution, instance, evRouteAux))
    {
        string erro;
        if(!solution.checkSolution(erro, instance))
        {
            PRINT_DEBUG("", "ERRO. mvEvSwapIntraRota:\n");
            cout<<erro<<"\n";
            exit(-1);
        }

        //cout<<"#######################################################################\n\n";
    }

//cout<<"***************************************************************************\n\n";



}

/*
  double valOrig = sol.distancia;

            while(mvEvShifitIntraRota(sol, instance, evRoute, SELECAO_PRIMEIRO) && mv)
            {
                mv = true;

                //PRINT_DEBUG("\t", "checkSolution");

                if(!sol.checkSolution(erro, instance))
                {
                    cout << "MV SHIFIT\n";

                    cout << "\n\nSOLUCAO:\n\n";
                    sol.print(instance);

                    cout << erro
                         << "\n****************************************************************************************\n\n";

                    mv = false;
                    sol.viavel = false;
                    delete solBest;
                    throw "ERRO";
                }
                else
                {
                    //cout<<"MV SHIFIT ATUALIZACAO!!\n\n";

                    if((sol.distancia < solBest->distancia || !solBest->viavel) && mv)
                    {
                        solBest->copia(sol);
                        custoBest = solBest->distancia;
                        estat.ultimaAtualizacaoBest = i;

                        //solBest->print(instance);
                        //cout<<"i: "<<i<<"\n";

                    }

                }

                //PRINT_DEBUG("\t", "END");
            }
 */