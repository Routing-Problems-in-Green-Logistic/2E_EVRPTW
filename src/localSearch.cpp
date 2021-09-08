//
// Created by abrolhus on 06/09/2021.
//

#include "localSearch.h"

bool lsh::swapMove(std::vector<int> &routeI, std::vector<int> &routeJ, int i, int j, const Instance& Inst, float& discount) {
    std::vector<int> cpyRI = routeI;
    std::vector<int> cpyRJ = routeJ;
    int aux = cpyRI.at(i);
    cpyRI.at(i) = cpyRJ.at(j);
    cpyRJ.at(j) = aux;
    if(!isFeasibleRoute(cpyRI, Inst) || !isFeasibleRoute(cpyRJ, Inst)){
        return false;
    }
    discount = getRouteCost(cpyRJ, Inst) + getRouteCost(cpyRI, Inst) - getRouteCost(routeI, Inst) - getRouteCost(routeJ, Inst);
    if(discount >= 0){
        return false;
    }
    routeI = cpyRI;
    routeJ = cpyRJ;
    return true;
}

bool lsh::swap(Solution &Sol, const Instance &Inst) {
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    bool hasImproved = false;
    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        for(int s = r + 1; s < routes.size(); s++) { // triangular. swap(a,b) = swap(b,a)
            if (r == s) { continue; }
            std::vector<int> &routeI = routes.at(r);
            std::vector<int> &routeJ = routes.at(s);
            bool improving = true;
            float currentDiscount, bestDiscount;
            std::vector<int> bestRouteI, bestRouteJ;
            while (improving) {
                improving = false;
                for (int i = 1; i < routeI.size() - 1; i++) {
                    for (int j = 1; j < routeJ.size() - 1; j++) {
                        std::vector<int> cpyRouteI = routes.at(r); //copy route;
                        std::vector<int> cpyRouteJ = routes.at(s);
                        bool inserted = swapMove(cpyRouteI, cpyRouteJ, i, j, Inst, currentDiscount);
                        if (!inserted) { continue; }
                        if (currentDiscount < bestDiscount) {
                            bestRouteI = cpyRouteI;
                            bestRouteJ = cpyRouteJ;
                            bestDiscount = currentDiscount;
                            improving = true;
                        }
                    }
                }
                if (improving) {
                    routeI = bestRouteI;
                    routeJ = bestRouteJ;
                    hasImproved = true;
                }
            }
        }
    }
    return hasImproved;
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
    bool hasImproved = false;
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
                hasImproved = true;
            }
        }
    }
    return hasImproved;
}

bool lsh::shiftMove(std::vector<int> &loserRoute, std::vector<int> &winnerRoute, int i, const Instance &Inst, float& discount) {
    float loserCost, winnerCost, beforeCost, loserCostAfter, winnerCostAfter, afterCost;
    if(Inst.isRechargingStation(loserRoute.at(i))){ return false; }
    loserCost = getRouteCost(loserRoute, Inst);
    winnerCost = getRouteCost(loserRoute, Inst);
    beforeCost = loserCost + winnerCost;
    Item insertion;
    bool canInsert = getSafeInsertionIn(winnerRoute, loserRoute.at(i), Inst, insertion);
    if(!canInsert){ return false; }
    loserCostAfter = loserCost + Inst.getDistance(loserRoute.at(i-1), loserRoute.at(i+1)) - Inst.getDistance(loserRoute.at(i), loserRoute.at(i-1)) -Inst.getDistance(loserRoute.at(i), loserRoute.at(i+1));
    winnerCostAfter = winnerCost + insertion.solutionCost;
    afterCost = loserCostAfter + winnerCostAfter;
    if(afterCost >= beforeCost){
        return false;
    }
    insertInRoute(loserRoute.at(i), winnerRoute, insertion.insertionPosition);
    if(insertion.rs != -1) {
        insertInRoute(insertion.rs, winnerRoute, insertion.insertionPosition); // inserts recharging station before the client;
    }
    loserRoute.erase(loserRoute.begin() + i);
    discount = afterCost - beforeCost;
    return true;
}

