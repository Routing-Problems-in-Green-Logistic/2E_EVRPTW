// Considerando dist(i,j) == dist(j,i) !!

#include <cmath>
#include "Instancia.h"
#include <fstream>

#include "../Auxiliary.h"
#include "../CONSTRUTIVO/Construtivo.h"
#include "../PreProcessamento.h"

using namespace std;

// Getters and Setters
double Instancia::getDemand(int node) const {
    return vectCliente[node].demanda;
}

double Instancia::getTruckCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

double Instancia::getEvCap(const int id) const {
    return vectVeiculo[id].capacidade;
}

double Instancia::getEvBattery(const int id) const {

    if(vectVeiculo[id].capacidadeBateria < 0.0)
    {
        cout<<"INDICE NAO EH DE VEIC EV\nFILE: " << string(__FILE__) << "\nLINHA: " << to_string(__LINE__)<<"\n\n";
        throw "ERRO";
    }
    else
        return vectVeiculo[id].capacidadeBateria;
}

double Instancia::getEvTaxaConsumo(const int id) const
{
    if(vectVeiculo[id].taxaConsumoDist < 0.0)
    {

        cout<<"INDICE NAO EH DE VEIC EV\nFILE: " << string(__FILE__) << "\nLINHA: " << to_string(__LINE__);
        throw "ERRO";
    }
    else
        return vectVeiculo[id].taxaConsumoDist;
}

int Instancia::getNSats() const {
    return numSats;
}

int Instancia::getNClients() const {
    return numClients;
}

int Instancia::getN_RechargingS() const {
    return numRechargingS;
}


int Instancia::getN_Evs() const
{
    return numEv;
}

int Instancia::getN_Trucks() const
{

    return numTruck;
}

double Instancia::getDistance(int n1, int n2) const {

    return matDist(n1,n2);
}

int Instancia::getFirstClientIndex() const {
    return 1 + numSats + numRechargingS;

}
int Instancia::getEndClientIndex() const
{
    return getFirstClientIndex()+getNClients()-1;
}

int Instancia::getFirstRS_index() const {
    return 1 + numSats;
}

int Instancia::getEndRS_index() const
{
    return getFirstRS_index() + numRechargingS - 1;
}

int Instancia::getFirstSatIndex() const {
    return 1;
}

int Instancia::getEndSatIndex() const
{
    return numSats;
}

bool Instancia::isClient(int node) const {
    return node >= this->getFirstClientIndex() && node < (this->getFirstClientIndex() + this->getNClients());
}

bool Instancia::isRechargingStation(int node) const {
    return node >= this->getFirstRS_index() && node < this->getFirstRS_index() +
                                                      this->getN_RechargingS();
}

bool Instancia::isSatelite(int node) const {
    return node >= this->getFirstSatIndex() && node < this->getFirstSatIndex() + this->getNSats();
}

bool Instancia::isDepot(int node) const {
    return node == 0;
}

int Instancia::getNNodes() const {
    return numNos;
}


