//
// Created by ****** on 21/08/22.
//

/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    21/08/22
 *  Arquivo: Parametros.cpp
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
#include "../Vnd.h"

using namespace std;
using namespace NS_parametros;

void NS_parametros::escreveSolCompleta(Parametros &paramEntrada, Solucao &sol, Instancia &instancia)
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

        outfile<<"CARGA POR SAT: ";
        for(int i=1; i <= instancia.getNSats(); ++i)
            outfile<<i<<": "<<sol.satelites[i].demanda<<"; ";

        outfile<<"\nVIAVEL: "<<sol.viavel<<"\n";
        outfile<<"SEGUNDO NIVEL VIAVEL: "<<instancia.nivel2Viavel<<"\n";
        outfile.close();
    }
    else
    {
        cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<strSaida<<"\n";
    }

}

void NS_parametros::escreveSolParaPrint(Parametros &paramEntrada, Solucao &solution, Instancia &instance)
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
    if(!sol.viavel || sol.distancia == DOUBLE_INF)
        return;

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
                    else if(param == "dist")
                    {
                        paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_MIN);
                        paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_STD);
                    }
                    else if(param == "t(s)")
                        paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_STD);
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
            const int size = paramSaida.mapNoSaida.size();
            string saida = paramEntrada.nomeInstancia+" \t ";
            for(int i=0; i < size; ++i)
                saida += "* \t ";


            const string fileResul = paramEntrada.caminhoPasta+"/"+paramEntrada.resultadoCSV;
            bool exists = std::filesystem::exists(fileResul);
            std::ofstream outfile;
            outfile.open(fileResul, std::ios_base::app);

            if(!exists)
            {
                string strCab;

                {
                    std::ofstream saidaCap;
                    saidaCap.open(paramEntrada.caminhoPasta + "/temp.txt", ios::out);


                    string strSaidaCab;
                    paramSaida.getCabecalho(strSaidaCab);
                    saidaCap<<strSaidaCab;
                    saidaCap.close();

                    std::ifstream lerCap;
                    lerCap.open(paramEntrada.caminhoPasta + "/temp.txt", ios::in);

                    BoostC::vector<string> cabec;

                    for(int i=0; i < paramSaida.mapNoSaida.size(); ++i)
                    {
                        string param;
                        lerCap>>param;
                        cabec.push_back(param);

                        if(param != "sem")
                        {
                            paramConsol.mapNoSaida[param] = NoSaida(param);
                            paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_AVG);

                            if(param == "ultimaA")
                                paramConsol.mapNoSaida[param].tipo = SAIDA_TIPO_INT;
                            else if(param == "dist")
                            {

                                paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_MIN);
                                paramConsol.mapNoSaida[param].addSaida(SAIDA_EXEC_STD);

                            }
                        }
                    }

                    lerCap.close();
                    string fineName = paramEntrada.caminhoPasta + "/temp.txt";
                    remove(fineName.c_str());

                    paramConsol.getCabecalho(strCab);


                }

                paramEntrada.data.resize(paramEntrada.data.size()-2);

                outfile<<"DATA: '"<<paramEntrada.data<<"'\n\n";
                outfile<<"instancia \t "<<strCab<<"\n";
            }

            outfile<<saida<<"\n";
            outfile.close();

            cout<<"\tNAO EXISTEM SOLUCOES VIAVEIS\n";
            //throw "ERRO";
        }

    }


}

void NS_parametros::saida(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol, Instancia &instancia)
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