bool lsh::shift(Solution &Sol, const Instance &Inst) {
    bool hasImproved = false;
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    //float bestDiscount = 0, currentDiscount;
    //float routeCost, afterInsertionCost;
    // std::vector<std::vector<int>> bestRoutes = routes; //copy
    // std::vector<float> bestDiscounts(routes.size(), 0);
    //int bestRouteIndex = -1;

    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        for(int s = Sol.getNTrucks(); s < routes.size(); s++) {
            if (r == s) { continue; }
            std::vector<int> &loserRoute = routes.at(r);
            std::vector<int> &winnerRoute = routes.at(s);
            bool improving = true;

            float currentDiscount, bestDiscount;
            std::vector<int> bestWinnerRoute, bestLoserRoute;
            while (improving) {
                improving = false;
                for (int i = 1; i < loserRoute.size() - 1; i++) {
                    std::vector<int> cpyloserRoute = routes.at(r); //copy route;
                    std::vector<int> cpywinnerRoute = routes.at(s);
                    // float routeCost = getRouteCost(cpyRoute, Inst);
                    //bool inserted = reinsertMove(cpyRoute, i, Inst);
                    bool inserted = shiftMove(cpyloserRoute, cpywinnerRoute, i, Inst, currentDiscount);
                    if (!inserted) { continue; }
                    // float currentDiscount = getRouteCost(cpyRoute, Inst) - routeCost;
                    if (currentDiscount < bestDiscount) {
                        bestLoserRoute = cpyloserRoute;
                        bestWinnerRoute = cpywinnerRoute;
                        bestDiscount = currentDiscount;
                        improving = true;
                    }
                }
                if (improving) {
                    winnerRoute = bestWinnerRoute;
                    loserRoute = bestLoserRoute;
                    hasImproved = true;
                }
            }
        }
    }
    return hasImproved;
}

void lsh::swapVectorElements(std::vector<int>& v, int i, int j){
    int aux;
    aux = v[i];
    v[i] = v[j];
    v[j] = aux;
}
bool lsh::twoOptMove(std::vector<int> &route, int a, int b, const Instance &Inst, float &discount) {

    std::vector<int> cpyRoute = route;
    for(int i = 0; i < route.size(); i++){
        if(route.size() < 4){ return false; }
        if(i >=a && i <= a + (b - a)/2){
            lsh::swapVectorElements(cpyRoute, i, a + b - i);
        }
    }
    if(!isFeasibleRoute(cpyRoute, Inst)){
        return false;
    }
    float currentDiscount;
    float routeCost = getRouteCost(route, Inst);
    float afterCost = getRouteCost(cpyRoute, Inst);
    currentDiscount = afterCost - routeCost;
    if(currentDiscount >= 0){
        return false;
    }
    route = cpyRoute;
    discount = currentDiscount;
    return true;

}

bool lsh::twoOpt(Solution &Sol, const Instance &Inst) {
    bool hasImproved = false;
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    float bestDiscount = 0, currentDiscount;
    float routeCost, afterInsertionCost;
    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        std::vector<int>& route = routes.at(r);
        bool improving = true;
        std::vector<int> bestRoute;
        while(improving){
            improving = false;
            for(int i = 1; i < route.size()-1; i++) {
                for(int j = i+1; j < route.size()-1; j++) {
                    std::vector<int> cpyRoute = routes.at(r); //copy route;
                    bool inserted = twoOptMove(cpyRoute, i, j, Inst, currentDiscount);
                    if (!inserted) { continue; }
                    //currentDiscount = getRouteCost(cpyRoute, Inst) - routeCost;
                    if (currentDiscount < bestDiscount) {
                        bestRoute = cpyRoute; // copy
                        bestDiscount = currentDiscount;
                        improving = true;
                    }
                }
            }
            if(improving){
                routes.at(r) = bestRoute;
                hasImproved = true;
            }
        }
    }
    return hasImproved;
}
