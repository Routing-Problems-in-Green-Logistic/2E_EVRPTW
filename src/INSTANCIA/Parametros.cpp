//
// Created by igor on 21/08/22.
//

/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/08/22
 *  Arquivo: ParametrosEntrada.cpp
 * ****************************************
 * ****************************************/

#include <chrono>
#include "Parametros.h"
using namespace NS_parametros;

void NS_parametros::escreveSolCompleta(ParametrosGrasp &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol)
{

}

void NS_parametros::escreveSolParaPrint(ParametrosGrasp &paramEntrada, Solucao &sol)
{

}

void NS_parametros::escreveResultadosAcumulados(ParametrosGrasp &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol)
{

}

void NS_parametros::consolidaResultados(ParametrosGrasp &paramEntrada)
{

}

void NS_parametros::saida(ParametrosGrasp &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol)
{

    escreveSolCompleta(paramEntrada, paramSaida, sol);
    escreveSolParaPrint(paramEntrada, sol);
    escreveResultadosAcumulados(paramEntrada, paramSaida, sol);
    consolidaResultados(paramEntrada);

}

// argv[0] eh a instancia
void NS_parametros::caregaParametros(ParametrosGrasp &paramEntrada, int argc, char* argv[])
{

    if(argc >= 1)
    {
        paramEntrada.instancia = string(argv[0]);
        paramEntrada.nomeInstancia = getNomeInstancia(paramEntrada.instancia);

        if(argc > 1)
        {
            for(int i=1; i < argc; i+=2)
            {
                string opcao(argv[i]);

                if((i+1)==argc)
                {
                    cout<<"ERRO; OPCAO("<<opcao<<") FALTA O VALOR\n";
                    throw "ERRO";
                }

                if(opcao == "--seed")
                {
                    paramEntrada.semente = atoll(argv[i+1]);
                }
                else if(opcao == "--pasta")
                {
                    paramEntrada.caminhoPasta = string(argv[i+1]);
                }
                else if(opcao == "--execTotal")
                {
                    paramEntrada.numExecucoesTotais = atoi(argv[i+1]);
                }
                else if(opcao == "--execAtual")
                {
                    paramEntrada.execucaoAtual = atoi(argv[i + 1]);
                }
                else if(opcao == "--resulCSV")
                    paramEntrada.resultadoCSV = string(argv[i+1]);
                else
                {
                    cout<<"ERRO ENTRADA: OPCAO("<<opcao<<") EH INVALIDA\n";
                    cout<<"OPCOES VALIDAS: --seed --pasta --execTotal --execAtual --resulCSV\n\n";
                    throw "ERRO";
                }

            }

            if(paramEntrada.semente == 0)
                paramEntrada.semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            if(paramEntrada.caminhoPasta != "")
            {
                if(paramEntrada.numExecucoesTotais == 0 || paramEntrada.execucaoAtual == -1)
                {
                    cout<<"ERRO: OPCOES(--execTotal e --execAtual) TEM QUE SER UTILIZADAS EM CONJUNTO COM --pasta\n\n";
                    throw "ERRO";
                }
            }
            else if(paramEntrada.caminhoPasta == "" && (paramEntrada.numExecucoesTotais != 0 || paramEntrada.execucaoAtual != 0))
            {

                cout<<"ERRO: OPCOES(--execTotal e --execAtual) TEM QUE SER UTILIZADAS EM CONJUNTO COM --pasta\n\n";
                throw "ERRO";
            }
        }
    }
    else
    {
        cout<<"ERRO ENTRADA; FALTA A INSTANCIA!\nOPCOES: --seed --pasta --execTotal --execAtual --resulCSV\nEXECUCAO: ./run instancia <--opcao val>\n\n";
        cout<<"Compilado em: "<<__DATE__<<", "<<__TIME__<<".\n";
        cout<<"Commit: \tf7d1f0fbd4de1288fc027810a969822e3ba4e3dc\n\n";

        throw "ERRO";
    }

    std::time_t result = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto data = std::asctime(std::localtime(&result));
    paramEntrada.data = string(data);


    string sementeStr;
    sementeStr += "INSTANCIA: " + string(paramEntrada.nomeInstancia) + "\t";
    sementeStr += "SEMENTE: " + to_string(paramEntrada.semente)  + "\t"+data;
    cout<<sementeStr;

/*    cout<<"Semente: "<<paramEntrada.semente<<"\ncaminhoPasta: "<<paramEntrada.caminhoPasta<<"\nexecucoesTotais: "<<paramEntrada.numExecucoesTotais<<"\n";
    cout<<"execucaoAtual: "<<paramEntrada.execucaoAual<<"\nnomeInstancia: "<<paramEntrada.nomeInstancia<<"\nresultadoCSV: "<<paramEntrada.resultadoCSV<<"\n";
    cout<<"DATA: "<<paramEntrada.data<<"\n";*/

    exit(-1);

}


string NS_parametros::getNomeInstancia(string str)
{
    int posNome = -1;

    for(int i=0; i < str.size(); ++i)
    {
        if(str[i] == '/')
            posNome = i+1;
    }

    if(posNome < str.size())
    {
        string nome = str.substr(posNome);

        int posPonto = -1;

        for(int i=0; i < nome.size(); ++i)
        {
            if(nome[i] == '.')
            {
                posPonto = i - 1;
                break;
            }

            //delete bestSol;
        }

        if(posPonto > 0)
        {   //cout<<"posNome: "<<posNome<<"\n\n";
            return nome.substr(0, (posPonto+1));
        }
        else
            return nome;
    }
    else
        return "ERRO";
}
