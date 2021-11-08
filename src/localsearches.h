//
// Created by abrolhus on 27/08/2021.
//

#ifndef INC_2E_EVRP_LOCALSEARCHES_H
#define INC_2E_EVRP_LOCALSEARCHES_H
#include "Instance.h"
#include "algorithm.h"

void intra2optMov(std::vector<int>& route, int a, int b);
void intraReverseRouteMov(std::vector<int> &route);
// inter vet_route
void interShift01Mov(std::vector<int>& route, int node);
bool shift01Mov(std::vector<int>& route, int node, const Instance& Inst);
void interSwapMove(std::vector<int>& route1, std::vector<int> route2, int n1, int n2);
/** Local Searches.
 *
 */
namespace ls {
    void intra2opt(std::vector<std::vector<int>>& routes, const Instance& Inst, std::vector<std::pair<float,float>>& costs);
    void intraReverseRoute(std::vector<std::vector<int>>& routes, const Instance& Inst, std::vector<std::pair<float,float>>& costs);
    void interShift01(std::vector<std::vector<int>>& routes, const Instance& Inst);
}
#endif //INC_2E_EVRP_LOCALSEARCHES_H

