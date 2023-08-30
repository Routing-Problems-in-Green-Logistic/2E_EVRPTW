/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Variaveis.cpp
 * ****************************************
 * ****************************************/

#include "Variaveis.h"
#include "../ViabilizadorRotaEv.h"


using namespace VariaveisNs;
using namespace NS_viabRotaEv;
using namespace NS_VetorHash;

MatrixGRBVar::MatrixGRBVar(GRBModel &model, int numLin_, int numCol_, const string &&nome, char type)
{
    inicializado = true;
    numLin = numLin_;
    numCol = numCol_;
    typeVar = type;
    vetVar = model.addVars(numLin*numCol, type);

    for(int i=0; i < numLin; ++i)
    {

        for(int j=0; j < numCol; ++j)
        {
            const int index = i*numCol+j;
            vetVar[index].set(GRB_StringAttr_VarName, nome+"_("+ to_string(i)+","+ to_string(j)+")");

            if(i == j)
            {
                vetVar[index].set(GRB_DoubleAttr_LB, 0.0);
                vetVar[index].set(GRB_DoubleAttr_UB, 0.0);
            }
        }
    }

}

void MatrixGRBVar::setUB(const double ub)
{
    for(int i=0; i < numCol*numLin; ++i)
        vetVar[i].set(GRB_DoubleAttr_UB, ub);
}

void MatrixGRBVar::setLB(double lb)
{

    for(int i=0; i < numCol*numLin; ++i)
        vetVar[i].set(GRB_DoubleAttr_LB, lb);
}

void MatrixGRBVar::setUB_LB(double ub, double lb)
{

    for(int i=0; i < numCol*numLin; ++i)
    {
        vetVar[i].set(GRB_DoubleAttr_UB, ub);
        vetVar[i].set(GRB_DoubleAttr_LB, lb);
    }
}

void MatrixGRBVar::printVars()
{
    for(int i=0; i < numLin; ++i)
    {
        for(int j=0; j < numCol; ++j)
        {
            const int index = i * numCol + j;
            const auto nome = vetVar[index].get(GRB_StringAttr_VarName);
            cout<<nome<<" ";
        }
        cout<<"\n";
    }
}

void MatrixGRBVar::inicializa(GRBModel &model, int numLin_, int numCol_, const string &&nome, char type)
{
    if(inicializado)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, matrix ja foi inicializada";
        ERRO();
    }

    inicializado = true;

    numLin = numLin_;
    numCol = numCol_;
    typeVar = type;
    vetVar = model.addVars(numLin*numCol, type);

    for(int i=0; i < numLin; ++i)
    {

        for(int j=0; j < numCol; ++j)
        {
            const int index = i*numCol+j;
            vetVar[index].set(GRB_StringAttr_VarName, nome+"_("+ to_string(i)+","+ to_string(j)+")");

            if(i == j)
            {
                vetVar[index].set(GRB_DoubleAttr_LB, 0.0);
                vetVar[index].set(GRB_DoubleAttr_UB, 0.0);
            }
        }
    }

}

VariaveisNs::Variaveis::Variaveis(const Instancia &instancia, GRBModel &modelo, const Vector<RotaEvMip> &vetRotasEv)
{
    numVertices = instancia.numSats+1;

    matrix_x.inicializa(modelo, numVertices, numVertices, "x", GRB_BINARY);
    matrixDem.inicializa(modelo, numVertices, numVertices, "dem", GRB_CONTINUOUS);

    vetY.inicializa(modelo, int(vetRotasEv.size()), "y", GRB_BINARY);
    vetZ.inicializa(modelo, numVertices, "z", GRB_BINARY);
    vetT.inicializa(modelo, numVertices, "t", GRB_CONTINUOUS);

    modelo.update();

    vetZ.setUB_LB(0.0, 0.0, 0);

    /*
    matrix_x.printVars();
    matrixDem.printVars();

    cout<<"apos printVars\n";

    vetY.printVars();
    vetZ.printVars();
    vetT.printVars();
     */

}

void Variaveis::setVetDoubleAttr_X(GRBModel &model, bool Xn)
{
    matrix_x.setVetDoubleAttr_X(model, Xn);
    matrixDem.setVetDoubleAttr_X(model, Xn);
    vetZ.setVetDoubleAttr_X(model, Xn);
    vetY.setVetDoubleAttr_X(model, Xn);
    vetT.setVetDoubleAttr_X(model, Xn);
}

