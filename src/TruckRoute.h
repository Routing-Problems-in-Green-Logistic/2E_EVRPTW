#ifndef INC_2E_EVRP_TRUCKROUTE_H
#define INC_2E_EVRP_TRUCKROUTE_H
#include "Route.h"
#include "Instance.h"
class TruckRoute{

public:
    TruckRoute(int depot, float truckCapacity);
    int size() const;
    float getDemand() const;
    float getMinDemand() const;
    float getCost() const;
    void print() const;
    float getCurrentCapacity() const;
    bool insert(int node, int pos, const Instance& Inst);
private:
    float totalDemand;
    float remainingCapacity;
    float cost;
    int depot;
    float initialCapacity;
    std::vector<int> nodes;
};


#endif //INC_2E_EVRP_TRUCKROUTE_H