//
// Created by abrolhus on 07/09/2021.
//

#include "vns.h"
#include "localSearch.h"
#include <random>


#define MAX_ITER 1000
/*
void swapVectorElements(std::vector<int>& v, int i, int j){
    int aux;
    aux = v[i];
    v[i] = v[j];
    v[j] = aux;
}
 */
std::vector<int> getRandomNeighbourhoodList(int nNeighbourhoods){
    if(nNeighbourhoods <= 0){
        exit(-1);
    }
    std::vector<int> list;
    for(int i = 0; i < nNeighbourhoods; i++){
        list.push_back(i);
    }
    return list;

}
void shuffleVector(std::vector<int>& vec) {
    for(int i = 0; i < vec.size() -1; i++) {
        int j = i + (int)random()%(vec.size() - i);
        lsh::swapVectorElements(vec, i, j);
    }
}
void vns::rvnd(Solution &Sol, const Instance &Inst) {
    std::vector<int> neighbourhoods = getRandomNeighbourhoodList(4);
    bool hasImproved;
    for(int it = 0; it < MAX_ITER; it++) {
        shuffleVector(neighbourhoods);
        for(int nh : neighbourhoods) {
            switch (nh) {
                case 0:
                    hasImproved = lsh::swap(Sol, Inst);
                    break;
                case 1:
                    hasImproved = lsh::shift(Sol, Inst);
                    break;
                case 2:
                    hasImproved = lsh::twoOpt(Sol, Inst);
                    break;
                case 3:
                    hasImproved = lsh::reinsertion(Sol, Inst);
                    break;
                default:
                    break;
            }
            if(hasImproved) {
                break;
            }
        }
    }
}
