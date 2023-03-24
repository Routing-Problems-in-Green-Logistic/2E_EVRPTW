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
                            const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                            BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida, NS_parametros::Parametros &parametros);

    void printVetDadosIg(BoostC::vector<DadosIg> &vetDadosIg, NS_parametros::Parametros &parametros);
    void atualizaTempoSaidaInstancia(Solucao &solucao, Instancia &instancia);

    extern std::string strDescricaoIg;

    enum TipoConstrutivo{CONSTRUTIVO1, CONSTRUTIVO2};

    class ParametrosIG
    {
    public:

        float alfaSeg100    = 0.2;                          // Valor do parametro alfa do segundo nivel para inst de 100 clientes
        float alfaSeg15     = 0.6;                          // Valor do parametro alfa do segundo nivel para inst <= 15 clientes

        float betaPrim100   = 0.15;                         // Valor do parametro beta do primeiro nivel para inst de 100 clientes
        float betaPrim15    = 0.15;                         // Valor do parametro beta do primeiro nivel para inst <= 15 clientes

        bool  torneio100    = true;                         // Utilizacao de torneio em inst de 100 clientes
        bool  torneio15     = false;                        // Utilizacao de torneio em inst <= 15 clientes

        TipoConstrutivo tipoConstrutivo15 = CONSTRUTIVO1;   // Tipo de construtivo para inst <= 15

        ParametrosIG()=default;
        ParametrosIG(const std::string& fileStr);
        std::string printParam();
    };
}

#endif //INC_2E_EVRP_IG_H
