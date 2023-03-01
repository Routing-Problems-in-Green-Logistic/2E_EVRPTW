#ifndef INC_2E_EVRP_SOLUCAO_H
#define INC_2E_EVRP_SOLUCAO_H
#include <boost/container/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "Satelite.h"

namespace BoostC = boost::container;
using namespace boost::numeric;

class Solucao
{
public:
    explicit Solucao(Instancia& inst);
    Solucao(const Solucao &solution) = delete;
    void copia(Solucao &solution);

    int getNSatelites() const;
    Satelite* getSatelite(int index);
    bool checkSolution(std::string &erro, Instancia &inst);
    void print(std::string &saida, const Instancia &instance);
    void print(const Instancia& Inst);
    void printPlot(std::string &saida, const Instancia &instance);
    void atualizaVetSatTempoChegMax( Instancia &instance);
    void inicializaVetClientesAtend(Instancia &instance);
    double getDist1Nivel();
    double getDist2Nivel();
    void recalculaDist();
    void resetaPrimeiroNivel(Instancia &instancia);  // Recalcula distancia com somente a dist dos satelites!
    void recalculaDistSat(Instancia &instancia);
    int numSatVazios();
    int getNumEvNaoVazios();
    void resetaSat(int satId, Instancia &instancia, BoostC::vector<int> &vetClienteDel);    // Somente o sat eh alterado, 1 nivel nao se altera
    void reseta1Nivel(Instancia &instancia);
    void resetaIndiceEv(Instancia &instancia);

    // Possui numSat + 1 !!
    BoostC::vector<Satelite> satelites;

    // Guarda o ultimo tempo de chegada do veiculo do 1° nivel
    BoostC::vector<double> satTempoChegMax;
    int numTrucksMax = -1;
    int numEvMax     = -1;
    int numEv        = 0;                   // Num Ev utilizados
    int numSats      = 1;
    bool viavel      = true;
    BoostC::vector<Route> primeiroNivel;
    double distancia = 0.0;
    BoostC::vector<int8_t> vetClientesAtend;

    int ultimaA = -1;


    BoostC::vector<ublas::matrix<int>> vetMatSatEvMv;  // Sat, ev, mv
    /*
     * Se vetMatSatEvMv[1](0,0) (sat 1, ev 0, mv 0) = 1:
     *
     *  Entao, nao eh necessario refazer o mv 0 no ev 0 do sat 1 se nao houver alteracao nessa rota!
     */

    void rotaEvAtualizada(int sat, int ev);
    void todasRotasEvAtualizadas()
    {
        vetMatSatEvMv = BoostC::vector<ublas::matrix<int>>((numSats+1), ublas::matrix<int>(numEvMax, NUM_MV, 0));
    }

};
#endif //INC_2E_EVRP_SOLUCAO_H
