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
using namespace NS_parametros;

void ModeloNs::modelo(Instancia &instancia,
                      const SetVetorHash &hashSolSet,
                      Solucao &solucao,
                      ParametrosMip paramMip)
{

/*
    string solStr;
    solucao.print(solStr, instancia, true);
    cout<<solStr<<"\n";
*/
    //cout<<"Solucao IG: "<<solucao.distancia<<"\n";
    //cout<<"Hash size: "<<hashSolSet.size()<<"\n\n";

    clock_t clockStart = clock();


    Vector<RotaEvMip> vetRotasEv(hashSolSet.size(), RotaEvMip(instancia.evRouteSizeMax, instancia));
    auto itHashSol = hashSolSet.begin();
    for(int i=0; i < hashSolSet.size(); ++i)
    {
        vetRotasEv[i].inicializa(instancia, (*itHashSol));
        ++itHashSol;
    }


    //Vector<RotaEvMip> vetRotasEv;
    //vetRotasEv.reserve(instancia.numEv);

    const int idRotaEvSolIni = vetRotasEv.size();

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
    Vector<int> vetNumRotasSat(instancia.numSats+1, 0);
    Matrix<std::list<int>> matRotasCliSat(instancia.numNos, (instancia.numSats+1));

    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotasEv[r];
        vetNumRotasSat[rotaEvMip.sat] += 1;

        for(int i=1; i < (rotaEvMip.evRoute.routeSize-1); ++i)
        {
            const int cliente = rotaEvMip.evRoute.route[i].cliente;
            if(instancia.isClient(cliente))
                matRotasCliSat(cliente, rotaEvMip.sat).push_back(r);
        }
    }


    // Armazena os indices das rotas para cada satelite
    Matrix<int> matrixSat(instancia.numSats+1, maiorElem(vetNumRotasSat, vetNumRotasSat.size()));
    std::fill(vetNumRotasSat.begin(), vetNumRotasSat.end(), 0);

    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotasEv[r];
        const int sat = rotaEvMip.sat;
        matrixSat(sat, vetNumRotasSat[sat]) = r;
        vetNumRotasSat[sat] += 1;
    }


    //cout << "vetSol size: " << vetRotasEv.size() << "\n";

    Solucao solModelo(instancia);

    try
    {

        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        setParametrosModelo(model, paramMip);

        Variaveis variaveis(instancia, model, vetRotasEv);

        criaFuncObj(instancia, model, variaveis, vetRotasEv);
        criaRestParaRotasEVs(instancia, model, variaveis, vetRotasEv, paramMip);
        criaRestVar_X(instancia, model, variaveis);
        criaRestVar_Dem(instancia, model, variaveis, matrixSat, vetNumRotasSat, vetRotasEv);
        criaRestVar_T(instancia, model, variaveis, vetRotasEv, matrixSat, vetNumRotasSat, matRotasCliSat, paramMip);
        setSolIniMip(model, solucao, idRotaEvSolIni, variaveis, instancia);

        model.update();
        model.optimize();

        recuperaSolucao(model, variaveis, solModelo, instancia, vetRotasEv, false);
        //std::list<std::unique_ptr<Solucao>> listPtrSol;
        //recuperaK_Solucoes(model, variaveis, solModelo, instancia, vetRotasEv, listPtrSol);

/*        int k=0;
        for(auto &ptr:listPtrSol)
            cout<<"Sol k="<<k++<<": "<<ptr->distancia<<"\n";*/

        //cout<<"Solucao IG: "<<solucao.distancia<<"\n";

        clock_t clockEnd = clock();
        //cout<<"Tempo total: "<<double(clockEnd-clockStart)/CLOCKS_PER_SEC<<" S\n";
        //double gap = ((solModelo.distancia-solucao.distancia)/solucao.distancia)*100.0;
        //cout<<"GAP: "<<gap<<"%\n\n";

        if(solModelo.distancia < solucao.distancia)
        {
            solucao.copia(solModelo);
            solucao.ultimaA = 2499;
        }

    }
    catch(GRBException &grbException)
    {
        cout<<"ERRO CODE: "<<grbException.getErrorCode()<<"\nMessage: ";
        cout<<grbException.getMessage()<<"\n";
    }
}

