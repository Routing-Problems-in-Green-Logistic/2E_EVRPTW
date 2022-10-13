/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    02/06/22
 *  Arquivo: Grasp.cpp
 * ****************************************
 * ****************************************/

#include "Grasp.h"
#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"
#include "PreProcessamento.h"
#include "Vnd.h"
#include <fstream>
#include "k_means.h"

#define NUM_EST_INI 3

using namespace GreedyAlgNS;
using namespace NameS_Grasp;
using namespace NS_LocalSearch;
using namespace NS_vnd;

const float fator = 0.1;

/**
 *
 * @param instance
 * @param parametros
 * @param estat
 * @param retPrimeiraSol
 * @param matClienteSat         Para uma posicao: matClienteSat(clienteI, sat_0) = 0,1: indica se o clienteI pode ser atendido pelo sat_0
 * @return
 */
Solucao * NameS_Grasp::grasp(Instance &instance, ParametrosGrasp &parametros, Estatisticas &estat, const bool retPrimeiraSol, const ublas::matrix<int> &matClienteSat)
{

    Solucao *solBest = new Solucao(instance);
    solBest->distancia = DOUBLE_MIN;
    solBest->viavel = false;
    EvRoute evRoute(1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);


    vector<int> vetSatAtendCliente(instance.numNos, -1);
    vector<int> satUtilizado(instance.numSats+1, 0);

//cout<<"&vetSatAntendCliente: "<<&vetSatAtendCliente<<"\n&satUtilizado: "<<&satUtilizado<<"\n\n";

    estat.numSol = 0.0;
    estat.numIte = parametros.numIteGrasp;
    estat.distAcum = 0.0;
    estat.erro = "";

    const int tamAlfa = parametros.vetAlfa.size();

    // Solucao para inicializar reativo
    Solucao gul(instance);

    construtivo(gul, instance, 0.0, 0.0, matClienteSat);
    const double gulCusto = getDistMaisPenalidade(gul, instance);
    double custoBest = gulCusto;

    //Vetores para o reativo
    std::vector<double> vetorProbabilidade(tamAlfa);
    std::fill(vetorProbabilidade.begin(), vetorProbabilidade.begin()+tamAlfa, 100.0/float(tamAlfa));

    std::vector<int>    vetorFrequencia(tamAlfa);
    std::fill(vetorFrequencia.begin(), vetorFrequencia.begin()+tamAlfa, 1);

    std::vector<double> solucaoAcumulada(tamAlfa);
    std::fill(solucaoAcumulada.begin(), solucaoAcumulada.begin()+tamAlfa, gulCusto);

    std::vector<double> vetorMedia(tamAlfa);
    std::fill(vetorMedia.begin(), vetorMedia.begin()+tamAlfa, 0.0);

    std::vector<double> proporcao(tamAlfa);
    std::fill(proporcao.begin(), proporcao.begin()+tamAlfa, 0.0);


    auto atualizaProb = [&]()
    {
        double somaProporcoes = 0.0;

        //Calcular média
        for(int i = 0; i < tamAlfa; ++i)
            vetorMedia[i] = solucaoAcumulada[i]/double(vetorFrequencia[i]);

        //Calcula proporção.
        for(int i = 0; i < tamAlfa; ++i)
        {
            proporcao[i] = custoBest/vetorMedia[i];
            somaProporcoes += proporcao[i];
        }

        //Calcula probabilidade
        for(int i = 0; i< tamAlfa; ++i)
            vetorProbabilidade[i] = 100.0*(proporcao[i]/somaProporcoes);

        //cout<<"vet prob: \n";
        //string vet = NS_Auxiliary::printVector(vetorProbabilidade, int64_t(vetorProbabilidade.size()));

        //cout<<vet<<"\n";

        double sum = 0.0;
        for(int i=0; i < tamAlfa; ++i)
            sum += vetorProbabilidade[i];

        //cout<<"sum: "<<sum<<"\n\n";

    };

    auto convIndClienteVet = [&](int cliente)
    {
        return cliente-instance.getFirstClientIndex();
    };

    auto convClienteIndVet = [&](int i)
    {
        return i+instance.getFirstClientIndex();
    };

    // Guarda o numero de vezes que o cliente i NAO a parece na solucao
    std::vector<QuantCliente> vetQuantCliente(instance.getNClients());
    std::vector<QuantCliente> vetQuantProb0;

    for(int i=instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
        vetQuantCliente[convIndClienteVet(i)].cliente = i;


    double somaProb = 0.0;
    int posAlfa = 0;
    int valAleatorio = 0;
    float alfa = 0;

    int addRotaClienteProbIgual = 0;
    int clienteAdd = -1;


    for(int i=0; i < parametros.numIteGrasp; ++i)
    {
/*        if(i>0 && (i%100)==0)
            cout<<"ITERACAO: "<<i<<"\n";*/

        Solucao sol(instance);
        //setSatParaCliente(instance, vetSatAtendCliente, satUtilizado, parametros);
        //N_k_means::k_means(instance, vetSatAtendCliente, satUtilizado, true);


        if(i == parametros.iteracoesCalProb && instance.shortestPath) //&& (i%parametros.iteracoesCalProb)==0)
        {

            for(int t=0; t < instance.getNClients(); ++t)
            {
                //cout<<"t: "<<t<<"\n";
                //cout<<"conv(t): "<<convClienteIndVet(t)<<"\n";

                const int sat = vetSatAtendCliente[convClienteIndVet(t)];

                if(sat < 0)
                    continue;

                //cout<<"vetSatAtendCliente: "<<sat<<"\n";

                const EvRoute &evRouteAux = instance.shortestPath[sat].getEvRoute(convClienteIndVet(t));

//cout<<"evRouteAux.routeSize="<<evRouteAux.routeSize<<"\n";

                vetQuantCliente.at(t).calculaProb(i);
                if(vetQuantCliente.at(t).prob == 0 && evRouteAux.routeSize > 2)
                    addRotaClienteProbIgual += 1;

                else if(vetQuantCliente.at(t).prob >= 90 && evRouteAux.routeSize > 2)
                    vetQuantCliente[t].prob = 90;

                else if(evRouteAux.routeSize <= 2)
                    vetQuantCliente[t].prob = 100;
            }

            std::sort(vetQuantCliente.begin(), vetQuantCliente.end());

            if(estat.numSol == 0)
            {
                parametros.numMaxClie += 1;
                if(parametros.numMaxClie > instance.getN_Evs())
                    parametros.numMaxClie -= 1;

            }

        }

        Solucao solTemp(instance);

        // Insere  rotas da forama: sat cliente sat
        if(i >= parametros.iteracoesCalProb && parametros.iteracoesCalProb > 0 && instance.shortestPath)
        {
            int clientesAdd = 0;

            bool add   = false;
            bool igual = false;

            int t = rand_u32()%instance.getNClients();
            const int inicio = t;


            if(vetQuantCliente[0].prob != 100)
            {

                std::vector<int> vetSatRotaInicializada(1+(instance.numSats*instance.numEv), 0);

                do
                {
                    if(vetQuantCliente[t].prob != 100)
                    {

                        int rand = rand_u32() % 100;
                        if(rand >= vetQuantCliente[t].prob)
                        {

                            int cliente = vetQuantCliente[t].cliente;

                            int sat = rand_u32()%instance.numSats;
                            const int satIni = sat;

                            do
                            {
                                int satId = sat+instance.getFirstSatIndex();
                                int evEscolhido = 0;

                                if(matClienteSat(t, satId) == 1)
                                {
                                    for(int ev = 0; ev < instance.numEv; ++ev)
                                    {
                                        if(vetSatRotaInicializada[1+(satId-1)*instance.numEv+ev] == 0)
                                        {
                                            evEscolhido = ev;
                                            break;
                                        }
                                    }

                                    if(vetSatRotaInicializada[1+(satId-1)*instance.numEv+evEscolhido] == 0)
                                    {
                                        EvRoute &evRouteSP = instance.shortestPath[satId].getEvRoute(cliente);
                                        auto shortestPath = instance.shortestPath[satId].getShortestPath(cliente);

                                        if(shortestPath.distIdaVolta < DOUBLE_INF)
                                        {

                                            addRotaCliente(sol, instance, evRouteSP, cliente);
                                            clientesAdd += 1;
                                            vetSatRotaInicializada[1 + (satId-1)*instance.numEv + evEscolhido] = 1;
                                            break;
                                        }
                                    }

                                }

                                sat += 1;
                                sat = sat%instance.numSats;

                            }while(sat != satIni);

                            /*const int pos = vetSatAtendCliente[cliente];
                            if(pos >= 0)
                            {

                                //clienteAdd = cliente;
                                //cout<<"cliente: "<<cliente<<": "<<vetSatAtendCliente[cliente]<<"\n";
                                EvRoute &evRouteSP = instance.shortestPath[pos].getEvRoute(cliente);
                                auto shortestPath = instance.shortestPath[pos].getShortestPath(cliente);


                                if(shortestPath.distIdaVolta < DOUBLE_INF)
                                {

                                    addRotaCliente(sol, instance, evRouteSP, cliente);
                                    clientesAdd += 1;

                                    if(clientesAdd == sol.numEvMax)
                                        break;
                                }

                                add = true;
                            }*/


                        }

                        if(clientesAdd >= parametros.numMaxClie)
                            break;
                    }

                    t = (t+1) % instance.getNClients();

                }while(t != inicio);

            }
        }

        somaProb = 0.0;
        posAlfa = 0;

        valAleatorio = rand_u32() % 100;

        for(int j=0;somaProb < valAleatorio; ++j)
        {

            if(j >= tamAlfa)
            {
                break;
            }

            somaProb+= (vetorProbabilidade[j]);
            posAlfa = j;
        }


        alfa = parametros.vetAlfa[posAlfa];
        vetorFrequencia[posAlfa] += 1;

        solTemp.copia(sol);
        construtivo(sol, instance, alfa, alfa, matClienteSat);

        // Add 1 se o cliente t nao foi atendido
        if(!sol.viavel && parametros.iteracoesCalProb > 0 && instance.shortestPath)
        {
            int quantCliInv = 0;
            for(int t=instance.getFirstClientIndex(); t <= instance.getEndClientIndex(); ++t)
            {
                if(sol.vetClientesAtend[t] == 0)
                {

                    quantCliInv += 1;

                    for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
                    {
                        if(matClienteSat(t,sat) == 1)
                        {   // ERRO vetSatAtendCliente NAO Guarda mais o sat que atende o cliente t
                            const EvRoute &evRouteAux = instance.shortestPath[sat].getEvRoute(t);

                            if(evRouteAux.routeSize > 2)
                            {
                                vetQuantCliente[convIndClienteVet(t)].add1Quant();
                                break;
                            }
                        }
                    }
                }
            }
        }

        if(sol.viavel)
        {
            string erro;
            bool mv = true;

            if(!sol.checkSolution(erro, instance))
            {

                cout << "\n\nSOLUCAO:\n\n";
                sol.print(instance);

                cout<<"############################################################################################\n\n";
                cout<<"SOLUCAO INICIAL:\n\n";
                solTemp.print(instance);


                cout << erro
                     << "\n****************************************************************************************\n\n";
                mv = false;
                delete solBest;
                throw "ERRO";
            } else
            {

                rvnd(sol, instance);

                if(sol.distancia < solBest->distancia || !solBest->viavel)
                {

                    solBest->copia(sol);

                    if(retPrimeiraSol)
                    {   solBest->ultimaA = i;
                        return solBest;
                    }

                    //string fileBestAntes = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/solBestAntes.txt";
                    //escreveSol(fileBestAntes, *solBest, instance);

                    custoBest = solBest->distancia;
                    estat.ultimaAtualizacaoBest = i;
                    //solBest->print(instance);
                    //cout<<"i: "<<i<<"\n";

/*                    if(i == 17)
                    {
                        PRINT_DEBUG("", "ULTIMA ATUALIZACAO");
                        cout<<"!ULTIMA ATUALILZACAO\n";

                        cout<<"checkSolution: "<<sol.checkSolution(erro, instance)<<"\n";
                        if(!erro.empty())
                            cout<<"ERRO: "<<erro<<"\n";

                        erro = "";

                        cout<<"checkSolution best: "<<solBest->checkSolution(erro, instance)<<"\n";

                        string file  = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/sol.txt";
                        string file2 = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/solBest.txt";

                        escreveSol(file, sol, instance);
                        escreveSol(file2, *solBest, instance);

                        if(!erro.empty())
                            cout<<"ERRO: "<<erro<<"\n";

                        throw "ULTIMA ATUALIZACAO";
                    }*/

                }


            }

            if(sol.viavel)
            {

                estat.numSol += 1;
                estat.distAcum += sol.distancia;
            }


            //if(mv)
            //    cout<<"\tMV VIAVEL: Incr: "<<sol.distancia-valOrig<<"\n\n";

        }
        else if(!solBest->viavel && !sol.viavel)
        {


            {
                //cout<<"ATUAL\n";
                solBest->copia(sol);

                double aux = sol.distancia + getPenalidade(sol, instance, fator);
                if(aux < custoBest)
                    custoBest = aux;

/*                cout<<"\n\n***************************************************************************\n";
                sol.print(instance);
                solBest->print(instance);
                cout<<"\n\n***************************************************************************!!\n";*/




            }
        }

        if(solBest->viavel && i >= 500 && (i-solBest->ultimaA) >= 400)
            break;

        if(!sol.viavel)
            solucaoAcumulada[posAlfa] += sol.distancia + getPenalidade(sol, instance, fator);
        else
            solucaoAcumulada[posAlfa] += sol.distancia;

        if(i>0 && (i%parametros.numAtualProbReativo)==0)
            atualizaProb();


    }
    // END FOR GRASP

    solBest->ultimaA = estat.ultimaAtualizacaoBest;

    if(solBest->viavel)
    {
        estat.erro = "";

        //cout<<"ULTIMA ATUALIZACAO: "<<estat.ultimaAtualizacaoBest<<"\n\n";
        //PRINT_DEBUG("\n\t", "checkSolution best");

        if(!solBest->checkSolution(estat.erro, instance))
        {
            cout<<"\n\nSOLUCAO:\n\n";
            //solBest->print(instance);

            cout << estat.erro<< "\n****************************************************************************************\n\n";
            delete solBest;
            return nullptr;
        }

        //PRINT_DEBUG("", "ULTIMA ATUALIZACAO: "<<solBest->ultimaA);

        //PRINT_DEBUG("\t", "checkSolution end");
    }

    return solBest;

}

double NameS_Grasp::getPenalidade(Solucao &sol, Instance &instancia, float f)
{
    static double penalidade = 1.2*instancia.penalizacaoDistEv;


    if(sol.viavel)
        return 0.0;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == 0)
            num += 1;
    }

    return f*num*penalidade;
}

