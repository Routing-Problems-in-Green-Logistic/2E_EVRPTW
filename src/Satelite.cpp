#include "Satelite.h"
#include "Auxiliary.h"



Satelite::Satelite(const Instancia& instance, const int _satId)
{

    const int max = instance.getEvRouteSizeMax();

    vetEvRoute.reserve(instance.getN_Evs());
    tamVetEvRoute = instance.getN_Evs();

    for(int i=0; i < instance.getN_Evs(); ++i)
    {
        const VeiculoInst &veic = instance.vectVeiculo[instance.getFirstEvIndex()+i];
        vetEvRoute.emplace_back(_satId, instance.getFirstEvIndex()+i, max, instance);
    }

    //vetTempoSaidaEvRoute.reserve(instance.getN_Evs());

    for(int i=0; i < instance.getN_Evs(); ++i)
    {
        EvRoute *evRoute = &vetEvRoute[i];
        //vetTempoSaidaEvRoute.emplace_back(evRoute);
    }

//    cout<<"vetTempoSaidaEvRoute size: "<<vetTempoSaidaEvRoute.size()<<"\n";

    sateliteId = _satId;


}

int Satelite::getNRoutes() const
{
    return this->vetEvRoute.size();
}

// Recebe i = [0, numEvs-1]
EvRoute &Satelite::getRoute(int i)
{
    //cout<<"id: "<<(i-primeiroEv)<<"\n";
    //cout<<"i: "<<i<<"\n";
    return this->vetEvRoute.at(i);
}

bool Satelite::checkSatellite(std::string &erro, const Instancia &Inst)
{
    double demandaAux = 0.0;
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

void Satelite::print(std::string &str, const Instancia &instance, const bool somenteNo)
{
    str += "SATELLITE ID: "+ std::to_string(sateliteId)+"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        if(evRoute.getDemand() > 0)
        {
            str += "\tROTA ID: " + std::to_string(evRoute.idRota) + ".:  ";
            evRoute.print(str, instance, somenteNo);
            str += "\n";
        }
    }

    str += "Dist: " + NS_Auxiliary::float_to_string(distancia, 2) + "; Demanda: " + to_string(int(demanda));
    str += "\n\n";
}

void Satelite::print(const Instancia &instance)
{

    std::cout<<"SATELLITE ID: "<<sateliteId<<"\n\n";
    int i=0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        if(evRoute.routeSize > 2)
        {
            std::cout << "\tROTA ID: " << evRoute.idRota << ".:  ";
            evRoute.print(instance, false);
            std::cout << "\n";
        }
    }

    cout<<"\n\n***********\nDemanda sat: "<<demanda<<"; dist sat: "<<distancia;
    std::cout<<"\n\n";
}

void Satelite::copia(Satelite &satelite)
{
    //cout<<"tamVetEvRoute: "<<tamVetEvRoute<<"  "<<vetEvRoute.size()<<"\n";
    for(int i=0; i < vetEvRoute.size(); ++i)
    {

        vetEvRoute.at(i).copia(satelite.vetEvRoute[i], false, nullptr);

    }

    sateliteId = satelite.sateliteId;
    demanda = satelite.demanda;
    distancia = satelite.distancia;
}

void Satelite::recalculaDist()
{

    distancia = 0.0;

    for(int i=0; i < vetEvRoute.size(); ++i)
    {
        EvRoute &evRoute = vetEvRoute.at(i);
        if(evRoute.routeSize > 2)
            distancia += evRoute.distancia;

    }

}

bool Satelite::vazio()
{

    return demanda <= 0.0;

/*    bool resultado = true;

    for(int i=0; i < vetEvRoute.size(); ++i)
    {
        EvRoute &evRoute = vetEvRoute.at(i);
        if(evRoute.routeSize > 2)
           return false;
    }

    return true;*/
}

int Satelite::numEv()
{
    int num=0;
    for(int i=0; i < vetEvRoute.size(); ++i)
    {
        EvRoute &evRoute = vetEvRoute[i];
        if(evRoute.routeSize > 2)
            num += 1;
    }

    return num;
}


void Satelite::recalculaDemanda()
{
    demanda = 0.0;

    for(int i=0; i < vetEvRoute.size(); ++i)
    {
        EvRoute &evRoute = vetEvRoute.at(i);
        if(evRoute.routeSize > 2)
            demanda += evRoute.demanda;
    }
}

double Satelite::excluiSatelite(Instancia &instancia, Vector<int8_t> &vetClientesAtend)
{

    const double distTemp = distancia;
    distancia = 0.0;
    demanda = 0.0;

    for(EvRoute &evRoute:vetEvRoute)
    {
        if(evRoute.routeSize <= 2)
            continue;

        for(int i=1; i < (evRoute.routeSize-1); ++i)
        {
            const int cliente = evRoute[i].cliente;
            if(instancia.isClient(cliente))
                vetClientesAtend[cliente] = int8_t(0);
        }

        evRoute = EvRoute(sateliteId, evRoute.idRota, evRoute.routeSizeMax, instancia);
    }

    return distTemp;
}

void Satelite::resetaSat()
{
    demanda = 0.0;
    distancia = 0.0;

    for(EvRoute &evRoute:vetEvRoute)
        evRoute.resetaEvRoute();
}
