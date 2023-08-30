#ifndef INC_2E_EVRP_SOLUCAO_H
#define INC_2E_EVRP_SOLUCAO_H

#include "Satelite.h"




class Solucao
{
public:
    explicit Solucao(Instancia& inst);
    Solucao(const Solucao &solution) = delete;
    void copia(Solucao &solution);

    int getNSatelites() const;
    Satelite* getSatelite(int index);
    bool checkSolution(std::string &erro, const Instancia &inst);
    void print(std::string &saida, const Instancia &instance, const bool somenteNo);
    void print(const Instancia& Inst);
    void printPlot(std::string &saida, const Instancia &instance);
    void atualizaVetSatTempoChegMax(const Instancia &instance);
    void inicializaVetClientesAtend(Instancia &instance);
    double getDist1Nivel();
    double getDist2Nivel();
    void recalculaDist();
    void resetaPrimeiroNivel(Instancia &instancia);  // Recalcula distancia com somente a dist dos satelites!
    void recalculaDistSat(Instancia &instancia);
    int numSatVazios();
    int getNumEvNaoVazios();
    void resetaSol();
    void resetaSat(int satId, Instancia &instancia, Vector<int> &vetClienteDel);    // Somente o sat eh alterado, 1 nivel nao se altera
    void reseta1Nivel(Instancia &instancia);
    void resetaIndiceEv(Instancia &instancia);

    bool ehSplit(const Instancia &instancia)const;
    void atualizaDemandaRoute(const Instancia &instancia);

    // Possui numSat + 1 !!
    Vector<Satelite> satelites;

    // Guarda o ultimo tempo de chegada do veiculo do 1° nivel
    Vector<double> satTempoChegMax;
    int numTrucksMax = -1;
    int numEvMax     = -1;
    int numEv        = 0;                   // Num Ev utilizados
    int numSats      = 1;
    bool viavel      = true;
    Vector<Route> primeiroNivel;
    double distancia = 0.0;
    Vector<int8_t> vetClientesAtend;

    int ultimaA = -1;


    Vector<Matrix<int>> vetMatSatEvMv;  // Sat, ev, mv
    /*
     * Se vetMatSatEvMv[1](0,0) (sat 1, ev 0, mv 0) = 1:
     *
     *  Entao, nao eh necessario refazer o mv 0 no ev 0 do sat 1 se nao houver alteracao nessa rota!
     */

    void rotaEvAtualizada(int sat, int ev);
    void todasRotasEvAtualizadas();

};
#endif //INC_2E_EVRP_SOLUCAO_H
//sat 7; sat novo: 6