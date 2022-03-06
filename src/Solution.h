#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H
#include <vector>
#include "Satelite.h"
#include "Depot.h"

class Solution
{
public:
    Solution(const Instance& Inst);
    Solution(const Solution &solution);
    void copia(const Solution &solution);

    int getNSatelites() const;
    int findSatellite(int id) const;
    Satelite* getSatelite(int index);
    Depot* getDepot();
    bool checkSolution(std::string &erro, const Instance &Inst);
    void print(std::string &str,  const Instance &instance);
    void print(const Instance& Inst);
    double calcCost(const Instance&);
    float getDistanciaTotal();

    std::vector<Satelite*> satelites;
    Depot* Dep;
    int nTrucks;
    int nEvs;

    bool viavel = true;

    std::vector<Route> primeiroNivel;

    bool mvShiftIntraRota = false;
    bool mvShiftInterRotas = false;
    bool mvCross = false;

};
#endif //INC_2E_EVRP_SOLUTION_H
