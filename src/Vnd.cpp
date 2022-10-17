

#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;

#define MV_EV_SHIFIT_INTRA_ROTA     0
#define MV_EV_SWAP_INTRA_ROTA       1
#define MV_EV_2OPT                  2
#define MV_EV_SHIFIT_INTER_ROTAS    3

#define NUM_MV 1

void NS_vnd::rvnd(Solucao &solution, Instance &instance)
{
    //cout<<"\tRVND INICIO\n";

    static int vetMv[NUM_MV] = {3};

/*    for(int i=0; i < NUM_MV; ++i)
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
    }*/


    EvRoute evRouteAux(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);
    EvRoute evRouteAux1(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);

    int i=0;
    bool valEsp = false;
    while(i < NUM_MV)
    {
        bool aplicacao = false;
        double val = solution.distancia;

        switch(vetMv[i])
        {
            case MV_EV_SHIFIT_INTRA_ROTA:
                aplicacao = mvEvShifitIntraRota(solution, instance, evRouteAux, SELECAO_PRIMEIRO);
                break;

            case MV_EV_SWAP_INTRA_ROTA:
                aplicacao = mvEvSwapIntraRota(solution, instance, evRouteAux);
                break;

            case MV_EV_2OPT:
                aplicacao = mvEv2opt(solution, instance, evRouteAux);
                break;

            case MV_EV_SHIFIT_INTER_ROTAS:
                aplicacao = mvEvShifitInterRotas(solution, instance, evRouteAux, evRouteAux1, false);
                break;

            default:
                cout<<"ERRO: MV("<<i<<") NAO EXISTE\n";
                throw "ERRO";
                break;

        }

        if(aplicacao)
        {

            string erro;
            if(!solution.checkSolution(erro, instance))
            {
                PRINT_DEBUG("", "ERRO. MV: "<<i<<"\n");
                cout<<erro<<"\n";
                exit(-1);
            }

            i = 0;
        }
        else
            i += 1;


    }

}