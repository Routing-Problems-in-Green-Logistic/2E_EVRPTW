/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    22/02/23
 *  Arquivo: Construtivo3.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_CONSTRUTIVO4_H
#define INC_2E_EVRP_CONSTRUTIVO4_H


#include "../Solucao.h"
#include "../ViabilizadorRotaEv.h"
#include <list>




namespace NS_Construtivo4
{

    class CandidatoVeicComb
    {
    public:

        int rotaId = -1;
        int satelliteId = -1;
        double demand = 0.0;
        double incrementoDistancia = DOUBLE_MAX;
        int pos = -1;
        double tempoSaida = 0.0;

        CandidatoVeicComb(int _rotaId, int _satelliteId, double _demand, double _incrDist):rotaId(_rotaId), satelliteId(_satelliteId),
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
        NS_viabRotaEv::InsercaoEstacao insercaoEstacao;

        CandidatoEV(const CandidatoEV &outro)
        {
            pos         = outro.pos;
            clientId    = outro.clientId;
            routeId     = outro.routeId;
            satId       = outro.satId;
            incremento  = outro.incremento;
            incrP       = outro.incrP;
            demand      = outro.demand;
            penalidade  = outro.penalidade;
            insercaoEstacao = outro.insercaoEstacao;
        }

        void atualizaPenalidade()
        {
            incrP = incremento + penalidade;
        }

        void atualizaPenalidade(double p)
        {
            penalidade = p;
            incrP = p + incremento;
        }

        CandidatoEV(int pos, int clientId, double cost, double demand, double batteryCost, int routeId, int satId, int rsPos,
                    int rsId, NS_viabRotaEv::InsercaoEstacao insercaoEstacao_)
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

        bool operator < (const CandidatoEV& that) const
        {
            //return (incremento + penalidade) < (that.incremento + that.penalidade);
            return incrP < that.incrP;
        }

        bool operator == (const CandidatoEV &that) const {
            return clientId == that.clientId && routeId==that.routeId && satId == that.satId;
        }

    };

    class CoordCandMat
    {
    public:

        int sat     = -1;
        int ev      = -1;
        int cliente = -1;
    };

    bool construtivoSegundoNivelEV(Solucao &sol, Instancia &instance, const float alfaSeg,
                                   const Matrix<int> &matClienteSat, bool listaRestTam,
                                   const Vector<int> &satUtilizados, bool print,
                                   Vector<int> &vetInviabilidade, const bool torneio);

    void construtivoPrimeiroNivel(Solucao &sol, Instancia &instance, const float betaPrim, bool listaRestTam,
                                  const bool split);
    void construtivo(Solucao &Sol, Instancia &instancia, const float alfaSeg, const float betaPrim,
                     const Matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil, bool print,
                     Vector<int> *vetInviabilidate, const bool torneio, const bool split);
    bool visitAllClientes(Vector<int8_t> &visitedClients, const Instancia &instance);
    bool existeDemandaNaoAtendida(Vector<double> &demandaNaoAtendida);

    bool canInsert(EvRoute &evRoute, int cliente, Instancia &instance, CandidatoEV &candidatoEv, const int satelite,
                   const double tempoSaidaSat, EvRoute &evRouteAux, Vector<int> &vetInviabilidade);
    bool canInsertSemBateria(EvRoute &evRoute, int node, const Instancia &Instance, CandidatoEV &insertion);
    bool insert(EvRoute &evRoute, CandidatoEV & insertion, const Instancia &instance, const double tempoSaidaSat, Solucao &sol);
    bool verificaViabilidadeSatelite(double tempoChegada, Satelite &satelite, const Instancia &instance, bool modficaSatelite);

    Vector<double> calculaTempoSaidaInicialSat(Instancia &instance, const float beta);
    void setSatParaCliente(Instancia &instancia, vector<int> &vetSatAtendCliente, vector<int> &satUtilizado, ParametrosGrasp &param);
}

#endif //INC_2E_EVRP_CONSTRUTIVO4_H
