#include "Solution.h"

Solution::Solution() {
    nEvs = 0;
    nTrucks = 0;
}
Solution::Solution(std::vector<std::vector<int>> &routes, int nTrucks, int nEvs, float cost){
    this->routes = routes; // TODO: see if this actually works
    this->nTrucks = nTrucks;
    this->nEvs = nEvs;
    this->cost = cost;
}

int Solution::getNTrucks() const {
    return nTrucks;
}

int Solution::getNEvs() const {
    return nEvs;
}

const std::vector<std::vector<int>>& Solution::getRoutes() {
    return routes;
}

float Solution::getCost() const {
    return cost;
}

std::vector<std::vector<int>> &Solution::acessRoutes() {
    return this->routes;
}
