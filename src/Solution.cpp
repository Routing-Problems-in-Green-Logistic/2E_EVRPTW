#include "Solution.h"

Solution::Solution() {
    nEvs = 0;
    nTrucks = 0;
}
Solution::Solution(std::vector<std::vector<int>> &routes, int nTrucks, int nEvs){
    this->routes = routes; // TODO: see if this actually works
    this->nTrucks = nTrucks;
    this->nEvs = nEvs;
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
