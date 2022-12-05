#ifndef INC_2E_EVRP_SOLUCAO_H
#define INC_2E_EVRP_SOLUCAO_H
#include <boost/container/vector.hpp>
namespace BoostC = boost::container;
#include "Satelite.h"

class Solucao
{
public:
    explicit Solucao(Instancia& Inst);
    Solucao(const Solucao &solution) = delete;
    void copia(Solucao &solution);

    int getNSatelites() const;
    int findSatellite(int id) const;
    Satelite* getSatelite(int index);
    bool checkSolution(std::string &erro, Instancia &inst);
    void print(std::string &saida, const Instancia &instance);
    void print(const Instancia& Inst);
    double calcCost(const Instancia&);
    float getDistanciaTotal();
    void atualizaVetSatTempoChegMax( Instancia &instance);
    void inicializaVetClientesAtend(Instancia &instance);
    double getDist1Nivel();
    double getDist2Nivel();
    void recalculaDist();
    void resetaPrimeiroNivel(Instancia &instancia);  // Recalcula distancia com somente a dist dos satelites!
    void recalculaDistSat(Instancia &instancia);
    double distSat();
    int numSatVazios();
    int getNumEvNaoVazios();
    bool viavel2Nivel(Instancia &instancia);
    void resetaSat(int satId, Instancia &instancia, BoostC::vector<int> &vetClienteDel);    // Somente o sat eh alterado, 1 nivel nao se altera
    void reseta1Nivel(Instancia &instancia);

    // Possui numSat + 1 !!
    BoostC::vector<Satelite> satelites;

    // Guarda o ultimo tempo de chegada do veiculo do 1° nivel
    BoostC::vector<double> satTempoChegMax;
    int numTrucksMax = -1;
    int numEvMax     = -1;
    int numEv        = 0;                   // Num Ev utilizados
    bool viavel      = true;
    BoostC::vector<Route> primeiroNivel;
    bool mvShiftIntraRota = false;
    bool mvShiftInterRotas = false;
    bool mvCross = false;
    double distancia = 0.0;
    BoostC::vector<int8_t> vetClientesAtend;

    bool solInicializada = false;
    int ultimaA = -1;


};
#endif //INC_2E_EVRP_SOLUCAO_H
