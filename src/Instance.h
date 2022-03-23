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
    const float taxaConsumoDist;

    explicit VeiculoInst(float _cap):eletrico(false), capacidade(_cap), capacidadeBateria(0.0), taxaRecarga(0.0), taxaConsumoDist(0.0){}
    VeiculoInst(float _cap, float _capBat, float _taxaR, float _taxaC):eletrico(true), capacidade(_cap), capacidadeBateria(_capBat), taxaRecarga(_taxaR), taxaConsumoDist(_taxaC){}

};

struct ClienteInst
{

    const float coordX;
    const float coordY;
    const float demanda;
    const float inicioJanelaTempo;
    const float fimJanelaTempo;
    const float tempoServico;


};

class Instance{
public:
    Instance(const std::string &file);
    float getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
    double getDistance(int n1, int n2) const;

    std::pair<float,float> getCoordinate(int node) const;
    static int getDepotIndex() {return 0;}
    int getFirstClientIndex() const;
    int getEndClientIndex() const;
    int getFirstRechargingSIndex() const;
    int getEndRechargingSIndex() const;
    int getFirstSatIndex() const;
    int getEndSatIndex() const;

    int getFirstTruckIndex() const;
    int getEndTruckIndex() const;


    int getFirstEvIndex() const;
    int getEndEvIndex() const;

    float getTruckCap(const int id) const;
    float getEvCap(const int id) const;
    float getEvBattery(const int id) const;
    int getNSats() const;
    int getNClients() const;

    int getN_RechargingS() const;
    int getN_Evs() const;
    int getN_Trucks() const;

    bool isClient(int node) const;
    bool isRechargingStation(int node) const;
    bool isSatelite(int node) const;
    bool isDepot(int node) const;

    int getNNodes() const;
    const ClienteInst& getClient(int clientId) const { return this->vectCliente.at(clientId);}


//private:
    ublas::matrix<double> matDist;

    std::vector<VeiculoInst> vectVeiculo;           // veic a combustao + veic EV
    std::vector<ClienteInst> vectCliente;           // deposito, satellites, estacoes e clientes

    int numSats, numClients, numRechargingS, numEv, numTruck;
    int numNos; // deposito + numSats + numRechargingS + numClients
    const int numUtilEstacao = 3;



};

#endif
