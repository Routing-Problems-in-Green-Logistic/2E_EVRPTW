#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <iostream>
#include "Solution.h"
#include "Instance.h"

/** Checks the feasibility of the Solution, returning false if infeasible.
 *
 * @param Sol Solution
 * @param Inst Instance
 * @return true if feasible
 */
bool isFeasibleSolution(Solution& Sol, const Instance& Inst);
void getCheapestInsertionTo(int node, const std::vector<int>& route, const Instance& Inst, float& cost, int& place);
Solution* construtivo(const Instance& instance);
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
bool getCheapestSafeInsertionIn(const std::vector<int>& route, int nodeIndex, const Instance& Inst, float& evCost, int& place);

#endif