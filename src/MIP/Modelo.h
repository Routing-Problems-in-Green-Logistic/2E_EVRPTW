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
#include "../Solucao.h"

namespace ModeloNs
{

    void modelo(const Instancia &instancia);
    void setParametrosModelo(GRBModel &model);
}

#endif //INC_2E_EVRP_MODELO_H
