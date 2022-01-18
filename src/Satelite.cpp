#include "Satelite.h"

Satelite::Satelite(int id, const Instance& Inst)
{
    this->evCapacity = Inst.getEvCap();
    this->evBattery = Inst.getEvBattery();
    this->position= {0,0};
    this->id = id;
    this->demand= 0;

    const int max = 2+Inst.getNClients()+Inst.getN_RechargingS();

    vetEvRoute.reserve(Inst.getN_Evs());
    tamVetEvRoute = Inst.getN_Evs();

    for(int i=0; i < Inst.getN_Evs(); ++i)
    {
        vetEvRoute.emplace_back(this->evBattery, this->evCapacity, this->id, max);
    }

}

float Satelite::getDemand(const Instance& Inst) const { // O(n)
    float totDemand = 0;
    for(const auto& route : this->vetEvRoute){
        totDemand += route.getDemand();
    }
    return totDemand;
}

int Satelite::getNRoutes() const {
    return this->vetEvRoute.size();
}

EvRoute &Satelite::getRoute(int i) {
    return this->vetEvRoute.at(i);
}

bool Satelite::checkSatellite(std::string &erro, const Instance &Inst)
{
    // Verifica os satellite
    for(EvRoute &evRoute:vetEvRoute)
    {
        if(this->id !=  evRoute.satelite)
        {
            erro += "ERRO, SATELLITE NA ROTA EH DIFERENTE NO SATELLITE. ID SATELLITE: "+ std::to_string(id) + " != ID SATELLITE ROTA: "+std::to_string(evRoute.satelite);
            return false;
        }

        if(!evRoute.checkRoute(erro, Inst))
            return false;
    }

    return true;
}

void Satelite::print(std::string &str)
{
    str += "SATELLITE ID: "+ std::to_string(id)+"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        str += "\tROTA ID: "+ std::to_string(i)+".:  ";
        evRoute.print(str);
        str+= "\n";
    }

    str += "\n\n";
}

void Satelite::print()
{

    std::cout<<"SATELLITE ID: "<<id<<"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        std::cout<<"\tROTA ID: "<<i<<".:  ";
        evRoute.print();
        std::cout<<"\n";
    }

    std::cout<<"\n\n";
}
int Satelite::getId() const {
    return id;
}
