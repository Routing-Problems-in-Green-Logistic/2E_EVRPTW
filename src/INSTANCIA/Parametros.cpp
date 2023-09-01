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

void NS_parametros::escreveSolCompleta(const Parametros &paramEntrada, Solucao &sol, Instancia &instancia, const string &file)
{
    string saida;
    sol.print(saida, instancia, false);

    std::ofstream outfile;
    outfile.open(file, std::ios_base::out);

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
        cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<file<<"\n";
    }

}

void NS_parametros::escreveSolParaPrint(const Parametros &paramEntrada, Solucao &solution, Instancia &instance, const string &file)
{

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
                    cout<<paramEntrada.getParametros()<<"\n\n";

                    throw "ERRO";
                }
            };

            paramEntrada.numItTotal          = std::stoi(getKey("--numItIG"));
            paramEntrada.paramIg.alfaSeg     = std::stof(getKey("--alphaSeg"));
            paramEntrada.paramIg.betaPrim    = std::stof(getKey("--betaPrim"));
            paramEntrada.paramIg.difBest     = std::stod(getKey("--difBest"));
            paramEntrada.paramIg.torneio     = bool(std::stoi(getKey("--torneio")));
            paramEntrada.paramIg.taxaRm      = std::stod(getKey("--taxaRm"));
            paramEntrada.paramIg.fatorNumCh  = std::stod(getKey("--fatNumCh"));


            if(mapParam.count("--mip") > 0)
                paramEntrada.mip = bool(std::stoi(getKey("--mip")));

//#if !AJUSTE_DE_PARAMETRO
            if constexpr(!AjusteDeParametro)
            {
                //paramEntrada.execucaoAtual      = std::stoi(getKey("--execAtual"));
                //paramEntrada.numExecucoesTotais = std::stoi(getKey("--execTotal"));
                paramEntrada.resultadoCSV       = getKey("--resulCSV");
                //paramEntrada.caminhoPasta     = getKey("--pasta");
                paramEntrada.fileNum            = getKey("--fileNum");


                if(mapParam.count("--seed") > 0)
                {
                    paramEntrada.semente = std::stoll(getKey("--seed"));
                    semente = true;
                } else
                {
                    paramEntrada.fileSeed = getKey("--fileSeed");
                    semente = false;
                }

                if(mapParam.count("--fileSol") > 0)
                    paramEntrada.fileSol = getKey("--fileSol");

                if(mapParam.count("--fileSolPrint") > 0)
                    paramEntrada.fileSolPrint = getKey("--fileSolPrint");

                if(paramEntrada.mip)
                {
                    paramEntrada.parametrosMip.presolve = std::stoi(getKey("--mip_presolve"));
                    paramEntrada.parametrosMip.cuts = std::stoi(getKey("--mip_cuts"));
                    paramEntrada.parametrosMip.mipGap = std::stod(getKey("--mip_gap"));
                    paramEntrada.parametrosMip.restTempo = std::stoi(getKey("--mip_restTempo"));

                    if(mapParam.count("--mip_outputFlag"))
                        paramEntrada.parametrosMip.outputFlag = std::stoi(getKey("--mip_outputFlag"));
                }
            }
            else
            {
                semente = true;
                paramEntrada.semente                    = std::stoll(getKey("--seed"));
                paramEntrada.paramIg.fileSaida          = string(getKey("--fileSaida"));

                if(paramEntrada.mip)
                {
                    paramEntrada.parametrosMip.presolve = std::stoi(getKey("--mip_presolve"));
                    paramEntrada.parametrosMip.cuts = std::stoi(getKey("--mip_cuts"));
                    paramEntrada.parametrosMip.mipGap = std::stod(getKey("--mip_gap"));
                    paramEntrada.parametrosMip.outputFlag = 0;
                    paramEntrada.parametrosMip.restTempo = std::stoi(getKey("--mip_restTempo"));
                }
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

    string saida = "\nPARAMETROS: \n"
                   "\t--seed: \tsemente (opcional)\n"
                   "\t--resulCSV: \tnome do arquivo csv para escrever os resultados consolidados \n"
                   "\t--fileNum: \tnome do arquivo para escrever o numero de execucoes\n"
                   "\t--fileSeed: \tnome do arquivo para escrever a semente\n"
                   "\t--fileSol: \tnome do arquivo para salvar a solucao completa (opcional).\n"
                   "\t--fileSolPrint: nome do arquivo para salvar a solucao para ser utilizada pelo matplotlib (opcional)\n"
                   "\t--numItIG: \tnumero total de iteracoes do IG \n"
                   "\t--alphaSeg: \tparametro alpha \n"
                   "\t--betaPrim \tparametro beta \n"
                   "\t--difBest \tparametro difBest \n"
                   "\t--torneio \tparametro torneio \n"
                   "\t--taxaRm \tparametro taxaRm\n"
                   "\t--mip: \t\texecuta mip\n"
                   "\t--mip_presolve \n "
                   "\t--mip_cuts \n"
                   "\t--mip_gap \n"
                   "\t--mip_restTempo\n\n";


    return saida;
}

