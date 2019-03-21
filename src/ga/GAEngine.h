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
class Initializer : protected EngineComponent {
public:

    virtual void init(Chromosome<Gene> *chromosomes, int required);
};

template<typename Gene>
class Selector : protected EngineComponent {
public:
    static Selector<Gene> *getAverageBoundSelector();

public:

    virtual bool survive(const Chromosome<Gene> &chromosome);
};

template<typename Gene>
class ChromosomeGenerator : protected EngineComponent {
public:

    virtual Chromosome<Gene> generate();
};

template<typename Gene>
class BreedSelector : protected EngineComponent {
public:
    static BreedSelector<Gene> *getInbreedingSelector();

    static BreedSelector<Gene> *getOutbreedingSelector();

    static BreedSelector<Gene> *getPanmixingSelector();

public:

    virtual void sort(Chromosome<Gene> *chromosomes, int count);
};

template<typename Gene>
class CrossoverExecutor : protected EngineComponent {
public:
    static CrossoverExecutor<Gene> *getSinglePointCrossoverExecutor();

    static CrossoverExecutor<Gene> *getTwoPointCrossoverExecutor();

public:

    virtual void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                         Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2);
};

template<typename Gene>
class Mutator : protected EngineComponent {
public:
    static Mutator<Gene> *getBitFlipMutator();

public:

    /**
     * Default behaviour - no mutation
     * @param chromosome Source genes
     * @return new chromosome with mutation
     */
    virtual Chromosome<Gene> execute(const Chromosome<Gene> &chromosome);
};

class Stage {
public:
    static const int INITIALIZATION = 0;
    static const int GENERATION = 1;
    static const int SELECTION = 2;
    static const int BREEDING = 3;
    static const int CROSSOVER = 4;
    static const int MUTATION = 6;
    static const int TERMINATION = 8;

    int operator()(const int &key) const {
        return key;
    }
};


template<typename Gene>
class EngineState;

template<typename Gene>
class Engine {
private:
    EngineState<Gene> *state;

public:
    Engine();

    ~Engine();

    Fitness getCurrentAverageFitness();

    int getPopulationSize();

    void setFitnessFunction(FitnessFunction<Gene> function);

    /**
     *
     * @tparam E Engine component for stage executing
     * @param stage Stage to set
     * @param component Pointer to component
     */
    template<typename E>
    void setStage(int stage, E &component);

    void setPopulationSize(int size);

    Chromosome<Gene> run();

};

GA_NAMESPACE_END
#endif //EVOLVING_IMAGES_GAENGINE_H
