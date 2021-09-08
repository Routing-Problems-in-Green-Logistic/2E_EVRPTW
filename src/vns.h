//
// Created by abrolhus on 07/09/2021.
//

#ifndef INC_2E_EVRP_VNS_H
#define INC_2E_EVRP_VNS_H

#include "localSearch.h"
#include "algorithm.h"
#include "Solution.h"
namespace vns {
    void rvnd(Solution &Sol, const Instance &Inst);

    void gvns(Solution &Sol, const Instance &Inst);
}
#endif //INC_2E_EVRP_VNS_H
