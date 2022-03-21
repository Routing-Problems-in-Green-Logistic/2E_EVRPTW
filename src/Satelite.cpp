#include "Satelite.h"
#include "Auxiliary.h"



Satelite::Satelite(const Instance& instance, const int _satId)
{

    const int max = 2 + instance.getNClients() + instance.getN_RechargingS();

    vetEvRoute.reserve(instance.getN_Evs());
    tamVetEvRoute = instance.getN_Evs();

    for(int i=0; i < instance.getN_Evs(); ++i)
    {
        const VeiculoInst &veic = instance.vectVeiculo[instance.getFirstEvIndex()+i];
        vetEvRoute.emplace_back(_satId, instance.getFirstEvIndex()+i, max);
    }

    sateliteId = _satId;

}

int Satelite::getNRoutes() const {
    return this->vetEvRoute.size();
}

EvRoute &Satelite::getRoute(int i) {
    return this->vetEvRoute.at(i);
}

bool Satelite::checkSatellite(std::string &erro, const Instance &Inst)
{
    float demandaAux = 0.0;
    double distanciaAux = 0.0;

    // Verifica os satellite
    for(EvRoute &evRoute:vetEvRoute)
    {
        demandaAux += evRoute.getDemand();
        distanciaAux += evRoute.distancia;

        if(sateliteId !=  evRoute.satelite)
        {
            erro += "ERRO, SATELLITE NA ROTA EH DIFERENTE NO SATELLITE. ID SATELLITE: "+ std::to_string(sateliteId) + " != ID SATELLITE ROTA: "+std::to_string(evRoute.satelite);
            return false;
        }

        if(!evRoute.checkRoute(erro, Inst))
            return false;
    }


    if(abs(demandaAux - demanda) > TOLERANCIA_DEMANDA)
    {
        erro += "ERRO NO SATELLITE: " + to_string(sateliteId) + "; SOMATORIO DAS DEMANDAS DOS EV'S(" + to_string(demandaAux) + ") EH DIFERENTE DA DEMANDA DO SATELLITE(" + to_string(demanda) + ");\n";
        return false;
    }

    if(abs(distanciaAux-distancia) > TOLERANCIA_DISTANCIA)
    {
        erro += "ERRO NO SATELLITE: " + to_string(sateliteId) + "; SOMATORIO DAS DISTANCIAS DOS EV'S(" + to_string(distanciaAux) + ") EH DIFERENTE DA DISTANCIA DO SATELLITE(" + to_string(distancia) + ");\n";
        return false;

    }


    return true;
}

void Satelite::print(std::string &str, const Instance &instance)
{
    str += "SATELLITE ID: "+ std::to_string(sateliteId)+"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        str += "\tROTA ID: "+ std::to_string(i)+".:  ";
        evRoute.print(str, instance);
        str+= "\n";
    }

    str += "\n\n";
}

void Satelite::print(const Instance &instance)
{

    std::cout<<"SATELLITE ID: "<<sateliteId<<"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        std::cout<<"\tROTA ID: "<<evRoute.idRota<<".:  ";
        evRoute.print(instance);
        std::cout<<"\n";
    }

    std::cout<<"\n\n";
}
