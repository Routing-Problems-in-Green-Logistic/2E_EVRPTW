//
// Created by abrolhus on 06/09/2021.
//

#ifndef INC_2E_EVRP_LOCALSEARCH_H
#define INC_2E_EVRP_LOCALSEARCH_H

#include "algorithm.h"

namespace lsh {
    /** swaps routeI[i] and routeJ[j].
     *
     */
    bool swapMove(std::vector<int>& routeI, std::vector<int>& routeJ, int i, int j, const Instance& Inst, float& discount);
    bool swap(Solution& Sol, const Instance& Inst);
    bool twoOptMove(std::vector<int>& route, int i, int j, const Instance& Inst, float& discount);
    bool twoOpt(Solution& Sol, const Instance& Inst);

    /** moves loserRoute[i] to winnerRoute[j].
     *
     */
    bool shiftMove(std::vector<int>& loserRoute, std::vector<int>& winnerRoute, int i, const Instance& Inst, float& discount);
    bool reinsertMove(std::vector<int> &route, int position, const Instance& Inst);
    bool reinsertion(Solution& Sol, const Instance& Inst);
    bool shift(Solution& Sol, const Instance& Inst);
    void swapVectorElements(std::vector<int>& v, int i, int j);

    bool swap22Moves();


}
#endif //INC_2E_EVRP_LOCALSEARCH_H
