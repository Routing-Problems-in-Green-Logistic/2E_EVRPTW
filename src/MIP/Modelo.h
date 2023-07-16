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
#include "Variaveis.h"

typedef std::unordered_set<NS_VetorHash::VetorHash, NS_VetorHash::VetorHash::HashFunc> SetVetorHash;

namespace ModeloNs
{
    void modelo(const Instancia &instancia, const SetVetorHash &hashSolSet, const Solucao &solucao);
    void criaFuncObj(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis, const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotaEv);
    void criaRestParaRotasEVs(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis, const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotaEv);
    void setParametrosModelo(GRBModel &model);
}

#endif //INC_2E_EVRP_MODELO_H
