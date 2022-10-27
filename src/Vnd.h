#include "Solucao.h"
#include "Instancia.h"

#ifndef INC_2E_EVRP_VND_H
#define INC_2E_EVRP_VND_H


#define MV_EV_SHIFIT_INTRA_ROTA             0
#define MV_EV_SWAP_INTRA_ROTA               1
#define MV_EV_2OPT                          2
#define MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAT  3
#define MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT  4
#define MV_EV_SWAP_INTER_ROTAS_INTRA_SAT    5

#define NUM_MV    1
#define NUM_MV_LS 6

namespace NS_vnd
{
    class MvValor
    {
    public:

        int mv = -1;
        double dif = 0.0;
        int quant = 0;

        MvValor()=default;

        double getMedia()
        {
            if(quant == 0)
                return 0.0;
            else
                return dif/quant;
        }

        void add(double disOrig, double novaDist)
        {
            quant += 1;
            dif += (disOrig-novaDist);
        }

    };

    void rvnd(Solucao &solution, Instancia &instance, const float beta, std::vector<MvValor> &vetMvValor);
}

#endif //INC_2E_EVRP_VND_H
