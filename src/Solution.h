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
    int findSatellite(int id) const;
    Satelite* getSatelite(int index);
    Depot* getDepot();
    bool checkSolution(std::string &erro, const Instance &Inst);
    void print(std::string &str);
    void print(const Instance& Inst);
    double calcCost(const Instance&);
    void print();
    float getDistanciaTotal()
    {
        float distancia = 0.0;

        for(auto sat:satelites)
        {
            for(auto route:sat->vetEvRoute)
                distancia += route.distance;
        }

        for(auto route:primeiroNivel)
            distancia += route.totalDistence;

        return distancia;
    }

    std::vector<Satelite*> satelites;
    Depot* Dep;
    int nTrucks;
    int nEvs;

    bool viavel = true;

    std::vector<Route> primeiroNivel;

};
#endif //INC_2E_EVRP_SOLUTION_H
