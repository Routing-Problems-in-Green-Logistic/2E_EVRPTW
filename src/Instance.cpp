#include <cmath>
#include "Instance.h"
#include <fstream>
#include <boost/format.hpp>
#include "Auxiliary.h"

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
double Instance::getDemand(int node) const {
    return vectCliente[node].demanda;
}

double Instance::getTruckCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

double Instance::getEvCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

double Instance::getEvBattery(const int id) const {

    if(vectVeiculo[id].capacidadeBateria < 0.0)
    {
        cout<<"INDICE NAO EH DE VEIC EV\nFILE: " << string(__FILE__) << "\nLINHA: " << to_string(__LINE__)<<"\n\n";
        throw "ERRO";
    }
    else
        return vectVeiculo[id].capacidadeBateria;
}

double Instance::getEvTaxaConsumo(const int id) const
{
    if(vectVeiculo[id].taxaConsumoDist < 0.0)
    {

        cout<<"INDICE NAO EH DE VEIC EV\nFILE: " << string(__FILE__) << "\nLINHA: " << to_string(__LINE__);
        throw "ERRO";
    }
    else
        return vectVeiculo[id].taxaConsumoDist;
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

double Instance::getDistance(int n1, int n2) const {
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
    return numSats;
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

    }

    for(int i=0; i < numEv; ++i)
    {
        float cap, capBat, taxaR, taxaC, temp;
        file>>cap>>temp>>temp>>capBat>>taxaR>>taxaC;
        getline(file, lixo);
        vectVeiculo.emplace_back(cap, capBat, taxaR, taxaC);

    }

    vectCliente.reserve(numNos);

    for(int i=0; i < numNos; ++i)
    {
        double x,y, dem, temp, tw_i, tw_f, serv;

        file>>x>>y>>dem>>temp>>temp>>tw_i>>tw_f>>serv;
        getline(file, lixo);

        vectCliente.push_back({x, y, dem, tw_i, tw_f, serv});


    }

    // Cria matriz de distancia
    matDist.resize(numNos, numNos, false);

/*    cout<<"DEP: "<<getDepotIndex()<<"\nSAT ID INICIO: "<<getFirstSatIndex()<<"\nSAT ID FIM: "<<getEndSatIndex();
    cout<<"\nEST ID INICIO: "<<getFirstRechargingSIndex()<<"\nEST ID FIM: "<<getEndRechargingSIndex()<<"\n";
    cout<<"CLIENTE ID INICIO: "<<getFirstClientIndex()<<"\nCLIENTE ID FIM: "<<getEndClientIndex()<<"\n\n";
*/

    //cout<<"I J DIST(I,J)\n\n";

    for(int i=0; i < numNos; ++i)
    {
        matDist(i,i) = 0.0;

        for(int j=i+1; j < numNos; ++j)
        {
            double dist = sqrt(pow(vectCliente[i].coordX - vectCliente[j].coordX,2) + pow(vectCliente[i].coordY - vectCliente[j].coordY,2));
            matDist(i,j) = matDist(j,i) = dist;
            //cout<<i<<" "<<j<<": "<<dist<<"\n";
        }
    }

    evRouteSizeMax = 2 + numClients + numUtilEstacao*numRechargingS;

    //cout<<"\n";


/*    for(int i=0; i < numNos; ++i)
    {
        for(int j=0; j < numNos; ++j)
        {
            cout<<boost::format("%.1f\t") % matDist(i,j);
        }

        cout<<"\n";
    }*/

}

void Instance::print() const
{
    cout<<"INSTANCIA:\n";
    cout<<"\tnum satelites: "<<numSats<<"\n";
    cout<<"\tnum estacoes de recarga: "<<numRechargingS<<"\n";
    cout<<"\tnum clientes: "<<numClients<<"\n";

    cout<<"\tnum veiculos a combustao: "<<numTruck<<"\n";
    cout<<"\tnum veiculos eletricos: "<<numEv<<"\n";
    cout<<"\n\tbateria veiculos eletricos: ";

    for(int i=getFirstEvIndex(); i <= getEndEvIndex(); ++i)
        cout<<i<<": "<< vectVeiculo[i].capacidadeBateria<<" ";

    cout<<"\n\n";

    cout<<"SATELITE ID \t TW INICIO \t TW FIM\n\n";
    for(int i=getFirstSatIndex(); i <= getEndSatIndex(); ++i)
    {
        const ClienteInst &clienteInst = vectCliente[i];
        cout<<"\t"<<i<< " \t " <<clienteInst.inicioJanelaTempo<<" \t\t "<<clienteInst.fimJanelaTempo<<"\n";
    }

    cout<<"\nCLIENTE ID \t TW INICIO \t TW FIM\n\n";
    for(int i=getFirstClientIndex(); i <= getEndClientIndex(); ++i)
    {
        const ClienteInst &clienteInst = vectCliente[i];
        cout<< "\t"<< i << " \t " <<clienteInst.inicioJanelaTempo<<" \t\t "<<clienteInst.fimJanelaTempo<<"\n";
    }

    cout<<"\n\n";

    cout<<"DISTANCIA DEPOSITO <SATELITE>\n\n";

    const int dep = getDepotIndex();
    for(int i=getFirstSatIndex(); i <= getEndSatIndex(); ++i)
        cout<<"\tSAT ID: "<<i<<"\t"<<getDistance(dep, i);

    cout<<"\n\n";


}

// 1 11 9 12 4 15
//
