/**
 * Created by ilya on 3/17/19.
 */

#ifndef EVOLVING_IMAGES_GAENGINE_H
#define EVOLVING_IMAGES_GAENGINE_H

#include "GATypes.h"

GA_NAMESPACE_BEGIN

template<typename Gene>
class BreedSelector {
public:
    static BreedSelector<Gene> *getInbreedingSelector();

    static BreedSelector<Gene> *getOutbreedingSelector();

    static BreedSelector<Gene> *getPanmixingSelector();

private:
    Engine *engine = nullptr;

protected:
    friend class Engine;

    void attachTo(Engine *engine) {
        this->engine = engine;
    }

public:
    BreedSelector() = default;

    ~BreedSelector() = default;

    virtual void sort(Chromosome<Gene> *chromosomes, int count);
};

class Crossover {

};

class Engine {
private:

};

GA_NAMESPACE_END
#endif //EVOLVING_IMAGES_GAENGINE_H
