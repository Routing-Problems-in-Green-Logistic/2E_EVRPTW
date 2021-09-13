//
// Created by abrolhus on 07/09/2021.
//

#include "vns.h"
#include "localSearch.h"
#include "algorithm.h"
#include <random>


#define MAX_ITER 200
/*
void swapVectorElements(std::vector<int>& v, int i, int j){
    int aux;
    aux = v[i];
    v[i] = v[j];
    v[j] = aux;
}
 */
std::vector<int> getRandomNeighbourhoodList(int nNeighbourhoods){
    if(nNeighbourhoods <= 0){
        exit(-1);
    }
    std::vector<int> list;
    for(int i = 0; i < nNeighbourhoods; i++){
        list.push_back(i);
    }
    return list;

}
void shuffleVector(std::vector<int>& vec) {
    for(int i = 0; i < vec.size() -1; i++) {
        int j = i + (int)rand()%(vec.size() - i);
        lsh::swapVectorElements(vec, i, j);
    }
}
void vns::rvnd(Solution &Sol, const Instance &Inst) {
    std::vector<int> neighbourhoods = getRandomNeighbourhoodList(4);
    bool hasImproved;
    for(int it = 0; it < MAX_ITER; it++) {
        shuffleVector(neighbourhoods);
        for(int nh : neighbourhoods) {
            switch (nh) {
                case 0:
                    hasImproved = lsh::swap(Sol, Inst);
                    break;
                case 1:
                    hasImproved = lsh::shift(Sol, Inst);
                    it++;
                    break;
                case 2:
                    hasImproved = lsh::twoOpt(Sol, Inst);
                    it++;
                    break;
                case 3:
                    hasImproved = lsh::reinsertion(Sol, Inst);
                    it++;
                    break;
                default:
                    break;
            }
            if(hasImproved) {
                break;
            }
        }
    }
}

void vns::gvns(Solution &Sol, const Instance &Inst) {
    std::vector<int> neighbourhoods = getRandomNeighbourhoodList(3);
    bool hasImproved;
    bool hasMoved;
    int n = 0;
    for(int it = 0; it < MAX_ITER; it++){
        shuffleVector(neighbourhoods);
        hasImproved = false;
        hasMoved = false;
        int nh = neighbourhoods.at(n);
        Solution cpySol = Sol;
        switch (nh) {
            case 0:
                //hasMoved = vns::randomSwap22(cpySol, Inst);
                break;
            case 1:
                //hasMoved = vns::randomClientReinsertion(cpySol, Inst);
                //hasMoved = vns::randomSwap22(cpySol, Inst);
                it++;
                break;
            case 2:
                //hasMoved = vns::randomSwap22(cpySol, Inst);
                it++;
                break;
            default:
                //hasMoved = false;
                it++;
                break;
        }
        if(!hasMoved){
            continue;
        }
        rvnd(cpySol, Inst);
        float solCost = getSolCost(Sol, Inst);
        float newSolCost = getSolCost(cpySol, Inst);
        if(newSolCost < solCost){
            hasImproved = true;
            Sol = cpySol; // copy
        }
        else{
            hasImproved = false;
        }
        n++;
        if(n >= neighbourhoods.size()){
            n = 0;
        } else if(hasImproved){
            n = 0;
        }
    }
    auto& solRoutes = Sol.acessRoutes();
    int routesSize = solRoutes.size();
    for(int i = 0; i < routesSize; i++){
        if(solRoutes.at(i).size() <= 2){
            solRoutes.erase(solRoutes.begin() + i);
        }
        i--;
        routesSize--;
    }
    auto first = solRoutes.begin() + Sol.getNTrucks();
    auto last = solRoutes.end();
    /*
    std::vector<std::vector<int>> secEchelonRoutes(first, last);
    std::vector<std::vector<int>> routes;
    float totalCost=0;
    firstEchelonRoutes(secEchelonRoutes, Inst, routes, totalCost);
    Sol.acessRoutes() = routes;
    Sol.acessRoutes().insert(Sol.acessRoutes().end(), secEchelonRoutes.begin(), secEchelonRoutes.end());
     */

}
bool swap22Move(std::vector<int> &routeI, std::vector<int> &routeJ, int i, int j, const Instance& Inst, float& discount) {
    std::vector<int> cpyRI = routeI;
    std::vector<int> cpyRJ = routeJ;
    int aux1 = cpyRI.at(i);
    cpyRI.at(i) = cpyRJ.at(j);
    cpyRJ.at(j) = aux1;
    int aux2 = cpyRI.at(i+1);
    cpyRI.at(i+1) = cpyRJ.at(j+1);
    cpyRJ.at(j+1) = aux2;
    if(!isFeasibleRoute(cpyRI, Inst) || !isFeasibleRoute(cpyRJ, Inst)){
        return false;
    }
    discount = getRouteCost(cpyRJ, Inst) + getRouteCost(cpyRI, Inst) - getRouteCost(routeI, Inst) - getRouteCost(routeJ, Inst);
    //if(discount >= 0){
     //   return false;
    //}
    routeI = cpyRI;
    routeJ = cpyRJ;
    return true;
}

