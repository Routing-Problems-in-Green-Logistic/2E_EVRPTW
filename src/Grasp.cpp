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

#define NUM_EST_INI 3

using namespace GreedyAlgNS;
using namespace NameS_Grasp;

const float fator = 0.1;

Solucao * NameS_Grasp::grasp(Instance &instance, const int numIte, const std::vector<float> &vetAlfa, const int numAtuaProb, Estatisticas &estat)
{
    Solucao *solBest = new Solucao(instance);
    solBest->distancia = DOUBLE_MIN;
    solBest->viavel = false;

    estat.numSol = 0.0;
    estat.numIte = numIte;
    estat.distAcum = 0.0;
    estat.erro = "";

    const int tamAlfa = vetAlfa.size();

    // Solucao para inicializar reativo
    Solucao gul(instance);
    construtivo(gul, instance, 0.0, 0.0);
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

    double somaProb = 0.0;
    int posAlfa = 0;
    int valAleatorio = 0;

    float alfa = 0;

    for(int i=0; i < numIte; ++i)
    {
        Solucao sol(instance);

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




        alfa = vetAlfa[posAlfa];
        vetorFrequencia[posAlfa] += 1;
        construtivo(sol, instance, alfa, alfa);

        if(sol.viavel)
        {

/*            if(!sol.checkSolution(estat.erro, instance))
            {
                cout<<"\n\nSOLUCAO:\n\n";
                sol.print(instance);

                cout << estat.erro<< "\n****************************************************************************************\n\n";
                delete solBest;
                return nullptr;
            }
            else*/
            {

                estat.numSol += 1;
                estat.distAcum += sol.distancia;

                if(sol.distancia < solBest->distancia || !solBest->viavel)
                {
                    solBest->copia(sol);
                    custoBest = solBest->distancia;
                    //solBest->print(instance);
                    //cout<<"i: "<<i<<"\n";

                }

            }

            solucaoAcumulada[posAlfa] += sol.distancia;

        }
        else if(!solBest->viavel && !sol.viavel)
        {

            solBest->copia(sol);

            double aux = sol.distancia + getPenalidade(sol, instance, fator) ;
            if(aux < custoBest)
                custoBest = aux;

        }

        if(!sol.viavel)
            solucaoAcumulada[posAlfa] += sol.distancia + getPenalidade(sol, instance, fator);

        if(i>0 && (i%numAtuaProb)==0)
            atualizaProb();


    }

    if(solBest->viavel)
    {
        estat.erro = "";
        if(!solBest->checkSolution(estat.erro, instance))
        {
            cout<<"\n\nSOLUCAO:\n\n";
            solBest->print(instance);

            cout << estat.erro<< "\n****************************************************************************************\n\n";
            delete solBest;
            return nullptr;
        }
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

        for(int r=instance.getFirstRechargingSIndex(); r <= instance.getEndRechargingSIndex(); ++r)
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