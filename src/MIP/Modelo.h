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
#include "Parametros.h"

typedef std::unordered_set<NS_VetorHash::VetorHash, NS_VetorHash::VetorHash::HashFunc> SetVetorHash;

namespace ModeloNs
{
    void modelo(Instancia &instancia, const SetVetorHash &hashSolSet, Solucao &solucao, NS_parametros::ParametrosMip paramMip);

    void criaFuncObj(const Instancia &instancia,
                     GRBModel &modelo,
                     VariaveisNs::Variaveis &variaveis,
                     const Vector<VariaveisNs::RotaEvMip> &vetRotaEv);

    void criaRestParaRotasEVs(const Instancia &instancia,
                              GRBModel &modelo,
                              VariaveisNs::Variaveis &variaveis,
                              const Vector<VariaveisNs::RotaEvMip> &vetRotaEv,
                              const NS_parametros::ParametrosMip &paramMip);

    void criaRestVar_X(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis);

    void criaRestVar_Dem(const Instancia &instancia,
                         GRBModel &modelo,
                         VariaveisNs::Variaveis &variaveis,
                         const Matrix<int> &matrixSat,
                         const Vector<int> &vetNumRotasSat,
                         const Vector<VariaveisNs::RotaEvMip> &vetRotasEv);

    void criaRestVar_T(const Instancia &instancia,
                       GRBModel &modelo,
                       VariaveisNs::Variaveis &variaveis,
                       const Vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                       const Matrix<int> &matrixSat,
                       const Vector<int> &vetNumRotasSat,
                       const Matrix<std::list<int>> &matRotasCliSat,
                       const NS_parametros::ParametrosMip &paramMip);

    void recuperaSolucao(GRBModel &modelo, VariaveisNs::Variaveis &variaveis, Solucao &solucao, Instancia &instancia,
                         const Vector<VariaveisNs::RotaEvMip> &vetRotasEv, const bool Xn);

    void recuperaK_Solucoes(GRBModel &modelo,
                            VariaveisNs::Variaveis &variaveis,
                            Solucao &solucao,
                            Instancia &instancia,
                            const Vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                            std::list<std::unique_ptr<Solucao>> &listSolucoes);

    void setParametrosModelo(GRBModel &model, NS_parametros::ParametrosMip paramMip);

    void setSolIniMip(GRBModel &model,
                      const Solucao &solucao,
                      const int idRotaEvSolIni,
                      VariaveisNs::Variaveis &variaveis,
                      const Instancia &instancia);
}

#endif //INC_2E_EVRP_MODELO_H
