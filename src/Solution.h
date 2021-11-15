#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H
#include <vector>
#include "Satelite.h"
#include "Depot.h"

class Solution {
public:
    Solution(const Instance& Inst);
    int getNSatelites() const;
    Satelite* getSatelite(int index);
    Depot* getDepot();
private:
    std::vector<Satelite*> satelites;
    Depot* Dep;
    int nTrucks;
    int nEvs;

};
#endif //INC_2E_EVRP_SOLUTION_H
