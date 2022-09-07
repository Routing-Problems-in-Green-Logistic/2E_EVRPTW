#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include <cfloat>
#include "EvRoute.h"
#include "Solucao.h"
#include "Auxiliary.h"
#include "greedyAlgorithm.h"
#include "ViabilizadorRotaEv.h"

using namespace GreedyAlgNS;


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
        NameViabRotaEv::InsercaoEstacao estacao;
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



    bool mvEvShifitIntraRota(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux, const int selecao);
    int setRotaMvEvShifitIntraRota(EvRoute &evRoute, EvRoute &evRouteAux, int i, int pos, Instance &instancia);
    void getMov(int movId, string &mov);
    void insereEstacaoRota(EvRoute &evRoute, NameViabRotaEv::InsercaoEstacao &insercaoEstacao, Instance &instance, double tempoSaida);
    bool mvEvSwapIntraRota(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux);
    bool mvEv2opt(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux);

    bool mvEvShifitInterRotas(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const bool interSat = false);

}


#endif //INC_2E_EVRP_LOCALSEARCH_H
