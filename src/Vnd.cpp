

#include "Vnd.h"
#include "mersenne-twister.h"
#include "LocalSearch.h"

using namespace NS_vnd;
using namespace NS_LocalSearch;

/*

void NS_vnd::rvnd(Solution &solution, const Instance &instance)
{

    int vetMV[NUM_MAX];

    for(int i=0; i < NUM_MAX; ++i)
    {
        vetMV[i] = rand_u32() % NUM_MAX;

        int j=0;
        while(j < i)
        {
            if(vetMV[i] == vetMV[j])
            {
                vetMV[i] = (vetMV[i]+1) % NUM_MAX;
                j = 0;
            }
            else
                j = j+1;
        }
    }

    int i = 0;

    while(i < NUM_MAX)
    {
        bool melhora = false;
        float aux = 0;

        switch(i)
        {
            case VND_SHIFIT_INTRA_ROTA:
                melhora = mvShifitIntraRota(solution, instance);
                break;

            case VND_SHIFIT_INTER_ROTAS:
                melhora = mvShiftInterRotasIntraSatellite(solution, instance);
                break;

            case VND_SWAP_INTRA_ROTA:

                //melhora = interSatelliteSwap(solution, instance, aux);
                break;

        }

        if(melhora)
            i = 0;
        else
            i = i+1;
    }

}

*/