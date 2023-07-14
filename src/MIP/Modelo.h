/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Modelo.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_MODELO_H
#define INC_2E_EVRP_MODELO_H

#include <unordered_set>
#include "gurobi_c++.h"
#include "../Solucao.h"
#include "../VetorHash.h"

typedef std::unordered_set<NS_VetorHash::VetorHash, NS_VetorHash::VetorHash::HashFunc> SetVetorHash;

namespace ModeloNs
{

    class RotaEvMip
    {
    private:
        bool inicializado = false;

    public:

        EvRoute evRoute;
        double tempoSaidaMax;
        BoostC::vector<Int8> vetAtend;      // indica se um cliente eh atendido
        int sat;

        RotaEvMip(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash);
        RotaEvMip()= default;
        explicit RotaEvMip(int evRouteSizeMax, const Instancia &instancia);
        void inicializa(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash);
    };

    void modelo(const Instancia &instancia, const SetVetorHash &hashSolSet, const Solucao &solucao);
    void setParametrosModelo(GRBModel &model);
}

#endif //INC_2E_EVRP_MODELO_H
