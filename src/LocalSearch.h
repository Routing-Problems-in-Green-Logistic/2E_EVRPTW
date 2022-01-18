//
// Created by igor on 19/11/2021.
// 351; 352

#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include <cfloat>
#include "EvRoute.h"
#include "Solution.h"
#include "Auxiliary.h"


#define MOV_SHIFIT      0
#define MOV_SWAP        1
#define MOV_2_OPT       2
#define MOV_CROSS       3


namespace NS_LocalSearch {

    class LocalSearch {

    public:

        bool satellites2 = false;
        int idSat0       = -1;
        int idSat1       = -1;

        bool interRoutes = false;
        int mov          = -1;
        float incrementoDistancia = FLOAT_MAX;

        // Shifit: insert0 -> insert1
        Insertion inser0;
        Insertion inser1;

        void print(string &str);
        void print() const;
    };

    class LocalSearch2 {
    public:
        bool satellites2    = false;
        int idSat0          = -1;
        int idSat1          = -1;
        bool interRoutes    = false;
        int mov             = -1;
        // Shifit: insert0 -> insert1
        int idRoute0 = -1;
        int idRoute1 = -1;
        int pos0 = -1;
        int pos1 = -1;
        float incrementoDistancia = FLOAT_MAX;
        // TODO: add Recharge Station changes for each route;
    };


    bool intraRouteSwap(Solution& Sol, float& improvement);
    bool intraSatelliteSwap(Solution& Sol, int SatId, const Instance& Inst, float& improvement);
    bool interSatelliteSwap(Solution&, const Instance& Inst, float& improvement);
    bool mvShifitIntraRota(Solution &solution, const Instance &instance);
    bool mvShiftInterRotas(Solution &solution, const Instance &instance);

    bool mvCross(Solution &solution, const Instance &instance);
    void crossAux(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, const EvRoute &evRoute0,
                  const EvRoute &evRoute1, LocalSearch &localSearchBest, const Instance &instance);

    void swapMov(Solution& Sol, const LocalSearch2& mov, const Instance& Inst);
    void shifitInterRotasMvDuasRotas(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, const EvRoute &evRoute0, const EvRoute &evRoute1, LocalSearch &localSearchBest, const Instance &instance, const float distSol);
    void getMov(int movId, string &mov);

    int buscaEstacao(const std::vector<PosRoute0PosRoute1RechS_ID> &vector, const int estacao);

    // Assumi-se que a sequencia esta correta, vector bateriaRestante deve ser corrigido apos <pos>
    bool ajustaBateriaRestante(EvRoute &evRoute, const int pos, const Instance &instance);

}

#endif //INC_2E_EVRP_LOCALSEARCH_H

/*
 * Movimentos:
 *
 * Entre satellites, intra e inter rotas
 * -Swap        <- Samuel
 * -Shift       <- Igor
 *
 * intra rota
 * -2 opt       <- Samuel
 *
 * Entre satellites, 'inter rotas'
 * -cross       <- Igor
 *
 */
