/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Modelo.cpp
 * ****************************************
 * ****************************************/

#include "Modelo.h"
#include "gurobi_c++.h"
#include "Variaveis.h"
#include "../ViabilizadorRotaEv.h"

using namespace ModeloNs;
using namespace VariaveisNs;
using namespace NS_viabRotaEv;
using namespace NS_VetorHash;

void ModeloNs::modelo(const Instancia &instancia, const SetVetorHash &hashSolSet, const Solucao &solucao)
{
    //cout<<"Hash size: "<<hashSolSet.size()<<"\n\n";

    /*
    BoostC::vector<RotaEvMip> vetHashSol(hashSolSet.size(), RotaEvMip(instancia.evRouteSizeMax, instancia));
    auto itHashSol = hashSolSet.begin();
    for(int i=0; i < hashSolSet.size(); ++i)
    {
        vetHashSol[i].inicializa(instancia, (*itHashSol));
        ++itHashSol;
    }
    */

    BoostC::vector<RotaEvMip> vetRotasEv;
    vetRotasEv.reserve(instancia.numEv);

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        const Satelite &satelite = solucao.satelites[sat];

        if(satelite.demanda <= 0.0)
            continue;

        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            const EvRoute &evRoute = satelite.vetEvRoute[ev];
            if(evRoute.routeSize <= 2)
                continue;

            // Verificar se a rota atende pelo menos um cliente
            for(int i=1; i < evRoute.routeSize; ++i)
            {
                if(instancia.isClient(evRoute.route[i].cliente))
                {
                    vetRotasEv.push_back(RotaEvMip(instancia, evRoute));
                    break;
                }
            }
        }
    }

    cout << "vetSol size: " << vetRotasEv.size() << "\n";

    try
    {

        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        setParametrosModelo(model);

        Variaveis variaveis(instancia, model, vetRotasEv);

        criaFuncObj(instancia, model, variaveis, vetRotasEv);
        criaRestParaRotasEVs(instancia, model, variaveis, vetRotasEv);

        model.update();
        model.write("modelo.lp");
        model.optimize();
    }
    catch(GRBException &grbException)
    {
        cout<<"ERRO CODE: "<<grbException.getErrorCode()<<"\nMessage: ";
        cout<<grbException.getMessage()<<"\n";
    }
}

void ModeloNs::setParametrosModelo(GRBModel &model)
{

    model.set(GRB_IntParam_Threads, 1);

}

// $\textbf{Min}$:  $z = \sum\limits_{r \in Rota} D_r . y_r$  + $\sum\limits_{(i,j) \in A1} Dist_{(i,j)} . x_{(i,j)}$
void ModeloNs::criaFuncObj(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis, const BoostC::vector<RotaEvMip> &vetRotaEv)
{
    GRBLinExpr obj;

    // Parte das variaveis y(rotas) da func. obj.
    for(int r=0; r < variaveis.vetY.getNum(); ++r)
        obj += vetRotaEv[r].evRoute.distancia*variaveis.vetY(r);

    // Parte das variaveis x(i,j)
    for(int i=0; i <= instancia.numSats; ++i)
    {
        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(j==i)
                continue;

            obj += instancia.getDistance(i,j)*variaveis.matrix_x(i,j);
        }
    }

    modelo.setObjective(obj, GRB_MINIMIZE);
}

/*
 * Cria as restricoes relacionadas a variavel y(rotas EVs)
 *
 * $\sum\limits_{r\in Rota} Atend_r^i . y_r \geq 1$     $\forall i \in V_c$
 * $\sum\limits_{r' \in Rotas^i} y_r \leq z_i . |EV|$   $\forall i \in V_s$
 * $\sum\limits_{r \in Rotas} y_r \leq |EV|$
 *
 * @param instancia
 * @param modelo
 * @param variaveis
 */
void ModeloNs::criaRestParaRotasEVs(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis, const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotaEv)
{
    // Pre processamento, cria para cada cliente as rotas que o possuem
    ublas::matrix<int> matrix(instancia.numClients, variaveis.vetY.getNum());
    BoostC::vector<int>   vetNumRotas(instancia.numClients, 0);
    const int firstClientIndex = instancia.getFirstClientIndex();
    const int endClientIndex   = instancia.getEndClientIndex();

    auto funcConveteClienteIdex = [&](int i){return (i-firstClientIndex);};

    for(int r=0; r < vetRotaEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotaEv[r];

        for(int i=firstClientIndex; i <= endClientIndex; ++i)
        {
            if(rotaEvMip.vetAtend[i] == Int8(1))
            {
                const int convI = funcConveteClienteIdex(i);

                matrix(convI, vetNumRotas[convI]) = r;
                vetNumRotas[convI] += 1;
            }
        }
    }

    // 1º Restricao: $\sum\limits_{r\in Rota} Atend_r^i . y_r \geq 1$     $\forall i \in V_c$

    for(int i=firstClientIndex; i <= endClientIndex; ++i)
    {
        const int convI = funcConveteClienteIdex(i);
        GRBLinExpr linExpr;

        for(int posJ=0; posJ < vetNumRotas[convI]; ++posJ)
        {
            const int r = matrix(convI, posJ);
            linExpr += variaveis.vetY(r);
        }

        modelo.addConstr(linExpr, '>', 1.0, "RotasEv_rest0_"+ to_string(i));
    }

    // 2º Restricao:
}
