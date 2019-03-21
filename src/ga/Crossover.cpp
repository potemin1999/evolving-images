/**
 * Created by ilya on 3/17/19.
 */

#include "GAEngine.h"

using namespace ga;

GA_NAMESPACE_BEGIN

template<typename Gene>
class SinglePointCrossoverExecutor : public CrossoverExecutor<Gene> {
public:
    void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                 Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) override;
};

template<typename Gene>
class TwoPointCrossoverExecutor : public CrossoverExecutor<Gene> {
public:
    void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                 Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) override;
};

GA_NAMESPACE_END

template<typename Gene>
void CrossoverExecutor<Gene>::execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                                      Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) {
    outChromosome1 = inChromosome1;
    outChromosome2 = inChromosome2;
}

template<typename Gene>
void SinglePointCrossoverExecutor<Gene>::execute(const Chromosome<Gene> &inChromosome1,
                                                 const Chromosome<Gene> &inChromosome2,
                                                 Chromosome<Gene> &outChromosome1,
                                                 Chromosome<Gene> &outChromosome2) {
    auto intersectionCoefficient = 0.3f;
    auto intersectionPoint = static_cast<int>(intersectionCoefficient * inChromosome1.getGenesCount());
    auto oldGenes1 = inChromosome1.getGenes();
    auto oldGenes2 = inChromosome2.getGenes();
    auto newGenes1 = new Gene[inChromosome1.getGenesCount()];
    auto newGenes2 = new Gene[inChromosome2.getGenesCount()];
    for (int i = 0; i < intersectionPoint; i++) {
        newGenes1[i] = oldGenes1[i];
        newGenes2[i] = oldGenes2[i];
    }
    for (int i = intersectionPoint; i < inChromosome1.getGenesCount(); i++) {
        newGenes1[i] = oldGenes2[i];
        newGenes2[i] = oldGenes1[i];
    }
    outChromosome1.setGenes(newGenes1, inChromosome1.getGenesCount());
    outChromosome2.setGenes(newGenes2, inChromosome2.getGenesCount());
}

template<typename Gene>
void TwoPointCrossoverExecutor<Gene>::execute(const Chromosome<Gene> &inChromosome1,
                                              const Chromosome<Gene> &inChromosome2,
                                              Chromosome<Gene> &outChromosome1,
                                              Chromosome<Gene> &outChromosome2) {
    auto intersectionCoefficient1 = 0.2f;
    auto intersectionCoefficient2 = 0.7f;
    auto intersectionPoint1 = static_cast<int>(intersectionCoefficient1 * inChromosome1.getGenesCount());
    auto intersectionPoint2 = static_cast<int>(intersectionCoefficient2 * inChromosome1.getGenesCount());
    auto oldGenes1 = inChromosome1.getGenes();
    auto oldGenes2 = inChromosome2.getGenes();
    auto newGenes1 = new Gene[inChromosome1.getGenesCount()];
    auto newGenes2 = new Gene[inChromosome2.getGenesCount()];
    for (int i = 0; i < intersectionPoint1; i++) {
        newGenes1[i] = oldGenes1[i];
        newGenes2[i] = oldGenes2[i];
    }
    for (int i = intersectionPoint1; i < intersectionPoint2; i++) {
        newGenes1[i] = oldGenes2[i];
        newGenes2[i] = oldGenes1[i];
    }
    for (int i = intersectionPoint2; i < inChromosome1.getGenesCount(); i++) {
        newGenes1[i] = oldGenes1[i];
        newGenes2[i] = oldGenes2[i];
    }
    outChromosome1.setGenes(newGenes1, inChromosome1.getGenesCount());
    outChromosome2.setGenes(newGenes2, inChromosome2.getGenesCount());
}

template<typename Gene>
CrossoverExecutor<Gene> *CrossoverExecutor<Gene>::getSinglePointCrossoverExecutor() {
    static SinglePointCrossoverExecutor<Gene> singlePointCrossoverExecutor;
    return &singlePointCrossoverExecutor;
}

template<typename Gene>
CrossoverExecutor<Gene> *CrossoverExecutor<Gene>::getTwoPointCrossoverExecutor() {
    static TwoPointCrossoverExecutor<Gene> twoPointCrossoverExecutor;
    return &twoPointCrossoverExecutor;
}
