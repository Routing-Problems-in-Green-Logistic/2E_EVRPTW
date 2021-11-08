#include "Solution.h"

Solution::Solution() {
    nEvs = 0;
    nTrucks = 0;
}
Solution::Solution(std::vector<std::vector<int>> &routes, int nTrucks, int nEvs, float cost){
    this->routes = routes; // TODO: see if this actually works
    this->nTrucks = nTrucks;
    this->nEvs = nEvs;
    this->cost = cost;
}

int Solution::getNTrucks() const {
    return nTrucks;
}

int Solution::getNEvs() const {
    return nEvs;
}

const std::vector<std::vector<int>>& Solution::getRoutes() const{
    return routes;
}

float Solution::getCost() const {
    return cost;
}

std::vector<std::vector<int>> &Solution::acessRoutes() {
    return this->routes;
}



ElectricVehicle::ElectricVehicle(const int NumMax_, const int SatelliteId) : NumMax(NumMax_)
{
    // Inicializa o veiculo vazio

    numCustomers = 2;

    // Reserva memoria para vet_route e vet_batteryCapacity
    vet_route.reserve(NumMax);
    vet_batteryCapacity.reserve(NumMax);

    // Prenche os dois vetores com NumMax
    for(int i=0; i < NumMax; ++i)
    {
        vet_route.emplace_back(Index(0, -1));
        vet_batteryCapacity.emplace_back(0.0);
    }

    // Escreve o type e id de uma rota vazia com TYPE_SATELLITE e SatelliteId
    vet_route[0].type = TYPE_SATELLITE;
    vet_route[1].type = TYPE_SATELLITE;

    vet_route[0].id = SatelliteId;
    vet_route[1].id = SatelliteId;


}

ElectricVehicle::ElectricVehicle(ElectricVehicle &electricVehicleAux):NumMax(electricVehicleAux.NumMax)
{
    numCustomers  = electricVehicleAux.numCustomers;
    demands       = electricVehicleAux.demands;
    transportCost = electricVehicleAux.transportCost;
    energyConsum  = electricVehicleAux.energyConsum;

    vet_route = electricVehicleAux.vet_route;
    vet_batteryCapacity = electricVehicleAux.vet_batteryCapacity;
}

void ElectricVehicle::swap(ElectricVehicle &electricVehicleAux)
{

    int numCustomersSW = numCustomers;
    numCustomers = electricVehicleAux.numCustomers;
    electricVehicleAux.numCustomers = numCustomersSW;

    int demandsSW = demands;
    demands = electricVehicleAux.demands;
    electricVehicleAux.demands = demandsSW;

    int numMaxSW = NumMax;
    NumMax = electricVehicleAux.NumMax;
    electricVehicleAux.NumMax = numMaxSW;

    double transportCostSW = transportCost;
    transportCost = electricVehicleAux.transportCost;
    electricVehicleAux.transportCost = transportCostSW;

    double energyConsumSW = energyConsum;
    energyConsum = electricVehicleAux.energyConsum;
    electricVehicleAux.energyConsum = energyConsumSW;

    std::swap(vet_route, electricVehicleAux.vet_route);
    std::swap(vet_batteryCapacity, electricVehicleAux.vet_batteryCapacity);

}

std::string ElectricVehicle::getRoute(const int idSatellite, const bool cost) const
{
    std::string route = "SATELLITE ID"+ std::to_string(idSatellite) + ": ";

    for(auto &it:vet_route)
    {
        if(it.type == TYPE_RECHARGING_STATION)
            route += "RS("+ std::to_string(it.id) + ") ";
        else
            route += std::to_string(it.id) + " ";
    }

    if(cost)
        route += "; TransportCost: " + std::to_string(transportCost) + ", EnergyConsum: " + std::to_string(energyConsum) + "; ";

    return route;
}

void ElectricVehicle::getRoute(const int idSatellite, std::string &routeStr, const bool cost) const
{
    routeStr = "SATELLITE ID"+ std::to_string(idSatellite) + ": ";

    for(auto &it:vet_route)
    {
        if(it.type == TYPE_RECHARGING_STATION)
            routeStr += "RS("+ std::to_string(it.id) + ") ";
        else
            routeStr += std::to_string(it.id) + " ";
    }

    if(cost)
        routeStr += "; TransportCost: " + std::to_string(transportCost) + ", EnergyConsum: " + std::to_string(energyConsum) + "; ";

}

Satellite::Satellite(const Instance &instance, const int satelliteId):NumVhicles(instance.getNSats())
{

    vetElectricVehicle.reserve(NumVhicles);

    int numMax = instance.getNClients() + instance.getNrs() + 1;

    for(int i=0; i < NumVhicles; ++i)
    {
        vetElectricVehicle.emplace_back(numMax, satelliteId);
    }


    // Falta adicionar o vetServeCustomer.

}
