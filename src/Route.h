#ifndef ROUTE_H
#define ROUTE_H

#include "Instance.h"
#include <vector>
#include <iostream>

class Route {
public:
    virtual int size() = 0;
    virtual float getDemand() = 0;
    virtual float getMinDemand() = 0;
    virtual float getCost() = 0;
    virtual void print() = 0;
protected:
    std::vector<int> nodes;
};

#endif
