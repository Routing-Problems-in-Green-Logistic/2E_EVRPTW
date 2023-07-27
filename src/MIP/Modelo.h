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
    void modelo(Instancia &instancia, const SetVetorHash &hashSolSet, Solucao &solucao);

    void criaFuncObj(const Instancia &instancia,
                     GRBModel &modelo,
                     VariaveisNs::Variaveis &variaveis,
                     const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotaEv);

    void criaRestParaRotasEVs(const Instancia &instancia,
                              GRBModel &modelo,
                              VariaveisNs::Variaveis &variaveis,
                              const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotaEv);

    void criaRestVar_X(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis);

    void criaRestVar_Dem(const Instancia &instancia,
                         GRBModel &modelo,
                         VariaveisNs::Variaveis &variaveis,
                         const ublas::matrix<int> &matrixSat,
                         const BoostC::vector<int> &vetNumRotasSat,
                         const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv);

    void criaRestVar_T(const Instancia &instancia,
                       GRBModel &modelo,
                       VariaveisNs::Variaveis &variaveis,
                       const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                       const ublas::matrix<int> &matrixSat,
                       const BoostC::vector<int> &vetNumRotasSat);

    void recuperaSolucao(GRBModel &modelo,
                         VariaveisNs::Variaveis &variaveis,
                         Solucao &solucao,
                         const Instancia &instancia,
                         const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv);

    void setParametrosModelo(GRBModel &model);

    void setSolIniMip(GRBModel &model,
                      const Solucao &solucao,
                      const int idRotaEvSolIni,
                      VariaveisNs::Variaveis &variaveis,
                      const Instancia &instancia);
}

#endif //INC_2E_EVRP_MODELO_H