Instancia::Instancia(const std::string &str, const std::string &nome_)
{
    nome = nome_;

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

        if(!isClient(i))
            tw_f = DOUBLE_INF;

        vectCliente.push_back({x, y, dem, tw_i, tw_f, serv});

    }

    // Cria matriz de distancia
    matDist = Matrix<double>(numNos, numNos);

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

    numEstacoesPorArco = min(numRechargingS, NUM_MAX_EST_POR_ARC);

    int tamMat = 1 + numClients + numRechargingS + numSats;
    matEstacao = Matrix<int*>(tamMat, tamMat, nullptr);

    Vector<EstMaisProx> vetEstMaisProx(numRechargingS);

    for(int i= getFirstRS_index(); i <= getEndClientIndex(); ++i)
    {
        for(int j=getFirstSatIndex(); j < i; ++j)
        {
            int prox = 0;
            for(int est= getFirstRS_index(); est <= getEndRS_index(); ++est)
            {
                if(i==est || j==est)
                    continue;

                EstMaisProx *estMaisProx = &vetEstMaisProx[prox];

                estMaisProx->clienteI = i;
                estMaisProx->clienteJ = j;
                estMaisProx->est      = est;
                estMaisProx->dist     = matDist(i, est) + matDist(est, j);
                prox += 1;

            }

            std::sort(vetEstMaisProx.begin(), vetEstMaisProx.end());
            matEstacao(i,j) = new int[numEstacoesPorArco];

            int est;
            for(est=0; est < numEstacoesPorArco; ++est)
            {
                if(vetEstMaisProx[est].est == -1)
                {
                    for(int aux=est; aux < numEstacoesPorArco; ++aux)
                        matEstacao(i,j)[aux] = -1;
                    break;
                }

                matEstacao(i,j)[est] = vetEstMaisProx[est].est;

                //cout<<vetEstMaisProx[est].est<<", "<<vetEstMaisProx[est].dist<<"\t";
            }

        }

        matEstacao(i,i)= nullptr;
    }

    penalizacaoDistEv   = calculaPenalizacaoDistEv();
    penalizacaoDistComb = calculaPenalizacaoDistComb();
    vetTempoSaida = Vector<double>(1+numSats, 0.0);

    for(int i=1; i <= getEndSatIndex(); ++i)
        vetTempoSaida[i] = getDistance(0, i);

    vetVetDistClienteSatelite = vector<vector<DistSatelite>>(numNos);

    for(int i=getFirstClientIndex(); i <= getEndClientIndex(); ++i)
    {
        vector<DistSatelite> &vetDistClienteSat = vetVetDistClienteSatelite[i];

        vetDistClienteSat = vector<DistSatelite>(numSats+1);
        vetDistClienteSat[numSats].satelite = 0;
        vetDistClienteSat[numSats].dist     = DOUBLE_INF;

        for(int s=getFirstSatIndex(); s <= getEndSatIndex(); ++s)
        {
            vetDistClienteSat[s-1].satelite = s;
            vetDistClienteSat[s-1].dist = getDistance(s, i);
        }

        std::sort(vetDistClienteSat.begin(), vetDistClienteSat.end());
    }

}

void Instancia::calculaVetVoltaRS_sat()
{

    const int tamVet = (numSats)*numRechargingS;
    vetVoltaRS_sat = vector<int8_t>(tamVet, int8_t(0));

    for(int sat=getFirstSatIndex(); sat <= getEndSatIndex(); ++sat)
    {
        for(int rs= getFirstRS_index(); rs <= getEndRS_index(); ++rs)
        {
            const double consumoBat = getDistance(rs, sat)* getEvTaxaConsumo(getFirstEvIndex());
            const double capBat     = vectVeiculo[getFirstEvIndex()].capacidadeBateria;

            if(consumoBat <= capBat)
            {
                int id = getIndiceVetVoltaRS_sat(sat, rs);
                //cout<<"sat, rs: "<<sat<<", "<<rs<<"\n";

                vetVoltaRS_sat.at(id) = 1;

                //cout<<"sat: "<<sat<<"; RS: "<<rs<<"; dist: "<<consumoBat<<"\n";
            }
        }
    }

    vetVetDistSatSat.resize(numSats+1);
    for(int sat=getFirstSatIndex(); sat <= getEndSatIndex(); ++sat)
    {
        Vector<DistSatelite> &vetDistSat = vetVetDistSatSat[sat];
        vetDistSat.resize(numSats-1);
        //cout<<"Sat("<<sat<<"): ";

        int prox = 0;
        for(int satOutro=getFirstSatIndex(); satOutro <= getEndSatIndex(); ++satOutro)
        {
            if(satOutro != sat)
            {
                vetDistSat[prox].satelite = satOutro;
                vetDistSat[prox].dist = getDistance(sat, satOutro);
                prox += 1;
            }
        }

        std::sort(vetDistSat.begin(), vetDistSat.end());
/*        cout<<"Sat("<<sat<<"): ";
        for(auto it:vetDistSat)
            cout<<"("<<it.satelite<<"; "<<it.dist<<") ";
        cout<<"\n";*/
    }

}

Instancia::~Instancia()
{

    for(int i= getFirstRS_index(); i <= getEndClientIndex(); ++i)
    {
        for(int j = getFirstSatIndex(); j < i; ++j)
        {
            delete []matEstacao(i, j);
            matEstacao(i,j) = nullptr;
        }
    }

    if(shortestPath != nullptr)
        delete []shortestPath;
}

int Instancia::getIndiceVetVoltaRS_sat(int sat, int rs)
{
    return (sat-1)*numRechargingS + (rs-getFirstRS_index());
}