double NameS_Grasp::getDistMaisPenalidade(Solucao &sol, Instance &instancia)
{
    static double penalidade = 1.2*instancia.penalizacaoDistEv;// + instancia.penalizacaoDistComb;

    if(sol.viavel)
        return sol.distancia;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == 0)
            num += 1;
    }

    return sol.distancia + num*penalidade;
}

void NameS_Grasp::inicializaSol(Solucao &sol, Instance &instance)
{

    std::vector<EstDist> vetEstDist(instance.getN_RechargingS());
    EvRoute evRoute(-1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);

    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {
        int vetTam = 0;
        evRoute.satelite = i;

        for(int r= instance.getFirstRS_index(); r <= instance.getEndRS_index(); ++r)
        {
            vetEstDist[vetTam].estacao = r;
            vetEstDist[vetTam].distancia = instance.getDistance(i, r);
            ++vetTam;
        }

        std::sort(vetEstDist.begin(), vetEstDist.begin()+vetTam);

        const int numEst = min(NUM_EST_INI, instance.getN_RechargingS());
        const int pos = (rand_u32()%numEst);
        int p = pos;
        evRoute[0].cliente = evRoute[2].cliente = i;

        double taxaConsumoDist = instance.vectVeiculo[instance.getFirstEvIndex()].taxaConsumoDist;

        do
        {
            evRoute[1].cliente = vetEstDist[p].estacao;


            if(vetEstDist[p].distancia * instance.vectVeiculo[instance.getFirstEvIndex()].taxaConsumoDist < instance.vectVeiculo[instance.getFirstEvIndex()].capacidadeBateria)
            {

            }
            p += 1;
        }
        while(p != pos);
    }


}

