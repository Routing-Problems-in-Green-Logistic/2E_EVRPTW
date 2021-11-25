//
// Created by igor on 19/11/2021.
//

#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include "EvRoute.h"
#include "Solution.h"


#define MOV_SHIFIT      0
#define MOV_SWAP        1
#define MOV_2_OPT       2
#define MOV_CROSS       3

namespace NS_LocalSearch {

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
    bool intraSwap(const LocalSearch& ls, Solution& Sol, float& improvement);
    bool intraSatelliteSwap(Solution& Sol, int SatId, const Instance& Inst, float& improvement);


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
