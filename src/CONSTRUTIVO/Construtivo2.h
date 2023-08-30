/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    13/12/22
 *  Arquivo: Construtivo2.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_CONSTRUTIVO2_H
#define INC_2E_EVRP_CONSTRUTIVO2_H

#include "Construtivo.h"

namespace NS_Construtivo2
{

    // Guarda o cliente e a menor dist ate um dos sat
    class DistClienteSat
    {
    public:

        int cliente = -1;
        double dist = 0.0;

        DistClienteSat()=default;
        DistClienteSat(int cliente_, double dist_)
        {
            cliente = cliente_;
            dist    = dist_;
        }


        bool operator < (const DistClienteSat &outro) const
        {
            return dist < outro.dist;
        }
    };

    bool construtivo2SegundoNivelEV(Solucao &sol, Instancia &instancia, const float alpha, const Matrix<int> &matClienteSat,
                                    bool listaRestTam, const float beta, const Vector<int> &satUtilizados);

    void construtivo2(Solucao &Sol, Instancia &instancia, const float alpha, const float beta,
                      const Matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil);

}

#endif //INC_2E_EVRP_CONSTRUTIVO2_H
