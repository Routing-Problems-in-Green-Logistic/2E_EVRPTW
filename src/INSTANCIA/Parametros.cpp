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
#include "../mersenne-twister.h"
#include <fstream>

#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace NS_parametros;

void NS_parametros::escreveSolCompleta(Parametros &paramEntrada, Solucao &sol, Instance &instancia)
{


    const string caminhoSolComp = paramEntrada.caminhoPasta + "/solCompleta/";

    if(mkdir(caminhoSolComp.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminhoSolComp << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }


    // Cria diretorio paramEntrada.caminhoPasta + "/solCompleta/" + nomeInst/

    const string caminho = caminhoSolComp + paramEntrada.nomeInstancia + "/";

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }
    //cout<<"DIRETORIO: "<<caminho<<" criado\n";

    string saida;
    sol.print(saida, instancia);
    string strSaida = caminho+paramEntrada.nomeInstancia+"_EXE_"+ to_string(paramEntrada.execucaoAtual)+".txt";

    std::ofstream outfile;
    outfile.open(strSaida, std::ios_base::out);

    if(outfile.is_open())
    {
        outfile<<"INSTANCIA: "<<paramEntrada.nomeInstancia<<"\nDATA: "<<paramEntrada.data<<"\n\n";
        outfile<<saida<<"\n\n";
        outfile.close();
    }
    else
    {
        cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<strSaida<<"\n";
    }

}

void NS_parametros::escreveSolParaPrint(Parametros &paramEntrada, Solucao &solution, Instance &instance)
{



    const string caminhoSolPrint = paramEntrada.caminhoPasta + "/solPrint/";

    if(mkdir(caminhoSolPrint.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminhoSolPrint << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }


    // Cria diretorio paramEntrada.caminhoPasta + "/solCompleta/" + nomeInst/
    const string caminho =  caminhoSolPrint+ paramEntrada.nomeInstancia + "/";

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }


    string file = caminho+paramEntrada.nomeInstancia+"_EXE_"+ to_string(paramEntrada.execucaoAtual)+".txt";

    std::ofstream outfile;
    outfile.open(file, std::ios_base::out);

    if(outfile.is_open())
    {

        for(int s=instance.getFirstSatIndex(); s <= instance.getEndSatIndex(); ++s)
        {

            for(int e=0; e < solution.satelites[s].vetEvRoute.size(); ++e)
            {
                EvRoute &evRoute = solution.satelites[s].vetEvRoute[e];

                if(evRoute.routeSize > 2)
                {
                    string rota;

                    for(int i=0; i < evRoute.routeSize; ++i)
                    {
                        rota += to_string(evRoute[i].cliente) + " ";
                    }

                    outfile<<rota<<"\n";


                }
            }
        }

        for(int i = 0; i < solution.primeiroNivel.size(); ++i)
        {
            Route &veic = solution.primeiroNivel[i];
            string rota;

            if(veic.routeSize > 2)
            {
                for(int t=0; t < veic.routeSize; ++t)
                    rota += to_string(veic.rota[t].satellite) + " ";

                outfile<<rota<<"\n";
            }

        }

        outfile.close();

    }
    else
    {
        cout<<"Nao foi possivel abrir o arquivo: "<<file<<"\n";
        throw "ERRO FILE";
    }

}

void NS_parametros::escreveResultadosAcumulados(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol)
{


    // Cria diretorio paramEntrada.caminhoPasta + "/solCompleta/" + nomeInst/

    const string caminho = paramEntrada.caminhoPasta + "/solAcumu/";

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }

    const string fileStr = caminho + paramEntrada.nomeInstancia+".csv";


    bool exists = std::filesystem::exists(fileStr);
    std::ofstream outfile;
    outfile.open(fileStr, std::ios_base::app);

    if(!outfile.is_open())
    {

        cout << "ERRO, NAO FOI POSSIVEL ABRIR O ARQUIVO: " << fileStr << "\nErro: " << strerror(errno) << "\n\n";
        throw "ERRO";
    }

    if(!exists)
    {
        string cabecalho;
        paramSaida.getCabecalho(cabecalho);

        paramEntrada.data.resize(paramEntrada.data.size()-2);
        outfile<<"INSTANCIA: "<<paramEntrada.nomeInstancia<<"\nDATA: '"<<paramEntrada.data<<"'\n\n";
        outfile<<cabecalho<<"\n";
    }

    string val;
    paramSaida.getVal(val);
    outfile<<val<<"\n";

    outfile.close();


}

