#ifndef INC_2E_EVRP_GREEDYALGORITHM_H
#define INC_2E_EVRP_GREEDYALGORITHM_H

#include "Solution.h"
#include <list>
#include <vector>

namespace GreedyAlgNS
{

    bool secondEchelonGreedy(Solution& Sol, const Instance& Inst, float alpha);
    void firstEchelonGreedy(Solution &Sol, const Instance &Inst);
    void greedy(Solution &Sol, const Instance &Inst);
    bool visitAllClientes(std::vector<int> &visitedClients, const Instance &Inst);
}

#endif //INC_2E_EVRP_GREEDYALGORITHM_H

