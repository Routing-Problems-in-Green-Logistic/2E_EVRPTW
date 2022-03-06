#include <cfloat>
#include "EvRoute.h"
#include "Auxiliary.h"
#include "ViabilizadorRotaEv.h"
#include "greedyAlgorithm.h"

using namespace NS_Auxiliary;
using namespace std;
using namespace NameViabRotaEv;
using namespace GreedyAlgNS;

EvRoute::EvRoute(float evBattery, float evCapacity, int satelite, const int RouteSizeMax) {
    this->satelite = satelite;

    route.reserve(RouteSizeMax);
    route.push_back(satelite);
    route.push_back(satelite);

    for(int i=2; i < RouteSizeMax; ++i) {
        route.push_back(-1);
    }

    vetRemainingBattery.reserve(RouteSizeMax);

    for(int i=0; i < RouteSizeMax; ++i)
        vetRemainingBattery.push_back(evBattery);


    routeSize = 2;
    routeSizeMax = RouteSizeMax;
    distance = 0.0;
    totalDemand = 0.0;

}

EvRoute::EvRoute(const Instance &instance)
{

    const int max = 2+instance.getNClients()+instance.getN_RechargingS();

    route.reserve(max);
    route.push_back(-1);
    route.push_back(-1);

    for(int i=2; i < max; ++i)
        route.push_back(-1);

    vetRemainingBattery.reserve(max);

    for(int i=0; i < max; ++i)
        vetRemainingBattery.push_back(instance.getEvBattery());


    routeSize = 2;
    routeSizeMax = max;
    distance = 0.0;
    totalDemand = 0.0;
}

int EvRoute::size() const{
    return routeSize;
}
float EvRoute::getDemand() const{
    return totalDemand;
}

void EvRoute::print(const Instance &instance) const
{
    for(int i=0; i < routeSize; ++i)
    {
        if(instance.isRechargingStation(route[i]))
            std::cout<<"RS("<<route[i]<<")_"<<vetRemainingBattery[i]<<"  ";
        else
            std::cout<<"("<<route[i]<<")_"<<vetRemainingBattery[i]<<"  ";
    }
    std::cout <<"\nDistance: "<<distance<<"\n\n";
}
float EvRoute::getMinDemand() const {
    if(this->size() <= 2){
        return 0;
    }
    return minDemand;
}

float EvRoute::getMaxDemand() const {
    return maxDemand;
}

bool EvRoute::checkRoute(std::string &erro, const Instance &Inst) const
{

    if(routeSize < 2)
        return false;

    // Verifica se a rota eh vazia
    if(routeSize == 2)
    {
        if((route[0]==satelite) && (route[1]==satelite) && totalDemand == 0.0)
            return true;
        else
        {
            if(!((route[0]==satelite) && (route[1]==satelite)))
                erro += "ERRO SATELLITE "+ std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: "+
                                                                  std::to_string(route[0]) + " "+ std::to_string(route[1])+"\n\n";
            else
                erro += "ERRO SATELLITE "+ std::to_string(satelite) + "\nDEMANDA ("+ to_string(totalDemand)+") DEVERIA SER ZERO, ROTA VAZIA\n\n";

            return false;
        }

    }

    // Verifica se o inicio e o final da rota sao iguais ao satellite
    if(!((route[0]==satelite) && (route[routeSize-1]==satelite)))
    {
        erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: ";
        print(erro, Inst);
        return false;
    }

    float battery = Inst.getEvBattery();
    float distanceAux = 0.0;
    float demanda = 0.0;

    // Verifica se a bateria no  inicio da rota eh igual a quantidade de bateria
    if(vetRemainingBattery[0] != battery)
    {
        erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA RESTANTE INCORRETO. POSICAO: "+std::to_string(0)+"\nROTA: ";

        print(erro, Inst);
        return false;
    }

    // Percorre todas as posicoes da rota
    for(int i=1; i < routeSize; ++i)
    {


        float aux = Inst.getDistance(route[i-1], route[i]);
        demanda += Inst.getDemand(route[i]);

        distanceAux += aux;
        battery -= aux;


        if(battery < -BATTERY_TOLENCE)
        {
            erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA!  "+" POSICAO: "+
                    std::to_string(i)+"\nROTA: ";

            print(erro, Inst);

            erro += "BATERIA: "+ to_string(battery);

            return false;
        }

        if(Inst.isRechargingStation(route[i]))
        {
            // Verifica se eh possivel chegar a estacao de recarga
            if(battery < -BATTERY_TOLENCE)
            {
                erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA EH INSUFICIENTE PARA CHEGAR NA ESTACAO DE RECARGA NA "+" POSICAO: "+
                        std::to_string(i)+"\nROTA: ";

                print(erro, Inst);

                erro += "BATERIA: "+ to_string(battery);

                return false;
            }

            battery = Inst.getEvBattery();
        }

    }


    if(abs(demanda - totalDemand) > DEMAND_TOLENCE)
    {
        string erroPrint;
        print(erroPrint, Inst);

        erro += "ERRO, SATELLITE: "+ to_string(satelite)+", ROTA: "+ erroPrint+"; DEMANDA ESTA ERRADA, DEMANDA ROTA: "+
                to_string(totalDemand)+", DEMANDA CALCULADA: "+to_string(demanda)+"\n\n";

        for(int j = 0; j < routeSize; ++j)
        {
            cout<<route[j]<<": "<<Inst.getDemand(route[j])<<"\n";
        }
        cout<<"\n";

        return false;

    }

    return true;
}

void EvRoute::print(std::string &str, const Instance &instance) const
{
    if(routeSize > 2)
    {
        for(int i = 0; i < routeSize; ++i)
        {
            if(instance.isRechargingStation(route[i]))
                str += "RS(" + std::to_string(route[i]) + ")_" + std::to_string(vetRemainingBattery[i]) + "  ";
            else
                str += "("+std::to_string(route[i]) + ")_" + std::to_string(vetRemainingBattery[i]) + "  ";
        }

        str += "\n";
    }
}


float EvRoute::getDemandOf(int i, const Instance &Inst) const {
    return Inst.getDemand(this->route.at(i));
}

int EvRoute::getNodeAt(int pos) const {
    return this->route.at(pos);
}

void EvRoute::setAuxStructures(const Instance& Inst) {
    float remainingBattery = Inst.getEvBattery();
    this->maxDemand = 0;
    this->minDemand = FLOAT_MAX;

    for(int i = 1; i < this->size()-1; i++){
        remainingBattery -= Inst.getDistance(route[i-1], route[i]);
        if(Inst.isRechargingStation(route[i])){
            //addRechargeToList(i, route.at(i), remainingBattery);
            remainingBattery = Inst.getEvBattery();
        }
        else{
            float currentDemand = getDemandOf(i, Inst);
            if(currentDemand > maxDemand){
                maxDemand = currentDemand;
            }
            if(currentDemand < minDemand){
                minDemand = currentDemand;
            }
        }
    }
    int i = size()-1;
    remainingBattery -= Inst.getDistance(route[i-1], route[i]);
    // add satelite at the end of list of recharges;
//    addRechargeToList(i, satelite, remainingBattery);
}