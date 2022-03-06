#ifndef EV_ROUTE_H
#define EV_ROUTE_H

#include "Instance.h"
#include "Route.h"
#include <list>

class Recharge {
public:
    int pos = -1;
    int id = -1;
    float remainingBattery = 0.0;
    Recharge() = default;
    Recharge(int pos, int id, float remainingBattery)
        : pos(pos), id(id), remainingBattery(remainingBattery){};
};

struct PosicaoEstacao
{
    int pos = -1;
    int rechargingStationId = -1;

    friend bool operator < (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId < posRouteRechSId1.rechargingStationId;
    }


    friend bool operator > (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId > posRouteRechSId1.rechargingStationId;
    }

    friend bool operator == (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId == posRouteRechSId1.rechargingStationId;
    }
};

struct PosRota0Rota1Estacao
{

    int posRoute0 = -1;
    int posRoute1 = -1;
    int rechargingStationId = -1;

    friend bool operator <(const PosRota0Rota1Estacao &aux0, const PosRota0Rota1Estacao &aux1)
    {
        return aux0.rechargingStationId < aux1.rechargingStationId;
    }

    void swapPos()
    {
        int aux = posRoute0;
        posRoute0 = posRoute1;
        posRoute1 = aux;
    }

};

class EvRoute{
public:
    ///---------- info -------------///
    int size() const;
    float getDemand() const;
    float getMinDemand() const;
    float getMaxDemand() const;
    float getCurrentCapacity() const;
    EvRoute(float evBattery, float evCapacity, int satelite, int RouteSizeMax);
    EvRoute(const Instance &instance);
    void print(const Instance &instance) const;
    void print(std::string &str, const Instance &instance) const;
    bool checkRoute(std::string &erro, const Instance &Inst) const;
    void setAuxStructures(const Instance& Inst);
    int getNodeAt(int pos) const;
    float getDemandOf(int i, const Instance& Inst) const;
    //float getRemainingBatteryBefore(int i) const;

    int operator [](int pos) const
    {
        return route[pos];
    }


    int& operator [](int pos)
    {
        return route[pos];
    }

    std::vector<int> route;
    std::vector<float> vetRemainingBattery;
    int satelite = 0;
    float distance = 0.0;
    int routeSize = 2;
    float totalDemand = 0.0;
    int routeSizeMax = -1;
    float minDemand = 1e8;
    float maxDemand = 0;


};
#endif
