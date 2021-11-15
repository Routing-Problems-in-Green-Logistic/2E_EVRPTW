#ifndef EV_ROUTE_H
#define EV_ROUTE_H

#include "Instance.h"
#include "Route.h"
#include <list>
class Insertion {
public:
    int pos;
    int node;
    int rs;
    int rsPos;
    float cost;
    float demand;
    float batteryCost;
    Insertion(int pos, int node, float cost, float demand, float batteryCost, int rs=-1, int rsPos=-1){
        this->pos = pos;
        this->node = node;
        this->cost = cost;
        this->demand = demand;
        this->rs = rs;
        this->batteryCost = batteryCost;
        this->rsPos = rsPos;
    }
    Insertion() = default;
    bool operator< (const Insertion& that) const {
        return (this->cost < that.cost);
    }
};

class EvRoute{
public:
    EvRoute(float evBattery, float evCapacity, int satelite);
    float getBatteryAt(int pos, const Instance& Inst) const;
    int size() const;
    float getDemand(const Instance& Inst) const;
    float getMinDemand() const;
    float getCurrentCapacity() const;
    float getCost(const Instance& Inst) const;
    bool insert(int node, int pos, const Instance& Inst);
    bool insert(const Insertion& Ins, const Instance& Inst);
    void print() const;
    bool canInsert(int node, const Instance& Inst, Insertion& Ins);
private:
    float totalDemand;
    float remainingCapacity; // TODO: deixar soh total de demanda
    float cost;
    int satelite;
    float initialCapacity;
    float initialBattery;
    std::list<std::pair<int, float>> rechargingStations; // pair {position, remainingBatteryBefore}. Also includes the last node, the satelite.
    std::vector<int> nodes;
};
#endif
