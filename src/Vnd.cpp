#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"
#include "LocalSearch2.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;
using namespace NS_LocalSearch2;

#define CHECK_SOLUTION FALSE


/**
 *
 * @param solution
 * @param instance
 * @param beta
 * @param vetMvValor
 * @param vetMvValor1Nivel      0: MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT; 1: MV_EV_SWAP_INTER_ROTAS_INTER_SAT
 */

void NS_vnd::rvnd(Solucao &solution, Instancia &instance, const float beta, BoostC::vector<MvValor> &vetMvValor, BoostC::vector<MvValor> &vetMvValor1Nivel)
{


    static int vetMv[NUM_MV];// = {MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS};

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
    EvRoute evRouteAux1(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);

    int i = 0;

    try
    {
        double valOrig   = 0.0;
        double val1Nivel = 0.0;

        while(i < NUM_MV)
        {
            if(i == 0)
            {
                valOrig   = solution.distancia;
                val1Nivel = solution.getDist1Nivel();
            }

            bool aplicacao = false;
            double val = solution.distancia;
            //cout << "\t\tMV: " << vetMv[i] << "\n";

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

                case MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAT:
                    aplicacao = mvEvShifitInterRotasIntraSat(solution, instance, evRouteAux, evRouteAux1);
                    break;

                case MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT:
                    aplicacao = mvEvShifitInterRotasInterSats(solution, instance, evRouteAux, evRouteAux1, beta);
                    break;


                case MV_EV_SWAP_INTER_ROTAS_INTRA_SAT:
                    aplicacao = mvEvSwapInterRotasIntraSat(solution, instance, evRouteAux, evRouteAux1);
                    break;


                case MV_EV_SWAP_INTER_ROTAS_INTER_SAT:
                    aplicacao = mvEvSwapInterRotasInterSats(solution, instance, evRouteAux, evRouteAux1, beta);
                    break;

                case MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTRA_SAT:
                    aplicacao = mvEvShifit2Nos_interRotasIntraSat(solution, instance, evRouteAux, evRouteAux1);
                    break;

                case MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS:

                    aplicacao = mvEvShifit2Nos_interRotasInterSats(solution, instance, evRouteAux, evRouteAux1, beta);
                    break;

                default:
                    cout << "ERRO: MV(" << vetMv[i] << ") NAO EXISTE\n";
                    throw "ERRO";
                    break;

            }

            if(aplicacao)
            {

//cout<<"APLICACAO MV: "<<((solution.distancia-valOrig)/valOrig)*100.0<<"\n";
                vetMvValor[vetMv[i]].add(valOrig, solution.distancia);

                if(NS_Auxiliary::menor(solution.getDist1Nivel(), val1Nivel))
                {

                    if(vetMv[i] == MV_EV_SWAP_INTER_ROTAS_INTER_SAT)
                        vetMvValor1Nivel[1].add(val1Nivel, solution.getDist1Nivel());
                    else if(vetMv[i] == MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT)
                        vetMvValor1Nivel[0].add(val1Nivel, solution.getDist1Nivel());
                }

                #if CHECK_SOLUTION

                    string erro;
                    if(!solution.checkSolution(erro, instance))
                    {
                        PRINT_DEBUG("", "ERRO. MV: " << i << "\n");
                        cout << erro << "\n\n";
                        solution.print(instance);

                        throw "ERRO";
                    }
                #endif
                i = 0;
            } else
                i += 1;

            //cout<<"#############################################################\n\n";

        }

    }
    catch(const char *erro)
    {

        PRINT_DEBUG("", "ERRO. MV: " << i << "\n");
        cout << erro << "\n\n";
        solution.print(instance);

        throw "ERRO";
    }

    //cout<<"//////////////////////////////////////////////////////////////////\n\n";

}