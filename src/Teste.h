//
// Created by ****** on 01/02/2022.
//

#ifndef INC_2E_EVRP_TESTE_H
#define INC_2E_EVRP_TESTE_H

/*
#include "Auxiliary.h"
#include "Instancia.h"


namespace NameTeste
{

#define T_HEURISTICA                                0
#define T_MV_SHIFIT_INTRA_ROTA                      1
#define T_MV_SHIFIT_INTER_ROTAS_INTRA_SAT           2
#define T_MV_CROSS_INTRA_SAT                        3
#define T_TAM 4

    class MovimentoEst
    {

    public:
        float n = 0.0;
        float somaDist = 0.0;
        double somaTempo = 0.0;
        float best = FLOAT_MAX;

        void soma(const float dist, const double temp)
        {
            n += 1.0;
            somaDist += dist;
            somaTempo += temp;
            if(dist < best)
                best = dist;
        }

        string print(string nome)
        {
            if(n > 0)
            {
                string saida;

                saida += (boost::format("%.2f ") %(somaDist / n)).str() + ";" + (boost::format("%.2f S") % float(somaTempo)).str()+ ";" + (boost::format("%.2f ") %(best)).str() +
                        "; " + to_string(int(n));
                //cout<<somaTempo<<"\n";
                return saida;
            }
            else
                return nome+"_n; 0; ";
        }


    };

    void testeMovimentos(string &saida, const Instancia &instance, long semente, int k);

}
*/

#endif //INC_2E_EVRP_TESTE_H
