/**
 * Created by ilya on 3/17/19.
 */

#include <random>
#include "GAEngine.h"
#include "Exception.h"

using namespace jl;

GA_NAMESPACE_BEGIN

template<typename Gene>
void BreedSelector<Gene>::sort(Chromosome<Gene> *chromosomes, int count) {
    if (count % 2 != 0)
        throw IllegalArgumentException("Chromosomes count should be multiple 2");
    //default implementation does not do any sorting
}


template<typename Gene>
class BreedSelectorLambda : public BreedSelector<Gene> {
public:
    void (*sortFunc)(Chromosome<Gene> *, int);

    void sort(Chromosome<Gene> *chromosomes, int count) override {
        (*sortFunc)(chromosomes, count);
    }
};

/**
 * Fitness <50,49>, <48,47>, <46,45>, ... , <2,1>
 * @tparam Gene
 */
template<typename Gene>
class ClosestBreedSelector : public BreedSelector<Gene> {
public:
    ClosestBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    ~ClosestBreedSelector() = default;

    void sort(Chromosome<Gene> *chromosomes, int count) override;
};

template<typename Gene>
void ClosestBreedSelector<Gene>::sort(Chromosome<Gene> *chromosomes, int count) {
    BreedSelector<Gene>::sort(chromosomes, count);
    std::qsort(chromosomes, static_cast<size_t>(count), sizeof(Chromosome<Gene>),
               [](const Chromosome<Gene> &chromosome1, Chromosome<Gene> &chromosome2) {
                   if (chromosome1.getFitness() > chromosome2.getFitness()) return 1;
                   if (chromosome1.getFitness() < chromosome2.getFitness()) return -1;
                   return 0;
               }
    );
}

/**
 * Fitness <50,25>, <49,24>, <48,23>, ... , <26,1>
 * @tparam Gene
 */
template<typename Gene>
class FurthestBreedSelector : public BreedSelector<Gene> {
public:
    FurthestBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    ~FurthestBreedSelector() = default;

    void sort(Chromosome<Gene> *chromosomes, int count) override;
};

template<typename Gene>
void FurthestBreedSelector<Gene>::sort(Chromosome<Gene> *chromosomes, int count) {
    BreedSelector<Gene>::sort(chromosomes, count);
    std::qsort(chromosomes, static_cast<size_t>(count), sizeof(Chromosome<Gene>),
               [](const Chromosome<Gene> &chromosome1, Chromosome<Gene> &chromosome2) {
                   if (chromosome1.getFitness() > chromosome2.getFitness()) return 1;
                   if (chromosome1.getFitness() < chromosome2.getFitness()) return -1;
                   return 0;
               }
    );
    int half = count / 2;
    for (int i = 1, j = half; j < count; i += 2, j++) {
        Chromosome chromosome = chromosomes[j];
        chromosomes[j] = chromosomes[i];
        chromosomes[i] = chromosome;
    }
}

/**
 * Pairs will be selected in random
 * @tparam Gene
 */
template<typename Gene>
class RandomBreedSelector : public BreedSelector<Gene> {
public:
    RandomBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    ~RandomBreedSelector() = default;

    void sort(Chromosome<Gene> *chromosomes, int count) override;
};

template<typename Gene>
void RandomBreedSelector<Gene>::sort(Chromosome<Gene> *chromosomes, int count) {
    BreedSelector<Gene>::sort(chromosomes, count);
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type>
            distribution(0, static_cast<unsigned long>(count - 1));
    for (int i = 0; i < count; i++) {
        auto j = static_cast<int>(distribution(rng));
        Chromosome chromosome = chromosomes[j];
        chromosomes[j] = chromosomes[i];
        chromosomes[i] = chromosome;
    }
}


template<typename Gene>
BreedSelector<Gene> BreedSelector<Gene>::of(void (*sortFunc)(Chromosome<Gene> *, int)) {
    BreedSelectorLambda<Gene> breedSelector;
    breedSelector.sortFunc = sortFunc;
    return breedSelector;
}

template<typename Gene>
BreedSelector<Gene> *BreedSelector<Gene>::getInbreedingSelector() {
    return new ClosestBreedSelector<Gene>();
}

template<typename Gene>
BreedSelector<Gene> *BreedSelector<Gene>::getOutbreedingSelector() {
    return new FurthestBreedSelector<Gene>();
}

template<typename Gene>
BreedSelector<Gene> *BreedSelector<Gene>::getPanmixingSelector() {
    return new RandomBreedSelector<Gene>();
}

GA_NAMESPACE_END


