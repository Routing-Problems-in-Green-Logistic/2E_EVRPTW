#include <cmath>
#include "Instance.h"
#include <fstream>

using namespace std;

/*
Instance::Instance(std::vector<std::vector<double>> &distMat, float truckCap, float evCap, float evBattery, int nSats,
                   int nClients, int nRS, std::vector<std::pair<float, float>> &coordinates,
                   std::vector<float> &demands) {
    this->demands = demands;
    this->coordinates = coordinates;
    this->distMat = distMat;
    this->truckCap = truckCap;
    this->evCap = evCap;
    this->evBattery = evBattery;
    this->nRechargingS = nRS;
    this->nClients = nClients;
    this->nSats = nSats;
    this->evCost = 0;
    this->truckCost = 0;


    float sumDemands = 0.0;
    for(auto demand:demands)
        sumDemands += demand;

    int div = int(ceil(sumDemands/evCap));
    numEv = div + int(ceil(div * 0.1));

    div =  int(ceil(sumDemands/truckCap));
    numTruck = div + int(ceil(div * 0.2));
}
*/



// Getters and Setters
float Instance::getDemand(int node) const {
    return vectCliente[node].demanda;
}

float Instance::getTruckCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

float Instance::getEvCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

float Instance::getEvBattery(const int id) const {
    return vectVeiculo[id].capacidadeBateria;
}

int Instance::getNSats() const {
    return numSats;
}

int Instance::getNClients() const {
    return numClients;
}

int Instance::getN_RechargingS() const {
    return numRechargingS;
}


int Instance::getN_Evs() const
{
    return numEv;
}

int Instance::getN_Trucks() const
{

    return numTruck;
}

float Instance::getDistance(int n1, int n2) const {
    if(n1==n2)
        return 0.0;

    return matDist(n1,n2);
}

int Instance::getFirstClientIndex() const {
    return 1 + numSats + numRechargingS;

}
int Instance::getEndClientIndex() const
{
    return getFirstClientIndex()+getNClients()-1;
}

int Instance::getFirstRechargingSIndex() const {
    return 1 + numSats;
}

int Instance::getEndRechargingSIndex() const
{
    return getFirstRechargingSIndex() + numRechargingS  - 1;
}

int Instance::getFirstSatIndex() const {
    return 1;
}

int Instance::getEndSatIndex() const
{
    return 1 + numSats - 1;
}

bool Instance::isClient(int node) const {
    return node >= this->getFirstClientIndex() && node < (this->getFirstClientIndex() + this->getNClients());
}

bool Instance::isRechargingStation(int node) const {
    return node >= this->getFirstRechargingSIndex() && node < this->getFirstRechargingSIndex() +
                                                                      this->getN_RechargingS();
}

bool Instance::isSatelite(int node) const {
    return node >= this->getFirstSatIndex() && node < this->getFirstSatIndex() + this->getNSats();
}

bool Instance::isDepot(int node) const {
    return node == 0;
}

int Instance::getNNodes() const {
    return numNos;
}


Instance::Instance(const std::string &str)
{

    std::ifstream file;
    file.open(str);

    if(!file.is_open())
    {
        std::cout<<"ERRO, NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<str<<"\n\n";
        throw "ERRO";
    }

    int a;
    file>>numTruck>>numEv>>a>>numSats>>numRechargingS>>numClients;

    if(numTruck <= 0 || numEv <= 0 || numSats <= 0 || numRechargingS <= 0 || numClients <= 0)
    {
        std::cout<<"ERRO, INSTANCIA ESTA ERRADA. 1° LINHA\n\n";
        throw "ERRO";
    }

    //cout<<"numTruck: "<<numTruck<<"; numEv: "<<numEv<<"; numSat: "<<numSats<<"; numRechargingS: "<<numRechargingS<<"; numClientes: "<<numClients<<"\n\n";

    numNos = numSats + numRechargingS + numClients + 1;

    string lixo;

    getline(file, lixo);

    vectVeiculo.reserve(numTruck+numEv);

    for(int i=0; i < numTruck; ++i)
    {
        float cap=0;
        file>>cap;
        getline(file, lixo);
        vectVeiculo.emplace_back(cap);
        //cout<<"cap: "<<cap<<"\n";
    }
    //cout<<"\n";

    for(int i=0; i < numEv; ++i)
    {
        float cap, capBat, taxaR, taxaC, temp;
        file>>cap>>temp>>temp>>capBat>>taxaR>>taxaC;
        getline(file, lixo);
        vectVeiculo.emplace_back(cap, capBat, taxaR, taxaC);

        //cout<<"cap: "<<cap<<"; capBat: "<<capBat<<"; taxaR: "<<taxaR<<"; taxaC: "<<taxaC<<"\n";

    }

    //cout<<"\n";

    vectCliente.reserve(numNos);

    for(int i=0; i < numNos; ++i)
    {
        float x,y, dem, temp, tw_i, tw_f, serv;

        file>>x>>y>>dem>>temp>>temp>>tw_i>>tw_f>>serv;
        getline(file, lixo);

        //cout<<"x: "<<x<<"; y: "<<y<<"; dem: "<<dem<<"; tw_i: "<<tw_i<<"; tw_f: "<<tw_f<<"; serv: "<<serv<<"\n";

        vectCliente.push_back({x, y, dem, tw_i, tw_f, serv});


    }

    // Cria matriz de distancia

    matDist.resize(numNos, numNos, false);

    for(int i=0; i < numNos; ++i)
    {
        matDist(i,i) = 0.0;

        for(int j=i+1; j < numNos; ++j)
        {
            float dist = sqrtf(pow(vectCliente[i].coordX - vectCliente[j].coordX,2) + pow(vectCliente[i].coordY - vectCliente[j].coordY,2));
            matDist(i,j) = matDist(j,i) = dist;
        }
    }

}