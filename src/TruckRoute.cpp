#include "TruckRoute.h"

int TruckRoute::size() const {
    return this->nodes.size();
}

float TruckRoute::getDemand() const {
    return this->totalDemand;
}

float TruckRoute::getMinDemand() const {
    return -1;
}

float TruckRoute::getCost() const {
    return this->cost;
}

void TruckRoute::print() const {
    for(auto node : this->nodes){
        std::cout << node << ", ";
    }
    std::cout << std::endl;
}

float TruckRoute::getCurrentCapacity() const {
    return remainingCapacity;
}

bool TruckRoute::insert(int node, int pos, const Instance &Inst) {
    return false;
}

TruckRoute::TruckRoute(int depot, float truckCapacity) {
    this->depot = depot;
    this->remainingCapacity = truckCapacity;
    this->initialCapacity = truckCapacity;
}
int TruckRoute::getNodeAt(int pos) const {
    return this->nodes.at(pos);
}
