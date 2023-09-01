/* ****************************************
 * ****************************************
 *  Nome: ***********************
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

    struct RotaInfo
    {
        int satId       = -1;
        int evRouteId   = -1;

        RotaInfo(){}
        RotaInfo(int sat, int ev)
        {
            satId = sat;
            evRouteId = ev;
        }
    };

    Solucao* iteratedGreedy(Instancia &instancia, ParametrosGrasp &parametrosGrasp, NameS_Grasp::Estatisticas &estat,
                            Matrix<int> &matClienteSat, Vector<NS_vnd::MvValor> &vetMvValor, Vector<NS_vnd::MvValor> &vetMvValor1Nivel,
                            NS_parametros::ParametrosSaida &parametrosSaida, NS_parametros::Parametros &parametros,
                            string &cabecalho, string &valores);

    void printVetDadosIg(Vector<DadosIg> &vetDadosIg, NS_parametros::Parametros &parametros);
    void atualizaTempoSaidaInstancia(Solucao &solucao, Instancia &instancia);

    extern std::string strDescricaoIg;

//    enum TipoConstrutivo{CONSTRUTIVO1, CONSTRUTIVO2};


}

namespace ValBestNs
{
    inline double distBest = -1.0;
}

#endif //INC_2E_EVRP_IG_H
