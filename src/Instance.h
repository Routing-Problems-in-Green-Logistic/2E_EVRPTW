#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>

class Instance{
public:
    Instance(std::vector<std::vector<double>>& distMat, float truckCap, float evCap, float evBattery,
             int nSats, int nClients, int nRS, std::vector<std::pair<float,float>>& coordinates, std::vector<float>& demands);
    float getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
                                    // https://stackoverflow.com/questions/751681/meaning-of-const-last-in-a-function-declaration-of-a-class
    float getDistance(int n1, int n2) const;
    int getFirstClientIndex() const;
    float getTruckCap() const;
    float getEvCap() const;
    float getEvBattery() const;
    int getNSats() const;
    int getNClients() const;
    /** gets the number of Recharging Stations.
     * @return number of Recharging Stations
     */
    int getNrs() const;

private:
    std::vector<std::vector<double>> distMat;
    std::vector<float> demands;
    std::vector<std::pair<float,float>> coordinates;
    float truckCap, evCap, evBattery;
    int nSats, nClients, nRS;
};

#endif
