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
#include "../Auxiliary.h"

using namespace ModeloNs;
using namespace VariaveisNs;
using namespace NS_viabRotaEv;
using namespace NS_VetorHash;
using namespace NS_Auxiliary;

void ModeloNs::modelo(Instancia &instancia, const SetVetorHash &hashSolSet, const Solucao &solucao)
{
    cout<<"Solucao IG: "<<solucao.distancia<<"\n";

    //cout<<"Hash size: "<<hashSolSet.size()<<"\n\n";


    BoostC::vector<RotaEvMip> vetRotasEv(hashSolSet.size(), RotaEvMip(instancia.evRouteSizeMax, instancia));
    auto itHashSol = hashSolSet.begin();
    for(int i=0; i < hashSolSet.size(); ++i)
    {
        vetRotasEv[i].inicializa(instancia, (*itHashSol));
        ++itHashSol;
    }


    //BoostC::vector<RotaEvMip> vetRotasEv;
    //vetRotasEv.reserve(instancia.numEv);


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



    //cria para cada sat o num de rotas existentes
    BoostC::vector<int> vetNumRotasSat(instancia.numSats+1, 0);
    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotasEv[r];
        vetNumRotasSat[rotaEvMip.sat] += 1;
    }


    // Armazena os indices das rotas para cada satelite
    ublas::matrix<int> matrixSat(instancia.numSats+1, maiorElem(vetNumRotasSat, vetNumRotasSat.size()));
    std::fill(vetNumRotasSat.begin(), vetNumRotasSat.end(), 0);

    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotasEv[r];
        const int sat = rotaEvMip.sat;
        matrixSat(sat, vetNumRotasSat[sat]) = r;
        vetNumRotasSat[sat] += 1;
    }


    cout << "vetSol size: " << vetRotasEv.size() << "\n";

    Solucao solModelo(instancia);

    try
    {

        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        setParametrosModelo(model);

        Variaveis variaveis(instancia, model, vetRotasEv);

        criaFuncObj(instancia, model, variaveis, vetRotasEv);
        criaRestParaRotasEVs(instancia, model, variaveis, vetRotasEv);
        criaRestVar_X(instancia, model, variaveis);
        criaRestVar_Dem(instancia, model, variaveis, matrixSat, vetNumRotasSat, vetRotasEv);
        criaRestVar_T(instancia, model, variaveis, vetRotasEv, matrixSat, vetNumRotasSat);

        model.update();
        model.write("modelo.lp");
        model.optimize();
        model.write("modelo.sol");

        recuperaSolucao(model, variaveis, solModelo, instancia, vetRotasEv);
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
    BoostC::vector<int> vetNumRotas(instancia.numClients, 0);
    BoostC::vector<int> vetNumRotasSat(instancia.numSats+1, 0);

    const int firstClientIndex = instancia.getFirstClientIndex();
    const int endClientIndex   = instancia.getEndClientIndex();

    auto funcConveteClienteIdex = [&](int i){return (i-firstClientIndex);};

    for(int r=0; r < vetRotaEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotaEv[r];
        vetNumRotasSat[rotaEvMip.sat] += 1;

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

    // Armazena os indices das rotas para cada satelite
    ublas::matrix<int> matrixSat(instancia.numSats+1, maiorElem(vetNumRotasSat, vetNumRotasSat.size()));
    std::fill(vetNumRotasSat.begin(), vetNumRotasSat.end(), 0);

    for(int r=0; r < vetRotaEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotaEv[r];
        const int sat = rotaEvMip.sat;
        matrixSat(sat, vetNumRotasSat[sat]) = r;
        vetNumRotasSat[sat] += 1;
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

    // 2º Restricao: $\sum\limits_{r' \in Rotas^i} y_r \leq z_i . |EV|$  $\forall i \in V_s$

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        GRBLinExpr linExpr;
        for(int i=0; i < vetNumRotasSat[sat]; ++i)
        {
            const int r = matrixSat(sat, i);
            linExpr += variaveis.vetY(r);
        }

        modelo.addConstr(linExpr <= instancia.numEv*variaveis.vetZ(sat), "RotasEv_rest1_"+ to_string(sat));

    }

    // 3º Restricao: $\sum\limits_{r \in Rotas} y_r \leq |EV|$
    GRBLinExpr linExpr;
    for(int r=0; r < variaveis.vetY.getNum(); ++r)
        linExpr += variaveis.vetY(r);

    modelo.addConstr(linExpr <= instancia.numEv, "RotasEv_rest2");
}

void ModeloNs::criaRestVar_X(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis)
{
    // 4º Restricao: $CV_{min} \leq \sum\limits_{j \in V_s} x_{(0,j)} \leq |CV|$
    GRBLinExpr linExpr4;

    for(int i=1; i <= instancia.numSats; ++i)
        linExpr4 += variaveis.matrix_x(0,i);

    double sumDem = 0.0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        sumDem += instancia.vectCliente[i].demanda;

    const int cvMin = upperVal(sumDem/instancia.getTruckCap(instancia.getFirstTruckIndex()));
    modelo.addConstr(linExpr4, '>', cvMin, "VarX_rest0_0");
    modelo.addConstr(linExpr4, '<', instancia.numTruck, "VarX_rest0_1");

    // 5º Restricao: $\sum\limits_{j \in V_{s}^0, j \not= i} x_{(i,j)} = \sum\limits_{j \in V_{s}^0, j \not= i} x_{(j,i)}$ $\forall i \in V_s^0$
    for(int i = 0; i <= instancia.numSats; ++i)
    {
        GRBLinExpr linExpr;

        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(j!=i)
                linExpr += variaveis.matrix_x(i, j);
        }


        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(j!=i)
                linExpr += -variaveis.matrix_x(j, i);
        }

        modelo.addConstr(linExpr, '=', 0.0, "VarX_rest1_"+ to_string(i));
    }

    // 6º Restricao: $\sum\limits_{i\in V_s^0, i\not=j} x_{(i,j)} = z_j$ $\forall j \in V_S$
    for(int j=1; j <= instancia.numSats; ++j)
    {
        GRBLinExpr linExpr;

        for(int i=0; i <= instancia.numSats; ++i)
        {
            if(i!=j)
                linExpr += variaveis.matrix_x(i,j);
        }

        linExpr += -variaveis.vetZ(j);
        modelo.addConstr(linExpr, '=', 0, "VarX_rest2_"+ to_string(j));
    }


