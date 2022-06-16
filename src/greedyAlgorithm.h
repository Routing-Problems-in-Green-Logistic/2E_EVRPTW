#ifndef INC_2E_EVRP_GREEDYALGORITHM_H
#define INC_2E_EVRP_GREEDYALGORITHM_H

#include "Solucao.h"
#include "ViabilizadorRotaEv.h"
#include <list>
#include <vector>

namespace GreedyAlgNS
{

    class CandidatoVeicComb
    {
    public:

        int rotaId = -1;
        int satelliteId = -1;
        float demand = 0.0;
        double incrementoDistancia = DOUBLE_MAX;
        int pos = -1;
        double tempoSaida = 0.0;

        CandidatoVeicComb(int _rotaId, int _satelliteId, float _demand, double _incrDist):rotaId(_rotaId), satelliteId(_satelliteId),
            demand(_demand), incrementoDistancia(_incrDist){};

        bool operator < (const CandidatoVeicComb &candidato) const {return incrementoDistancia < candidato.incrementoDistancia;}

    };


    class CandidatoEV
    {
    public:

        int pos             = -1;       // Verificar se o indice rechargingS_Pos eh menor antes de inserir na solucao. O indice maior ja esta considerando +1
        int clientId        = -1;
        int routeId         = -1;
        int satId           = -1;
        double incremento   = DOUBLE_MAX;
        double incrP        = DOUBLE_MAX;
        double demand       = 0.0;
        double penalidade   = 0.0;

        void atualizaPenalidade()
        {
            incrP = incremento + penalidade;
        }

        void atualizaPenalidade(double p)
        {
            penalidade = p;
            incrP = p + incremento;
        }

        NameViabRotaEv::InsercaoEstacao insercaoEstacao;

        CandidatoEV(int pos, int clientId, double cost, float demand, double batteryCost, int routeId, int satId, int rsPos,
                    int rsId, NameViabRotaEv::InsercaoEstacao insercaoEstacao_)
        {
            this->pos = pos;
            this->clientId = clientId;
            this->incremento = cost;
            this->demand = demand;
            this->routeId = routeId;
            this->satId = satId;

            if(insercaoEstacao_.distanciaRota < DOUBLE_MAX)
                this->insercaoEstacao = insercaoEstacao_;
        }
        CandidatoEV(int routeId) { this->routeId = routeId;}
        CandidatoEV() = default;
        CandidatoEV(const CandidatoEV &outro)=default;

        bool operator < (const CandidatoEV& that) const
        {
            //return (incremento + penalidade) < (that.incremento + that.penalidade);
            return incrP < that.incrP;
        }

        bool operator == (const CandidatoEV &that) const {
            return clientId == that.clientId && routeId==that.routeId && satId == that.satId;
        }

    };


    bool secondEchelonGreedy(Solucao& Sol, Instance& instance, float alpha);
    void firstEchelonGreedy(Solucao &Sol, const Instance &Inst, const float beta);
    void construtivo(Solucao &Sol, Instance &Inst, const float alpha, const float beta);
    bool visitAllClientes(std::vector<int> &visitedClients, const Instance &instance);
    bool existeDemandaNaoAtendida(std::vector<double> &demandaNaoAtendida);
    float palpiteTempoFinalPrimeiroNivel(const Instance& inst);
    bool insereEstacao(int rotaId, int satId);

    bool canInsert(EvRoute &evRoute, int node, Instance &instance, CandidatoEV &candidatoEv, const int satelite, const double tempoSaidaSat, EvRoute &evRouteAux);
    bool canInsertSemBateria(EvRoute &evRoute, int node, const Instance &Instance, CandidatoEV &insertion);
    bool insert(EvRoute &evRoute, CandidatoEV & insertion, const Instance &instance, const double tempoSaidaSat, Solucao &sol);
    bool verificaViabilidadeSatelite(double tempoChegada, Satelite &satelite, const Instance &instance, bool modficaSatelite);

    std::vector<double> calculaTempoSaidaInicialSat(const Instance &instance);



}

#endif //INC_2E_EVRP_GREEDYALGORITHM_H

