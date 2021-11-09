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



ElectricVehicle::ElectricVehicle(const int numCustomers, const int numRechargingS, const int SatelliteId)
{
    // Inicializa o veiculo vazio
    numAttendances = 2;

    numAttendances = 2 + numCustomers + numRechargingS;

    idSatellite = SatelliteId;

    // Reserva memoria para vet_route e vet_batteryCapacity
    vet_route.reserve(NumMaxAttendances);
    vet_batteryCapacity.reserve(NumMaxAttendances);

    // Prenche os dois vetores com NumMaxAttendances
    for(int i=0; i < NumMaxAttendances; ++i)
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

ElectricVehicle::ElectricVehicle(ElectricVehicle &electricVehicleAux): NumMaxAttendances(electricVehicleAux.NumMaxAttendances)
{
    numAttendances  = electricVehicleAux.numAttendances;
    demands       = electricVehicleAux.demands;
    transportCost = electricVehicleAux.transportCost;
    energyConsum  = electricVehicleAux.energyConsum;

    vet_route = electricVehicleAux.vet_route;
    vet_batteryCapacity = electricVehicleAux.vet_batteryCapacity;
}

void ElectricVehicle::swap(ElectricVehicle &electricVehicleAux)
{

    int numCustomersSW = numAttendances;
    numAttendances = electricVehicleAux.numAttendances;
    electricVehicleAux.numAttendances = numCustomersSW;

    int demandsSW = demands;
    demands = electricVehicleAux.demands;
    electricVehicleAux.demands = demandsSW;

    int numMaxSW = NumMaxAttendances;
    NumMaxAttendances = electricVehicleAux.NumMaxAttendances;
    electricVehicleAux.NumMaxAttendances = numMaxSW;

    double transportCostSW = transportCost;
    transportCost = electricVehicleAux.transportCost;
    electricVehicleAux.transportCost = transportCostSW;

    double energyConsumSW = energyConsum;
    energyConsum = electricVehicleAux.energyConsum;
    electricVehicleAux.energyConsum = energyConsumSW;

    std::swap(vet_route, electricVehicleAux.vet_route);
    std::swap(vet_batteryCapacity, electricVehicleAux.vet_batteryCapacity);

}

std::string ElectricVehicle::getRoute(const bool cost) const
{
    std::string route = "SATELLITE ID"+ std::to_string(idSatellite) + ": ";

    for(auto &it:vet_route)
    {
        if(it.type == TYPE_RECHARGING_STATION)
            route += "RECHARGING_S.("+ std::to_string(it.id) + ") ";
        else
            route += std::to_string(it.id) + " ";
    }

    if(cost)
        route += "; TransportCost: " + std::to_string(transportCost) + ", EnergyConsum: " + std::to_string(energyConsum) + "; ";

    return route;
}

void ElectricVehicle::getRoute(std::string &routeStr, const bool cost) const
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

Satellite::Satellite(const Instance &instance, const int satelliteId):numMaxVhicles(instance.getNSats()), idSatellite(satelliteId)
{

    vetElectricVehicle.reserve(numMaxVhicles);

    for(int i=0; i < numMaxVhicles; ++i)
    {
        vetElectricVehicle.emplace_back(instance.getNClients(), instance.getNumRechargingS(), satelliteId);
    }

    const int TamVetServeCustomers = instance.getNClients() + instance.getNSats() + instance.getNumRechargingS() + 1;

    vetServeCustomer.reserve(TamVetServeCustomers);

    // A diciona deposito
    vetServeCustomer.emplace_back(-1);

    // Adiciona satellite
    for(int i=1; i <= instance.getLastSatIndex(); ++i)
        vetServeCustomer.emplace_back(-1);

    // Adiciona cliente
    for(int i=instance.getFirstClientIndex(); i <= instance.getLastClienteIndex(); ++i)
        vetServeCustomer.emplace_back(0);

    // Adiciona RechargingStation
    for(int i=instance.getFirstRechargingStationIndex(); i <= instance.getLastClienteIndex(); ++i)
        vetServeCustomer.emplace_back(0);



}

Satellite::Satellite(Satellite &satellite)
{
    numMaxVhicles   = satellite.numMaxVhicles;
    numVhicles      = satellite.numVhicles;

    vetServeCustomer = satellite.vetServeCustomer;
    vetElectricVehicle.reserve(numMaxVhicles);

    for(int i=0; i < numMaxVhicles; ++i)
        vetElectricVehicle.emplace_back(satellite.vetElectricVehicle[i]);

    sumDemands          = satellite.sumDemands;
    sumEnergyConsum     = satellite.sumEnergyConsum;
    sumTransportCost    = satellite.sumTransportCost;
}

void Satellite::swap(Satellite &satellite)
{

    std::swap(satellite.numMaxVhicles, numMaxVhicles);
    std::swap(satellite.numVhicles, numVhicles);

    std::swap(satellite.sumDemands, sumDemands);
    std::swap(satellite.sumTransportCost, sumTransportCost);
    std::swap(satellite.sumEnergyConsum, sumEnergyConsum);

    std::swap(satellite.vetServeCustomer, vetServeCustomer);
    std::swap(satellite.vetElectricVehicle, vetElectricVehicle);

}


Truck::Truck(Instance &instance, const int _truckId): truckId(_truckId)
{
    // Inicializa o truck vazio

    tamMaxVetRoute = 2 + instance.getNSats();
    numVetRoute    = 2;

    vetRoute.reserve(tamMaxVetRoute);

    for(int i=0; i < tamMaxVetRoute; ++i)
        vetRoute.emplace_back();

    // Inicio e final da rota eh composta pelo deposito

    vetRoute[0].id   = vetRoute[1].id   = 0;
    vetRoute[0].type = vetRoute[1].type = TYPE_DEPOT;

    // Calcula o tamaho do vetor como numero de satellites e de clientes

    tamVetDemand = 1 + instance.getNSats() + instance.getNClients();
    vetDemand.reserve(tamVetDemand);

    // Inicializa cada possicao com 0

    for(int i=0; i < tamVetDemand; ++i)
        vetDemand.emplace_back(0);


}

Truck::Truck(Truck &truck)
{

    tamMaxVetRoute  = truck.tamMaxVetRoute;
    numVetRoute     = truck.numVetRoute;
    vetRoute        = truck.vetRoute;

    sumDemand       = truck.sumDemand;
    sumDistance     = truck.sumDistance;
    truckId         = truck.truckId;

    tamVetDemand    = truck.tamVetDemand;
    vetDemand       = truck.vetDemand;
}

void Truck::swap(Truck &truck)
{

    std::swap(tamMaxVetRoute, truck.tamMaxVetRoute);
    std::swap(numVetRoute, truck.numVetRoute);
    std::swap(vetRoute, truck.vetRoute);

    std::swap(sumDemand, truck.sumDemand);
    std::swap(sumDistance, truck.sumDistance);
    std::swap(truckId, truck.truckId);

    std::swap(tamVetDemand, truck.tamVetDemand);
    std::swap(vetDemand, truck.vetDemand);

}

std::string Truck::getTruck(const bool cost, const bool demand)
{
    std::string strTruck;

    strTruck += "TRUCK ID: " + std::to_string(truckId) + ":";

    if(demand)
        strTruck += "\n\t";

    for(auto &it:vetRoute)
        strTruck += " " + std::to_string(it.id);

    if(demand)
    {
        strTruck += "\n\tDEMAND: ";

        for(auto it:vetDemand)
            strTruck += std::to_string(it) + " ";

        strTruck += "\n\t";
    }

    if(cost)
        strTruck += "DISTANCE: " + std::to_string(sumDistance) + "; DEMAND: " + std::to_string(sumDemand) + "\n";

    return strTruck;

}

void Truck::getTruck(std::string &strTruck, const bool cost, const bool demand)
{

    strTruck += "TRUCK ID: " + std::to_string(truckId) + ":";

    if(demand)
        strTruck += "\n\t";

    for(auto &it:vetRoute)
        strTruck += " " + std::to_string(it.id);

    if(demand)
    {
        strTruck += "\n\tDEMAND: ";

        for(auto it:vetDemand)
            strTruck += std::to_string(it) + " ";

        strTruck += "\n\t";
    }

    if(cost)
        strTruck += "DISTANCE: " + std::to_string(sumDistance) + "; DEMAND: " + std::to_string(sumDemand) + "\n";
}

