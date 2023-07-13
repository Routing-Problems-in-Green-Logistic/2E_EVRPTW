/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Modelo.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_MODELO_H
#define INC_2E_EVRP_MODELO_H
#include "gurobi_c++.h"

namespace ModeloNs
{

    void modelo();
    void setParametrosModelo(GRBModel &model);
}

#endif //INC_2E_EVRP_MODELO_H
