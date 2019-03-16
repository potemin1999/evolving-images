/**
 * Created by ilya on 3/17/19.
 */

#include <cstring>
#include "GATypes.h"

using namespace ga;
using namespace std;

template<typename Gene>
Chromosome<Gene>::Chromosome(Gene *genes, int genesCount) {
    this->setGens(genes,genesCount);
}

template<typename Gene>
Chromosome<Gene>::~Chromosome() {
    delete[] genes;
}

template<typename Gene>
void Chromosome<Gene>::setGenes(Gene *genes, int genesCount) {
    this->genes = genes;
    this->genesCount = genesCount;
}

template<typename Gene>
const Gene *Chromosome<Gene>::getGenes() {
    return this->genes;
}

template<typename Gene>
int Chromosome<Gene>::getGenesCount() {
    return this->genesCount;
}

template<typename Gene>
int Chromosome<Gene>::getBitCount() {
    return this->genCount*sizeof(Gene)*8;
}
