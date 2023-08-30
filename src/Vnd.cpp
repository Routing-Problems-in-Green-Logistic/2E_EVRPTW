#include <chrono>
#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"
#include "LocalSearch2.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;
using namespace NS_LocalSearch2;

#define CHECK_SOLUTION FALSE
#define TESTE_UM_MV    FALSE



double NS_TimeMV::vetTempoCpuMV[NUM_MV];



/**
 *
 * @param solution
 * @param instance
 * @param beta
 * @param vetMvValor
 * @param vetMvValor1Nivel      0: MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT; 1: MV_EV_SWAP_INTER_ROTAS_INTER_SAT
 */

void NS_vnd::rvnd(Solucao &solution, Instancia &instance, const float beta, Vector<MvValor> &vetMvValor, Vector<MvValor> &vetMvValor1Nivel)
{

    static bool iniVetTempoCpuMV = false;
    if(!iniVetTempoCpuMV)
    {
        for(int i=0; i < NUM_MV; ++i)
            NS_TimeMV::vetTempoCpuMV[i] = 0.0;

        iniVetTempoCpuMV = true;
    }


#if TESTE_UM_MV == FALSE
    static int vetMv[NUM_MV];
#endif

#if TESTE_UM_MV == TRUE
    static int vetMv[NUM_MV] = {MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAiT};
#endif

#if TESTE_UM_MV == FASLE

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

#endif


    static EvRoute evRouteAux(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);
    static EvRoute evRouteAux1(1, instance.getFirstEvIndex(), instance.evRouteSizeMax, instance);

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

            // switch(vetMv[i])


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

                case MV_EV_CROSS_INTRA_SAT:
                    aplicacao = mvInterRotasIntraSat(solution, instance, evRouteAux, evRouteAux1, cross,
                                                     MV_EV_CROSS_INTRA_SAT);
                    break;


                case MV_EV_CROSS_INTER_SATS:
                    aplicacao = mvInterRotasInterSats(solution, instance, evRouteAux, evRouteAux1, cross, beta,
                                                      MV_EV_CROSS_INTER_SATS);
                    break;

                case MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTRA_SAT:
                    aplicacao = mvEvShifit2Nos_interRotasIntraSat(solution, instance, evRouteAux, evRouteAux1);
                    break;

                case MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS:

                    aplicacao = mvEvShifit2Nos_interRotasInterSats(solution, instance, evRouteAux, evRouteAux1, beta);
                    break;

                case MV_SHIFIT_EV_SATS:
                    aplicacao = mvShifitEvs_interRotasInterSats(solution, instance, evRouteAux, beta);
                    break;

                default:
                    cout << "ERRO: MV(" << vetMv[i] << ") NAO EXISTE\n";
                    throw "ERRO";
                    break;
            }



            //aplicacao = mvShifitEvs_interRotasInterSats(solution, instance, evRouteAux, beta);

            vetMvValor[vetMv[i]].quantChamadas += 1;

            if(aplicacao)
            {
                vetMvValor[vetMv[i]].add(valOrig, solution.distancia);
                i = 0;
                //cout<<"Aplicacao\n";
                string erro;
                if(!solution.checkSolution(erro, instance))
                {
                    cout<<"ERRO MOVIMENTO: "<<vetMv[i]<<"\n";
                    cout<<erro<<"\n";
                    throw "ERRO";
                }

            }
            else
                i += 1;

        }

        //mvSplitCarga(solution, instance);

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