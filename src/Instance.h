
#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include "Auxiliary.h"
#include "common.h"
//#include "PreProcessamento.h"

using namespace boost::numeric;

#define TESTE_JANELA_TEMPO(tempoCheg, cliente, instancia) (tempoCheg <= instancia.vectCliente[cliente].fimJanelaTempo) || \
                                                        (abs(tempo - instancia.vectCliente[cliente].fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO)

struct VeiculoInst
{
    const bool  eletrico;
    const double  capacidade;

    const double capacidadeBateria;
    const double taxaRecarga;
    const double taxaConsumoDist;

    explicit VeiculoInst(double _cap):eletrico(false), capacidade(_cap), capacidadeBateria(-1.0), taxaRecarga(-1.0), taxaConsumoDist(-1.0){}
    VeiculoInst(double _cap, double _capBat, double _taxaR, double _taxaC):eletrico(true), capacidade(_cap), capacidadeBateria(_capBat), taxaRecarga(_taxaR), taxaConsumoDist(_taxaC){}

};

struct ClienteInst
{

    const double coordX;
    const double coordY;
    const double demanda;
    const double inicioJanelaTempo;
    const double fimJanelaTempo;
    const double tempoServico;


};

struct EstMaisProx
{
    int clienteI = -1;
    int clienteJ = -1;
    int est      = -1;
    double dist  = DOUBLE_MAX;

    bool operator < (const EstMaisProx& estMaisProx) const
    {
        return dist < estMaisProx.dist;
    }
};

class Instance{
public:
    Instance(const std::string &file);
    Instance()=default;
    ~Instance();
    double getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
    double getDistance(int n1, int n2) const;

    std::pair<float,float> getCoordinate(int node) const;
    static int getDepotIndex() {return 0;}
    int getFirstClientIndex() const;
    int getEndClientIndex() const;
    int getFirstRechargingSIndex() const;
    int getEndRechargingSIndex() const;
    int getFirstSatIndex() const;
    int getEndSatIndex() const;

    int getFirstTruckIndex() const {return 0;}
    int getEndTruckIndex() const {return numTruck-1;}


    int getFirstEvIndex() const {return numTruck;}
    int getEndEvIndex() const {return numTruck + numEv -1;}

    double getTruckCap(const int id) const;
    double getEvCap(const int id) const;
    double getEvBattery(const int id) const;
    double getEvTaxaConsumo(const int id) const;
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

    void print() const;

    double tempoRecarga(const int idRota, const double bat) const
    {
        if(bat > 0)
            return vectVeiculo[idRota].taxaRecarga * (vectVeiculo[idRota].capacidadeBateria-bat);
        else
            return vectVeiculo[idRota].taxaRecarga * vectVeiculo[idRota].capacidadeBateria;
    }

    double getInicioJanelaTempoCliente(int id) const {return vectCliente[id].inicioJanelaTempo;}
    double getFimJanelaTempoCliente(int id) const {return vectCliente[id].fimJanelaTempo;}

    int getEvRouteSizeMax() const {return evRouteSizeMax;}

    int *getEstacoes(const int clienteI, const int clienteJ);

    bool verificaJanelaTempo(double tempoCheg, int cliente) const;

    double calculaPenalizacaoDistEv();
    double calculaPenalizacaoDistComb();


//private:
    ublas::matrix<double> matDist;

    std::vector<VeiculoInst> vectVeiculo;           // veic a combustao + veic EV
    std::vector<ClienteInst> vectCliente;           // deposito, satellites, estacoes e clientes

    int numSats, numClients, numRechargingS, numEv, numTruck;
    int numNos; // deposito + numSats + numRechargingS + numClients
    const int numUtilEstacao = 3;

    int evRouteSizeMax = -1;

    ublas::triangular_matrix<int*, ublas::lower> matEstacao; // matriz eh triangular inferior: i >= j

    int numEstacoesPorArco=-1;
    double penalizacaoDistEv = 0.0;
    double penalizacaoDistComb = 0.0;
    std::vector<double> vetTempoSaida;
    bool bestInsViabRotaEv = false;

    ShortestPathSatCli *shortestPath;

};

#endif