/*    for(int i=1; i <= instancia.numSats; ++i)
    {
        GRBLinExpr linExpr;

        for(int j = 1; j <= instancia.numSats; ++j)
        {
            if(i==j)
                continue;

            linExpr = 0;
            linExpr += variaveis.matrix_x(i,j);
            linExpr += variaveis.matrix_x(j,i);
            modelo.addConstr(linExpr, '<', 1);
        }
    }*/
}


void ModeloNs::criaRestVar_Dem(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis,
                               const ublas::matrix<int> &matrixSat, const BoostC::vector<int> &vetNumRotasSat,
                               const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv)
{

    // 7º Restricao: $\sum\limits_{j \in V_s^0, j \not= i} dem_{(j,i)}-\sum\limits_{j \in V_s^0, j \not= i} dem_{(i,j)}=
    //                 \sum\limits_{r' \in Rotas^i} Q_{r'}.y_{r'}$$\forall i \in V_s$

    for(int i=1; i <= instancia.numSats; ++i)
    {
        GRBLinExpr linExpr;

        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(i!=j)
                linExpr += variaveis.matrixDem(i,j);
        }


        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(i!=j)
                linExpr += -variaveis.matrixDem(j,i);
        }

        for(int idR=0; idR < vetNumRotasSat[i]; ++idR)
        {
            const int r = matrixSat(i, idR);
            linExpr += -vetRotasEv[r].evRoute.demanda*variaveis.vetY(r);
        }

        modelo.addConstr(linExpr, '=', 0, "VarDem_rest0_"+ to_string(i));
    }


    // 8º Restricao: $dem_{(i,j)} \leq Cap_{CV}.x_{ij}$ $\forall (i,j) \in A1$
    const double capCV = instancia.getTruckCap(instancia.getFirstTruckIndex());
    for(int i=0; i <= instancia.numSats; ++i)
    {

        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(i!=j)
                modelo.addConstr(variaveis.matrixDem(i,j)-capCV*variaveis.matrix_x(i,j), '<', 0, "VarDem_rest1_"+
                                                                                    to_string(i)+"_"+to_string(j));


        }
    }


    // 9º Restricao: $\sum\limits_{i \in V_s} dem_{(i,0)}= 0$
    GRBLinExpr linExpr;
    for(int i=1; i <= instancia.numSats; ++i)
        linExpr += variaveis.matrixDem(0,i);

    modelo.addConstr(linExpr, '=', 0, "VarDem_rest2");
}

