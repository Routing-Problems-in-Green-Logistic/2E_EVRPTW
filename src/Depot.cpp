#include "Depot.h"

Depot::Depot(const Instance& Inst, int id) {
    this->truckCapacity = Inst.getTruckCap();
    this->position= {0,0};
    this->id = id;
    this->demand= 0;
    this->routes = {TruckRoute(this->id, this->truckCapacity)};
}

float Depot::getDemand(const Instance &) const {
    return 0;
}

float Depot::getNRoutes() const {
    return 0;
}

TruckRoute& Depot::getRoute(int i) {
    return this->routes.at(i);
}