/*
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
                    cout<<paramEntrada.getParametros()<<"\n\n";
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
                else if(opcao == "--numItTotal")
                    paramEntrada.numItTotal = atoi(argv[i+1]);
                else if(opcao == "--resulCSV")
                    paramEntrada.resultadoCSV = string(argv[i+1]);
                else if(opcao == "--mt")
                {
                    string metodoStr = string(argv[i+1]);
                    if(metodoStr == "G" || metodoStr == "Grasp")
                        paramEntrada.metodo = METODO_GRASP;

                    else if(metodoStr == "ACO")
                        paramEntrada.metodo = METODO_ACO;

                    else if(metodoStr == "IG")
                        paramEntrada.metodo = METODO_IG;

                    else
                    {
                        cout<<"OPCAO --mt "<<metodoStr<<" EH INVALIDA. METODO "<<metodoStr<<" NAO EXISTE\n";
                        cout<<paramEntrada.getParametros()<<"\n\n";
                        throw "ERRO";
                    }
                }
                else if(opcao == "--paramIg")
                {
                    paramEntrada.paramIg = string(argv[i+1]);
                }
                else
                {
                    cout<<"ERRO ENTRADA: OPCAO("<<opcao<<") EH INVALIDA\n";
                    cout<<paramEntrada.getParametros()<<"\n\n";
                    throw "ERRO";
                }

            }

            if(paramEntrada.numExecucoesTotais != 0 && paramEntrada.execucaoAtual == -1)
            {
                cout<<"ERRO: OPCAO(--execAtual) TEM QUE SER UTILIZADA COM --execTotal\n";
                cout<<paramEntrada.getParametros()<<"\n\n";
                throw "ERRO";
            }


            if(paramEntrada.numExecucoesTotais == 0 && paramEntrada.execucaoAtual != -1)
            {
                cout<<"ERRO: OPCAO(--execTotal) TEM QUE SER UTILIZADA COM --execAtual\n";
                cout<<paramEntrada.getParametros()<<"\n\n";
                throw "ERRO";
            }

            if(paramEntrada.caminhoPasta == "" && (paramEntrada.numExecucoesTotais != 0 || paramEntrada.execucaoAtual != 0))
            {

                cout<<"ERRO: OPCOES(--execTotal e --execAtual) TEM QUE SER UTILIZADAS EM CONJUNTO COM --pasta\n";
                cout<<paramEntrada.getParametros()<<"\n\n";
                throw "ERRO";
            }
        }


        if(!semente)
            paramEntrada.semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    }
    else
    {
        cout<<"ERRO ENTRADA; FALTA A INSTANCIA!\n";
        cout<<paramEntrada.getParametros()<<"\n\n";

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

    //exit(-1);

}

*/
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
            std::unordered_map<std::string, std::string> mapParam;
            for(int i=1; i < argc; i+=2)
            {
                mapParam[string(argv[i])] = string(argv[i+1]);

            }

            auto getKey = [&](string key) -> std::string
            {
                if(mapParam.count(key) > 0)
                    return mapParam[key];
                else
                {
                    cout<<"Erro, parametro: "<<key<<" faltando !\n";
                    throw "ERRO";
                }
            };

            paramEntrada.numItTotal          = std::stoi(getKey("--numItIG"));
            paramEntrada.paramIg.alfaSeg     = std::stof(getKey("--alphaSeg"));
            paramEntrada.paramIg.betaPrim    = std::stof(getKey("--betaPrim"));
            paramEntrada.paramIg.difBest     = std::stod(getKey("--difBest"));
            paramEntrada.paramIg.torneio     = bool(std::stoi(getKey("--torneio")));
            paramEntrada.paramIg.taxaRm      = std::stod(getKey("--taxaRm"));
            paramEntrada.paramIg.fatorNumChamadas = std::stod(getKey("--fatNumCh"));

            //paramEntrada.paramIg.fileSaida   = string(getKey("--fileSaida"));



            paramEntrada.execucaoAtual       = std::stoi(getKey("--execAtual"));
            paramEntrada.numExecucoesTotais  = std::stoi(getKey("--execTotal"));
            paramEntrada.resultadoCSV        = getKey("--resulCSV");
            paramEntrada.caminhoPasta        = getKey("--pasta");

            if(mapParam.count("--seed") > 0)
            {
                paramEntrada.semente = std::stoll(mapParam["--seed"]);
                semente = true;
            }
            else
                semente = false;

            if(mapParam.count("--mip_presolve"))
                paramEntrada.parametrosMip.presolve = std::stoi(getKey("--mip_presolve"));

            if(mapParam.count("--mip_cuts"))
                paramEntrada.parametrosMip.cuts = std::stoi(getKey("--mip_cuts"));

            if(mapParam.count("--mip_gap"))
                paramEntrada.parametrosMip.mipGap = std::stod(getKey("--mip_gap"));

            if(mapParam.count("--mip_outputFlag"))
                paramEntrada.parametrosMip.outputFlag = std::stoi(getKey("--mip_outputFlag"));

            if(mapParam.count("--mip_restTempo"))
                paramEntrada.parametrosMip.restTempo = std::stoi(getKey("--mip_restTempo"));

            //paramEntrada.parametrosMip.print();


        }


        if(!semente)
            paramEntrada.semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    }
    else
    {
        cout<<"ERRO ENTRADA; FALTA A INSTANCIA!\n";
        cout<<paramEntrada.getParametros()<<"\n\n";

        throw "ERRO";
    }

    std::time_t result = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto data = std::asctime(std::localtime(&result));
    paramEntrada.data = string(data);

    if(paramEntrada.semente == 0)
        paramEntrada.semente = result;

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

