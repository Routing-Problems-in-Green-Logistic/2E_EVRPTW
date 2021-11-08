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

    int numCustomers = 0;
    int NumMax = -1;
    std::vector<Index> vet_route;                                           // Armazena a sequencia de Index (cliente, recharging s., satellite)
    std::vector<float> vet_batteryCapacity;                                 // Armazena a capacidade atual da bateria no cliente

    int     demands       = 0;
    double  transportCost = 0.0;
    double  energyConsum  = 0.0;

    ElectricVehicle(int NumMax_, int SatelliteId);
    ElectricVehicle(ElectricVehicle &electricVehicleAux);
    ~ElectricVehicle()=default;
    ElectricVehicle()=default;

    ElectricVehicle(const ElectricVehicle&)=delete;
    ElectricVehicle(ElectricVehicle&&)=default;

    void swap(ElectricVehicle &electricVehicleAux);
    std::string getRoute(int idSatellite, bool cost) const;
    void getRoute(int idSatellite, std::string &routeStr, bool cost)  const;



};

class Satellite
{

public:

    int NumVhicles;
    std::vector<ElectricVehicle> vetElectricVehicle;

    /*
     *  Indica se o cliente eh atendido por esse satellite.
     *  Possui tamanho: Customer + satellites + Depot
     *
     *  Possicao de customer: 1 ou 0
     *  Possicao de satellite e depot -1
     */
    std::vector<int8_t> vetServeCustomer;

    double sumTransportCost = 0.0;
    double sumEnergyConsum  = 0.0;
    int    sumDemands       = 0;

    Satellite(const Instance &instance, int satelliteId);


};

class Solution2
{



};

#endif //INC_2E_EVRP_SOLUTION_H