void NS_parametros::NoSaida::getVal(string &saidaStr)
{
    Vector<double> vectorSaida(listVal.size());
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

                std = std::sqrt(double(std/vectorSaida.size()));
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
    saida += "\nfatorNumCh \t\t " + to_string(fatorNumCh);


    return saida;
}


void NS_parametros::saidaNew(Solucao &sol, Instancia &instancia, double tempoCpu, const Parametros &parametros, string &cabecalho, string &valores)
{
    if(!sol.viavel)
    {
        cerr<<"\tNao foi possivel gerar uma solucao viavel\n";
        throw "ERRO";
    }

    string erro;

    if(!sol.checkSolution(erro, instancia))
    {
        cerr<<"Solucao nao eh viavel\nERRO!\n\n";
        string strSol;
        sol.print(strSol, instancia, false);
        cerr<<"Solucao:\n\n"<<strSol<<"\n\n";
        cerr<<"ERRO:\n\t"<<erro<<"\n\n";
        throw "ERRO";
    }


    const string resultCsv = parametros.resultadoCSV;
    const string &fileNum  = parametros.fileNum;
    const string &fileSeed = parametros.fileSeed;

    bool exists = std::filesystem::exists(resultCsv);
    std::ofstream outCsv;
    outCsv.open(resultCsv, std::ios_base::app);

    if(!outCsv.is_open())
    {
        cout << "ERRO, NAO FOI POSSIVEL ABRIR O ARQUIVO: " << resultCsv << "\nErro: " << strerror(errno) << "\n\n";
        throw "ERRO";
    }

    if(!exists)
        outCsv<<"dist, tempoCpu, "<<cabecalho<<"\n";

    outCsv<<NS_Auxiliary::float_to_string(sol.distancia,3)<<", "<<NS_Auxiliary::float_to_string(tempoCpu,3)<<", "<<valores<<"\n";
    outCsv.close();

    exists = std::filesystem::exists(fileNum);

    int num = 0;

    if(exists)
    {
        std::fstream fstreamNum(fileNum, std::ios_base::in);
        fstreamNum >> num;
        fstreamNum.close();
    }

    std::ofstream outNum(fileNum, std::ios_base::out);
    outNum<<num+1;
    outNum.close();

    if(fileSeed.empty())
        return;

    exists = std::filesystem::exists(fileSeed);
    std::ofstream seedOf;

    if(!exists)
    {
        seedOf.open(fileSeed, std::ios_base::out);
        seedOf << "seed\n";
    }
    else
        seedOf.open(fileSeed, std::ios_base::app);

    seedOf<<to_string(parametros.semente)<<"\n";
    seedOf.close();

    if(!parametros.fileSol.empty())
        escreveSolCompleta(parametros, sol, instancia, parametros.fileSol);

    if(!parametros.fileSolPrint.empty())
        escreveSolParaPrint(parametros, sol, instancia, parametros.fileSolPrint);


}