ParametrosSaida NS_parametros::getParametros()
{
    ParametrosSaida parametrosSaida;


    parametrosSaida.mapNoSaida["dist"] = NoSaida("dist");
    parametrosSaida.mapNoSaida["t(s)"] = NoSaida("t(s)");
    parametrosSaida.mapNoSaida["sem"] = NoSaida("sem");
    parametrosSaida.mapNoSaida["ultimaA"] = NoSaida("ultimaA", SAIDA_TIPO_INT);
    parametrosSaida.mapNoSaida["numSatVazios"] = NoSaida("numSatVazios");
    parametrosSaida.mapNoSaida["numEV"] = NoSaida("numEV");

/*
    for(int k=0; k < NUM_MV_LS; ++k)
    {
        string nome  = "mv_"+ to_string(k);
        string nome1 = nome+"_num";
        string nome2 = nome + "_percent";
        string nome3 = nome1+"total";

        parametrosSaida.mapNoSaida[nome] = NoSaida(nome);
        parametrosSaida.mapNoSaida[nome1] = NoSaida(nome1);
        parametrosSaida.mapNoSaida[nome2] = NoSaida(nome2);
        parametrosSaida.mapNoSaida[nome3] = NoSaida(nome3);

        parametrosSaida.mapNoSaida[nome].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida[nome1].addSaida(SAIDA_EXEC_VAL);

        parametrosSaida.mapNoSaida[nome2].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida[nome3].addSaida(SAIDA_EXEC_VAL);
    }

    parametrosSaida.mapNoSaida["mv_4_1Nivel"] = NoSaida("mv_4_1Nivel");
    parametrosSaida.mapNoSaida["mv_4_1Nivel"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["mv_6_1Nivel"] = NoSaida("mv_6_1Nivel");
    parametrosSaida.mapNoSaida["mv_6_1Nivel"].addSaida(SAIDA_EXEC_VAL);

    parametrosSaida.mapNoSaida["mv_4_1Nivel_num"] = NoSaida("mv_4_1Nivel_num");
    parametrosSaida.mapNoSaida["mv_4_1Nivel_num"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["mv_6_1Nivel_num"] = NoSaida("mv_6_1Nivel_num");
    parametrosSaida.mapNoSaida["mv_6_1Nivel_num"].addSaida(SAIDA_EXEC_VAL);
*/



    parametrosSaida.mapNoSaida["dist"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["t(s)"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["sem"].addSaida(SAIDA_EXEC_SEM);
    parametrosSaida.mapNoSaida["ultimaA"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["numSatVazios"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["numEV"].addSaida(SAIDA_EXEC_VAL);

    return std::move(parametrosSaida);
}

string NS_parametros::Parametros::getParametros()
{
    string saida = "OPCOES VALIDAS: \n\t--seed: semente \n\t--pasta: pasta onde os resultados serao guardados \n\t--execTotal: numero de execucoes totais"
                   "  \n\t--execAtual: execucao atual \n\t--resulCSV: nome do arquivo csv para escrever os resultados consolidados \n\t --mt: metodo((Grasp ou G) para GRASP, (ACO)\n\t --numItTotal: numero total de iteracoes\n\n";
    return saida;
}

void NS_parametros::NoSaida::getVal(string &saidaStr)
{
    BoostC::vector<double> vectorSaida(listVal.size());
    std::copy(listVal.begin(), listVal.end(), vectorSaida.begin());
    double min = DOUBLE_MAX;
    double max = DOUBLE_MIN;
    double avg = 0.0;

    for(const double i:vectorSaida)
    {
        if(i < min)
            min = i;

        if(i > max)
            max = i;

        avg += i;
    }

    avg = avg/double(vectorSaida.size());
    double std = 0.0;
    const int numCasas = 3;

    for(int &saida:saidaExec)
    {

        switch(saida)
        {
            case SAIDA_EXEC_MIN:
                if(tipo == SAIDA_TIPO_FLOAT)
                    saidaStr += NS_Auxiliary::float_to_string(min, numCasas) + " \t ";
                else
                    saidaStr += to_string(int(min)) + " \t ";

                break;

            case SAIDA_EXEC_MAX:
                if(tipo == SAIDA_TIPO_FLOAT)
                    saidaStr += NS_Auxiliary::float_to_string(max, numCasas) + " \t ";
                else
                    saidaStr += to_string(int(max)) + " \t ";
                break;

            case SAIDA_EXEC_AVG:
                saidaStr += NS_Auxiliary::float_to_string(avg, numCasas) + " \t ";
                break;

            case SAIDA_EXEC_STD:

                for(const double i:vectorSaida)
                    std += pow(i-avg, double(2));

                std = sqrt(std/vectorSaida.size());
                saidaStr += NS_Auxiliary::float_to_string(std, numCasas) + " \t ";
                break;

            case SAIDA_EXEC_VAL:

                if(listVal.size() > 1)
                {

                    cout<<"OPCAO(SAIDA_EXEC_VAL) SO PODE SER UTILIZADA SOZINHA!\n";
                    throw "ERRO";
                }

                if(tipo == SAIDA_TIPO_FLOAT)
                    saidaStr += NS_Auxiliary::float_to_string(min, numCasas) + " \t ";
                else
                    saidaStr += to_string(int(min)) + " \t ";

                break;

            case SAIDA_EXEC_SEM:

                if(saidaExec.size() > 1)
                {
                    cout<<"OPCAO(SAIDA_EXEC_SEM) SO PODE SER UTILIZADA SOZINHA!\n";
                    throw "ERRO";
                }

                saidaStr += to_string(semente)+" \t ";
                break;
        }
    }
}

ParametrosIG::ParametrosIG(const std::string& fileStr)
{

    std::ifstream file(fileStr);
    if(!file.is_open())
    {
        std::cout<<"Nao foi possuivel abrir arquivo: "<<fileStr<<"\n";
        PRINT_DEBUG("","");
        throw "ERRO";
    }


    std::map<std::string, std::string> entradaParam;

    while(file.peek() != EOF)
    {
        std::string param, val;
        file>>param>>val;
        entradaParam[param] = val;
    }

    alfaSeg  = std::stof(entradaParam["alfaSeg"]);
    betaPrim = std::stof(entradaParam["betaPrim"]);
    difBest  = std::stof(entradaParam["difBest"]);
    torneio  = (std::stoi(entradaParam["torneio"]) == 1);
    taxaRm   = std::stod(entradaParam["taxaRm"]);
    tipoConstrutivo15 = static_cast<TipoConstrutivo>(std::stoi(entradaParam["tipoConstrutivo15"]));

    file.close();
}

std::string ParametrosIG::printParam()
{
    std::string saida;

    saida += "alfaSeg \t\t " + std::to_string(alfaSeg);
    saida += "\nbetaPrim \t\t " + std::to_string(betaPrim);
    saida += "\ntorneio \t\t " + std::to_string(torneio);
    saida += "\ntipoConstrutivo15 \t " + std::to_string(tipoConstrutivo15);
    saida += "\ndifBest \t\t " + std::to_string(difBest);
    saida += "\ntaxaEvRm \t\t " + to_string(taxaRm);
    saida += "\nfileSaida \t\t "+ fileSaida;
    saida += "\nfatorNumCh \t\t " + to_string(fatorNumChamadas);


    return saida;
}


/*
 *
 * --pasta
--execTotal
--execAtual
--resulCSV
 */