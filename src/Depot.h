#ifndef INC_2E_EVRP_DEPOT_H
#define INC_2E_EVRP_DEPOT_H


#include "TruckRoute.h"
class Depot {
public:
    Depot(const Instance&, int id = 0);
    float getDemand(const Instance&) const;
    float getNRoutes() const;
    TruckRoute& getRoute(int i);
private:
    std::vector<TruckRoute> routes;
    int id; // number of the node in the instance;
    float demand;
    float truckCapacity;
    std::pair<int, int> position; // dont know if needed
};


#endif //INC_2E_EVRP_DEPOT_H
