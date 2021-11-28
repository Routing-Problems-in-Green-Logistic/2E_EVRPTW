#ifndef INC_2E_EVRP_GREEDYALGORITHM_H
#define INC_2E_EVRP_GREEDYALGORITHM_H

#include "Solution.h"
#include <list>
#include <vector>

namespace GreedyAlgNS
{

    bool secondEchelonGreedy(Solution& Sol, const Instance& Inst, float alpha);
    void firstEchelonGreedy(Solution &Sol, const Instance &Inst, const float beta);
    void greedy(Solution &Sol, const Instance &Inst, const float alpha, const float beta);
    bool visitAllClientes(std::vector<int> &visitedClients, const Instance &Inst);
    bool existeDemandaNaoAtendida(std::vector<float> &demandaNaoAtendida);

    class Candidato
    {
    public:

        int rotaId = -1;
        int satelliteId = -1;
        float demand = 0.0;
        float incrementoDistancia = 0.0;
        int pos = -1;

        Candidato(int _rotaId, int _satelliteId, float _demand, float _incrDist):rotaId(_rotaId), satelliteId(_satelliteId),
            demand(_demand), incrementoDistancia(_incrDist){};

        bool operator < (const Candidato &candidato) const {return incrementoDistancia < candidato.incrementoDistancia;}

    };
}

#endif //INC_2E_EVRP_GREEDYALGORITHM_H