void ModeloNs::setParametrosModelo(GRBModel &model, NS_parametros::ParametrosMip paramMip)
{

    model.set(GRB_IntParam_OutputFlag, paramMip.outputFlag);
    model.set(GRB_StringAttr_ModelName, "2E_EVRP_TW__SET_COVER");
    model.set(GRB_IntParam_Threads, 1);

    model.set(GRB_IntParam_Presolve, paramMip.presolve);
    model.set(GRB_IntParam_Cuts, paramMip.cuts);

    model.set(GRB_DoubleParam_MIPGap, paramMip.mipGap);
    //model.set(GRB_IntParam_MIPFocus, GRB_MIPFOCUS_BESTBOUND);


/*    model.set(GRB_IntParam_PoolSolutions, 100);
    model.set(GRB_DoubleParam_PoolGap, paramMip.mipGap);//0.01);
    model.set(GRB_IntParam_PoolSearchMode, 2);*/

}

// $\textbf{Min}$:  $z = \sum\limits_{r \in Rota} D_r . y_r$  + $\sum\limits_{(i,j) \in A1} Dist_{(i,j)} . x_{(i,j)}$
void ModeloNs::criaFuncObj(const Instancia &instancia,
                           GRBModel &modelo,
                           VariaveisNs::Variaveis &variaveis,
                           const Vector<RotaEvMip> &vetRotaEv)
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
void ModeloNs::criaRestParaRotasEVs(const Instancia &instancia,
                                    GRBModel &modelo,
                                    VariaveisNs::Variaveis &variaveis,
                                    const Vector<VariaveisNs::RotaEvMip> &vetRotaEv,
                                    const NS_parametros::ParametrosMip &paramMip)
{
    // Pre processamento, cria para cada cliente as rotas que o possuem
    Matrix<int> matrix(instancia.numClients, variaveis.vetY.getNum());
    Vector<int> vetNumRotas(instancia.numClients, 0);
    Vector<int> vetNumRotasSat(instancia.numSats+1, 0);

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
    Matrix<int> matrixSat(instancia.numSats+1, maiorElem(vetNumRotasSat, vetNumRotasSat.size()));
    std::fill(vetNumRotasSat.begin(), vetNumRotasSat.end(), 0);

    for(int r=0; r < vetRotaEv.size(); ++r)
    {
        const RotaEvMip &rotaEvMip = vetRotaEv[r];
        const int sat = rotaEvMip.sat;
        matrixSat(sat, vetNumRotasSat[sat]) = r;
        vetNumRotasSat[sat] += 1;
    }

    // 1º Restricao: $\sum\limits_{r\in Rota} Atend_r^i . y_r = 1$     $\forall i \in V_c$

    const char tipoRest = (paramMip.restTempo==0)? GRB_GREATER_EQUAL : GRB_EQUAL;

    for(int i=firstClientIndex; i <= endClientIndex; ++i)
    {
        const int convI = funcConveteClienteIdex(i);
        GRBLinExpr linExpr;

        for(int posJ=0; posJ < vetNumRotas[convI]; ++posJ)
        {
            const int r = matrix(convI, posJ);
            linExpr += variaveis.vetY(r);
        }

        modelo.addConstr(linExpr, tipoRest, 1.0, "RotasEv_rest0_"+ to_string(i));
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

void ModeloNs::criaRestVar_X(const Instancia &instancia,
                             GRBModel &modelo,
                             VariaveisNs::Variaveis &variaveis)
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

void ModeloNs::criaRestVar_Dem(const Instancia &instancia,
                               GRBModel &modelo,
                               VariaveisNs::Variaveis &variaveis,
                               const Matrix<int> &matrixSat,
                               const Vector<int> &vetNumRotasSat,
                               const Vector<VariaveisNs::RotaEvMip> &vetRotasEv)
{

    // 7º Restricao: $\sum\limits_{j \in V_s^0, j \not= i} dem_{(j,i)}-\sum\limits_{j \in V_s^0, j \not= i} dem_{(i,j)}=
    //                 \sum\limits_{r' \in Rotas^i} Q_{r'}.y_{r'}$$\forall i \in V_s$

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        GRBLinExpr linExpr;

        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(sat != j)
                linExpr += variaveis.matrixDem(sat, j);
        }


        for(int j=0; j <= instancia.numSats; ++j)
        {
            if(sat != j)
                linExpr += -variaveis.matrixDem(j, sat);
        }

        GRBLinExpr linExpr1;
        for(int idR=0; idR < vetNumRotasSat[sat]; ++idR)
        {
            const int r = matrixSat(sat, idR);
            linExpr1 += -vetRotasEv[r].evRoute.demanda*variaveis.vetY(r);
        }

        linExpr += linExpr1;
        GRBLinExpr linExpr2 = linExpr1;
        //modelo.addConstr(variaveis.vetDemSat(sat)==-linExpr2);
        linExpr2 += variaveis.vetZ(sat);

        modelo.addConstr(linExpr, '=', 0, "VarDem_rest0_"+ to_string(sat));
        modelo.addConstr(linExpr2, GRB_LESS_EQUAL, 0.0, "VarZ_rest0_"+ to_string(sat));
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

void ModeloNs::criaRestVar_T(const Instancia &instancia,
                             GRBModel &modelo,
                             VariaveisNs::Variaveis &variaveis,
                             const Vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                             const Matrix<int> &matrixSat,
                             const Vector<int> &vetNumRotasSat,
                             const Matrix<std::list<int>> &matRotasCliSat,
                             const NS_parametros::ParametrosMip &paramMip)
{
    variaveis.vetT.setUB_LB(0.0, 0.0, 0);

    GRBLinExpr linExpr;
    double M = -DOUBLE_MAX;
    for(int r=0; r < vetRotasEv.size(); ++r)
    {
        if(vetRotasEv[r].tempoSaidaMax > M)
            M = vetRotasEv[r].tempoSaidaMax;
    }

    // 9º Restricao: $t^j \leq t^i + (Dist_{(i,j)}.x_{i,j}) + M.(1-x_{(i,j)})$ $\forall i \in V_s^0, \forall j \in V_s$
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

    if(paramMip.restTempo == 0)
    {   //cout<<"restTempo 0\n";
        // 10º Restricao: $t^i \leq ( T^{max}_r.Sat^i_r.y_r) +  M.(1 - Sat^i_r.y_r)$  $\forall i \in V_s, \forall r \in Rota$
        for(int sat = 1; sat <= instancia.numSats; ++sat)
        {

            for(int idR = 0; idR < vetNumRotasSat[sat]; ++idR)
            {
                const int r = matrixSat(sat, idR);
                linExpr = 0;
                linExpr += variaveis.vetT(sat);
                linExpr += -vetRotasEv[r].tempoSaidaMax * variaveis.vetY(r);
                linExpr += -M * (1 - variaveis.vetY(r));

                modelo.addConstr(linExpr, '<', 0, "VarT_rest1_" + to_string(sat) + "_" + to_string(r));
            }
        }
    }
    else
    {
        //cout<<"restTempo 1\n";
        // 10º Restricao: $t_i \leq \sum\limits_{r \in Rota_{c}^{cli} \cap Rota_{i}^{sat}} T^{max}_r . y_r + \sum\limits_{r \in Rota_{c}^{cli} \setminus Rota_{i}^{sat}}  M.y_r
        // \forall i \in V_s, \forall c \in V_c$

        for(int sat = 1; sat <= instancia.numSats; ++sat)
        {
            for(int c = instancia.getFirstClientIndex(); c <= instancia.getEndClientIndex(); ++c)
            {
                linExpr = 0;
                for(int r: matRotasCliSat(c, sat))
                    linExpr += -vetRotasEv[r].tempoSaidaMax * variaveis.vetY(r);

                for(int sat2 = 1; sat2 <= instancia.numSats; ++sat2)
                {
                    if(sat == sat2)
                        continue;

                    for(int r: matRotasCliSat(c, sat2))
                        linExpr += -M * variaveis.vetY(r);
                }

                linExpr += variaveis.vetT(sat);
                modelo.addConstr(linExpr, GRB_LESS_EQUAL, 0, "VarT_rest1_" + to_string(sat) + "_" + to_string(c));

            }
        }
    }
}


void
ModeloNs::recuperaSolucao(GRBModel &modelo,
                          VariaveisNs::Variaveis &variaveis,
                          Solucao &solucao, Instancia &instancia,
                          const Vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                          const bool Xn)
{
    solucao.resetaSol();

    variaveis.setVetDoubleAttr_X(modelo, Xn);

    Vector<int> rotasEvPorSat(instancia.numSats+1, 0);
    Vector<int> vetClienteAtend(instancia.numNos, 0);
    bool solmultCli = false;

    // Copias as rotas EVs para solucao
    for(int r=0; r < variaveis.vetY.getNum(); ++r)
    {
        if(variaveis.vetY.getX_value(r) < 0.99)
            continue;

        const int sat = vetRotasEv[r].sat;
        EvRoute &evRouteSol = solucao.satelites[sat].vetEvRoute[rotasEvPorSat[sat]];
        const int tempId = evRouteSol.idRota;
        evRouteSol.copia(vetRotasEv[r].evRoute, false, nullptr);
        evRouteSol.idRota = tempId;
        bool validoEv = true;

        // Percorre a rota verificando se o cliente ja esta coberto
        int routeSize = evRouteSol.routeSize;
        int i = 1;

        while(i < routeSize)
        {
            const int cliente = evRouteSol[i].cliente;

            if(!instancia.isClient(cliente))
            {
                i += 1;
                continue;
            }

            if(vetClienteAtend[cliente] == 0)
            {
                i += 1;
                vetClienteAtend[cliente] = 1;
                continue;
            }

            // evRoute resultante eh vazio
            if(evRouteSol.routeSize == 3)
            {
                evRouteSol.routeSize = 2;
                evRouteSol[1].cliente = sat;
                evRouteSol.distancia = 0.0;
                evRouteSol.demanda = 0.0;

                validoEv = false;
                break;
            }


            solmultCli = true;

            // Exclui cliente
            shiftVectorClienteEsq(evRouteSol.route, i, evRouteSol.routeSize);
            evRouteSol.routeSize -= 1;

            // Verifica se existe uma rs duplicada
            if(evRouteSol[i-1].cliente == evRouteSol[i].cliente)
            {
                shiftVectorClienteEsq(evRouteSol.route, i, evRouteSol.routeSize);
                evRouteSol.routeSize -= 1;
            }

            // Atualiza routeSize e exclui demanda de cliente
            routeSize = evRouteSol.routeSize;
            evRouteSol.demanda -= instancia.getDemand(cliente);
        }

        if(validoEv)
        {
            solucao.satelites[sat].demanda += evRouteSol.demanda;
            //solucao.satelites[sat].distancia += evRouteSol.distancia;

            rotasEvPorSat[sat] += 1;
        }

    }


    int proxCV = 0;
    Vector<double> tempoSaidaEv(instancia.numSats+1, -DOUBLE_MAX);

    solucao.distancia = 0.0;

/*    if(solmultCli)
        cout<<"\n\nSolucao mip com mais de um ev atendendo cada cliente\n";
    else
        cout<<"\n\nSolucao mip com 1 ev atendendo cada cliente\n";*/

    // Recupera as rotas dos CVs
    for(int satIni=1; satIni <= instancia.numSats; ++satIni)
    {
        if(variaveis.matrix_x.getX_value(0, satIni) < 0.99)
            continue;

        solucao.primeiroNivel[proxCV].rota[0].satellite = 0;
        solucao.primeiroNivel[proxCV].rota[0].tempoChegada = 0.0;

        solucao.primeiroNivel[proxCV].rota[1].satellite     = satIni;
        const double tempTempo = variaveis.vetT.getX_value(satIni);

        if(tempTempo > tempoSaidaEv[satIni])
            tempoSaidaEv[satIni] = tempTempo;

        solucao.primeiroNivel[proxCV].rota[1].tempoChegada = instancia.getDistance(0, satIni);
        solucao.primeiroNivel[proxCV].totalDistence = instancia.getDistance(0, satIni);
        solucao.primeiroNivel[proxCV].totalDemand = solucao.satelites[satIni].demanda;
        solucao.primeiroNivel[proxCV].satelliteDemand[satIni] = solucao.satelites[satIni].demanda;

        int prox = 2;
        int satI = satIni;

        do
        {
            bool encontrou = false;

            for(int satJ=0; satJ <= instancia.numSats; ++satJ)
            {
                if(satI != satJ && variaveis.matrix_x.getX_value(satI, satJ) >= 0.99)
                {

                    solucao.primeiroNivel[proxCV].rota[prox].satellite = satJ;
                    //solucao.primeiroNivel[proxCV].rota[prox].tempoChegada = variaveis.vetT.getX_value(satJ);
                    solucao.primeiroNivel[proxCV].rota[prox].tempoChegada = (solucao.primeiroNivel[proxCV].rota[prox-1].tempoChegada +
                                                                             + instancia.getDistance(satI, satJ));

                    solucao.primeiroNivel[proxCV].totalDistence += instancia.getDistance(satI, satJ);
                    if(satJ != 0)
                    {
                        solucao.primeiroNivel[proxCV].totalDemand = solucao.satelites[satJ].demanda;
                        solucao.primeiroNivel[proxCV].satelliteDemand[satJ] = solucao.satelites[satJ].demanda;
                        //const double tempTempo = variaveis.vetT.getX_value(satJ);
                        const double tempTempo = solucao.primeiroNivel[proxCV].rota[prox].tempoChegada;

                        if(tempTempo > tempoSaidaEv[satJ])
                            tempoSaidaEv[satJ] = tempTempo;
                    }

                    satI = satJ;
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
        //cout<<"\n\n";
        solucao.primeiroNivel[proxCV].routeSize = prox;
        solucao.distancia += solucao.primeiroNivel[proxCV].totalDistence;
        proxCV += 1;
    }

    //cout<<"vetor tempo saida EVs: \n";
    //for(int i=1; i <= instancia.getEndSatIndex(); ++i)
    //    cout<<i<<"  "<<tempoSaidaEv[i]<<"\n";

    //cout<<"\n";

    string strErro;

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        solucao.satelites[sat].distancia = 0.0;

        if(solucao.satelites[sat].demanda <= 0.0)
            continue;

        // Conserta tempo de saida dos EVs
        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            EvRoute &evRoute = solucao.satelites[sat].vetEvRoute[ev];
            if(evRoute.routeSize <= 2)
                continue;

            strErro = "";
            //evRoute.route[0].tempoSaida = tempoSaidaEv[sat];

            if(!evRoute.alteraTempoSaida(tempoSaidaEv[sat], instancia, true))
            {

                cout<<"ERRO na rotaEv("<<ev<<") do sat("<<sat<<")\n";
                cout<<"Nao foi possivel alterar o tempo de saida da rota para: "<<tempoSaidaEv[sat]<<"\n";
            }

            solucao.satelites[sat].distancia += evRoute.distancia;
        }

        solucao.distancia += solucao.satelites[sat].distancia;
    }


    //cout<<"Distancia solucao MIP: "<<solucao.distancia<<"\n";
    //cout<<"*****************************************************\n\nApos testar todas os EVs\n";

    //solucao.print(instancia);

    solucao.atualizaDemandaRoute(instancia);

    strErro = "";
    if(!solucao.checkSolution(strErro, instancia))
    {
        cout << "Sol inviavel!\n" << strErro << "\n";
        string strSol;
        solucao.print(strSol, instancia, true);
        cout<<strSol<<"\n";

        double demTotal = 0.0;
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
            demTotal += instancia.vectCliente[i].demanda;

        int numMin = NS_Auxiliary::upperVal(demTotal/instancia.getTruckCap(instancia.getFirstTruckIndex()));
        cout<<"Num Min Trucks: "<<numMin<<"\n\n";

        PRINT_DEBUG("", "");
        ERRO();
    }

    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        solucao.vetClientesAtend[i] = Int8(1);

}


void ModeloNs::recuperaK_Solucoes(GRBModel &modelo,
                                  VariaveisNs::Variaveis &variaveis,
                                  Solucao &solucao,
                                  Instancia &instancia,
                                  const Vector<VariaveisNs::RotaEvMip> &vetRotasEv,
                                  std::list<std::unique_ptr<Solucao>> &listSolucoes)
{
    const int numSol = modelo.get(GRB_IntAttr_SolCount);
    cout<<"Numero de Solucoes: "<<numSol<<"\n";
    if(numSol <= 1)
        return;

    for(int k=0; k < numSol; ++k)
    {
        modelo.set(GRB_IntParam_SolutionNumber, k);
        variaveis.setVetDoubleAttr_X(modelo, true);
        auto solPtr = std::make_unique<Solucao>(instancia);
        recuperaSolucao(modelo, variaveis, *solPtr.get(), instancia, vetRotasEv, true);

        if(k == 1)
            solPtr->print(instancia);

        listSolucoes.push_back(std::move(solPtr));
    }

}

void ModeloNs::setSolIniMip(GRBModel &model,
                            const Solucao &solucao,
                            const int idRotaEvSolIni,
                            VariaveisNs::Variaveis &variaveis,
                            const Instancia &instancia)
{
    const bool solEhSplit = solucao.ehSplit(instancia);

    if(!solEhSplit)
        variaveis.setAttr_Start0();

    for(int i=idRotaEvSolIni; i < variaveis.vetY.getNum(); ++i)
        variaveis.vetY(i).set(GRB_DoubleAttr_Start, 1.0);

    if(solEhSplit)
        return;

    for(int cv=0; cv < instancia.numTruck; ++cv)
    {
        const Route &route = solucao.primeiroNivel[cv];
        if(route.routeSize <= 2)
            continue;

        int cliI = 0;
        int cliJ = -1;

        double carga = 0.0;

        for(int i=1; i < route.routeSize; ++i)
        {
            cliJ = route.rota[i].satellite;
            variaveis.matrixDem(cliI, cliJ).set(GRB_DoubleAttr_Start, carga);

//cout<<"set: ("<<cliI<<", "<<cliJ<<"); ";

            if(cliJ != 0)
            {
                variaveis.vetT(cliJ).set(GRB_DoubleAttr_Start, route.rota[i].tempoChegada);
                variaveis.vetZ(cliJ).set(GRB_DoubleAttr_Start, 1.0);
                carga += route.satelliteDemand[cliJ];

//cout<<"T("<<cliJ<<"): "<<route.rota[i].tempoChegada;

            }

            variaveis.matrix_x(cliI, cliJ).set(GRB_DoubleAttr_Start, 1.0);
            std::swap(cliI, cliJ);

//cout<<"\n";

        }
    }
}
