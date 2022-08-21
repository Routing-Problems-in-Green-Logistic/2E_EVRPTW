#include "Solucao.h"
#include "Instance.h"

#ifndef INC_2E_EVRP_VND_H
#define INC_2E_EVRP_VND_H

#define VND_SHIFIT_INTRA_ROTA           0
#define VND_SHIFIT_INTER_ROTAS          1
#define VND_SWAP_INTRA_ROTA             2
#define VND_SWAP_INTER_ROTAS            3
#define VND_2_OPT                       4
#define VND_CROSS                       5

#define NUM_MAX 6



namespace NS_vnd
{
    void rvnd(Solucao &solution, Instance &instance);
}

#endif //INC_2E_EVRP_VND_H
