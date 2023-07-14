/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Variaveis.cpp
 * ****************************************
 * ****************************************/

#include "Variaveis.h"
using namespace VariaveisNs;

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

VariaveisNs::Variaveis::Variaveis(const Instancia &instancia, GRBModel &modelo)
{
    numVertices = instancia.numSats+1;

    matrix_x.inicializa(modelo, numVertices, numVertices, "x", GRB_BINARY);
    matrixDem.inicializa(modelo, numVertices, numVertices, "dem", GRB_CONTINUOUS);

    vetY.inicializa(modelo, numVertices, "y", GRB_BINARY);
    vetZ.inicializa(modelo, numVertices, "z", GRB_BINARY);
    vetT.inicializa(modelo, numVertices, "t", GRB_CONTINUOUS);

    modelo.update();

    matrix_x.printVars();
    matrixDem.printVars();

    cout<<"apos printVars\n";

    vetY.printVars();
    vetZ.printVars();
    vetT.printVars();

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
