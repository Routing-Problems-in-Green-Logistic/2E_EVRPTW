#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H

#include <vector>
#include <string>
#include "Constants.h"
#include "Instance.h"

class Solution{

private:
    std::vector<std::vector<int>> routes;
    int nTrucks;
    int nEvs;
    float cost;

public:
    Solution();
    Solution(std::vector<std::vector<int>>& routes, int nTrucks, int nEvs, float cost);
    int getNTrucks() const;
    int getNEvs() const;
    float getCost() const;

    const std::vector<std::vector<int>>& getRoutes() const;
    std::vector<std::vector<int>>& acessRoutes();
};

class Index
{
public:

    int    id     = 0;
    int8_t type   = -1;

    Index()=default;
    Index(int id_, int8_t type_):id(id_), type(type_){}

};

class ElectricVehicle
{

public:

    int numAttendances      = 0;
    int NumMaxAttendances   = -1;                              // Customers + Recharging S. + 2
    std::vector<Index> vet_route;                              // Armazena a sequencia de Index (cliente, recharging s., satellite)
    std::vector<float> vet_batteryCapacity;                    // Armazena a capacidade atual da bateria no cliente

    int     demands       = 0;
    double  transportCost = 0.0;
    double  energyConsum  = 0.0;
    int     idSatellite   = -1;

    ElectricVehicle(int numCustomers, int numRechargingS, int SatelliteId);
    ElectricVehicle(ElectricVehicle &electricVehicleAux);
    ~ElectricVehicle()=default;
    ElectricVehicle()=default;

    ElectricVehicle(const ElectricVehicle&)=delete;
    ElectricVehicle(ElectricVehicle&&)=default;

    void swap(ElectricVehicle &electricVehicleAux);
    std::string getRoute(bool cost) const;
    void getRoute(std::string &routeStr, bool cost)  const;



};

class Satellite
{

public:

    int numMaxVhicles   = 0;
    int numVhicles      = 0;
    std::vector<ElectricVehicle> vetElectricVehicle;

    /*
     *  Indica se o cliente eh atendido por esse satellite.
     *  Possui tamanho: Customer + satellites + Depot + rechargingStation
     *
     *  Possicao de customer e rechargingStation: 1 ou 0
     *  Possicao de satellite e depot -1
     */
    std::vector<int8_t> vetServeCustomer;

    double sumTransportCost = 0.0;
    double sumEnergyConsum  = 0.0;
    int    sumDemands       = 0;
    int    idSatellite      = -1;

    Satellite(const Instance &instance, int satelliteId);
    Satellite(Satellite &satellite);

    Satellite(const Satellite&)=delete;
    Satellite(Satellite&&)=default;
    Satellite()=default;
    ~Satellite()=default;

    void swap(Satellite &satellite);

    std::string getSatellite(bool cost)=delete;
    void getSatellite(std::string &string, bool cost)=delete;


};

class Truck
{

public:

    std::vector<Index> vetRoute;
    int tamMaxVetRoute              =   -1;                   // Numero de satellites + 2
    int numVetRoute                 =   -1;                   // Numero de possicoes ocupadas em vetRoute

    double sumDistance              =   0.0;
    double sumDemand                =   0.0;

    /*
     * Indica a quantidade de demanda que eh atendida por esse truck
     * Possui tamanho: Depot + Satellite + customers
     * Utiliza o mesmo indice de customrs
     *
     * Depot:      0
     * Satellite: demand(satelliteId)
     * Customrs:  [0,demand(customrId)]
     */
    std::vector<int> vetDemand;

    int tamVetDemand = 0;
    int truckId = -1;

    Truck()=default;
    Truck(Instance &instance, int truckId);
    Truck(Truck &truck);
    Truck(const Truck &truck)=delete;
    ~Truck()=default;

    std::string getTruck(bool cost, bool demand);
    void getTruck(std::string &strTruck, bool cost, bool demand);

    void swap(Truck &truck);

};

class Solution2
{



};

#endif //INC_2E_EVRP_SOLUTION_H