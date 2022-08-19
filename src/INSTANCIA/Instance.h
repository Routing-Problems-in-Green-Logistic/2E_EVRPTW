// Considerando dist(i,j) == dist(j,i) !!

#ifndef INSTANCE_H
#define INSTANCE_H

#include <iostream>
#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include "../Auxiliary.h"
#include "../common.h"
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

// Guarda a distacia do cliente para um satelite
struct DistSatelite
{
   int satelite = -1;
   double dist  = 0.0;

   bool operator < (const DistSatelite &outro) const
   {
       return dist < outro.dist;
   }

};

class Instance{
public:
    Instance(const std::string &file, const std::string &nome_);
    Instance()=default;
    ~Instance();
    double getDemand(int node) const; // the const means that the method promises not to alter any members of the class.
    double getDistance(int n1, int n2) const;

    std::pair<float,float> getCoordinate(int node) const;
    static int getDepotIndex() {return 0;}
    int getFirstClientIndex() const;
    int getEndClientIndex() const;
    int getFirstRS_index() const;
    int getEndRS_index() const;
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


    // Considerando dist(i,j) == dist(j,i) !!
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

    // Guarda o menor caminho de cada cliente para todos os sat; tam: numSat+1; shortestPath EH VETOR !! VETOR !!
    ShortestPathSatCli *shortestPath = nullptr;

    // Guarda a distancia ate o satelite de forma ordenada para cada cliete
    vector<vector<DistSatelite>> vetVetDistClienteSatelite;

    // Guarda para cada par (sat, RS)  se eh possivel voltar ao sat
    vector<int8_t> vetVoltaRS_sat;

    int getIndiceVetVoltaRS_sat(int sat, int rs);

    string nome;

    void calculaVetVoltaRS_sat();

};


class Parametros
{

public:

    int numIteGrasp                     = 0;         // Numero de itracoes para o grasp
    int iteracoesCalProb                = 50;        // Numero de iteracoes para calcular a prob de inserir uma rota: sat ... EST ... CLIENTE ... EST ... sat
    std::vector<float> vetAlfa;
    int numAtualProbReativo             = 50;        // Numero de iteracoes para atualizar a probabilidade do reativo
    int numMaxClie                      = 0;         // Numero maximo de clientes(que NAO conseguem ser inseridos com heur) adicionados a solucao
    float paramFuncSetSatParaCliente    = 0.1;       // Percentual de sat a ser considerados na funcao GreedyAlgNS::setSatParaCliente
    Parametros()=default;
    Parametros(int numIteGrasp_, int iteracoesCalProb_, const std::vector<float> &vetAlfa_, int numAtualProbReativo_, int numMaxClie_, float paramFuncSetSatParaCliente_)
    {
        numIteGrasp                 = numIteGrasp_;
        iteracoesCalProb            = iteracoesCalProb_;
        vetAlfa                     = vetAlfa_;
        numAtualProbReativo         = numAtualProbReativo_;
        numMaxClie                  = numMaxClie_;
        paramFuncSetSatParaCliente  = paramFuncSetSatParaCliente_;
    }
};

#endif
