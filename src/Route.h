#ifndef ROUTE_H
#define ROUTE_H

#include "Instance.h"
#include <vector>
#include <iostream>

class RouteNo
{
public:

    int satellite;
    double tempoInicio;                     // Como nao existe tempo de atendimento, tempoInicio eh igual a tempoFim.
};

class Route
{

public:
    Route(const Instance &instance);
    int getSize() const {return routeSize;}
    float getDemand() const {return totalDemand;}
    float getCost() const {return totalDistence;}
    void print();
    void print(std::string &str);
    bool checkDistence(const Instance &instance, double *dist);

    std::vector<RouteNo> rota;
    int routeSize = 2;
    int routeSizeMax = -1;

    float totalDemand = 0.0;
    float totalDistence = 0.0;

    // Armazena a demanda levada pelo veiculo de cada satellite.
    // Possui uma entrada para cada satellite da instancia
    std::vector<float> satelliteDemand;
};

#endif
