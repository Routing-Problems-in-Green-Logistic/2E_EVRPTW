#ifndef ROUTE_H
#define ROUTE_H

#include "Instance.h"
#include <vector>
#include <iostream>

class RouteNo
{
public:

    int satellite       = -1;
    double tempoChegada = 0.0;            // Como nao existe tempo de atendimento, tempoChegada eh igual a tempoFim.
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
    bool checkDistence(const Instance &instance, double *dist, std::string &str);

    void copia(Route &route);

    std::vector<RouteNo> rota;
    int routeSize = 2;
    int routeSizeMax = -1;

    double totalDemand = 0.0;
    double totalDistence = 0.0;

    // Armazena a demanda levada pelo veiculo de cada satellite.
    // Possui uma entrada para cada satellite da instancia + deposito!
    std::vector<double> satelliteDemand;
};

#endif
