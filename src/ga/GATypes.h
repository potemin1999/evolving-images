/**
 * Created by ilya on 3/16/19.
 */

#ifndef EVOLVING_IMAGES_GATYPES_H
#define EVOLVING_IMAGES_GATYPES_H

#define GA_NAMESPACE_BEGIN namespace ga{
#define GA_NAMESPACE_END }

GA_NAMESPACE_BEGIN

typedef double Fitness;

template<typename Gene>
class Engine;

template<typename Gene>
class Initializer;

template<typename Gene>
class Selector;

template<typename Gene>
class BreedSelector;

template<typename Gene>
class CrossoverExecutor;

template<typename Gene>
class Mutator;

template<typename Gene>
class Chromosome {
private:
    Gene *genes;
    int genesCount;
    Fitness fitness;

public:
    Chromosome() = default;

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

    Fitness getFitness() const{
        return this->fitness;
    }

    Gene *getGenes() const {
        return this->genes;
    }

    int getGenesCount() const {
        return this->genesCount;
    }

    int getBitCount() const{
        return getGenesCount() * sizeof(Gene);
    }
};

template<typename Gene>
class FitnessFunction{
public:
    virtual Fitness apply(Chromosome<Gene> &chromosome) { return 0; };
};

GA_NAMESPACE_END

#endif //EVOLVING_IMAGES_GATYPES_H
