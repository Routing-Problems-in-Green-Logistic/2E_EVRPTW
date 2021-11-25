#include <cmath>
#include "Instance.h"

Instance::Instance(std::vector<std::vector<double>> &distMat, float truckCap, float evCap, float evBattery, int nSats,
                   int nClients, int nRS, std::vector<std::pair<float, float>> &coordinates,
                   std::vector<float> &demands) {
    this->demands = demands;
    this->coordinates = coordinates;
    this->distMat = distMat;
    this->truckCap = truckCap;
    this->evCap = evCap;
    this->evBattery = evBattery;
    this->nRechargingS = nRS;
    this->nClients = nClients;
    this->nSats = nSats;
    this->evCost = 0;
    this->truckCost = 0;


    float sumDemands = 0.0;
    for(auto demand:demands)
        sumDemands += demand;

    int div = int(ceil(sumDemands/evCap));
    nAproxEv = div + int(ceil(div*0.1));

    div =  int(ceil(sumDemands/truckCap));
    nAproxTruck =  div + int(ceil(div*0.2));
}

// Getters and Setters
float Instance::getDemand(int node) const {
    return this->demands[node];
}

float Instance::getTruckCap() const {
    return truckCap;
}

float Instance::getEvCap() const {
    return evCap;
}

float Instance::getEvBattery() const {
    return evBattery;
}

int Instance::getNSats() const {
    return nSats;
}

int Instance::getNClients() const {
    return nClients;
}

int Instance::getN_RechargingS() const {
    return nRechargingS;
}


int Instance::getN_Evs() const
{
    return nAproxEv;
}

int Instance::getN_Trucks() const
{

    return nAproxTruck;
}

float Instance::getDistance(int n1, int n2) const {
    if(n1==n2)
        return 0.0;

    return (float)this->distMat.at(n1).at(n2);
}

float Instance::getEvCost() const {
    return evCost;
}

float Instance::getTruckCost() const {
    return truckCost;
}

int Instance::getFirstClientIndex() const {
    return (int)this->demands.size() - this->getNClients() - this->getN_RechargingS(); // the last vetEvRoute are vetEvRoute;

}
int Instance::getEndClientIndex() const
{
    return getNSats()+getNClients();
}

int Instance::getFirstRechargingSIndex() const {
    return (int)this->demands.size() - this->getN_RechargingS();
}

int Instance::getEndRechargingSIndex() const
{
    return getNSats() + getNClients() + getN_RechargingS();
}

int Instance::getFirstSatIndex() const {
    return 1;
}

int Instance::getEndSatIndex() const
{
    return getNSats();
}

std::pair<float,float> Instance::getCoordinate(int node) const {
    return this->coordinates.at(node);
}

bool Instance::isClient(int node) const {
    return node >= this->getFirstClientIndex() && node < (this->getFirstClientIndex() + this->getNClients());
}

bool Instance::isRechargingStation(int node) const {
    return node >= this->getFirstRechargingSIndex() && node < this->getFirstRechargingSIndex() +
                                                                      this->getN_RechargingS();
}

bool Instance::isSatelite(int node) const {
    return node >= this->getFirstSatIndex() && node < this->getFirstSatIndex() + this->getNSats();
}

bool Instance::isDepot(int node) const {
    return node == 0;
}
