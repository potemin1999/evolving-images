/**
 * Created by ilya on 3/17/19.
 */

#include "GAEngine.h"

using namespace ga;

template<typename Gene>
Chromosome<Gene> CrossoverExecutor<Gene>::execute(Chromosome<Gene> &chromosome1, Chromosome<Gene> &chromosome2) {
    getAttachedTo<Gene>()->crossoverExecutor = nullptr;
    return Chromosome<Gene>(nullptr, 0);
}

template<typename Gene>
CrossoverExecutor<Gene> *CrossoverExecutor<Gene>::getSinglePointCrossoverExecutor() {
    return nullptr;
}

template<typename Gene>
CrossoverExecutor<Gene> *CrossoverExecutor<Gene>::getTwoPointCrossoverExecutor() {
    return nullptr;
}
