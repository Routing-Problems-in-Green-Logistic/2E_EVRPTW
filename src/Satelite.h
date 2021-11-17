#ifndef INC_2E_EVRP_SATELITE_H
#define INC_2E_EVRP_SATELITE_H

#include <vector>
#include "EvRoute.h"
#include "Instance.h"
class Satelite {
public:
    Satelite(int id, const Instance&);
    float getDemand(const Instance&) const;
    int getNRoutes() const;
    EvRoute& getRoute(int i);
private:
    std::vector<EvRoute> vetEvRoute;
    int tamVetEvRoute;
    int id; // number of the clientId in the instance;
    float demand;
    float evCapacity;
    float evBattery;
    std::pair<int, int> position; // dont know if needed
};
#endif //INC_2E_EVRP_SATELITE_H
