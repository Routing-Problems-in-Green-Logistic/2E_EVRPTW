#include "algorithm.h"
#include <set>
#include <map>

bool isFeasibleSolution(Solution& Sol, Instance& Inst){
    float trucksCurrentCap;
    float evCurrentCap, evCurrentBattery;
    std::vector<bool> clientVisited;
    int firstClient = Inst.getFirstClientIndex();
    for(int i = 0; i < Sol.getNTrucks(); i++){ // for each truck route:
        const std::vector<int>& route = Sol.getRoutes()[i]; // TODO: test this const reference;
        if(route[0] != 0 || route[route.size()-1] != 0){ // if the route doesn't start at the depot ("0");
            return false;
        }
        trucksCurrentCap = Inst.getTruckCap();
        for(int s = 0; s < route.size(); s++){ // for each satelite in route
            trucksCurrentCap -= Inst.getDemand(s);
            if(trucksCurrentCap < 0){
                return false;
            }
        }
    }
    for(int i = Sol.getNTrucks(); i < Sol.getNEvs() + Sol.getNTrucks(); i++){ // for each EV route:
        const std::vector<int>& evRoute = Sol.getRoutes()[i];
        if(evRoute[0] != evRoute[evRoute.size()-1]){ // if the route doesn't start at the depot ("0"); // TODO: check if the route starts at a satelite. Currently it only checks if it ends in the same node it starts;
            return false;
        }
        evCurrentCap = Inst.getEvCap();
        evCurrentBattery = Inst.getEvBattery();
        for(int c = 1; c < evRoute.size(); c++){ // for each client in route // whereas c=0 is the satelite;
            clientVisited[evRoute[c] - firstClient] = true;
            evCurrentCap -= Inst.getDemand(evRoute[c]);
            evCurrentBattery -= Inst.getDistance(evRoute[c - 1], evRoute[c]); // TODO: what if the battery coeficient != 1
            if(evCurrentBattery < 0 || evCurrentCap < 0){
                return false;
            }
        }
    }
    for(auto client : clientVisited){ // checks if all clients were visited;
        if(!client){
            return false;
        }
    }
    return true;
}

class Item{
public:
    int node;
    int routeIndex;
    int insertionPosition;
    float capacityCost;
    float batteryCost;
    Item(int node, int routeIndex, int insertionPosition, float capacityCost, float batteryCost):
            node(node), routeIndex(routeIndex), insertionPosition(insertionPosition), capacityCost(capacityCost), batteryCost(batteryCost){}
    bool operator< (const Item& that) const
    {
        if(this->batteryCost == that.batteryCost)
          return (this->capacityCost < that.capacityCost);
        return (this->batteryCost < that.batteryCost);
        return true;
    }
};
Solution* construtivo(Instance& Inst){
    std::vector<std::vector<int>> firstRoutes;
    std::vector<std::vector<int>> secondRoutes;
    secondEchelonRoutes(Inst, secondRoutes);
    firstEchelonRoutes(secondRoutes, Inst, firstRoutes);
    std::vector<std::vector<int>> allRoutes = firstRoutes;
    //TODO: DONT USE the copy operator twice. Prob use pointers to store routes.
    allRoutes.insert(allRoutes.end(), secondRoutes.begin(), secondRoutes.end()); // concat first and second routes arrays.
    auto* Sol = new Solution(allRoutes, (int)firstRoutes.size(), (int)secondRoutes.size());
    return Sol;
}
std::vector<std::vector<int>>& secondEchelonRoutes(Instance& Inst, std::vector<std::vector<int>>& routes){
    auto* Sol = new Solution();
    std::vector<bool> wasVisited(Inst.getNClients(), false);
    //std::vector<Item> reservedList;
    std::set<Item> reservedList;
    std::set<int> unvisitedClients;
    // std::vector<std::vector<int>> routes;
    routes.clear();
    std::vector<float> evCaps;
    std::vector<float> evBatteries;

    // fill unvisitedClients with all unvisited clients // TODO: place this inside the bigger for loop
    for(int i = Inst.getFirstClientIndex(); i < Inst.getFirstClientIndex()+Inst.getNClients(); i++){
        unvisitedClients.insert(i);
    }

    // creates an empty route in each satelite
    for(int i = 1; i < Inst.getNSats() + 1; i++){
        routes.push_back(std::vector<int>{i, i});
        evCaps.push_back(Inst.getEvCap());
        evBatteries.push_back(Inst.getEvBattery());
    }
    int place;
    float cost;

    // for each pair <unvisited client, route>, inserts in reservedList if possible
    for(auto client : unvisitedClients){
        for(int j = 0; j < routes.size(); j++){
            getCheapestInsertionTo(client, routes.at(j), Inst, cost, place);
            if(cost <= evBatteries.at(j) && Inst.getDemand(client) <= evCaps.at(j)){
                reservedList.insert(Item(client, j, place, Inst.getDemand(client), cost));
            }
        }
    }
    while(!unvisitedClients.empty()){
        if(reservedList.empty()){
            return routes;
        }
        auto topItem = reservedList.begin();
        insertInRoute(topItem->node, routes[topItem->routeIndex], topItem->insertionPosition);
        // updadte states ( vehicleCap, vehicleBattery, unvisitedClients...)
        unvisitedClients.erase(topItem->node);
        evCaps.at(topItem->routeIndex) -= topItem->capacityCost;
        evBatteries.at(topItem->routeIndex) -= topItem->batteryCost;
        // if route was a empty route, create another empty route to replace it;
        if(routes[topItem->routeIndex].size() == 3){
            int routeSat = routes[topItem->routeIndex].at(0);
            routes.push_back(std::vector<int>{routeSat, routeSat});
            evBatteries.push_back(Inst.getEvBattery());
            evCaps.push_back(Inst.getEvCap());
        }

        //update RL (Rebuilding actually)
        // reservedList = std::set<Item>();
        reservedList.clear();
        for(auto client : unvisitedClients){
            for(int j = 0; j < routes.size(); j++){
                getCheapestInsertionTo(client, routes.at(j), Inst, cost, place); // TODO: avoid recalculating on some invalid inserts;
                if(cost <= evBatteries.at(j) && Inst.getDemand(client) <= evCaps.at(j)){
                    reservedList.insert(Item(client, j, place, Inst.getDemand(client), cost));
                }
            }
        }
    }
    return routes;
}
void getCheapestInsertionTo(int node, const std::vector<int>& route, const Instance& Inst, float& cost, int& place){
    int bestInsertion = 1;
    float bestCost, insertionCost;
    bestCost = Inst.getDistance(route[0], node) + Inst.getDistance(route[1], node) - Inst.getDistance(route[1], route[0]);

    for(int i = 2; i < route.size()-1; i++){ // as rotas se iniciam e terminam com 0 (satelite);
        insertionCost = Inst.getDistance(route[i], node) + Inst.getDistance(route[i -1], node) - Inst.getDistance(route[i], route[i - 1]);
        if(insertionCost < bestCost){
            bestCost = insertionCost;
            bestInsertion = i;
        }
    }
    place = bestInsertion;
    cost = bestCost;
}
void insertInRoute(int node, std::vector<int>& route, int position){
    // O(n) (!!)
    route.insert(route.begin() + position, node); // inserts node in the middle of the route.

}

