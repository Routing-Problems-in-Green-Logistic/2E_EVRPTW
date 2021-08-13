#ifndef INC_2E_EVRP_SOLUTION_H
#define INC_2E_EVRP_SOLUTION_H
#include <vector>

class Solution {
private:
    std::vector<std::vector<int>> routes;
    int nTrucks;
    int nEvs;
public:
    Solution();
    Solution(std::vector<std::vector<int>>& routes, int nTrucks, int nEvs);
    int getNTrucks() const;
    int getNEvs() const;
    const std::vector<std::vector<int>>& getRoutes();
};

#endif //INC_2E_EVRP_SOLUTION_H