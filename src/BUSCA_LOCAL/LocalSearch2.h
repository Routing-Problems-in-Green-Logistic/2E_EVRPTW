/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    25/11/22
 *  Arquivo: LocalSearch2.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_LOCALSEARCH2_H
#define INC_2E_EVRP_LOCALSEARCH2_H


#include "../Solucao.h"
#include "Instancia.h"

namespace NS_LocalSearch2
{

    bool mvEvShifit2Nos_interRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1);
}

#endif //INC_2E_EVRP_LOCALSEARCH2_H