std::vector<std::vector<int>> &firstEchelonRoutes(std::vector<std::vector<int>> &firstEchelonRoutes, Instance &Inst,
                                                  std::vector<std::vector<int>> &routes) {
    //std::map<int, float> satDemands;
    std::map<int, float> satDemands; // array with the total demand needed in each Satelite ( #0 is the Inst->getFirstSat)
    for(int i = Inst.getFirstSatIndex(); i < Inst.getFirstSatIndex() + Inst.getNSats(); i++){
        satDemands.insert(std::pair<int,float>(i, 0));
    }
    std::set<int> unvisitedSats;
    // std::vector<std::vector<int>> routes;
    routes.clear();
    std::set<Item> reservedList;
    std::vector<float> trucksCap(1, Inst.getTruckCap()); // trucks capacities; Starts with "one" truck in the depot
    float aux_demand;
    for(const auto& route : firstEchelonRoutes){ // sums the demands in each
        aux_demand = 0;
        for(const auto node : route){
            aux_demand += Inst.getDemand(node);
        }
        satDemands.at(route.at(0)) += aux_demand; // adds the route total demand to the satelite position of the sat. demand array
    }
    for(int i = Inst.getFirstSatIndex(); i < Inst.getFirstSatIndex() + Inst.getNSats(); i++){
        if(satDemands.at(i) == 0){ // if no demand for sat, this sat doesnt need to be visited
            continue;
        }
        unvisitedSats.insert(i);
    }
    routes.push_back(std::vector<int>{0, 0}); //creates an empty route at the depot;
    int place; float cost;
    for(const auto sat : unvisitedSats){
        for(int i = 0; i < routes.size(); i++){
            if(trucksCap.at(i) == 0){
                continue;
            }
            getCheapestInsertionTo(sat, routes.at(i), Inst, cost, place);
            reservedList.insert(Item(sat, i, place, Inst.getDemand(sat), 0)); // batteryCost = 0 cus first echelon is with ICVs
        }
    }
    while(!unvisitedSats.empty()){
        if(reservedList.empty()){
            std::cerr << "Infeasible solution, " << std::endl; // TODO: inform user infeasible solution
            return routes;
        }

        auto topItem = reservedList.begin();
        insertInRoute(topItem->node, routes[topItem->routeIndex], topItem->insertionPosition);
        if(satDemands.at(topItem->node) > trucksCap.at(topItem->routeIndex)){
            satDemands.at(topItem->node) -= trucksCap.at(topItem->routeIndex);
            trucksCap.at(topItem->routeIndex) = 0;
        } else { // trucksCap >= satDemands, so after visit sat is empty, so erase it from the unvisitedSats list
            trucksCap.at(topItem->routeIndex) -= satDemands.at(topItem->node);
            satDemands.at(topItem->node) = 0;
            unvisitedSats.erase(topItem->node);
        }
        if(routes.at(topItem->routeIndex).size() == 3){
            routes.push_back(std::vector<int>{0,0});
            trucksCap.push_back(Inst.getTruckCap());
        }
        reservedList.clear();
        for(const auto sat : unvisitedSats){
            for(int i = 0; i < routes.size(); i++){
                if(trucksCap.at(i) == 0){
                    continue;
                }
                getCheapestInsertionTo(sat, routes.at(i), Inst, cost, place);
                reservedList.insert(Item(sat, i, place, Inst.getDemand(sat), 0)); // batteryCost = 0 cus first echelon is with ICVs
            }
        }
    }
    return routes;
}
