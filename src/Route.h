#ifndef ROUTE_H
#define ROUTE_H

#include "INSTANCIA/Instancia.h"


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
    Route(const Instancia &instance);
    int getSize() const {return routeSize;}
    float getDemand() const {return totalDemand;}
    float getCost() const {return totalDistence;}
    void print();
    void print(std::string &str);
    bool checkDistence(const Instancia &instance, double *dist, std::string &str);
    void copia(Route &route);
    void resetaRoute();
    void atualizaTotalDemand(const Instancia &instancia);

    Vector<RouteNo> rota;
    int routeSize = 2;
    int routeSizeMax = -1;

    double totalDemand = 0.0;
    double totalDistence = 0.0;

    // Armazena a demanda levada pelo veiculo de cada satellite.
    // Possui uma entrada para cada satellite da instancia + deposito!
    Vector<double> satelliteDemand;
};

#endif
