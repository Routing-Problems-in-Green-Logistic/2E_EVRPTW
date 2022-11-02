/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/07/22
 *  Arquivo: Aco.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_ACO_H
#define INC_2E_EVRP_ACO_H
#include "Instancia.h"
#include "../Solucao.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>


namespace N_Aco
{
    class AcoParametros
    {
    public:

        double alfa             = 0.2;              // Feromonio
        double beta             = 0.0;              // Heuristica: 1-alfa
        double alfaConst        = 0.5;
        double ro               = 0.1;              // Quantidade % de ferimonio que ira para a prox iteracao
        double q0               = 0.1;
        int numAnts             = 150;
        int8_t freqAtualAntBest = 25;
        int numIteracoes        = 100;               // Original 50
        int numItMaxHeur        = 20;
        double feromonioInicial = 0.0;
        double porcAtualFerom   = 1.10;             // As formigas com dist <= 1.05*ant_best sao atualizadas
        int numSol              = 10;

        AcoParametros()
        {
            beta = 1.0-alfa;
        }
    };

    class AcoEstatisticas
    {
    public:
        int nAntViaveis                 = 0;
        double mediaAntsViaveisPorIt    = 0.0;
        int nAntGeradas                 = 0;
        int ultimaAtualisacaoIt         = 0;
        double sumDistAntsViaveis       = 0.0;
        double distBestAnt              = -1.0;
    };

    class Ant
    {
    public:
        Satelite satelite;                          // Guarda a solucao do satelite
        vector<int8_t> vetNosAtend;                 // Guarda os vertices que sao atendidos. tam = numNos
        bool vazia = false;
        bool viavel = false;
        int clientesNaoAtend = 0;

        Ant(Instancia &instance, int satId, bool _vazia=false): satelite(Satelite(instance, satId))
        {
            vetNosAtend = vector<int8_t>(instance.numNos, 0);
            vazia = _vazia;
        }

        Ant(Ant &antOutra, Instancia &instancia, int satId):satelite(instancia, satId)
        {
            satelite.copia(antOutra.satelite);
            vetNosAtend = antOutra.vetNosAtend;
        }

        void copia(Ant &antOutra)
        {
            satelite.copia(antOutra.satelite);
            vetNosAtend = antOutra.vetNosAtend;
            viavel = antOutra.viavel;
            vazia = antOutra.vazia;
            clientesNaoAtend = antOutra.clientesNaoAtend;
        }

        bool operator < (const Ant &ant) const
        {
            if((viavel && !vazia) && (!ant.viavel || ant.vazia))
                return true;


            if((!viavel || vazia) && (ant.viavel && !ant.vazia))
                return false;

            if(!viavel && clientesNaoAtend != ant.clientesNaoAtend)
                return clientesNaoAtend < ant.clientesNaoAtend;

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

            //cout<<"\t\t\t\t"<<_cliente<<": dist("<<dist<<"); ferom("<<ferom<<")\n";

            cliente = _cliente;
            ferom_x_dist = 1.0;
            if(ferom > 0.0)
                ferom_x_dist = pow(ferom, acoParam.alfa);

            ferom_x_dist *= pow(1.0/dist, acoParam.beta);
            return ferom_x_dist;
        }

        bool operator < (const Proximo &proximo) const
        {
            return ferom_x_dist > proximo.ferom_x_dist;
        }
    };


    inline bool existeClienteNaoVisitado(Ant &ant, Instancia &instancia, const vector<int> &vetSatAtendCliente)
    {

        //return std::find((ant.vetNosAtend.begin()+instancia.getFirstClientIndex()), (ant.vetNosAtend.begin()+instancia.getEndClientIndex()+1), 0) == true;
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            if(vetSatAtendCliente[i] == ant.satelite.sateliteId && ant.vetNosAtend[i] == int8_t(0))
                return true;
        }

        return false;
    }

    inline double getDistSat(const Satelite &satelite)
    {
        return satelite.distancia;
    }

    bool aco(Instancia &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, int sateliteId, Satelite &satBest,
             const vector<int> &vetSatAtendCliente, ParametrosGrasp &param, const Solucao *solGrasp, const int numEVs);
    bool
    acoSol(Instancia &instancia, AcoParametros &acoPar, AcoEstatisticas &acoEst, ParametrosGrasp &param, Solucao &best);
    void atualizaFeromonio(ublas::matrix<double> &matFeromonio, ublas::matrix<double> &matAtualFeromonio,
                           Instancia &instancia, const AcoParametros &acoParam, const Ant &antBest, const vector<Ant> &vetAnt);
    void evaporaFeromonio(ublas::matrix<double> &matFeromonio, const vector<int> &vetSat, Instancia &instancia, const AcoParametros &acoParam, const double feromMin);
    bool clienteJValido(Instancia &instancia, const int i, const int j, const double bat, const vector<int8_t> &vetNosAtend, const int sat, const double tempoSaidaI);
    void atualizaClienteJ(EvRoute &evRoute, const int pos, const int clienteJ, Instancia &instance, Ant &ant);

}

#endif //INC_2E_EVRP_ACO_H
