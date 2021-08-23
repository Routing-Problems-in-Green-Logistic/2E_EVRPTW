#include <iostream>
#include "Solution.h"
#include "Instance.h"

/** Checks the feasibility of the Solution, returning false if infeasible.
 *
 * @param Sol Solution
 * @param Inst Instance
 * @return true if feasible
 */
bool isFeasibleSolution(Solution& Sol, Instance& Inst);
void getCheapestInsertionTo(int node, const std::vector<int>& route, const Instance& Inst, float& cost, int& place);
Solution* construtivo(Instance& instance);
void insertInRoute(int node, std::vector<int>& route, int position);
std::vector<std::vector<int>>& secondEchelonRoutes(Instance& Inst, std::vector<std::vector<int>>& routes);
std::vector<std::vector<int>>& firstEchelonRoutes(std::vector<std::vector<int>>& firstEchelonRoutes, Instance& Inst, std::vector<std::vector<int>>& routes);