void Variaveis::setAttr_Start0()
{
    matrix_x.setAttr_Start0();
    matrixDem.setAttr_Start0();

    vetZ.setAttr_Start0();
    vetT.setAttr_Start0();
    vetY.setAttr_Start0();
}

VectorGRBVar::VectorGRBVar(GRBModel &model, int num_, const string &&nome, char type)
{
    inicializado = true;
    num     = num_;
    typeVar = type;
    vetVar  = model.addVars(num, type);

    for(int i=0; i < num; ++i)
        vetVar[num].set(GRB_StringAttr_VarName, nome+"_("+ to_string(i)+")");
}

void VectorGRBVar::setUB(double ub)
{
    for(int i=0; i < num; ++i)
        vetVar[i].set(GRB_DoubleAttr_UB, ub);
}

void VectorGRBVar::setLB(double lb)
{
    for(int i=0; i < num; ++i)
        vetVar[i].set(GRB_DoubleAttr_UB, lb);
}

void VectorGRBVar::setUB_LB(double ub, double lb)
{
    for(int i=0; i < num; ++i)
    {
        vetVar[i].set(GRB_DoubleAttr_UB, lb);
        vetVar[i].set(GRB_DoubleAttr_UB, ub);
    }
}


void VectorGRBVar::setUB_LB(double ub, double lb, int i)
{
    vetVar[i].set(GRB_DoubleAttr_UB, lb);
    vetVar[i].set(GRB_DoubleAttr_UB, ub);
}

void VectorGRBVar::printVars()
{
    for(int i=0; i < num; ++i)
        cout<<vetVar[i].get(GRB_StringAttr_VarName)<<" ";

    cout<<"\n";
}

void VectorGRBVar::inicializa(GRBModel &model, int num_, const string &&nome, char type)
{
    if(inicializado)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, vetor ja foi inicializada";
        ERRO();
    }


    inicializado = true;
    num     = num_;
    typeVar = type;
    vetVar  = model.addVars(num, type);

    for(int i=0; i < num; ++i)
        vetVar[i].set(GRB_StringAttr_VarName, nome+"_("+ to_string(i)+")");

}

RotaEvMip::RotaEvMip(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash):evRoute(vetorHash.vet[0], -1, instancia.evRouteSizeMax, instancia)
{
    inicializado = true;
    vetAtend = Vector<Int8>(instancia.numNos, Int8(0));

    // Copia rota
    evRoute[0].cliente = vetorHash.vet[0];
    evRoute[0].tempoSaida = instancia.getDistance(0, evRoute[0].cliente);
    sat = evRoute[0].cliente;

    for(int i=1; i < instancia.evRouteSizeMax; ++i)
    {
        const int cliente = vetorHash.vet[i];
        evRoute[i].cliente = cliente;
        evRoute.demanda += instancia.getDemand(cliente);
        if(instancia.isClient(cliente))
            vetAtend[cliente] = Int8(1);

        if(evRoute[i].cliente == evRoute[0].cliente)
        {
            evRoute.routeSize = i+1;
            break;
        }
    }

    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true, evRoute[0].tempoSaida, 0, nullptr, nullptr);
    if(evRoute.distancia <= 0.0)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, rota ev eh inviavel\n";
        ERRO();
    }

    const int idMenorFolga  = evRoute[0].posMenorFolga;
    const int clienteMenorF = evRoute[idMenorFolga].cliente;

    const double inc = instancia.getFimJanelaTempoCliente(clienteMenorF) - evRoute[idMenorFolga].tempoCheg;
    if(inc < -1E-3)
    {
        PRINT_DEBUG("", "");
        cout<<"Possivel erro: inc("<<inc<<") < 0\n";
        ERRO();
    }

    tempoSaidaMax = evRoute[0].tempoSaida+inc;

    //string strRota;
    //evRoute.print(strRota, instancia, true);
    //cout<<"Rota: "<<strRota<<"\n";
    //cout<<"tempoSaidaMax: "<<tempoSaidaMax<<"\n";


    //ERRO();
}

RotaEvMip::RotaEvMip(int evRouteSizeMax, const Instancia &instancia): evRoute(1, instancia.getFirstEvIndex(), evRouteSizeMax, instancia){}

