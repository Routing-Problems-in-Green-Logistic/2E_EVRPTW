#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include <cfloat>
#include "../EvRoute.h"
#include "../Solucao.h"
#include "../Auxiliary.h"
#include "../CONSTRUTIVO/Construtivo.h"
#include "../ViabilizadorRotaEv.h"

using namespace NS_Construtivo;


#define MOV_SHIFIT_INTRA      0
#define MOV_SWAP_INTRA        1
#define MOV_2_OPT             2
#define MOV_CROSS             3

#define SELECAO_BEST          0
#define SELECAO_PRIMEIRO      1
#define SELECAO_ALEATORIO     2


namespace NS_LocalSearch
{

    class NoLocalS
    {
    public:
        int idSat       = -1;
        int idEvRota    = -1;
        int pos         = -1;
        int i           = -1;
        NS_viabRotaEv::InsercaoEstacao estacao;
    };

    class LocalSearch
    {

    public:

        bool rotaUnica              = true;
        NoLocalS noLocal0;
        NoLocalS noLocal1;
        int8_t mov                  = -1;
        double incrementoDistancia  = DOUBLE_MAX;

        LocalSearch()= default;
        void print(string &str);
        void print() const;
    };


    class SatN_evCarga
    {

    public:

        int sat          = -1;
        int nEv          = -1;
        double cargaUtil = 0.0;

        SatN_evCarga()=default;
        SatN_evCarga(int sat_, int nEv_, double cargaUtil_)
        {
            sat = sat_;
            nEv = nEv_;
            cargaUtil = cargaUtil_;
        }

        bool operator < (SatN_evCarga &outro) const
        {
            return cargaUtil < outro.cargaUtil;
        }

    };

    bool mvEvShifitIntraRota(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux, const int selecao);
    int setRotaMvEvShifitIntraRota(EvRoute &evRoute, EvRoute &evRouteAux, int i, int pos, Instancia &instancia);
    void getMov(int movId, string &mov);
    void insereEstacaoRota(EvRoute &evRoute, NS_viabRotaEv::InsercaoEstacao &insercaoEstacao, Instancia &instance, double tempoSaida);
    bool mvEvSwapIntraRota(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux);
    bool mvEv2opt(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux);

    bool mvEvShifitInterRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1);
    bool mvEvShifitInterRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const float beta);

    bool mvEvSwapInterRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1);
    bool mvEvSwapInterRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const float beta);

    bool conserta1Nivel(Solucao &solucao, const int sat0, const double novaCargaSat0, const int sat1, const double novaCargaSat1);

    void removeRS_Repetido(EvRoute &evRoute, Instancia &instancia, bool atualizaParam);

/*    bool satN_evCargaMenor(const SatN_evCarga &primeiro, const SatN_evCarga &segundo)
    {
        return primeiro.cargaUtil < segundo.cargaUtil;
    }


    bool satN_evCargaMaior(const SatN_evCarga &primeiro, const SatN_evCarga &segundo)
    {
        return primeiro.cargaUtil >= segundo.cargaUtil;
    }*/

}


#endif //INC_2E_EVRP_LOCALSEARCH_H
