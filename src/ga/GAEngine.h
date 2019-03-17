/**
 * Created by ilya on 3/17/19.
 */

#ifndef EVOLVING_IMAGES_GAENGINE_H
#define EVOLVING_IMAGES_GAENGINE_H

#include "GATypes.h"

GA_NAMESPACE_BEGIN

class EngineComponent {
protected:
    void *engine = nullptr;

public:
    template<typename Gene>
    void attachTo(Engine<Gene> *engine) {
        this->engine = engine;
    }

    template<typename Gene>
    Engine<Gene> *getAttachedTo() {
        return static_cast<Engine<Gene> *>(this->engine);
    }
};

template<typename Gene>
class BreedSelector : protected EngineComponent {
public:
    static BreedSelector<Gene> *getInbreedingSelector();

    static BreedSelector<Gene> *getOutbreedingSelector();

    static BreedSelector<Gene> *getPanmixingSelector();

public:
    BreedSelector() = default;

    ~BreedSelector() = default;

    virtual void sort(Chromosome<Gene> *chromosomes, int count);
};

template<typename Gene>
class CrossoverExecutor : protected EngineComponent {
public:
    static CrossoverExecutor<Gene> *getSinglePointCrossoverExecutor();

    static CrossoverExecutor<Gene> *getTwoPointCrossoverExecutor();

public:

    CrossoverExecutor() = default;

    ~CrossoverExecutor() = default;

    virtual Chromosome<Gene> execute(Chromosome<Gene> &chromosome1, Chromosome<Gene> &chromosome2);
};

template<typename Gene>
class Engine {
private:
    BreedSelector<Gene> *breedSelector;
    CrossoverExecutor<Gene> *crossoverExecutor;
};

GA_NAMESPACE_END
#endif //EVOLVING_IMAGES_GAENGINE_H