void RotaEvMip::inicializa(const Instancia &instancia, const VetorHash &vetorHash)
{
    if(inicializado)
    {
        PRINT_DEBUG("", "");
        cout<<"RotaEvMip ja foi inicializado!";
        ERRO();
    }

    inicializado = true;
    vetAtend = Vector<Int8>(instancia.numNos, Int8(0));

    // Copia rota
    evRoute[0].cliente    = vetorHash.vet[0];
    evRoute.satelite      = vetorHash.vet[0];
    evRoute[0].tempoSaida = instancia.getDistance(0, evRoute[0].cliente);
    sat = evRoute[0].cliente;

    for(int i=1; i < instancia.evRouteSizeMax; ++i)
    {
        const int cliente = vetorHash.vet[i];
        evRoute[i].cliente = cliente;
        evRoute.demanda += instancia.getDemand(cliente);

        if(instancia.isClient(cliente))
            vetAtend[cliente] = Int8(1);

        if(evRoute[i].cliente == evRoute[0].cliente)
        {
            evRoute.routeSize = i+1;
            break;
        }
    }

    static string strDebug;
    //strDebug.clear();
    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true, evRoute[0].tempoSaida, 0, nullptr, nullptr);

    if(evRoute.distancia < 0.0)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, rota ev eh inviavel\n";
        string strRota;
        evRoute.print(strRota, instancia, true);
        cout<<strRota<<"\n";
        cout<<"Debug: "<<strDebug<<"\n";

        ERRO();
    }

    const int idMenorFolga  = evRoute[0].posMenorFolga;
    const int clienteMenorF = evRoute[idMenorFolga].cliente;

    const double inc = instancia.getFimJanelaTempoCliente(clienteMenorF) - evRoute[idMenorFolga].tempoCheg;
    if(inc < -1E-3)
    {
        PRINT_DEBUG("", "");
        cout<<"Possivel erro: inc("<<inc<<") < 0\n";
        ERRO();
    }

    tempoSaidaMax = evRoute[0].tempoSaida+inc;

    /*
    string strRota;
    evRoute.print(strRota, instancia, true);
    cout<<"Rota: "<<strRota<<"\n";
    cout<<"tempoSaidaMax: "<<tempoSaidaMax<<"\n**************************\n\n";
    */
}

RotaEvMip::RotaEvMip(const Instancia &instancia, const EvRoute &evRoute_):evRoute(evRoute_)
{
    inicializado = true;
    vetAtend = Vector<Int8>(instancia.numNos, Int8(0));

    for(int i=1; i < evRoute.routeSize; ++i)
    {
        const int cliente = evRoute[i].cliente;
        if(instancia.isClient(cliente))
            vetAtend[cliente] = Int8(1);
    }

    sat = evRoute_.route[0].cliente;

    const int idMenorFolga  = evRoute[0].posMenorFolga;
    const int clienteMenorF = evRoute[idMenorFolga].cliente;

    const double inc = instancia.getFimJanelaTempoCliente(clienteMenorF) - evRoute[idMenorFolga].tempoCheg;
    if(inc < -1E-3)
    {
        PRINT_DEBUG("", "");
        cout<<"Possivel erro: inc("<<inc<<") < 0\n";
        ERRO();
    }

    tempoSaidaMax = evRoute[0].tempoSaida+inc;

}

MatrixGRBVar::~MatrixGRBVar()
{
    delete []vetVar;
    vetVar = nullptr;

    delete []vetDoubleAttr_X;
    vetDoubleAttr_X = nullptr;
}

void MatrixGRBVar::setVetDoubleAttr_X(GRBModel &model, bool X_n)
{
    delete []vetDoubleAttr_X;

    if(X_n)
        vetDoubleAttr_X  = model.get(GRB_DoubleAttr_Xn, vetVar, numCol * numLin);
    else
        vetDoubleAttr_X = model.get(GRB_DoubleAttr_X, vetVar, numCol * numLin);

}

void MatrixGRBVar::setAttr_Start0()
{

    for(int i=0; i < numLin; ++i)
    {
        for(int j=0; j < numCol; ++j)
            (*this)(i,j).set(GRB_DoubleAttr_Start, 0.0);

    }

}


void VectorGRBVar::setVetDoubleAttr_X(GRBModel &model, bool Xn)
{
    delete []vetDoubleAttr_X;

    if(Xn)
        vetDoubleAttr_X = model.get(GRB_DoubleAttr_Xn, vetVar, num);
    else
        vetDoubleAttr_X = model.get(GRB_DoubleAttr_X, vetVar, num);
}

void VectorGRBVar::setAttr_Start0()
{

    for(int i=0; i < num; ++i)
        (*this)(i).set(GRB_DoubleAttr_Start, 0.0);
}
