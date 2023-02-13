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
    class DadosIg
    {
    public:

        int it = 0;
        double solCorrente = 0.0;
        double solConst    = 0.0;
        double solVnd      = 0.0;
        double solBest     = 0.0;

        DadosIg(){}
        DadosIg(int it_, double solCorr_, double solVnd_, double solBest_)
        {
            it = it_;
            solCorrente = solCorr_;
            solVnd = solVnd_;
            solBest = solBest_;
        }
    };

    Solucao* iteratedGreedy(Instancia &instancia, ParametrosGrasp &parametrosGrasp, NameS_Grasp::Estatisticas &estat,
                            const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                            BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida, NS_parametros::Parametros &parametros);

    void printVetDadosIg(BoostC::vector<DadosIg> &vetDadosIg, NS_parametros::Parametros &parametros);
}

#endif //INC_2E_EVRP_IG_H
