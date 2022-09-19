

#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;

#define MV_EV_SHIFIT_INTRA_ROTA 0
#define MV_EV_SWAP_INTRA_ROTA   1
#define MV_EV_2OPT              2

#define NUM_MV 3

void NS_vnd::rvnd(Solucao &solution, Instance &instance)
{
    //cout<<"\tRVND INICIO\n";

    static int vetMv[NUM_MV];

    for(int i=0; i < NUM_MV; ++i)
    {
        vetMv[i] = rand_u32()%NUM_MV;
        bool invalido = true;

        while(invalido)
        {
            int j=0;
            for(j = 0; j < i; ++j)
            {
                if(vetMv[j] == vetMv[i])
                {
                    vetMv[i] = (vetMv[i]+1)%NUM_MV;
                    j=0;
                    break;
                }
            }

            if(j==i)
                invalido = false;

        }
    }


    EvRoute evRouteAux(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);

    //while(mvEvSwapIntraRota(solution, instance, evRouteAux))
/*    while(mvEv2opt(solution, instance, evRouteAux))
    {
        string erro;
        if(!solution.checkSolution(erro, instance))
        {
            PRINT_DEBUG("", "ERRO. mvEvSwapIntraRota:\n");
            cout<<erro<<"\n";
            exit(-1);
        }

        //cout<<"#######################################################################\n\n";
    }*/

//cout<<"***************************************************************************\n\n";

    int i=0;
    while(i < NUM_MV)
    {
        bool aplicacao = false;
        double val = solution.distancia;
        //cout<<"\t\t"<<vetMv[i]<<"; i: "<<i<<"\n";
        //cout<<"\t\t"<<val<<"\n";

        switch(vetMv[i])
        {
            case MV_EV_SHIFIT_INTRA_ROTA:
                aplicacao = mvEvShifitIntraRota(solution, instance, evRouteAux, SELECAO_PRIMEIRO);
                break;

            case MV_EV_SWAP_INTRA_ROTA:
                //aplicacao = mvEvSwapIntraRota(solution, instance, evRouteAux);
                break;

            case MV_EV_2OPT:
                aplicacao = mvEv2opt(solution, instance, evRouteAux);
                break;

            default:
                cout<<"ERRO: MV("<<i<<") NAO EXISTE\n";
                throw "ERRO";
                break;

        }

        if(aplicacao)
        {
            solution.recalculaDist();
            string erro;
            if(!solution.checkSolution(erro, instance))
            {
                PRINT_DEBUG("", "ERRO. MV: "<<i<<"\n");
                cout<<erro<<"\n";
                exit(-1);
            }

            if(solution.distancia < (val-1E-2))
                i = 0;
            else
                i += 1;
        }
        else
            i += 1;
    }
    //cout<<"\n";
    //cout<<"\tRVND FIM\n\n";


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