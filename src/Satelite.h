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
    bool checkSatellite(std::string &erro, const Instance &Inst);
    void print(std::string &str, const Instance &instance);
    void print(const Instance &instance);
    void setDemand(float demand_){demand = demand_;}

    int getId() const;

    std::vector<EvRoute> vetEvRoute;
    int tamVetEvRoute;
    int id; // number of the clientId in the instance;
    float demand;
    float evCapacity;
    float evBattery;
    std::pair<int, int> position; // dont know if needed
};
#endif //INC_2E_EVRP_SATELITE_H
