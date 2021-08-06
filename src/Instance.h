#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>

class Instance{
public:
    Instance(std::vector<std::vector<double>>& distMat, float truckCap, float evCap, float evBattery,
             int nSats, int nClients, int nRS, std::vector<std::pair<float,float>>& coordinates, std::vector<float>& demands){
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
private:
    std::vector<std::vector<double>> distMat;
    std::vector<float> demands;
    std::vector<std::pair<float,float>> coordinates;
    float truckCap, evCap, evBattery;
    int nSats, nClients, nRS;
};

#endif
