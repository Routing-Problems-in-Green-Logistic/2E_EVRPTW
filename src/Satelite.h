#ifndef INC_2E_EVRP_SATELITE_H
#define INC_2E_EVRP_SATELITE_H



#include "EvRoute.h"
#include "Instancia.h"


class TempoSaidaEvRoute
{

public:
    EvRoute *evRoute;

    explicit TempoSaidaEvRoute(EvRoute *evRouteAux){evRoute = evRouteAux;}

    ~TempoSaidaEvRoute()
    {
        evRoute = nullptr;
    }


    bool operator < (const TempoSaidaEvRoute &tempo2) const
    {
        if(evRoute->routeSize > 2 && tempo2.evRoute->routeSize <= 2)
            return true;
        else if(evRoute->routeSize <= 2 && tempo2.evRoute->routeSize > 2)
            return false;

        return evRoute->route.at(0).tempoSaida < tempo2.evRoute->route.at(0).tempoSaida;
    }



};

// OK
class Satelite
{
public:

    Satelite(const Instancia&, int _satId);
    //Satelite(const Satelite &satelite)

    int getNRoutes() const;
    EvRoute& getRoute(int i);
    bool checkSatellite(std::string &erro, const Instancia &Inst);
    void print(std::string &str, const Instancia &instance, const bool somenteNo);
    void print(const Instancia &instance);
    void copia(Satelite &satelite);
    void recalculaDist();
    void recalculaDemanda();        // Parte do principio de que as cargas nas rotas estao corretas
    bool vazio();
    int numEv();
    double excluiSatelite(Instancia &instancia, Vector<int8_t> &vetClientesAtend);
    void resetaSat();

    Vector<EvRoute> vetEvRoute;

    // O vetor deve estar ordenado antes de ser utilizado
    //Vector<TempoSaidaEvRoute> vetTempoSaidaEvRoute;
    //void sortVetTempoSaidaEvRoute(){std::sort(vetTempoSaidaEvRoute.begin(), vetTempoSaidaEvRoute.end());};

    int tamVetEvRoute = -1;
    int sateliteId = -1;
    double demanda = 0.0;
    double distancia = 0.0;


    // O vetor deve estar ordenado antes de ser utilizado
    //Vector<TempoSaidaEvRoute> vetTempoSaidaEvRoute;
    //void sortVetTempoSaidaEvRoute(){std::sort(vetTempoSaidaEvRoute.begin(), vetTempoSaidaEvRoute.end());};

};
#endif //INC_2E_EVRP_SATELITE_H
