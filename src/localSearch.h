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
    void swapMove(std::vector<int>& routeI, std::vector<int>& routeJ, int i, int j);
    /** moves loserRoute[i] to winnerRoute[j].
     *
     */
    void shiftMove(std::vector<int>& loserRoute, std::vector<int>& winnerRoute, int i, int j);
    bool reinsertMove(std::vector<int> &route, int position, const Instance& Inst);
    bool reinsertion(Solution& Sol, const Instance& Inst);


}
#endif //INC_2E_EVRP_LOCALSEARCH_H
