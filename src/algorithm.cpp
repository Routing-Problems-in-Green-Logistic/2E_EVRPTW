#include "algorithm.h"
#include <set>
#include <map>
#include <numeric>
#include <stdexcept>

bool isFeasibleSolution(Solution& Sol, const Instance& Inst){
    float trucksCurrentCap;
    float evCurrentCap, evCurrentBattery;
    std::vector<bool> clientVisited(Inst.getNClients(), false);
    int firstClient = Inst.getFirstClientIndex();
    for(int i = 0; i < Sol.getNTrucks(); i++){ // for each truck route:
        const std::vector<int>& route = Sol.getRoutes()[i]; // TODO: test this const reference;
        if(route[0] != 0 || route[route.size()-1] != 0){ // if the route doesn't start at the depot ("0");
            return false;
        }
        trucksCurrentCap = Inst.getTruckCap();trucksCurrentCap = (float)200;
        for(int s = 1; s < route.size()-1; s++){ // for each satelite in route
            trucksCurrentCap -= Inst.getDemand(route.at(s));
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
        for(int c = 1; c < evRoute.size()-1; c++){ // for each client in route // whereas c=0 is the satelite;
            if(Inst.isRechargingStation(c-1) && Inst.isRechargingStation(c)){
                return false;
            }
            if(evRoute.at(c) < Inst.getFirstRsIndex() && clientVisited.at(evRoute[c] - firstClient)){
                return false;
            }
            evCurrentCap -= Inst.getDemand(evRoute[c]);
            evCurrentBattery -= Inst.getDistance(evRoute[c - 1], evRoute[c]); // TODO: what if the battery coeficient != 1
            if(evCurrentBattery < 0 || evCurrentCap < 0){
                return false;
            }
            int frs = Inst.getFirstRsIndex();
            if(evRoute.at(c) >= Inst.getFirstRsIndex()){
                evCurrentBattery = Inst.getEvBattery();
                int a = 0;
            } else {
                clientVisited[evRoute[c] - firstClient] = true;
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

Solution* construtivo(const Instance& Inst, std::vector<std::vector<int>>& secondRoutes){
    float totalCost = 0;
    std::vector<std::vector<int>> firstRoutes;
    // std::vector<std::vector<int>> secondRoutes;
    secondEchelonRoutes(Inst, secondRoutes, totalCost);
    firstEchelonRoutes(secondRoutes, Inst, firstRoutes, totalCost);
    //TODO: DONT USE the copy operator twice. Prob use pointers to store routes.
    /*
    for(int i = 0; i < firstRoutes.size(); i++){
        if(firstRoutes.at(i).size() == 2){
            firstRoutes.erase(firstRoutes.begin() + i);
            i--;
        }
    }
    for(int i = 0; i < secondRoutes.size(); i++){
        if(secondRoutes.at(i).size() == 2){
            secondRoutes.erase(secondRoutes.begin() + i);
            i--;
        }
    }
     */
    // firstRoutes.clear();
    std::vector<std::vector<int>> allRoutes;
    int nFirstRoutes = 0, nSecondRoutes = 0;
    for(const auto & firstRoute : firstRoutes){
        if(firstRoute.size() > 2){
            allRoutes.push_back(firstRoute);
            nFirstRoutes++;
        }
    }
    for(const auto & secondRoute : secondRoutes){
        if(secondRoute.size() > 2){
            allRoutes.push_back(secondRoute);
            nSecondRoutes++;
        }
    }
    //std::vector<std::vector<int>> allRoutes = firstRoutes;
    // allRoutes.insert(allRoutes.end(), secondRoutes.begin(), secondRoutes.end()); // concat first and second routes arrays.
        // secondRoutes.clear();
    //auto* Sol = new Solution(allRoutes, (int)firstRoutes.size(), (int)secondRoutes.size(), totalCost);
    auto* Sol = new Solution(allRoutes, nFirstRoutes, nSecondRoutes, totalCost);
    return Sol;
}
std::vector<std::vector<int>>& secondEchelonRoutes(const Instance& Inst, std::vector<std::vector<int>>& routes, float& totalCost){

    totalCost = 0;
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
    float cost, solCost;
    int rs = -1;
    while(!unvisitedClients.empty()){
        reservedList.clear();
        for(auto client : unvisitedClients){
            for(int j = 0; j < routes.size(); j++){
                //getCheapestInsertionTo(client, routes.at(j), Inst, cost, place); // TODO: avoid recalculating on some invalid inserts;
                Item item;
                item.routeIndex = j;
                bool canSafeInsert = getSafeInsertionIn(routes.at(j), client, Inst, item);
                //bool canSafeInsert = getCheapestSafeInsertionIn(routes.at(j), client, Inst, cost, place, rs, solCost);
                if(Inst.getDemand(client) > evCaps.at(j)) {
                    continue;
                }
                //if(cost <= evBatteries.at(j)){
                if(canSafeInsert && rs == -1) {
                    // reservedList.insert(Item(client, j, place, Inst.getDemand(client), cost, solCost));
                    reservedList.insert(item);
                }
                else if (canSafeInsert && rs != -1) {
                    reservedList.insert(item);
                }
                    // TODO: create a separated function to do this (finding the best recharging station to go before going to the client.
                //}
            }
        }
        bool inserted = false;
        if(reservedList.empty()){
            for(auto& client : unvisitedClients) {
                for (auto &route: routes) {
                    if (route.size() == 2) {
                        for (int rs = Inst.getFirstRsIndex(); rs < Inst.getFirstRsIndex() + Inst.getNrs(); rs++) {
                            if(insertInNewRoute(route, client, Inst, -1, rs)){
                                unvisitedClients.erase(client);
                                inserted = true;
                                int routeSat = route.at(0);
                                routes.push_back(std::vector<int>{routeSat, routeSat});
                                evBatteries.push_back(Inst.getEvBattery());
                                evCaps.push_back(Inst.getEvCap());
                                totalCost += Inst.getEvCost();
                                break;
                            }
                            for(int rs2 = Inst.getFirstRsIndex(); rs2 < Inst.getFirstRsIndex() + Inst.getNrs(); rs2++){
                                if(insertInNewRoute(route, client, Inst, rs, rs2)){
                                    unvisitedClients.erase(client);
                                    inserted = true;
                                    int routeSat = route.at(0);
                                    routes.push_back(std::vector<int>{routeSat, routeSat});
                                    evBatteries.push_back(Inst.getEvBattery());
                                    evCaps.push_back(Inst.getEvCap());
                                    totalCost += Inst.getEvCost();
                                    break;
                                }

                            }
                            if(inserted){
                                break;
                            }
                        }
                        if(inserted){
                            break;
                        }
                    }
                }
                if(inserted){
                    break;
                }
            }
            if(!inserted){
                return routes;
            }
            continue;
        }
        // float alpha = 0.01;
        int randIndex = rand()%((int)(0.3*(int)reservedList.size() + 1));
        auto topItem = std::next(reservedList.begin(), randIndex); // gets the randomIndex element in the set //O(n)
        // auto topItem = reservedList.begin();
        auto& route = routes.at(topItem->routeIndex);
        insertInRoute(topItem->node, route, topItem->insertionPosition);
        totalCost += topItem->solutionCost;
        if(topItem->rs != -1){
            insertInRoute(topItem->rs, route, topItem->insertionPosition); // inserts recharging station before the client;
            evBatteries.at(topItem->routeIndex) = Inst.getEvBattery();
        }
        // updadte states ( vehicleCap, vehicleBattery, unvisitedClients...)
        unvisitedClients.erase(topItem->node);
        evCaps.at(topItem->routeIndex) -= topItem->capacityCost;
        evBatteries.at(topItem->routeIndex) -= topItem->batteryCost;
        // if route was a empty route, create another empty route to replace it;
        if(route.size() == 3 || (route.size() == 4 && Inst.isRechargingStation(route.at(1)))){
            int routeSat = route.at(0);
            routes.push_back(std::vector<int>{routeSat, routeSat});
            evBatteries.push_back(Inst.getEvBattery());
            evCaps.push_back(Inst.getEvCap());
            totalCost += Inst.getEvCost();
        }

        //update RL (Rebuilding actually)
        // reservedList = std::set<Item>();
    }
    return routes;
}
void getCheapestInsertionTo(int node, const std::vector<int>& route, const Instance& Inst, float& cost, int& place){
    int bestInsertion = -1;
    float bestCost, insertionCost;
    //bestCost = Inst.getDistance(route[0], node) + Inst.getDistance(route[1], node) - Inst.getDistance(route[1], route[0]);
    bestCost = 1e8;

    for(int i = 1; i < route.size(); i++){ // as rotas se iniciam e terminam com 0 (satelite);
        insertionCost = Inst.getDistance(route.at(i), node) + Inst.getDistance(route.at(i -1), node) - Inst.getDistance(route.at(i), route.at(i - 1));
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

std::vector<std::vector<int>>& firstEchelonRoutes(std::vector<std::vector<int>> &firstEchelonRoutes, const Instance &Inst,
                                                  std::vector<std::vector<int>> &routes, float totalCost) {
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
            reservedList.insert(Item(sat, i, place, Inst.getDemand(sat), 0, cost)); // batteryCost = 0 cus first echelon is with ICVs
        }
    }
    while(!unvisitedSats.empty()){
        if(reservedList.empty()){
            std::cerr << "Infeasible solution, " << std::endl; // TODO: inform user infeasible solution
            return routes;
        }

        auto topItem = reservedList.begin();
        insertInRoute(topItem->node, routes.at(topItem->routeIndex), topItem->insertionPosition);
        totalCost += topItem->solutionCost;
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
            totalCost += Inst.getTruckCost();
        }
        reservedList.clear();
        for(const auto sat : unvisitedSats){
            for(int i = 0; i < routes.size(); i++){
                if(trucksCap.at(i) == 0){
                    continue;
                }
                getCheapestInsertionTo(sat, routes.at(i), Inst, cost, place);
                reservedList.insert(Item(sat, i, place, Inst.getDemand(sat), 0, cost)); // batteryCost = 0 cus first echelon is with ICVs
            }
        }
    }
    return routes;
}

float getRouteCost(const std::vector<int>& route, const Instance &Inst) {
    float cost = 0;
    int node;
    int lastNode = route.at(0);
    for(int i = 1; i < route.size(); i++){
        node = route.at(i);
        cost += Inst.getDistance(lastNode, node);
        lastNode = node;
    }
    return cost;
}

float getRouteDemand(const std::vector<int> &route, const Instance &Inst) {
    float totalDemand = 0;
    for(int node : route){
        totalDemand += Inst.getDemand(node);
    }
    return totalDemand;
}

bool getCheapestSafeInsertionIn(const std::vector<int> &route, int nodeIndex, const Instance &Inst, float &evCost, int &place, int& rs, float& finalSolCot) { // rs = recharging station
    int bestNode = -1, bestRs = -1;
    int finalRs = -1;
    float bestCost, insertionCost, solCost, bestSolCost;
    int currentRechargingStation = -1;
    solCost = 1e8; // TODO: find better way than using BIG number
    // float evBattery = Inst.getEvBattery();
    float evCapacity = Inst.getEvCap();
    std::vector<std::pair<int, float>> rechargesInRoute;
    std::vector<float> evBattery(route.size());
    float routeDemand = getRouteDemand(route, Inst); // O(n) // TODO: get route demand inside another for loop; (maybe not, because 30 + 30 = 30*2).
    if(Inst.getDemand(nodeIndex) + routeDemand > evCapacity){
        return false;
    }
    float comulativeBattery = Inst.getEvBattery(); // TODO: fix english, 'comulative'
    std::vector<std::pair<int, float>> recharges;
    for(int i = 1; i < route.size(); i++){
        float dist = Inst.getDistance(route.at(i), route.at(i - 1));
        comulativeBattery -= dist;
        if(Inst.isRechargingStation(route.at(i)) || Inst.isSatelite(route.at(i))){
            recharges.emplace_back(route.at(i), comulativeBattery);
            comulativeBattery = Inst.getEvBattery();
        }
    }
    //recharges.emplace_back(*route.end(), Inst.getEvBattery());
    float batteryWaste = recharges.at(0).second; //= recharges.at(route.at(0));
    for(int i = 0; i < route.size(); i++){
        // auto it = recharges.find({route.at(i), });
        for(int j = 0; j < recharges.size() - 1; j++) {
            auto& station = recharges.at(j);
            if (route.at(i) == station.first){
                //batteryWaste = station.second;
                batteryWaste = recharges.at(j+1).second;
            }
        }
        evBattery.at(i) = batteryWaste;
    }

    for(int i = 1; i < route.size(); i++){
        int node = route.at(i);
        insertionCost = Inst.getDistance(route.at(i), nodeIndex) + Inst.getDistance(route.at(i -1), nodeIndex) - Inst.getDistance(route.at(i), route.at(i - 1));
        solCost = insertionCost;
        if(route.size() == 2){
            solCost += Inst.getEvCost();
        }
        currentRechargingStation = -1;
        if(insertionCost > evBattery.at(i-1)){
            bestRs = -1;
            float finalBatteryCost;
            //float rsCost = Inst.getDistance(place-1, Inst.getFirstRsIndex()); // TODO: handle instances without recharging stations
            //float bestTotalCost = -1;
            float rsCost = 1e8; // TODO: be lazier
            // float rsCost = Inst.getDistance(routes.at(j).at(place-1), Inst.getFirstRsIndex()) + Inst.getDistance(Inst.getFirstRsIndex(), client);
            for(int r = Inst.getFirstRsIndex(); r < Inst.getFirstRsIndex() + Inst.getNrs(); r++){
                float toRsCost = Inst.getDistance(route.at(i-1), r);
                int client = nodeIndex;
                float toClientCost = Inst.getDistance(r, client);
                float clientToRouteCost = Inst.getDistance(client, i);
                float totalCost = toRsCost + toClientCost + clientToRouteCost;
                if(route.size() == 2){
                    totalCost += Inst.getEvCost();
                }
                if(toRsCost <= evBattery.at(i-1) && toClientCost <= Inst.getEvBattery() &&  totalCost < rsCost){
                    bestRs = r;
                    rsCost = totalCost;
                    finalBatteryCost = toClientCost + clientToRouteCost;
                }
            }
            if(bestRs == -1){
                continue;
            }
            currentRechargingStation = bestRs;
            insertionCost = finalBatteryCost;
            solCost = rsCost;
        }

        if(solCost < bestSolCost){
            bestCost = insertionCost;
            bestNode = i;
            finalRs = currentRechargingStation;
        }
    }
    place = bestNode;
    evCost = bestCost;
    finalSolCot = bestSolCost;
    rs = finalRs;
    if(bestNode == -1){
        return false;
    }
    return true;
}
bool isFeasibleRoute(const std::vector<int>& route, const Instance& Inst){
    float evCap = Inst.getEvCap();
    float evBat = Inst.getEvBattery();
    for(int i = 1; i < route.size(); i++){
        int node = route.at(i);
        int prevNode = route.at(i-1);
        evCap -= Inst.getDemand(node);
        evBat -= Inst.getDistance(node, prevNode);
        if(evBat < 0 || evCap < 0){
            return false;
        }
        if(Inst.isRechargingStation(node)){
            evBat = Inst.getEvBattery();
        }
    }
    return true;
}
//bool getSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, float& finalEvCost, int& finalPlace, int& finalRs, float& finalSolCost){
bool getSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, Item& item){
    float routeDemand = 0, evCapacity;
    evCapacity = Inst.getEvCap();
    std::vector<float> avaliableBattery(route.size());
    std::vector<std::pair<int, float>> recharges; // {recharge_station_index, battery before recharge}
    item.insertionPosition = -1;
    item.rs = -1;

    for(int i = 1; i < route.size(); i++){
        //routeDemand += Inst.getDistance(route.at(i - 1), route.at(i));
        routeDemand += Inst.getDemand(route.at(i));
    }
    // checks if no capacity
    if(Inst.getDemand(nodeIndex) + routeDemand > evCapacity){
        return false;
    }
    float avBattery = Inst.getEvBattery();
    for(int i = 1; i < route.size(); i++){
        int node = route.at(i);
        int prevNode = route.at(i-1);
        float dist = Inst.getDistance(prevNode, node);;;
        avBattery -= dist;
        if(Inst.isRechargingStation(route.at(i)) || Inst.isSatelite(route.at(i))){
            recharges.emplace_back(i, avBattery);
            avBattery = Inst.getEvBattery();
        }
    }
    //const auto& nextStation = recharges.at(0);
    int nextStation = 0;
    for(int i = 0; i < route.size(); i++){
        int node = route.at(i);
        avaliableBattery.at(i) = recharges.at(nextStation).second;
        if(Inst.isRechargingStation(node) || (Inst.isSatelite(node) && i != 0)){
            nextStation++;
        }
    }

    float insertionCost, bestCost;
    int bestRs = -1;// errrrrrr
    float bestBatteryCost, batteryCost;
    bestCost = 1e8;
    for(int i = 1; i < route.size(); i++){
        int nextNode = route.at(i);
        int prevNode = route.at(i-1);
        insertionCost = Inst.getDistance(prevNode, nodeIndex) + Inst.getDistance(nodeIndex, nextNode) - Inst.getDistance(prevNode, nextNode);
        batteryCost = insertionCost;

        if(batteryCost > avaliableBattery.at(i)){ // se n acaba a bateria
            float bestRsCost = 1e8; // the solution cost;
            // checks for each Recharging station if by visiting it first than the client, can the EV complete the tour w/out emptying its battery
            for(int r = Inst.getFirstRsIndex(); r < Inst.getFirstRsIndex() + Inst.getNrs(); r++){
                float toRsCost = Inst.getDistance(prevNode, r);
                float clientVisitCost = Inst.getDistance(r, nodeIndex) + Inst.getDistance(nodeIndex, nextNode);
                float rsCost = toRsCost + clientVisitCost;
                // prevNode -> RS -> node -> nextNode

                if(toRsCost > avaliableBattery.at(i) || clientVisitCost > Inst.getEvBattery()){
                    continue;
                }
                if(rsCost < bestRsCost){
                    bestRsCost = rsCost;
                    batteryCost = clientVisitCost;
                    insertionCost = rsCost;
                    bestRs = r;
                }
            }
        }
        else{
            bestRs = -1;
            item.rs = -1;
        }
        if(insertionCost < bestCost && batteryCost <= avaliableBattery.at(i)){
            bestCost = insertionCost;
            // item.routeIndex = -1;
            item.insertionPosition = i;
            item.node = nodeIndex;
            item.solutionCost = bestCost;
            item.rs = bestRs;
            //item.batteryCost = bestBatteryCost;
            item.batteryCost = batteryCost;
            item.capacityCost =  Inst.getDemand(nodeIndex);
            // finalRs = bestRs;
            // finalPlace = i;
            // finalEvCost = batteryCost;
            // finalSolCost = bestCost;
        }
    }
    if(item.insertionPosition == -1){
        return false;
    }
    return true;
}

float getSolCost(const Solution &Sol, const Instance &Inst) {
    const std::vector<std::vector<int>>& routes = Sol.getRoutes();
    float cost = 0;
    for(const auto& route : routes){
        if(Inst.isSatelite(route.at(0))){
            cost += Inst.getEvCost();
        } else{
            cost += Inst.getTruckCost();
        }
        for(int i = 1; i < route.size(); i++){
            cost += Inst.getDistance(route.at(i-1), route.at(i));
        }
    }
    return cost;
}
bool getExpensiveSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, Item& item){
    float routeDemand = 0, evCapacity;
    evCapacity = Inst.getEvCap();
    std::vector<float> avaliableBattery(route.size());
    std::vector<std::pair<int, float>> recharges; // {recharge_station_index, battery before recharge}
    item.insertionPosition = -1;
    item.rs = -1;

    for(int i = 1; i < route.size(); i++){
        routeDemand += Inst.getDistance(route.at(i - 1), route.at(i));
    }
    // checks if no capacity
    if(Inst.getDemand(nodeIndex) + routeDemand > evCapacity){
        return false;
    }
    float avBattery = Inst.getEvBattery();
    for(int i = 1; i < route.size(); i++){
        int node = route.at(i);
        int prevNode = route.at(i-1);
        float dist = Inst.getDistance(prevNode, node);;;
        avBattery -= dist;
        if(Inst.isRechargingStation(route.at(i)) || Inst.isSatelite(route.at(i))){
            recharges.emplace_back(i, avBattery);
            avBattery = Inst.getEvBattery();
        }
    }
    //const auto& nextStation = recharges.at(0);
    int nextStation = 0;
    for(int i = 0; i < route.size(); i++){
        int node = route.at(i);
        avaliableBattery.at(i) = recharges.at(nextStation).second;
        if(Inst.isRechargingStation(node) || (Inst.isSatelite(node) && i != 0)){
            nextStation++;
        }
    }

    float insertionCost, bestCost;
    int bestRs = -1;// errrrrrr
    float bestBatteryCost, batteryCost;
    bestCost = 1e8;
    for(int i = 1; i < route.size(); i++){
        int nextNode = route.at(i);
        int prevNode = route.at(i-1);
        insertionCost = Inst.getDistance(prevNode, nodeIndex) + Inst.getDistance(nodeIndex, nextNode) - Inst.getDistance(prevNode, nextNode);
        batteryCost = insertionCost;

        if(batteryCost > avaliableBattery.at(i)){ // se n acaba a bateria
            float bestRsCost = 1e8; // the solution cost;
            // checks for each Recharging station if by visiting it first than the client, can the EV complete the tour w/out emptying its battery
            for(int r = Inst.getFirstRsIndex(); r < Inst.getFirstRsIndex() + Inst.getNrs(); r++){
                float toRsCost = Inst.getDistance(prevNode, r);
                float clientVisitCost = Inst.getDistance(r, nodeIndex) + Inst.getDistance(nodeIndex, nextNode);
                float rsCost = toRsCost + clientVisitCost;
                // prevNode -> RS -> node -> nextNode

                if(toRsCost > avaliableBattery.at(i) || clientVisitCost > Inst.getEvBattery()){
                    continue;
                }
                if(rsCost < bestRsCost){
                    bestRsCost = rsCost;
                    batteryCost = clientVisitCost;
                    insertionCost = rsCost;
                    bestRs = r;
                }
            }
        }
        else{
            bestRs = -1;
            item.rs = -1;
        }
        if(insertionCost < bestCost && batteryCost <= avaliableBattery.at(i)){
            bestCost = insertionCost;
            // item.routeIndex = -1;
            item.insertionPosition = i;
            item.node = nodeIndex;
            item.solutionCost = bestCost;
            item.rs = bestRs;
            //item.batteryCost = bestBatteryCost;
            item.batteryCost = batteryCost;
            item.capacityCost =  Inst.getDemand(nodeIndex);
            // finalRs = bestRs;
            // finalPlace = i;
            // finalEvCost = batteryCost;
            // finalSolCost = bestCost;
        }
    }
    if(item.insertionPosition == -1){
        return false;
    }
    return true;
}
bool insertInNewRoute(std::vector<int>& route, int nodeIndex, const Instance& Inst, int rs1, int rs2){
    int sat = route.at(0);
    insertInRoute(nodeIndex, route, 1);
    if(rs1 != -1){
        insertInRoute(rs1, route, 1);
    }
    if(rs2 != -1){
        insertInRoute(rs2, route, route.size()-1);
    }
    if(isFeasibleRoute(route, Inst)){
        return true;
    }
    route = {sat, sat};
    return false;
}

void removeEmptyRoutes(std::vector<std::vector<int>> &routes) {
    std::erase_if(routes, [](const std::vector<int>& route){ return route.size() <= 2;});
}
void removeFromRoute(std::vector<int>& route, int i){
    if(i >= route.size()){
        throw std::out_of_range("out of range!");
    }
    route.erase(route.begin() + i);
}

void printRoute(const std::vector<int> &route) {
    for(auto node : route) {
        std::cout << node << ", ";
    }
    std::cout << std::endl;

}
