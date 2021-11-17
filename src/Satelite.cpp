#include "Satelite.h"

Satelite::Satelite(int id, const Instance& Inst)
{
    this->evCapacity = Inst.getEvCap();
    this->evBattery = Inst.getEvBattery();
    this->position= {0,0};
    this->id = id;
    this->demand= 0;

    const int max = 2+Inst.getNClients()+Inst.getN_RechargingS();

    vetEvRoute.reserve(Inst.getN_Evs());
    tamVetEvRoute = Inst.getN_Evs();

    for(int i=0; i < Inst.getN_Evs(); ++i)
        vetEvRoute.emplace_back(this->evBattery, this->evCapacity, this->id, max);

}

float Satelite::getDemand(const Instance& Inst) const { // O(n)
    float totDemand = 0;
    for(const auto& route : this->vetEvRoute){
        totDemand += route.getDemand(Inst);
    }
    return totDemand;
}

int Satelite::getNRoutes() const {
    return this->vetEvRoute.size();
}

EvRoute &Satelite::getRoute(int i) {
    return this->vetEvRoute.at(i);
}
