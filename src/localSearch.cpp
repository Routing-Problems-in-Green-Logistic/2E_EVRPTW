//
// Created by abrolhus on 06/09/2021.
//

#include "localSearch.h"

void lsh::swapMove(std::vector<int> &routeI, std::vector<int> &routeJ, int i, int j) {

}

void lsh::shiftMove(std::vector<int> &loserRoute, std::vector<int> &winnerRoute, int i, int j) {

}

bool lsh::reinsertMove(std::vector<int> &route, int position, const Instance& Inst) {
    int node = route.at(position);
    float cost = getRouteCost(route, Inst);
    Item insertion;
    route.erase(route.begin() + position);
    bool canInsert = getSafeInsertionIn(route, node, Inst, insertion);
    if(canInsert) {
        float afterInsertCost = getRouteCost(route, Inst) + insertion.solutionCost;
        if(afterInsertCost < cost){
            insertInRoute(node, route, insertion.insertionPosition);
            if(insertion.rs != -1) {
                insertInRoute(insertion.rs, route, insertion.insertionPosition); // inserts recharging station before the client;
            }
            return true;
        }
    }
    insertInRoute(node, route, position); // reinsert removed node;
    return false;
}

bool lsh::reinsertion(Solution& Sol, const Instance& Inst) {
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    float bestDiscount = 0, currentDiscount;
    float routeCost, afterInsertionCost;
    std::vector<std::vector<int>> bestRoutes = routes; //copy
    std::vector<float> bestDiscounts(routes.size(), 0);
    //int bestRouteIndex = -1;

    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        std::vector<int>& route = routes.at(r);

        bool improving = true;
        while(improving){
            improving = false;
            for(int i = 1; i < route.size()-1; i++) {
                std::vector<int> cpyRoute = routes.at(r); //copy route;
                routeCost = getRouteCost(cpyRoute, Inst);
                bool inserted = reinsertMove(cpyRoute, i, Inst);
                if(!inserted){ continue; }
                currentDiscount = getRouteCost(cpyRoute, Inst) - routeCost;
                if(currentDiscount < bestDiscounts.at(r)){
                    bestRoutes.at(r) = cpyRoute; // copy
                    bestDiscounts.at(r) = currentDiscount;
                    improving = true;
                    //bestRouteIndex = r;
                }
            }
            if(improving){
                routes.at(r) = bestRoutes.at(r);
            }
        }
    }
    return false;
}
