#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
using namespace boost::numeric;

struct VeiculoInst
{
    const bool  eletrico;
    const float capacidade;
    const float capacidadeBateria;
    const float taxaRecarga;


};

struct ClienteInst
{

    const float coordX;
    const float coordY;
    const int   demanda;
    const float inicioJanelaTempo;
    const float fimJanelaTempo;
    const float tempoServico;

};

class Instance{
public:
    Instance(std::vector<std::vector<double>>& distMat, float truckCap, float evCap, float evBattery,
             int nSats, int nClients, int nRS, std::vector<std::pair<float,float>>& coordinates, std::vector<float>& demands);
    float getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
    float getDistance(int n1, int n2) const;

    std::pair<float,float> getCoordinate(int node) const;
    int getFirstClientIndex() const;
    int getEndClientIndex() const;
    int getFirstRechargingSIndex() const;
    int getEndRechargingSIndex() const;
    int getFirstSatIndex() const;
    int getEndSatIndex() const;

    float getTruckCap(const int i) const;
    float getEvCap(const int i) const;
    float getEvBattery() const;
    int getNSats() const;
    int getNClients() const;
    /** gets the number of Recharging Stations.
     * @return number of Recharging Stations
     */
    int getN_RechargingS() const;
    int getN_Evs() const;
    int getN_Trucks() const;

    bool isClient(int node) const;
    bool isRechargingStation(int node) const;
    bool isSatelite(int node) const;
    bool isDepot(int node) const;

    float getEvCost() const;

    float getTruckCost() const;
    int getNNodes() const;

//private:
    ublas::matrix<double> matDist;

    std::vector<VeiculoInst> vectVeiculo;
    std::vector<ClienteInst> vectCliente;

    int nSats, nClients, nRechargingS;

    int numEv;
    int numTruck;
    const int numUtilEstacao = 3;

};

#endif
