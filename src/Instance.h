#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>

class Instance{
public:
    Instance(std::vector<std::vector<double>>& distMat, float truckCap, float evCap, float evBattery,
             int nSats, int nClients, int nRS, std::vector<std::pair<float,float>>& coordinates, std::vector<float>& demands);
    float getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
    float getDistance(int n1, int n2) const;
    std::pair<float,float> getCoordinate(int node) const;
    static int getDepotIndex() {return 0;}
    int getFirstClientIndex() const;
    int getEndClientIndex() const;
    int getFirstRechargingSIndex() const;
    int getEndRechargingSIndex() const;
    int getFirstSatIndex() const;
    int getEndSatIndex() const;

    float getTruckCap() const;
    float getEvCap() const;
    float getEvBattery() const;
    int getNSats() const;
    int getNClients() const;
    /** gets the number of Recharging Stations.
     * @return number of Recharging Stations
     */
    int getN_RechargingS() const;
    int getN_Evs() const;
    int getN_Trucks() const;

    bool isClient(int node) const;
    bool isRechargingStation(int node) const;
    bool isSatelite(int node) const;
    bool isDepot(int node) const;

    float getEvCost() const;

    float getTruckCost() const;
    int getNNodes() const;

private:

    std::vector<std::vector<double>> distMat;
    std::vector<float> demands;
    std::vector<std::pair<float,float>> coordinates;
    float truckCap, evCap, evBattery;
    int nSats, nClients, nRechargingS;
    float evCost, truckCost;

    int nAproxEv;
    int nAproxTruck;
};

#endif
