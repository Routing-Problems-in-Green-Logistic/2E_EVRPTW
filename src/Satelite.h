#ifndef INC_2E_EVRP_SATELITE_H
#define INC_2E_EVRP_SATELITE_H

#include <vector>
#include "EvRoute.h"
#include "Instance.h"
class Satelite {
public:
    Satelite(const Instance&, int _satId);
    int getNRoutes() const;
    EvRoute& getRoute(int i);
    bool checkSatellite(std::string &erro, const Instance &Inst);
    void print(std::string &str, const Instance &instance);
    void print(const Instance &instance);


    std::vector<EvRoute> vetEvRoute;
    int tamVetEvRoute = -1;
    int sateliteId = -1;
    float demanda = 0.0;
};
#endif //INC_2E_EVRP_SATELITE_H
