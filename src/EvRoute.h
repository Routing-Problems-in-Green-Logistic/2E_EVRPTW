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

    Insertion(int pos, int clientId, float cost, float demand, float batteryCost, int routeId, int satId, int rsPos, int rsId)
    {
        this->pos = pos;
        this->clientId = clientId;
        this->cost = cost;
        this->demand = demand;
//        this->rs = rs;
        this->batteryCost = batteryCost;
        this->rechargingS_Pos = rsPos;
        this->routeId = routeId;
        this->satId = satId;
        rechargingS_Id = rsId;
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
    // float getBatteryAt(int pos, const Instance& Inst) const; // REPLACED WITH getBatteryBefore
    int size() const;
    float getDemand(const Instance& Inst) const;
    float getMinDemand() const;

    float getInitialCapacity() const;

    float getInitialBattery() const;

    float getMaxDemand() const;
    float getCurrentCapacity() const;
    float getCost(const Instance& Inst) const;
    float getDemandOf(int i, const Instance& Inst) const;
    float getRemainingBatteryBefore(int i) const;
    //bool insert(int node, int pos, const Instance& Inst);
    bool insert(Insertion& insertion, const Instance& Inst);
    void print() const;
    bool canInsert(int node, const Instance &Instance, Insertion &insertion) const;
    bool rechargingS_inUse(int id) const;
    bool isRechargingS(int pos, const Instance& Inst) const;
    int getNodeAt(int pos);


    float totalDemand = 0.0;
    float remainingCapacity; // TODO: deixar soh total de demanda
    float distance = 0.0;
    float batteryConsumption;
    int satelite = 0.0;
    float initialCapacity;
    float initialBattery;
    std::list<std::pair<int,int>> rechargingStationsPos_Rs;     // Armazena tuplas com (posicao de route; recharging station id)
    std::vector<bool> rechargingStationRoute;                   // Indica se a posicao eh uma estacao de recarga
    std::vector<int> route;
    std::vector<float> vetRemainingBattery;
    int routeSize = 2;
    int routeSizeMax = -1;
    float minDemand = 1e8;
    float maxDemand = 0;

    void print(std::string &str) const;

    bool checkRoute(std::string &erro, const Instance &Inst);
};
#endif
