/**
 * Created by ilya on 3/16/19.
 */

#ifndef EVOLVING_IMAGES_GATYPES_H
#define EVOLVING_IMAGES_GATYPES_H

#define GA_NAMESPACE_BEGIN namespace ga{
#define GA_NAMESPACE_END }

GA_NAMESPACE_BEGIN

typedef double Fitness;


class Engine;

template<typename Gene>
class BreedSelector;

class Crossover;

template<typename Gene>
class Chromosome {
private:
    Gene *genes;
    int genesCount;
    Fitness fitness;

public:
    Chromosome(Gene *genes, int genesCount) {
        this->setGenes(genes, genesCount);
    }

    virtual ~Chromosome() {
        delete[] genes;
    }

    void setFitness(Fitness fitness) {
        this->fitness = fitness;
    }

    void setGenes(Gene *gens, int genesCount) {
        this->genes = genes;
        this->genesCount = genesCount;
    }

    const Fitness getFitness() {
        return this->fitness;
    }

    const Gene *getGenes() {
        return this->genes;
    }

    const int getGenesCount() {
        return this->genesCount;
    }

    const int getBitCount() {
        return getGenesCount() * sizeof(Gene);
    }
};

GA_NAMESPACE_END

#endif //EVOLVING_IMAGES_GATYPES_H