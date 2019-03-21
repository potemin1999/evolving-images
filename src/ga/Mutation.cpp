/**
 * Created by ilya on 3/21/19.
 */

#include <random>
#include <cstring>
#include <Types.h>
#include "GAEngine.h"

using namespace jl;
using namespace ga;
using namespace std;

template<typename Gene>
class BitFlipMutator : public Mutator<Gene> {
private:
    random_device *randomDevice;
    mt19937 *randomEngine;
public:
    BitFlipMutator();

    Chromosome<Gene> execute(const Chromosome<Gene> &chromosome) override;
};

template<typename Gene>
BitFlipMutator<Gene>::BitFlipMutator() : Mutator<Gene>::Mutator() {
    randomDevice = new random_device();
    randomEngine = new mt19937(randomDevice);
}

template<typename Gene>
Chromosome<Gene> BitFlipMutator<Gene>::execute(const Chromosome<Gene> &chromosome) {
    int genesCount = chromosome.getGenesCount();
    uniform_int_distribution<mt19937::result_type> geneDistribution(
            0, static_cast<unsigned long>(genesCount - 1));
    int mutationIndex = static_cast<int>(geneDistribution(randomEngine));
    int geneBitSize = chromosome.getBitCount();
    uniform_int_distribution<mt19937::result_type> bitDistribution(
            0, static_cast<unsigned long>(geneBitSize - 1));
    int bitIndex = static_cast<int>(bitDistribution(randomEngine));
    int bytesOffset = bitIndex / 8;
    int bitOffset = bitIndex % 8;
    Gene* genes = chromosome.getGenes();
    Gene* genesCopy = new Gene[chromosome.getGenesCount()];
    memcpy(genesCopy,genes, static_cast<size_t>(genesCount*sizeof(Gene)));
    char* castedGenes = (char*) genesCopy;
    castedGenes[bytesOffset] ^= static_cast<UByte>(1 << bitOffset);
    return Chromosome<Gene>(genesCopy,chromosome.getGenesCount());
}

template<typename Gene>
Mutator<Gene> *Mutator<Gene>::getBitFlipMutator() {
    return new BitFlipMutator<Gene>();
}

template<typename Gene>
Chromosome<Gene> Mutator<Gene>::execute(const Chromosome<Gene> &chromosome) {
    return chromosome;
}