bool vns::randomSwap22(Solution& Sol, const Instance& Inst){
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    std::vector<std::pair<std::vector<int>, std::vector<int>>> afterSwapRoutes;
    std::vector<std::pair<int, int>> afterSwapRouteIndexes;

    float currentDiscount;
    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        for(int s = r + 1; s < routes.size(); s++) { // triangular. swap(a,b) = swap(b,a)
            if (r == s) { continue; }
            std::vector<int> &routeI = routes.at(r);
            std::vector<int> &routeJ = routes.at(s);
            for (int i = 1; i < routeI.size() - 2; i++) {
                for (int j = 1; j < routeJ.size() - 2; j++) {
                    std::vector<int> cpyRouteI = routes.at(r); //copy route;
                    std::vector<int> cpyRouteJ = routes.at(s);
                    bool inserted = swap22Move(cpyRouteI, cpyRouteJ, i, j, Inst, currentDiscount);
                    if (!inserted) { continue; }
                    afterSwapRoutes.emplace_back(cpyRouteI, cpyRouteJ);
                    afterSwapRouteIndexes.emplace_back(r,s);
                }
            }
        }
    }
    if(afterSwapRoutes.empty()){
        return false;
    }
    int randomIndex = (int)(rand())%afterSwapRoutes.size();
    auto& routePair = afterSwapRoutes.at(randomIndex);
    auto& routesIndexes = afterSwapRouteIndexes.at(randomIndex);
    routes.at(routesIndexes.first) = routePair.first;
    routes.at(routesIndexes.second) = routePair.second;
    return true;
}

bool clientReinsertionMove(std::vector<int>& loserRoute, std::vector<int>& winnerRoute, int i,  const Instance& Inst){
    float loserCost, winnerCost, beforeCost, loserCostAfter, winnerCostAfter, afterCost;
    if(Inst.isRechargingStation(loserRoute.at(i))){ return false; }
    loserCost = getRouteCost(loserRoute, Inst);
    winnerCost = getRouteCost(loserRoute, Inst);
    beforeCost = loserCost + winnerCost;
    Item insertion;
    bool canInsert = getSafeInsertionIn(winnerRoute, loserRoute.at(i), Inst, insertion);
    if(!canInsert){ return false; }
    //loserCostAfter = loserCost + Inst.getDistance(loserRoute.at(i-1), loserRoute.at(i+1)) - Inst.getDistance(loserRoute.at(i), loserRoute.at(i-1)) -Inst.getDistance(loserRoute.at(i), loserRoute.at(i+1));
    //winnerCostAfter = winnerCost + insertion.solutionCost;
    //afterCost = loserCostAfter + winnerCostAfter;
    //if(afterCost >= beforeCost){
    //    return false;
    //}
    insertInRoute(loserRoute.at(i), winnerRoute, insertion.insertionPosition);
    if(insertion.rs != -1) {
        insertInRoute(insertion.rs, winnerRoute, insertion.insertionPosition); // inserts recharging station before the client;
    }
    loserRoute.erase(loserRoute.begin() + i);
    //discount = afterCost - beforeCost;
    return true;


}
bool vns::randomClientReinsertion(Solution &Sol, const Instance &Inst) {
    std::vector<std::vector<int>>& routes = Sol.acessRoutes();
    std::vector<std::pair<std::vector<int>, std::vector<int>>> afterSwapRoutes;
    std::vector<std::pair<int, int>> afterSwapRouteIndexes;
    for(int r = Sol.getNTrucks(); r < routes.size(); r++){
        for(int s = Sol.getNTrucks(); s < routes.size(); s++) {
            //if (r == s) { continue; }
            std::vector<int> &loserRoute = routes.at(r);
            std::vector<int> &winnerRoute = routes.at(s);
            float currentDiscount, bestDiscount;
            std::vector<int> bestWinnerRoute, bestLoserRoute;
            for (int i = 1; i < loserRoute.size() - 1; i++) {
                std::vector<int> cpyloserRoute = routes.at(r); //copy route;
                std::vector<int> cpywinnerRoute = routes.at(s);
                bool inserted = clientReinsertionMove(cpyloserRoute, cpywinnerRoute, i, Inst);
                if (!inserted) { continue; }
                // float currentDiscount = getRouteCost(cpyRoute, Inst) - routeCost;
                afterSwapRoutes.emplace_back(cpyloserRoute, cpywinnerRoute);
                afterSwapRouteIndexes.emplace_back(r,s);
            }
        }
    }
    if(afterSwapRoutes.empty()){
        return false;
    }
    int randomIndex = (int)(rand())%afterSwapRoutes.size();
    auto& routePair = afterSwapRoutes.at(randomIndex);
    auto& routesIndexes = afterSwapRouteIndexes.at(randomIndex);
    routes.at(routesIndexes.first) = routePair.first;
    routes.at(routesIndexes.second) = routePair.second;
    return true;
}
/*
bool vns::recalculateFirstEchelon(Solution& Sol, const Instance& Inst){
    std::vector<std::vector<int>> firstEchelon;
    std::vector<std::vector<int>> secondEchelon;
    for(auto& route : Sol.acessRoutes()){
        if(route.at(0) != 0){
            secondEchelon.push_back(route);
        }
    }
    float cost;
    firstEchelonRoutes(firstEchelon, Inst, secondEchelon, cost);
    auto& solRoutes = Sol.acessRoutes();
    for(int i = 0; i < firstEchelon.size(); i++){

    }
}
*/
