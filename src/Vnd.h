#include "Solucao.h"
#include "Instancia.h"

#ifndef INC_2E_EVRP_VND_H
#define INC_2E_EVRP_VND_H

/*
#define MV_EV_SHIFIT_INTRA_ROTA                          0
#define MV_EV_SWAP_INTRA_ROTA                            1
#define MV_EV_2OPT                                       2

#define MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAT               3
#define MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT               4

#define MV_EV_SWAP_INTER_ROTAS_INTRA_SAT                 5
#define MV_EV_SWAP_INTER_ROTAS_INTER_SAT                 6

#define MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTRA_SAT     7
#define MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS    8


#define MV_EV_CROSS_INTRA_SAT                            9
#define MV_EV_CROSS_INTER_SATS                           10

#define NUM_MV                                           1

#define NUM_MV_LS                                        11*/

// Definido em Auxiliary.h

namespace  NS_TimeMV
{
    extern double vetTempoCpuMV[NUM_MV];
}


namespace NS_vnd
{
    class MvValor
    {
    public:

        int mv = -1;
        double dif = 0.0;
        double difPercent = 0.0;
        int quant = 0;
        int quantChamadas = 0;


        MvValor()=default;

        double getMedia()
        {
            if(quant == 0)
                return 0.0;
            else
                return dif/quant;
        }

        double getMediaPercent()
        {
            if(quant==0)
                return 0.0;
            else
                return difPercent/quant;
        }

        //              rnvd completo   rvnd retirando iº mv
        void add(double disOrig, double novaDist)
        {
            quant += 1;
            dif += (disOrig-novaDist);
            difPercent += ((disOrig-novaDist)/disOrig)*100.0;
        }

    };

     void rvnd(Solucao &solution, Instancia &instance, float beta, Vector<MvValor> &vetMvValor, Vector<MvValor> &vetMvValor1Nivel);
     void rvnd2(Solucao &solution, Instancia &instance, float beta, Vector<MvValor> &vetMvValor, Vector<MvValor> &vetMvValor1Nivel);
}

#endif //INC_2E_EVRP_VND_H