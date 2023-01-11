/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/12/22
 *  Arquivo: Construtivo2.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_CONSTRUTIVO2_H
#define INC_2E_EVRP_CONSTRUTIVO2_H

#include "Construtivo.h"

namespace NS_Construtivo2
{

    bool construtivo2SegundoNivelEV(Solucao &sol, Instancia &instancia, const float alpha, const ublas::matrix<int> &matClienteSat,
                                    bool listaRestTam, const float beta, const BoostC::vector<int> &satUtilizados);

    void construtivo2(Solucao &Sol, Instancia &instancia, const float alpha, const float beta,
                      const ublas::matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil);

}

#endif //INC_2E_EVRP_CONSTRUTIVO2_H
