//
// Created by igor on 19/11/2021.
//

#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include "EvRoute.h"
#include "Solution.h"


#define MOV_SHIFT       0
#define MOV_SWAP        1
#define MOV_2_OPT       2
#define MOV_CROSS       3

class LocalSearch {
public:
    bool satellites2    = false;
    int idSat0          = -1;
    int idSat1          = -1;
    bool interRoutes    = false;
    int mov             = -1;
    // Shifit: insert0 -> insert1
    Insertion inser0;
    Insertion inser1;
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
    // TODO: add Recharge Station changes for each route;
};

namespace NS_LocalSearch {
    bool intraRouteSwap(Solution& Sol, float& improvement);
    bool intraSatelliteSwap(Solution& Sol, int SatId, const Instance& Inst, float& improvement);
    bool interSatelliteSwap(Solution&, const Instance& Inst, float& improvement);

    void swapMov(Solution& Sol, const LocalSearch2& mov);
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
