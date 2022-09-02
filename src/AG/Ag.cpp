/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    01/09/22
 *  Arquivo: ag.cpp
 * ****************************************
 * ****************************************/

#include "Ag.h"
#include "../mersenne-twister.h"

NS_Ag::RandomKey::RandomKey(Instance &instancia)
{

    int tam   = instancia.numClients + instancia.numRechargingS*instancia.numUtilEstacao*instancia.numEv;
    vetRandom = std::vector<RandomNo>(tam);
    vetDecod  = std::vector<RandomNo>(tam);

    int vetIndex = 0;

    for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
    {

        for(int quant = 0; quant < instancia.numUtilEstacao*instancia.numEv; ++quant, ++vetIndex)
        {
            vetRandom[vetIndex].cliente = rs;
            vetRandom[vetIndex].chave   = (rand_u32()%100)/1000.0;
        }

    }

    for(int clie=instancia.getFirstClientIndex(); clie <= instancia.getEndClientIndex(); ++clie, ++vetIndex)
    {
        vetRandom[vetIndex].cliente = clie;
        vetRandom[vetIndex].chave   = (rand_u32()%1000)/1000.0;
    }

}

// Para cada cliente(RS, 2°, 3° RS, clientes) gera um valor aleatorio para chave [0, 1)
void NS_Ag::RandomKey::geraRandoKey()
{
    for(auto it:vetRandom)
        it.chave = (rand_u32()%1000)/1000.0;
}

void NS_Ag::RandomKey::ordenaVetDecod()
{
    std::copy(vetRandom.begin(), vetRandom.end(), vetDecod.begin());
    std::sort(vetDecod.begin(), vetDecod.end());

}

void NS_Ag::RandomKey::printVetDecod(string &str)
{
    for(auto clie:vetDecod)
        str += to_string(clie.cliente)+"\t";

    str += "\n";

    for(auto clie:vetDecod)
        str += to_string(clie.chave)+"\t";

    str += "\n";
}

/** ******************************************************************************
 ** ******************************************************************************
 **
 ** @param instancia
 ** @param randKey                  Chama ordenaVetDecod(); modificado
 ** @param sat                      Guarda a solucao para o satelite; modificado
 ** @param vetSatAtendCliente       Indica o sat para cada cliente; nao modificado
 ** @param tempoSaida               Tempo de saida para sat, se <= 0: tempoSaida=vetTempoSaida[sat]; nao modificado
 **
 ** ******************************************************************************
 ** ******************************************************************************/

void NS_Ag::decodificaSol(Instance &instancia, RandomKey &randKey, Satelite &sat, const vector<int> &vetSatAtendCliente, const double tempoSaida)
{

    randKey.ordenaVetDecod();

    double tempoS = tempoSaida;
    if(tempoS <= 0.0)
        tempoS = instancia.vetTempoSaida[sat.sateliteId];

    // Reseta o satelite
    for(int ev=0; ev < instancia.numEv; ++ev)
    {
        sat.vetEvRoute[ev].routeSize = 2;
        sat.vetEvRoute[ev].route[1].cliente = sat.vetEvRoute[ev].route[0].cliente = sat.sateliteId;
        sat.vetEvRoute[ev].route[0].tempoSaida = instancia.vetTempoSaida[sat.sateliteId];
    }


    // Seta 0 para o cliente que deve ser atendido por sat e 1 caso contrario
    vector<int> vetClieAtend(vetSatAtendCliente);

    for(int &it : vetClieAtend)
    {
        if(it == sat.sateliteId)
            it = 0;
        else
            it = 1;
    }


    int posAnt = 0;     // Guarda ...
    int pos    = 0;     // Guarda ...



}