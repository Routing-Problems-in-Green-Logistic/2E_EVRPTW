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
    std::pair<float,float> getCoordinate(int node) const;
    static int getDepotIndex() {return 0;}
    int getFirstClientIndex() const;
    int getLastClienteIndex() const {return getFirstRechargingStationIndex();};
    /** returns the index of the first Recharging station.
     *
     * @return index of the first recharging station (RS)
     */
    int getFirstRechargingStationIndex() const;
    int getLastRechargingStationIndex()const{return demands.size()-1;};
    int getFirstSatIndex() const;
    int getLastSatIndex() const {return getFirstClientIndex()-1;};
    float getTruckCap() const;
    float getEvCap() const;
    float getEvBattery() const;
    int getNSats() const;
    int getNClients() const;
    /** gets the number of Recharging Stations.
     * @return number of Recharging Stations
     */
    int getNumRechargingS() const;

    bool isClient(int node) const;
    bool isRechargingStation(int node) const;
    bool isSatelite(int node) const;
    bool isDepot(int node) const;

    float getEvCost() const;

    float getTruckCost() const;

private:

    std::vector<std::vector<double>> distMat;
    std::vector<float> demands;
    std::vector<std::pair<float,float>> coordinates;
    float truckCap, evCap, evBattery;
    int nSats, nClients, nRechargingS;
    float evCost, truckCost;
};

#endif
