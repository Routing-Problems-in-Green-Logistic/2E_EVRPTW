//
// Created by abrolhus on 27/08/2021.
//

#include "localsearches.h"

void ls::intra2opt(std::vector<std::vector<int>> &routes, const Instance &Inst, std::vector<std::pair<float,float>>& costs) {
    //if(costs.empty())
    costs.clear(); // TODO: only clear and fill costs if its empty;
    for(int i = 0; i < routes.size(); i++){
        float currentCost;
        std::vector<int>& route = routes.at(i);
        float bestRouteCost = getRouteCost(route, Inst);
        costs.emplace_back(bestRouteCost, bestRouteCost);
        bool improvement = true;
        while(improvement) {
            improvement = false;
            for (int j = 1; j < route.size()-1; j++) {
                for (int k = j + 1; k < route.size()-1; k++) {
                    // std::vector<int> cpyRoute = vet_route;
                    intra2optMov(route, j, k);
                    currentCost = getRouteCost(route, Inst);
                    if (currentCost < bestRouteCost && isFeasibleRoute(route, Inst)) {
                        std::cout << "before: " << bestRouteCost << ",  ";
                        bestRouteCost = currentCost;
                        improvement = true;
                        costs.at(i).second = bestRouteCost; // atualiza o vetor de custos
                        std::cout << "after: "<< bestRouteCost << ",  " << std::endl;
                    } else {
                        intra2optMov(route, j, k); // REVERTS 2OPT (aplying it again, but backwards)
                        //improvement = false;
                    }
                }
            }
        }
    }
    std::cout << "wait.";
}
void ls::intraReverseRoute(std::vector<std::vector<int>> &routes, const Instance &Inst, std::vector<std::pair<float,float>>& costs) {
    //if(costs.empty())
    costs.clear(); // TODO: only clear and fill costs if its empty;
    for(int i = 0; i < routes.size(); i++){
        float currentCost;
        std::vector<int>& route = routes.at(i);
        float bestRouteCost = getRouteCost(route, Inst);
        costs.emplace_back(bestRouteCost, bestRouteCost);
        bool improvement = true;
        while(improvement) {
            for (int j = 1; j < route.size()-1; j++) {
                for (int k = j + 1; k < route.size()-1; k++) {
                    // std::vector<int> cpyRoute = vet_route;
                    intraReverseRouteMov(route);
                    currentCost = getRouteCost(route, Inst);
                    if (currentCost < bestRouteCost) {
                        bestRouteCost = currentCost;
                        improvement = true;
                        costs.at(i).second = bestRouteCost; // atualiza o vetor de custos
                    } else {
                        intraReverseRouteMov(route); // REVERTS reverseRoute (aplying it again, but backwards)
                        improvement = false;
                    }
                }
            }
        }
    }
    std::cout << "wait.";
}

void ls::interShift01(std::vector<std::vector<int>> &routes, const Instance &Inst) {
    //bool shift01Mov(std::vector<int>& vet_route, int node, const Instance& Inst, float& evCost, float& solCost, int& place, int& rs);
//    bool shift01Mov(std::vector<int>& vet_route, int node, const Instance& Inst);
    for(int i = 0; i < routes.size(); i++){
        std::vector<int>& route = routes.at(i);
        float minDemand = getRouteDemand(route, Inst);
        for(int j = i + 1; j < routes.size(); j++){
            // para cada par de rotas;
            bool improving = true;
            std::vector<int>& loserRoute = routes.at(i);
            std::vector<int>& winnerRoute = routes.at(j);
            while(improving){
                improving = false;
                for(int n = 1; n < loserRoute.size()-1; n++){
                    int node = loserRoute.at(n);
                    // float evCost,
                    Item item;
                    getSafeInsertionIn(winnerRoute, node, Inst, item);
                }



            }
        }
    }
}

void swap(std::vector<int>& v, int i, int j){
    int aux;
    aux = v[i];
    v[i] = v[j];
    v[j] = aux;
}
void interSwapMove(std::vector<int>& route1, std::vector<int> route2, int n1, int n2){
    int aux =route1.at(n1);
    route1.at(n1) = route2.at(n2);
    route2.at(n2) = aux;
};

void intra2optMov(std::vector<int>& route, int a, int b){ // TODO: currently (generally) the user will pass a copy vet_route. so its O(n2)
    for(int i = 0; i < route.size(); i++){
        if(route.size() < 4){ return; }
        if(i >=a && i <= a + (b - a)/2){
            swap(route, i, a + b - i);
        }
    }
}

void intraReverseRouteMov(std::vector<int> &route) {
    for(int i = route.size()-1; i <= 0; i++){
        const int& node = route.at(i);
        swap(route, i, (int)route.size() - 1 - i);
        // dstRoute.push_back(node)
    }
}
bool shift01Mov(std::vector<int>& route, int node, const Instance& Inst){
    // float evCost, solCost;
    // int place, rs;
    //return getCheapestSafeInsertionIn(vet_route, node, Inst, evCost, place, rs, solCost);
    float cost;
    int place;
    getCheapestInsertionTo(node, route, Inst, cost, place);
    if(place != -1) {
        insertInRoute(node, route, place);
        return true;
    }
    return false;
}


