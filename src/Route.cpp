//
// Created by igor on 27/11/2021.
//

#include "Route.h"
#include "Auxiliary.h"

/*

Route::Route(const Instance &instance)
{

    //routeSizeMax = instance.getNSats() + 1;
    routeSizeMax = instance.getNSats() + 2;

    rota.reserve(routeSizeMax);
    satelliteDemand.reserve(routeSizeMax);
    this->routeSize = 2;

    for(int i=0; i < routeSizeMax; ++i)
    {
        rota.push_back(0);

        satelliteDemand.push_back(0.0);
    }

}

void Route::print()
{
    for(int i=0; i < routeSize; ++i)
    {

        std::cout<<rota[i];

        if(i!=0 && i != routeSize-1)
            std::cout<<"("<<satelliteDemand[rota[i]]<<")";

        cout<<" ";
    }

    std::cout<<"\nDISTANCIA: "<<totalDistence<<"\n\n";
}

void Route::print(std::string &str)
{
    for(int i=0; i < routeSize; ++i)
    {
        str += std::to_string(rota[i]);

        if(i!=0 && i != routeSize-1)
            str+="("+ to_string(satelliteDemand[rota[i]])+") ";

        str+=" ";
    }

    str += "\nDISTANCIA: "+to_string(totalDistence) +"\n";
}

bool Route::checkDistence(const Instance &instance, float *dist)
{

    *dist = 0.0;

    for(int i=0; (i+1)<routeSize; ++i)
        *dist += instance.getDistance(rota[i], rota[i+1]);

    if(std::abs(*dist-totalDistence) > DISTANCE_TOLENCE)
        return false;
    else
        return true;

}
*/