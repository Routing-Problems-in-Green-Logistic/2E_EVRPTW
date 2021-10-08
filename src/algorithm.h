#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <iostream>
#include "Solution.h"
#include "Instance.h"

class Item{
public:
    int node;
    int routeIndex;
    int insertionPosition;
    float capacityCost;
    float batteryCost;
    float solutionCost;
    int rs = -1; // pass in this recharging station before going to the client
    Item(int node, int routeIndex, int insertionPosition, float capacityCost, float batteryCost, float solutionCost, int rs=-1):
            node(node), routeIndex(routeIndex), insertionPosition(insertionPosition), capacityCost(capacityCost), batteryCost(batteryCost), rs(rs), solutionCost(solutionCost){}
    Item(){
        this->node = -1;
        this->routeIndex = -1;
        this->insertionPosition = -1;
        this->capacityCost = -1;
        this->batteryCost = -1;
        this->solutionCost = -1;
    }
    bool operator< (const Item& that) const
    {
        if(this->solutionCost == that.solutionCost)
            return (this->capacityCost < that.capacityCost);
        return (this->solutionCost < that.solutionCost);
        return true;
    }
};
/** Checks the feasibility of the Solution, returning false if infeasible.
 *
 * @param Sol Solution
 * @param Inst Instance
 * @return true if feasible
 */
bool isFeasibleSolution(Solution& Sol, const Instance& Inst);
void getCheapestInsertionTo(int node, const std::vector<int>& route, const Instance& Inst, float& cost, int& place);
Solution* construtivo(const Instance& instance, std::vector<std::vector<int>>& ser);
void insertInRoute(int node, std::vector<int>& route, int position);
std::vector<std::vector<int>>& secondEchelonRoutes(const Instance& Inst, std::vector<std::vector<int>>& routes, float& totalCost);
std::vector<std::vector<int>>& firstEchelonRoutes(std::vector<std::vector<int>>& firstEchelonRoutes, const Instance& Inst, std::vector<std::vector<int>>& routes, float totalCost);
float getRouteCost(const std::vector<int>& route, const Instance& Inst);
float getRouteDemand(const std::vector<int>& route, const Instance& Inst);
/** Finds best spot to insert node (nodeIndex) in route. if unable, returns false;
 *
 * @param route
 * @param nodeIndex
 * @param Inst
 * @param evCost [out] final cost of insertion
 * @param place [out] index to insert
 * @return if is possible to insert or not
 */
bool getCheapestSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, float& evCost, int& place, int& rs, float& solCost);
//bool getSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, float& finalEvCost, int& finalPlace, int& finalRs, float& finalSolCost);
bool getSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, Item& insertion);

bool isFeasibleRoute(const std::vector<int>& route, const Instance& Inst);
float getSolCost(const Solution& Sol, const Instance& Inst);
bool insertInNewRoute(std::vector<int>& route, int nodeIndex, const Instance& Inst, int rs1, int rs2);
void removeEmptyRoutes(std::vector<std::vector<int>>& routes); // TODO: maybe (mayyybe) get a list of indexes as parameters and then check each element with index in the list;

#endif