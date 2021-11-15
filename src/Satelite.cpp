#include "Satelite.h"

Satelite::Satelite(int id, const Instance& Inst) {
    this->evCapacity = Inst.getEvCap();
    this->evBattery = Inst.getEvBattery();
    this->position= {0,0};
    this->id = id;
    this->demand= 0;
    this->routes = { EvRoute(this->evBattery, this->evCapacity, this->id) }; // routes vector with one empty EV route in it
}

float Satelite::getDemand(const Instance& Inst) const { // O(n)
    float totDemand = 0;
    for(const auto& route : this->routes){
        totDemand += route.getDemand(Inst);
    }
    return totDemand;
}

int Satelite::getNRoutes() const {
    return this->routes.size();
}

EvRoute &Satelite::getRoute(int i) {
    return this->routes.at(i);
}
