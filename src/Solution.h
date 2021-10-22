#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H
#include <vector>

class Solution {
private:
    std::vector<std::vector<int>> routes;
    int nTrucks;
    int nEvs;
    float cost;
public:
    Solution();
    Solution(std::vector<std::vector<int>>& routes, int nTrucks, int nEvs, float cost);
    int getNTrucks() const;
    int getNEvs() const;
    float getCost() const;

    const std::vector<std::vector<int>>& getRoutes() const;
    std::vector<std::vector<int>>& acessRoutes();
};

#endif //INC_2E_EVRP_SOLUTION_H