void Instancia::print() const
{
    cout<<"INSTANCIA "<<nome<<":\n";
    cout<<"\tnum satelites: "<<numSats<<"\n";
    cout<<"\tnum estacoes de recarga: "<<numRechargingS<<"\n";
    cout<<"\tnum clientes: "<<numClients<<"\n";

    cout<<"\tnum veiculos a combustao: "<<numTruck<<"\n";
    cout<<"\tnum veiculos eletricos: "<<numEv<<"\n";
    cout<<"\n\tbateria veiculos eletricos: ";

    for(int i=getFirstEvIndex(); i <= getEndEvIndex(); ++i)
        cout<<i<<": "<< vectVeiculo[i].capacidadeBateria<<" ";

    cout<<"\n\n";

    cout<<"SATELITE ID \t TW INICIO \t TW FIM \t COORD X \t COORD Y\n\n";
    for(int i=getFirstSatIndex(); i <= getEndSatIndex(); ++i)
    {
        const ClienteInst &clienteInst = vectCliente[i];
        cout<<"\t"<<i<< " \t " <<clienteInst.inicioJanelaTempo<<" \t\t "<<clienteInst.fimJanelaTempo<<" \t\t "<<clienteInst.coordX<<" \t\t "<<clienteInst.coordY<<"\n";
    }

    cout<<"\nCLIENTE ID \t TW INICIO \t TW FIM \t TEMPO SERV \t DEM \t COORD X \t COORD Y\n\n";
    for(int i=getFirstClientIndex(); i <= getEndClientIndex(); ++i)
    {
        const ClienteInst &clienteInst = vectCliente[i];
        cout<< "\t"<< i << " \t " <<clienteInst.inicioJanelaTempo<<" \t\t "<<clienteInst.fimJanelaTempo<<" \t\t "<<clienteInst.tempoServico<<" \t\t "<<clienteInst.demanda<<
        " \t\t "<<clienteInst.coordX<<" \t\t "<<clienteInst.coordY<<"\n";
    }

    cout<<"\nESTACAO ID \t COORD X \t COORD Y \t TW INICIO \t TW FIM \n\n";

    for(int i= getFirstRS_index(); i <= getEndRS_index(); ++i)
    {
        const ClienteInst &clienteInst = vectCliente[i];

        cout<<"\t"<<i<<" \t "<<clienteInst.coordX<<" \t\t "<<clienteInst.coordY<<" \t\t "<<clienteInst.inicioJanelaTempo<<" \t\t "<<clienteInst.fimJanelaTempo<<"\n";
    }

    cout<<"\n\n";

    cout<<"DISTANCIA DEPOSITO <SATELITE>\n\n";

    const int dep = getDepotIndex();
    for(int i=getFirstSatIndex(); i <= getEndSatIndex(); ++i)
        cout<<"\tSAT ID: "<<i<<"\t"<<getDistance(dep, i);

    cout<<"\n\n";
}

int* Instancia::getEstacoes(const int clienteI, const int clienteJ)
{
    if(clienteI < clienteJ)
        return matEstacao(clienteJ, clienteI);
    else
        return matEstacao(clienteI, clienteJ);
}

double Instancia::calculaPenalizacaoDistEv()
{
    /*
    double dist = 0.0;
    for(int sat=getFirstSatIndex(); sat <= getEndSatIndex(); ++sat)
    {
        for(int j=getFirstClientIndex(); j <= getEndClientIndex(); ++j)
            dist += 2.0* getDistance(sat, j);
    }
    */


    double dist = -DOUBLE_MAX;

    for(int i=0; i < numNos; ++i)
    {
        for(int j=i+1; j < numNos; ++j)
        {
            const double tempDist = getDistance(i,j);
            if(tempDist > dist)
                dist = tempDist;
        }
    }

    return 2*dist;
}

double Instancia::calculaPenalizacaoDistComb()
{

    double dist = 0.0;
    int dep = getDepotIndex();

    for(int sat=getFirstSatIndex(); sat <= getEndSatIndex(); ++sat)
    {
        dist += 2.0* getDistance(dep, sat);
    }

    return dist*1.2;
}

// true se tempoCheg <= fim janela de tempo, false caso contrario
bool Instancia::verificaJanelaTempo(double tempoCheg, int cliente) const
{
//    cout<<"\t\tFIM JANELA TEMPO("<<cliente<<"): "<<vectCliente[cliente].fimJanelaTempo<<"\n";
    return ((tempoCheg <= vectCliente[cliente].fimJanelaTempo) || (abs(tempoCheg - vectCliente[cliente].fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO));
}
