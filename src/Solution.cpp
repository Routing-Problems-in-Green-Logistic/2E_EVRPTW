#include "Solution.h"

Solution::Solution(const Instance &Inst) {
    this->Dep = new Depot(Inst);
    this->satelites = {};
    for(int i = 0; i < Inst.getNSats(); i++){
        this->satelites.push_back(new Satelite(Inst.getFirstSatIndex() + i, Inst));
    }

}

int Solution::getNSatelites() const {
    return this->satelites.size();
}

Satelite* Solution::getSatelite(int index) {
    return this->satelites.at(index);
}

Depot* Solution::getDepot() {
    return this->Dep;
}
