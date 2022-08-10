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

    /*
    bool intraSatelliteSwap(Solucao& Sol, int SatId, const Instance& Inst, float& improvement);
    bool interSatelliteSwap(Solucao&, const Instance& Inst, float& improvement);
    bool mvShiftInterRotasIntraSatellite(Solucao &solution, const Instance &instance);



    void achaEstacoes(const EvRoute *evRoute, std::vector<PosicaoEstacao> &vectorEstacoes, const Instance &instance);
    void achaEstacoesEmComun(const std::vector<PosicaoEstacao> &vectorRota0Estacoes, const std::vector<PosicaoEstacao> &vectorRota1Estacoes, std::vector<PosRota0Rota1Estacao> &vectorEsracoesEmComun);


    bool mvCrossIntraSatellite(Solucao &solution, const Instance &instance);
    void crossAux(const pair<int, int> satIdPair, const pair<int, int> routeIdPair,  EvRoute *evRoute0, EvRoute *evRoute1, LocalSearch &localSearchBest, const Instance &instance);

    void swapMov(Solucao& Sol, const LocalSearch2& mov, const Instance& Inst);
    void shifitInterRotasMvDuasRotas(const pair<int, int> satIdPair, const pair<int, int> routeIdPair,
                                     const EvRoute &evRoute0, const EvRoute &evRoute1, LocalSearch &localSearchBest,
                                     const Instance &instance);

    int buscaEstacao(const std::vector<PosRota0Rota1Estacao> &vector, const int estacao);
    int buscaEstacao(const std::vector<PosicaoEstacao> &vector, const int estacao);

    // Assumi-se que a sequencia esta correta, vector bateriaRestante deve ser corrigido apos <pos>
    bool ajustaBateriaRestante(EvRoute *evRoute, const int pos, const Instance &instance);

    float calculaNovaDistanciaRoute0Cross(EvRoute *evRoute0, const std::vector<int> &evRoute1, const int tamEvRoute1, std::vector<PosRota0Rota1Estacao> &vectorEstacoesEmComun, const int pos0, const int pos1,
                                          const float distanciaAcumRota0, const Instance &instance, const bool escreveRoute0, const bool inverteRotaEmVectorEstacoesEmComun, NameViabRotaEv::InsercaoEstacao &insercaoEstacao);

    float calculaDistanciaAcumulada(const vector<int> &rota, const int pos, const Instance &instance);

    float calculaCargaEv(const EvRoute &rota, const Instance &instance);

     */

}


#endif //INC_2E_EVRP_LOCALSEARCH_H