void NS_parametros::consolidaResultados(Parametros &paramEntrada, ParametrosSaida &paramSaida)
{


    if((paramEntrada.execucaoAtual+1) == paramEntrada.numExecucoesTotais)
    {
        ParametrosSaida paramConsol;

        const string caminho = paramEntrada.caminhoPasta + "/solAcumu/" + paramEntrada.nomeInstancia+".csv";
        std::ifstream file;
        file.open(caminho, std::ios::in);
        if(file.is_open())
        {
            string line;

            for(int i=0; i < 3; ++i)
            {
                line = "";
                getline(file, line);
            }

            //cout<<"LINE: "<<line<<"\n\n";
            vector<string> cabec;

            for(int i=0; i < paramSaida.mapNoSaida.size(); ++i)
            {
                string param;
                file>>param;
                cabec.push_back(param);

                if(param != "sem")
                {
                    paramConsol.mapNoSaida[param] = NoSaida(param);
                    paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_AVG);

                    if(param == "ultimaA")
                        paramConsol.mapNoSaida[param].tipo = SAIDA_TIPO_INT;
                    else
                    {
                        if(param != "t(s)")
                        {
                            paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_MIN);
                            paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_STD);
                        }
                    }
                }
            }

            int p=0;
            double val;
            while(file>>val)
            {

                //cout<<p<<": ";
                for(int i=0; i < paramSaida.mapNoSaida.size(); ++i)
                {
                    //double val;
                    const string param = cabec[i];
                    //file>>val;

                    if(param != "sem")
                    {   //cout<<val<<" ";
                        paramConsol.mapNoSaida[param](val);
                    }
                    if(i+1 < paramSaida.mapNoSaida.size())
                        file>>val;
                }

                string lixo;
                //getline(file, lixo);
                p += 1;
                //cout<<"\n************\n\n";

            }

            string strCab;
            paramConsol.getCabecalho(strCab);
            //cout<<"CABECALHO: "<<strCab<<"\n";

            string strVal;
            paramConsol.getVal(strVal);
            //cout<<strVal<<"\n\n";

            // Escreve em resultados.csv


            const string fileResul = paramEntrada.caminhoPasta+"/"+paramEntrada.resultadoCSV;
            bool exists = std::filesystem::exists(fileResul);
            std::ofstream outfile;
            outfile.open(fileResul, std::ios_base::app);

            if(!exists)
            {
                paramEntrada.data.resize(paramEntrada.data.size()-2);

                outfile<<"DATA: '"<<paramEntrada.data<<"'\n\n";
                outfile<<"instancia \t "<<strCab<<"\n";
            }

            outfile<<paramEntrada.nomeInstancia<<" \t "<<strVal<<"\n";
            outfile.close();

/*
            line = "";
            getline(file, line);
            cout<<"LINE: "<<line<<"\n\n";*/
        }
        else
        {

            cout << "ERRO, NAO FOI POSSIVEL ABRIR O ARQUIVO: " << caminho << "\n\n";
            throw "ERRO";
        }

    }


}

void NS_parametros::saida(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol, Instance &instancia)
{


    const string caminho = paramEntrada.caminhoPasta;

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }

    escreveSolCompleta(paramEntrada, sol, instancia);
    escreveSolParaPrint(paramEntrada, sol, instancia);
    escreveResultadosAcumulados(paramEntrada, paramSaida, sol);
    consolidaResultados(paramEntrada, paramSaida);

}

// argv[0] eh a instancia
void NS_parametros::caregaParametros(Parametros &paramEntrada, int argc, char* argv[])
{

    bool semente = false;

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
                    semente = true;
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

/*            if(paramEntrada.caminhoPasta != "")
            {
                if(paramEntrada.numExecucoesTotais == 0 || paramEntrada.execucaoAtual == -1)
                {
                    cout<<"ERRO: OPCOES(--execTotal e --execAtual) TEM QUE SER UTILIZADAS EM CONJUNTO COM --pasta\n\n";
                    throw "ERRO";
                }
            }*/

            if(paramEntrada.numExecucoesTotais != 0 && paramEntrada.execucaoAtual == -1)
            {
                cout<<"ERRO: OPCAO(--execAtual) TEM QUE SER UTILIZADA COM --execTotal\n\n";
                throw "ERRO";
            }


            if(paramEntrada.numExecucoesTotais == 0 && paramEntrada.execucaoAtual != -1)
            {
                cout<<"ERRO: OPCAO(--execTotal) TEM QUE SER UTILIZADA COM --execAtual\n\n";
                throw "ERRO";
            }

            if(paramEntrada.caminhoPasta == "" && (paramEntrada.numExecucoesTotais != 0 || paramEntrada.execucaoAtual != 0))
            {

                cout<<"ERRO: OPCOES(--execTotal e --execAtual) TEM QUE SER UTILIZADAS EM CONJUNTO COM --pasta\n\n";
                throw "ERRO";
            }
        }


        if(!semente)
            paramEntrada.semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    }
    else
    {
        cout<<"ERRO ENTRADA; FALTA A INSTANCIA!\nOPCOES: --seed --pasta --execTotal --execAtual --resulCSV\nEXECUCAO: ./run instancia <--opcao val>\n\n";
        cout<<"Compilado em: "<<__DATE__<<", "<<__TIME__<<".\n";
        //cout<<"Commit: \tf7d1f0fbd4de1288fc027810a969822e3ba4e3dc\n\n";

        throw "ERRO";
    }

    std::time_t result = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto data = std::asctime(std::localtime(&result));
    paramEntrada.data = string(data);

    seed(paramEntrada.semente);

    string sementeStr;
    sementeStr += "INSTANCIA: " + string(paramEntrada.nomeInstancia) + "\t";
    sementeStr += "SEMENTE: " + to_string(paramEntrada.semente)  + "\t"+data;

    cout<<sementeStr;


/*    cout<<"Semente: "<<paramEntrada.semente<<"\ncaminhoPasta: "<<paramEntrada.caminhoPasta<<"\nexecucoesTotais: "<<paramEntrada.numExecucoesTotais<<"\n";
    cout<<"execucaoAtual: "<<paramEntrada.execucaoAtual<<"\nnomeInstancia: "<<paramEntrada.nomeInstancia<<"\nresultadoCSV: "<<paramEntrada.resultadoCSV<<"\n";
    cout<<"DATA: "<<paramEntrada.data<<"\n";*/
    //exit(-1);

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
