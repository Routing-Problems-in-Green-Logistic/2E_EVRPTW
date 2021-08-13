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
    this->nRS = nRS;
    this->nClients = nClients;
    this->nSats = nSats;
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

int Instance::getNrs() const {
    return nRS;
}

float Instance::getDistance(int n1, int n2) const {
    return (float)this->distMat[n1][n2];
}

int Instance::getFirstClientIndex() const {
    return (int)this->demands.size() - this->getNClients(); // the last nodes are clients;

}