void NameS_Grasp::addRotaCliente(Solucao &sol, Instance &instancia, const EvRoute &evRoute, const int cliente)
{

    if(evRoute.routeSize <= 2)
        return;

    string str;
    //evRoute.print(str, instancia, true);

    int sat = evRoute.satelite;

    int next = 0;

    for(int i=0; i < sol.satelites[sat].tamVetEvRoute; ++i)
    {
        next = i;
        if(sol.satelites[sat].vetEvRoute[i].routeSize <= 2)
            break;
    }

    if(next >= sol.satelites[sat].tamVetEvRoute)
        return;

    if(sol.satelites[sat].vetEvRoute[next].routeSize > 2)
        return;

    sol.numEv += 1;

    int aux = sol.satelites[sat].vetEvRoute[next].idRota;
    sol.satelites[sat].vetEvRoute[next].copia(evRoute, false, nullptr);
    sol.satelites[sat].vetEvRoute[next].idRota = aux;


    // Atualiza distacia e demanda
    double dist = sol.satelites[sat].vetEvRoute[next].distancia;
    //cout<<"DIST: "<<dist<<"\n";
    sol.satelites[sat].distancia += dist;
    sol.distancia += dist;
    sol.satelites[sat].demanda   += instancia.vectCliente[cliente].demanda;

    //

    sol.satelites[sat].tamVetEvRoute += 1;
    sol.vetClientesAtend[cliente] = 1;
    sol.solInicializada = true;

}