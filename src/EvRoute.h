#ifndef EV_ROUTE_H
#define EV_ROUTE_H

#include "Instance.h"
#include "Route.h"
#include <list>

class Insertion {
public:

    int pos             = -1;       // Verificar se o indice rechargingS_Pos eh menor antes de inserir na solucao. O indice maior ja esta considerando +1
    int clientId        = -1;
    int rechargingS_Id  = -1;
    int rechargingS_Pos = -1;       // Verificar se o indice pos eh menor antes de inserir na solucao. O indice maior ja esta considerando +1
    int routeId         = -1;
    int satId           = -1;
    float cost          = 0.0;
    float demand        = 0.0;
    float batteryCost   = 0.0;

    Insertion(int pos, int clientId, float cost, float demand, float batteryCost, int routeId, int satId, int rsPos = -1) {
        this->pos = pos;
        this->clientId = clientId;
        this->cost = cost;
        this->demand = demand;
//        this->rs = rs;
        this->batteryCost = batteryCost;
        this->rechargingS_Pos = rsPos;
        this->routeId = routeId;
        this->satId = satId;
    }
    Insertion(int routeId) { this->routeId = routeId;}
    Insertion() = default;
    bool operator< (const Insertion& that) const {
        return (this->cost < that.cost);
    }
};

class EvRoute{
public:
    EvRoute(float evBattery, float evCapacity, int satelite, int RouteSizeMax);
    float getBatteryAt(int pos, const Instance& Inst) const;
    int size() const;
    float getDemand(const Instance& Inst) const;
    float getMinDemand() const;
    float getCurrentCapacity() const;
    float getCost(const Instance& Inst) const;
    bool insert(int node, int pos, const Instance& Inst);
    bool insert(Insertion& insertion, const Instance& Inst);
    void print() const;
    bool canInsert(int node, const Instance &Instance, Insertion &insertion);
    bool rechargingS_inUse(int id);

private:
    float totalDemand;
    float remainingCapacity; // TODO: deixar soh total de demanda
    std::vector<float> vetRemainingBattery;
    float distance;
    float batteryConsumption;
    int satelite;
    float initialCapacity;
    float initialBattery;
    std::list<std::pair<int,int>> rechargingStationsPos_Rs;     // Armazena tuplas com (posicao de route; recharging station id)
    std::vector<bool> rechargingStationRoute;                   // Indica se a posicao eh uma estacao de recarga
    std::vector<int> route;
    int routeSize = 2;
    int routeSizeMax = -1;

};
#endif
