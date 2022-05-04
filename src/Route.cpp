//
// Created by igor on 27/11/2021.
//

#include "Route.h"
#include "Auxiliary.h"



Route::Route(const Instance &instance)
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

bool Route::checkDistence(const Instance &instance, double *dist, string &str)
{

    *dist = 0.0;


    for(int i=0; (i+1)<routeSize; ++i)
    {
        *dist += instance.getDistance(rota[i].satellite, rota[i + 1].satellite);

        if((i+1) != (routeSize-1))
        {
            if(satelliteDemand[i+1] == 0.0 || satelliteDemand[i+1] < TOLERANCIA_DEMANDA)
            {
                str += "SATELITE "+ to_string(rota[i+1].satellite) + "EH VISITADO LEVANDO 0 DE DEMANDA\n";
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