void ModeloNs::criaRestVar_T(const Instancia &instancia, GRBModel &modelo, VariaveisNs::Variaveis &variaveis,
                             const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv, const ublas::matrix<int> &matrixSat,
                             const BoostC::vector<int> &vetNumRotasSat)
{
    variaveis.vetT.setUB_LB(0.0, 0.0, 0);

    GRBLinExpr linExpr;
    double M = -DOUBLE_MAX;
    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        if(vetRotasEv[r].tempoSaidaMax > M)
            M = vetRotasEv[r].tempoSaidaMax;
    }

    // 10º Restricao: $t^j \leq t^i + (Dist_{(i,j)}.x_{i,j}) + M.(1-x_{(i,j)})$ $\forall i \in V_s^0, \forall j \in V_s$
    for(int i=0; i <= instancia.numSats; ++i)
    {

        for(int j = 1; j <= instancia.numSats; ++j)
        {
            if(i != j)
            {
                linExpr = 0;
                //linExpr += variaveis.vetT(j);
                linExpr += variaveis.vetT(i);
                linExpr += instancia.getDistance(i,j)*variaveis.matrix_x(i,j);
                linExpr += -M*(1-variaveis.matrix_x(i,j));

                modelo.addConstr(variaveis.vetT(j) >= linExpr, "VarT_rest0_"+ to_string(i)+"_"+ to_string(j));

            }
        }
    }

    // 11º Restricao: $t^i \leq (\lceil T^{max}_r\rceil.Sat^i_r.y_r)$ $\forall i \in V_s, \forall r \in Rota$
    for(int sat=1; sat <= instancia.numSats; ++sat)
    {

        for(int idR=0; idR < vetNumRotasSat[sat]; ++idR)
        {
            const int r = matrixSat(sat, idR);
            linExpr = 0;
            linExpr += variaveis.vetT(sat);
            linExpr += -vetRotasEv[r].tempoSaidaMax*variaveis.vetY(r);

            modelo.addConstr(linExpr, '>', 0, "VarT_rest1_"+ to_string(sat) + "_" + to_string(r));
        }
    }
}


void ModeloNs::recuperaSolucao(GRBModel &modelo, VariaveisNs::Variaveis &variaveis, Solucao &solucao, const Instancia &instancia,
                               const BoostC::vector<VariaveisNs::RotaEvMip> &vetRotasEv)
{
    solucao.resetaSol();

    variaveis.setVetDoubleAttr_X(modelo);

    BoostC::vector<int> rotasEvPorSat(instancia.numSats+1, 0);

    // Copias as rotas EVs para solucao
    for(int r=0; r < variaveis.vetY.getNum(); ++r)
    {
        if(variaveis.vetY.getX_value(r) >= 0.99)
        {
            const int sat = vetRotasEv[r].sat;
            EvRoute &evRouteSol = solucao.satelites[sat].vetEvRoute[rotasEvPorSat[sat]];
            evRouteSol.copia(vetRotasEv[r].evRoute, false, nullptr);
            evRouteSol.idRota = rotasEvPorSat[sat];
            solucao.satelites[sat].demanda += evRouteSol.demanda;
            solucao.satelites[sat].distancia += evRouteSol.distancia;

            rotasEvPorSat[sat] += 1;
        }
    }

    int proxCV = 0;

    // Recupera as rotas dos CVs
    for(int satIni=1; satIni <= instancia.numSats; ++satIni)
    {
        if(variaveis.matrix_x.getX_value(0, satIni) < 0.99)
            continue;

        solucao.primeiroNivel[proxCV].rota[0].satellite = 0;
        solucao.primeiroNivel[proxCV].rota[0].tempoChegada = 0;

        solucao.primeiroNivel[proxCV].rota[1].satellite     = satIni;
        solucao.primeiroNivel[proxCV].rota[1].tempoChegada  = variaveis.vetT.getX_value(satIni);
        solucao.primeiroNivel[proxCV].totalDistence = instancia.getDistance(0, satIni);
        solucao.primeiroNivel[proxCV].totalDemand = solucao.satelites[satIni].demanda;

        int prox = 2;
        int satI = satIni;

        cout<<"0\n"<<satIni<<"\n";

        do
        {
            bool encontrou = false;

            for(int satJ=0; satJ <= instancia.numSats; ++satJ)
            {
                if(satI != satJ && variaveis.matrix_x.getX_value(satI, satJ) >= 0.99)
                {
                    cout<<"prox: "<<prox<<"\n";

                    solucao.primeiroNivel[proxCV].rota[prox].satellite = satJ;
                    solucao.primeiroNivel[proxCV].rota[prox].tempoChegada = variaveis.vetT.getX_value(satJ);

                    solucao.primeiroNivel[proxCV].totalDistence += instancia.getDistance(satI, satJ);
                    if(satJ != 0)
                        solucao.primeiroNivel[proxCV].totalDemand = solucao.satelites[satJ].demanda;

                    satI = satJ;
                    cout<<satI<<"\n";
                    prox += 1;
                    encontrou = true;
                    break;
                }
            }

            if(!encontrou)
            {
                PRINT_DEBUG("", "");
                cout<<"ERRO, Nao foi encontrado uma variavel x=1 para X("<<satI<<", _)\n";
                ERRO();
            }

            //ERRO();
        }
        while(satI != 0);
        cout<<"\n\n";
        solucao.primeiroNivel[proxCV].routeSize = prox;
        solucao.distancia += solucao.primeiroNivel[proxCV].totalDistence;
        proxCV += 1;

    }


    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        solucao.distancia += solucao.satelites[sat].distancia;
    }

    cout<<"Distancia solucao MIP: "<<solucao.distancia<<"\n";
    solucao.print(instancia);

}
