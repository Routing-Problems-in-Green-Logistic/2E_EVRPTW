#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H
#include <vector>
#include "Satelite.h"
#include "Depot.h"

class Solution
{
public:
    Solution(const Instance& Inst);
    int getNSatelites() const;
    Satelite* getSatelite(int index);
    Depot* getDepot();
    bool checkSolution(std::string &erro, const Instance &Inst);
    void print(std::string &str);
    void print();


    std::vector<Satelite*> satelites;
    Depot* Dep;
    int nTrucks;
    int nEvs;

    bool viavel = true;

};
#endif //INC_2E_EVRP_SOLUTION_H
