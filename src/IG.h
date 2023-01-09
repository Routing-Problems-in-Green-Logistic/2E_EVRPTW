/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    09/01/23
 *  Arquivo: IG.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_IG_H
#define INC_2E_EVRP_IG_H

#include "Grasp.h"

namespace NameS_IG
{
    void iteratedGreedy(Instancia &instance, ParametrosGrasp &parametros, NameS_Grasp::Estatisticas &estat,
                        const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                        BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida);
}

#endif //INC_2E_EVRP_IG_H
