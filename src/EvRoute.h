#ifndef EV_ROUTE_H
#define EV_ROUTE_H

#include "Instancia.h"
#include "Route.h"
#include <list>
#include "Auxiliary.h"


class EvRecarga
{

public:

    int recargaId = -1;
    int utilizado = 0;

    EvRecarga()=default;
    EvRecarga(int id, int ut){recargaId = id; utilizado = ut;}
    ~EvRecarga()
    {

    }
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

    // Considerando somente as proximas posicoes, armazena a proxima estacao de recarga da rota
    int posProxEstacao = -1;

    // Se o no eh uma estacao de recarga estacaoBateriaRestante guarda a quantidade de bateria antes no momento da chegada (antes de recaregar)
    double estacaoBateriaRestante = -1.0;

    EvNo()= default;
    EvNo(int _cliente, double _bateeria, double _tempo_ini, double _tempo_f);
    EvNo(const EvNo &outro);

    friend std::ostream& operator <<(std::ostream &os, const EvNo &evNo)
    {
        os << evNo.cliente << "(bat: " << evNo.bateriaRestante << ";[" << evNo.tempoCheg << "; " << evNo.tempoSaida << "]) ";
        return os;
    }

};

class EvRoute
{
public:

    int size() const;
    float getDemand() const {return demanda;};

    EvRoute(const EvRoute &evRoute);

    EvRoute(int satelite, int idRota, int RouteSizeMax, const Instancia &instance);

    void print(const Instancia &instance, const bool somenteNo) const;
    void print(std::string &strRota, const Instancia &instance, const bool somenteNo) const;
    std::string getRota(const Instancia &instance, const bool somenteNo);
    bool checkRoute(std::string &erro, const Instancia &instance) const;
    void atualizaParametrosRota(const Instancia &instance);
    double getCurrentTime();
    bool alteraTempoSaida(const double novoTempoSaida, const Instancia &instance, const bool recalculaDist);

    void copiaCliente(const EvRoute &outro);
    void copia(const EvRoute &evRoute, const bool calculaDemanda, Instancia *instancia);
    void copiaN(const EvRoute &evRoute, int n, bool calculaDemanda, Instancia *instance);
    void copiaParametros(const EvRoute &outro);
    void resetaEvRoute();

    EvNo & operator [](int pos)
    {
        return route[pos];
    }

    Vector<EvNo> route;

    double distancia = 0.0;
    int routeSize = 2;
    double demanda = 0.0;
    int routeSizeMax = -1;
    int satelite = -1;
    int idRota = -1;

    int getUtilizacaoRecarga(int id);
    bool setUtilizacaoRecarga(int id, int utilizacao);

    /**
     * **************************************************************
     * **************************************************************
     * Guarda o numero de vezes que uma estacao eh utilizada,
     * INDICE NAO EH RELATIVO A CLIENTES.
     * 1° estacao: indice: 0: idEstacao - firstRechargingSIndex !!!
     * **************************************************************
     * **************************************************************
     */

    Vector<EvRecarga> vetRecarga;

    int numEstRecarga=-1;
    int numMaxUtilizacao=-1;
    int firstRechargingSIndex;
};
#endif
