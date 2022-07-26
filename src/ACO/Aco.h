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

        Ant(Instance &instance, int satId): satelite(Satelite(instance, satId))
        {
            vetNosAtend = vector<int8_t>(instance.numNos, 0);
        }
    };

    void aco(Instance &instance, AcoParametros &acoPar, AcoEstatisticas &acoEst, const vector<int8_t> &clientes, int sateliteId, Satelite &satBest);
    void atualizaFeromonio(ublas::matrix<double> &matFeromonio, Satelite &satelite);
    void evaporaFeromonio(ublas::matrix<double> &matFeromonio, int iInic, int iFim, int jInic, int jFim);

    inline double getDistSat(Satelite &satelite)
    {
        return satelite.distancia;
    }
}

#endif //INC_2E_EVRP_ACO_H
