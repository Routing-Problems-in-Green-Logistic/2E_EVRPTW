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
class Recharge {
public:
    int pos = -1;
    int id = -1;
    float remainingBattery = 0.0;
    Recharge() = default;
    Recharge(int pos, int id, float remainingBattery)
        : pos(pos), id(id), remainingBattery(remainingBattery){};
};

class EvRoute{
public:
    ///---------- info -------------///
    int size() const;
    float getDemand() const;
    float getMinDemand() const;
    float getMaxDemand() const;
    float getRemainingBatteryBefore(int i) const;
    float getCurrentCapacity() const;

    EvRoute(float evBattery, float evCapacity, int satelite, int RouteSizeMax);
    // float getBatteryAt(int pos, const Instance& Inst) const; // REPLACED WITH getBatteryBefore

    float getInitialCapacity() const;
    float getInitialBattery() const;

    float getCost(const Instance& Inst) const;
    float getDemandOf(int i, const Instance& Inst) const;
    //bool insert(int node, int pos, const Instance& Inst);

    // --- MODIFIERS --- //
    bool insert(Insertion& insertion, const Instance& Inst);
    void replace(int pos, int node, float distance, const Instance& Inst); // replaces route at pos;

    void print() const;
    bool canInsert(int node, const Instance &Instance, Insertion &insertion) const;
    bool rechargingS_inUse(int id) const;
    bool isRechargingS(int pos, const Instance& Inst) const;
    int getNodeAt(int pos) const;
    void print(std::string &str) const;
    bool checkRoute(std::string &erro, const Instance &Inst) const;
    std::vector<int> route;
    int satelite = 0.0;
    float distance = 0.0;
    int routeSize = 2;
    std::vector<float> vetRemainingBattery;
    void setAuxStructures(const Instance& Inst);

//private:
    float totalDemand = 0.0;
    float initialCapacity = 0.0;
    float initialBattery = 0.0;
    std::list<std::pair<int,int>> rechargingStationsPos_Rs;     // Armazena tuplas com (posicao de route; recharging station id)
    std::list<Recharge> recharges;     // armazena as estacoes de recarga, com a bateria disponivel ate elas;
    std::vector<bool> rechargingStationRoute;                   // Indica se a posicao eh uma estacao de recarga
    int routeSizeMax = -1;
    float minDemand = 1e8;
    float maxDemand = 0;
    /** Updates the recharges list
     *
     * @param pos posicao alterada, seja uma insercao nessa posicao ou alguma troca (swap)
     * @param distance distancia a mais (ou a menos) comparada ahh anterior
     * @param isInsertion se for insercao. Nesse caso a posicao das estacoes de recarga a frente teram que aumentar em 1.
     */
    void updateRecharge(int pos, float distance, bool isInsertion);
    void addRechargeToList(int pos, int rsId, const Instance& Inst);
    void addRechargeToList(int pos, int rsId, float remainingBattery);
    bool checkOutOfBounds(int pos, bool notSatellite=false) const;
    float getDistance(int pos0, int pos1, const Instance& Inst);
};
#endif
