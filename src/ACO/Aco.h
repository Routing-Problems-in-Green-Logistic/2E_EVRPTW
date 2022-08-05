/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/07/22
 *  Arquivo: Aco.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_ACO_H
#define INC_2E_EVRP_ACO_H
#include "Instance.h"
#include "../Solucao.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>


namespace N_Aco
{
    class AcoParametros
    {
    public:

        double alfa             = 1.0;
        double beta             = 5.0;
        double alfaConst        = 0.2;
        double ro               = 0.8;
        double q0               = 0.8;
        int numAnts             = 0;
        int8_t freqAtualAntBest = 25;
        int numIteracoes        = 400;
        int numItMaxHeur        = 20;
        double feromonioInicial = 0.0;

        AcoParametros()
        {
            numAnts = ceil(0.4/(q0* log(1.0-ro)));
        }

    };

    class AcoEstatisticas
    {
    public:
        int nAntViaveis     = 0;
        int nAntGeradas     = 0;
        double sumDistAnts  = 0.0;
    };

    class Ant
    {
    public:
        Satelite satelite;                          // Guarda a solucao do satelite
        vector<int8_t> vetNosAtend;                 // Guarda os vertices que sao atendidos. tam = numNos
        bool vazia = false;
        bool viavel = false;

        Ant(Instance &instance, int satId, bool _vazia=false): satelite(Satelite(instance, satId))
        {
            vetNosAtend = vector<int8_t>(instance.numNos, 0);
            vazia = _vazia;
        }

        Ant(Ant &antOutra, Instance &instancia, int satId):satelite(instancia, satId)
        {
            satelite.copia(antOutra.satelite);
            vetNosAtend = antOutra.vetNosAtend;
        }

        void copia(Ant &antOutra)
        {
            satelite.copia(antOutra.satelite);
            vetNosAtend = antOutra.vetNosAtend;
        }

        bool operator < (const Ant &ant) const
        {
            if(viavel && (!ant.viavel || ant.vazia))
                return true;


            if((!viavel || vazia) && ant.viavel)
                return false;

            return satelite.distancia < ant.satelite.distancia;
        }
    };

    class Proximo
    {
    public:
        int cliente         = -1;
        double ferom_x_dist = 0.0;  // Igual a: (feromonio ^ alfa) * ((1/dist)^beta)

        inline double atualiza(int _cliente, double dist, double ferom, const AcoParametros &acoParam)
        {
            ferom_x_dist = pow(ferom, acoParam.alfa) * pow(1.0/dist, acoParam.beta);
            return ferom_x_dist;
        }
    };


    inline bool existeClienteNaoVisitado(Ant &ant, Instance &instancia)
    {
        return std::find((ant.vetNosAtend.begin()+instancia.getFirstClientIndex()), (ant.vetNosAtend.begin()+instancia.getEndClientIndex()+1), 0) != ant.vetNosAtend.end();
    }

    inline double getDistSat(const Satelite &satelite)
    {
        return satelite.distancia;
    }

    void aco(Instance &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, const vector<int8_t> &clientes, int sateliteId, Satelite &satBest);
    void atualizaFeromonio(ublas::matrix<double> &matFeromonio, Instance &instancia, const AcoParametros &acoParam, const Ant &antBest, const double feromMin, const double feromMax);
    void evaporaFeromonio(ublas::matrix<double> &matFeromonio, const vector<int> &vetSat, Instance &instancia, const AcoParametros &acoParam, const double feromMin);
    bool clienteJValido(Instance &instancia, const int i, const int j, const double bat, const vector<int8_t> &vetNosAtend, const int sat);
    void atualizaClienteJ(EvRoute &evRoute, const int pos, const int clienteJ, Instance &instance, Satelite &sat);

}

#endif //INC_2E_EVRP_ACO_H
