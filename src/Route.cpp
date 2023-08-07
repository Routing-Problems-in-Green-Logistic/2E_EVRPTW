//
// Created by ****** on 27/11/2021.
//

#include "Route.h"
#include "Auxiliary.h"



Route::Route(const Instancia &instance)
{

    //routeSizeMax = instance.getNSats() + 1;
    routeSizeMax = instance.getNSats() + 2;

    rota.reserve(routeSizeMax);
    satelliteDemand.reserve(instance.numSats+1);
    this->routeSize = 2;

    for(int i=0; i < routeSizeMax; ++i)
        rota.push_back({0, 0.0});

    for(int i=0; i < instance.numSats+1; ++i)
        satelliteDemand.push_back(0.0);

    //cout<<routeSizeMax<<"\n\n";

}

void Route::print()
{
    for(int i=0; i < routeSize; ++i)
    {

        std::cout<<rota[i].satellite<<"(T: "<<rota[i].tempoChegada;

        if(i!=0 && i != routeSize-1)
            std::cout<<"; Dem: "<<satelliteDemand[rota[i].satellite]<<")";
        else
            std::cout<<")";

        cout<<" ";
    }

    std::cout<<"\nDISTANCIA: "<<totalDistence<<"\n\n";
}

void Route::print(std::string &str)
{
    for(int i=0; i < routeSize; ++i)
    {
        str += std::to_string(rota[i].satellite) + "(T: "+ to_string(rota[i].tempoChegada);

        if(i!=0 && i != routeSize-1)
            str+="; Dem: "+ to_string(satelliteDemand[rota[i].satellite])+") ";
        else
            str += ")";

        str+=" ";
    }

    str += "\nDISTANCIA: "+to_string(totalDistence) +"\n";
}

bool Route::checkDistence(const Instancia &instance, double *dist, string &str)
{

    double sumDemanda = 0.0;
    for(int sat = instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
        sumDemanda += satelliteDemand[sat];

    if(sumDemanda != totalDemand)
    {
        if(abs(sumDemanda-totalDemand) >= TOLERANCIA_DEMANDA)
        {
            str += "ERRO, demanda";
            str += "calculado: " + to_string(sumDemanda) + "\n"+"rota: "+ to_string(totalDemand)+"\n\n";
            return false;
        }
    }

    *dist = 0.0;


    for(int i=0; (i+1)<routeSize; ++i)
    {
        *dist += instance.getDistance(rota[i].satellite, rota[i + 1].satellite);

        if((i+1) != (routeSize-1))
        {
            if(satelliteDemand[rota[i+1].satellite] == 0.0 || satelliteDemand[rota[i+1].satellite] < TOLERANCIA_DEMANDA)
            {
                str += "SATELITE "+ to_string(rota[i+1].satellite) + "EH VISITADO LEVANDO "+ to_string(satelliteDemand[rota[i+1].satellite])+  " DE DEMANDA\n";
                return false;
            }
        }

    }

    if(std::abs(*dist-totalDistence) > TOLERANCIA_DISTANCIA)
    {
        str += "DISTANCIA CALCULADA: "+ std::to_string(*dist) + " != DISTANCIA R0TA: "+ std::to_string(totalDistence)+"\n";
        return false;
    }
    else
        return true;

}

void Route::copia(Route &route)
{
    if(route.routeSize <= 2)
    {
        routeSize = 2;
        rota[1].satellite = rota[0].satellite;

    }
    else
    {
      routeSize = route.routeSize;
      totalDistence = route.totalDistence;
      totalDemand = route.totalDemand;

      for(int i=0; i < routeSize; ++i)
          rota[i] = route.rota[i];

      for(int i=0; i < satelliteDemand.size(); ++i)
          satelliteDemand[i] = route.satelliteDemand[i];
    }
}

void Route::resetaRoute()
{

    routeSize = 2;
    //cout<<"vet size: "<<rota.size()<<"\n\n";
    rota[0].satellite = 0;
    rota[0].tempoChegada = 0.0;

    rota[1].satellite = 0;
    rota[1].tempoChegada = 0.0;

    totalDistence = 0.0;
    totalDemand = 0.0;
    std::fill(satelliteDemand.begin(), satelliteDemand.end(), 0.0);
}

void Route::atualizaTotalDemand(const Instancia &instancia)
{
    totalDemand = 0.0;

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
        totalDemand += satelliteDemand[sat];
}
