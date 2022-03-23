#ifndef EV_ROUTE_H
#define EV_ROUTE_H

#include "Instance.h"
#include "Route.h"
#include <list>

/*
class Recharge {
public:
    int pos = -1;
    int id = -1;
    float remainingBattery = 0.0;
    Recharge() = default;
    Recharge(int pos, int id, float remainingBattery)
        : pos(pos), id(id), remainingBattery(remainingBattery){};
};

struct PosicaoEstacao
{
    int pos = -1;
    int rechargingStationId = -1;

    friend bool operator < (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId < posRouteRechSId1.rechargingStationId;
    }


    friend bool operator > (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId > posRouteRechSId1.rechargingStationId;
    }

    friend bool operator == (const PosicaoEstacao &posRouteRechSId0, const PosicaoEstacao &posRouteRechSId1)
    {
        return posRouteRechSId0.rechargingStationId == posRouteRechSId1.rechargingStationId;
    }
};

struct PosRota0Rota1Estacao
{

    int posRoute0 = -1;
    int posRoute1 = -1;
    int rechargingStationId = -1;

    friend bool operator <(const PosRota0Rota1Estacao &aux0, const PosRota0Rota1Estacao &aux1)
    {
        return aux0.rechargingStationId < aux1.rechargingStationId;
    }

    void swapPos()
    {
        int aux = posRoute0;
        posRoute0 = posRoute1;
        posRoute1 = aux;
    }

};
*/

struct EvRecarga
{
    int recargaId = -1;
    int utilizado = 0;

    EvRecarga()=default;
    EvRecarga(int id, int ut){recargaId = id; utilizado = ut;}
};

class EvNo
{

public:
    int cliente = -1;
    double bateriaRestante = -1.0;
    double tempoCheg = -1.0;
    double tempoSaida = -1.0;

    // Considerando somente as proximas posicoes, armazena a de menor folga (tempo chegada - final janela)
    int posMenorFolga   = -1;

    EvNo()= default;
    EvNo(int _cliente, double _bateeria, double _tempo_ini, double _tempo_f);
    EvNo(const EvNo &outro);

    friend std::ostream& operator <<(std::ostream &os, const EvNo &evNo)
    {
        os << evNo.cliente << "(bat: " << evNo.bateriaRestante << ";[" << evNo.tempoCheg << "; " << evNo.tempoSaida << "]) ";
        return os;
    }

};

class EvRoute{
public:

    int size() const;
    float getDemand() const {return demanda;};

    EvRoute(int satelite, int idRota, int RouteSizeMax, const Instance &instance);

    void print(const Instance &instance) const;
    void print(std::string &str, const Instance &instance) const;
    bool checkRoute(std::string &erro, const Instance &instance) const;
    void atualizaPosMenorFolga(const Instance &instance);


    EvNo & operator [](int pos)
    {
        return route[pos];
    }

    std::vector<EvNo> route;

    double distancia = 0.0;
    int routeSize = 2;
    float demanda = 0.0;
    int routeSizeMax = -1;
    int satelite = -1;
    int idRota = -1;

    int getUtilizacaoRecarga(int id);
    bool setUtilizacaoRecarga(int id, int utilizacao);

private:
    std::vector<EvRecarga> vetRecarga;
    int numRecarga=-1;
    int numMaxUtilizacao=-1;
    const int firstRechargingSIndex;


};
#endif