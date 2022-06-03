/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    02/06/22
 *  Arquivo: Grasp.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_GRASP_H
#define INC_2E_EVRP_GRASP_H

#include "Instance.h"
#include "Solucao.h"

namespace NameS_Grasp
{

    struct Estatisticas
    {
        int numIte = 0;
        int numSol = 0;
        double distAcum = 0.0;
        string erro;

        double media(){return distAcum/numSol;}
    };

    Solucao* grasp(Instance &instance, const int numIte, const float alfa, const float beta, Estatisticas &estat);

    void inicializaSol(Solucao &sol, Instance &instance);

}

#endif //INC_2E_EVRP_GRASP_H
