/**
 * Created by ilya on 3/17/19.
 */

#ifndef EVOLVING_IMAGES_GAENGINE_H
#define EVOLVING_IMAGES_GAENGINE_H

#include <map>
#include <random>
#include <unordered_map>
#include <cstring>
#include "Exception.h"


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
    Chromosome(){
        genes = nullptr;
        genesCount = 0;
        fitness = 0.0;
    }

    Chromosome(Gene *genes, int genesCount) {
        this->genes = genes;
        this->genesCount = genesCount;
        fitness = 0.0;
    }

    virtual ~Chromosome() {
        delete[] genes;
    }

    void setFitness(Fitness fitness) {
        this->fitness = fitness;
    }

    void setGenes(Gene *genes, int genesCount) {
        this->genes = genes;
        this->genesCount = genesCount;
    }

    Fitness getFitness() const {
        return this->fitness;
    }

    Gene *getGenes() const {
        return this->genes;
    }

    int getGenesCount() const {
        return this->genesCount;
    }

    int getBitCount() const {
        return getGenesCount() * sizeof(Gene);
    }
};

template<typename Gene>
class FitnessFunction {
public:
    virtual Fitness apply(Chromosome<Gene> &chromosome) { return 0; };
};

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
class EngineState {
public:
    int generation = 0;
    int populationSize = 0;
    Fitness currentFitness = 0;
    FitnessFunction<Gene> *fitnessFunction;
    std::unordered_map<int, EngineComponent *> stagesMap;
    int currentGenerationSize = 0;
    Chromosome<Gene> *currentGeneration;
    int previousGenerationSize = 0;
    Chromosome<Gene> *previousGeneration;
};


template<typename Gene>
class InitializerLambda : public Initializer<Gene> {
public:
    Chromosome<Gene> * (*initFunc)(int);

    Chromosome<Gene> * init(int required) override {
        return (*initFunc)(required);
    }
};

template<typename Gene>
class Initializer : public EngineComponent {
public:
    static Initializer<Gene> of(Chromosome<Gene> * (*initFunc)( int)) {
        InitializerLambda<Gene> initializer;
        initializer.initFunc = initFunc;
        return initializer;
    }

public:

    virtual Chromosome<Gene> * init(int required) { return nullptr; }
};


template<typename Gene>
class SelectorLambda : public Selector<Gene> {
public:
    bool (*surviveFunc)(const Chromosome<Gene> &);

    bool survive(const Chromosome<Gene> &chromosome) override {
        return (*surviveFunc)(chromosome);
    }
};

template<typename Gene>
class Selector : public EngineComponent {
public:
    static Selector<Gene> of(bool (*surviveFunc)(const Chromosome<Gene> &)) {
        SelectorLambda<Gene> selector;
        selector.surviveFunc = surviveFunc;
        return selector;
    }

    static Selector<Gene> *getAverageBoundSelector() {
        static Selector<Gene> averageBoundSelector;
        return &averageBoundSelector;
    }

public:

    virtual bool survive(const Chromosome<Gene> &chromosome) {
        return true;
    }
};


template<typename Gene>
class ChromosomeGenerator : public EngineComponent {
public:

    virtual Chromosome<Gene> generate() {
        static std::random_device randomDevice;
        static std::mt19937 randomEngine(randomDevice());
        auto engineState = getAttachedTo<Gene>()->state;
        auto hasPreviousGen = (engineState->previousGeneration != nullptr);
        auto selectionSet = hasPreviousGen ? engineState->previousGeneration : engineState->currentGeneration;
        auto upperBound = hasPreviousGen ? engineState->previousGenerationSize : engineState->currentGenerationSize;
        std::uniform_int_distribution<std::mt19937::result_type>
                geneDistribution(0, static_cast<unsigned long>(upperBound));
        auto selectionIndex = static_cast<int>(geneDistribution(randomEngine));
        return selectionSet[selectionIndex];
    }
};

template<typename Gene>
class TerminationCondition : public EngineComponent {
public:
    virtual bool terminate() { return true; };
};

/*
 * BREED SELECTION
 */

template<typename Gene>
class BreedSelectorLambda : public BreedSelector<Gene> {
public:
    void (*sortFunc)(Chromosome<Gene> *, int);

    void sort(Chromosome<Gene> *chromosomes, int count) override {
        (*sortFunc)(chromosomes, count);
    }
};

template<typename Gene>
class ClosestBreedSelector : public BreedSelector<Gene> {
/**
 * Fitness <50,49>, <48,47>, <46,45>, ... , <2,1>
 * @tparam Gene
 */
public:
    ClosestBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    ~ClosestBreedSelector() = default;

    void sort(Chromosome<Gene> *chromosomes, int count) override {
        BreedSelector<Gene>::sort(chromosomes, count);
        std::qsort(chromosomes, static_cast<size_t>(count), sizeof(Chromosome<Gene>),
                   [](const Chromosome<Gene> &chromosome1, Chromosome<Gene> &chromosome2) {
                       if (chromosome1.getFitness() > chromosome2.getFitness()) return 1;
                       if (chromosome1.getFitness() < chromosome2.getFitness()) return -1;
                       return 0;
                   }
        );
    }
};

template<typename Gene>
class FurthestBreedSelector : public BreedSelector<Gene> {
/**
 * Fitness <50,25>, <49,24>, <48,23>, ... , <26,1>
 * @tparam Gene
 */
public:
    FurthestBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    ~FurthestBreedSelector() = default;

    void sort(Chromosome<Gene> *chromosomes, int count) override {
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
};

template<typename Gene>
class RandomBreedSelector : public BreedSelector<Gene> {
/**
 * Pairs will be selected in random
 * @tparam Gene
 */
public:


    ~RandomBreedSelector() = default;

    RandomBreedSelector() : BreedSelector<Gene>::BreedSelector() {}

    void sort(Chromosome<Gene> *chromosomes, int count) override {
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
};

template<typename Gene>
class BreedSelector : public EngineComponent {
public:
    static BreedSelector<Gene> of(void (*sortFunc)(Chromosome<Gene> *, int)) {
        BreedSelectorLambda<Gene> breedSelector;
        breedSelector.sortFunc = sortFunc;
        return breedSelector;
    }

    static BreedSelector<Gene> *getInbreedingSelector() {
        static ClosestBreedSelector<Gene> inbreedingSelector;
        return &inbreedingSelector;
    }

    static BreedSelector<Gene> *getOutbreedingSelector() {
        static FurthestBreedSelector<Gene> outbreedingSelector;
        return &outbreedingSelector;
    }

    static BreedSelector<Gene> *getPanmixingSelector() {
        static RandomBreedSelector<Gene> panmixingSelector;
        return &panmixingSelector;
    }

public:

    virtual void sort(Chromosome<Gene> *chromosomes, int count) {
        if (count % 2 != 0)
            throw jl::IllegalArgumentException("Chromosomes count should be multiple 2");
    }
};

/*
 * CROSSOVER
 */

template<typename Gene>
class CrossoverExecutorLambda : public CrossoverExecutor<Gene> {
public:
    void (*executeFunc)(const Chromosome<Gene> &, const Chromosome<Gene> &,
                        Chromosome<Gene> &, Chromosome<Gene> &);

    void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                 Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) override {
        (*executeFunc)(inChromosome1, inChromosome2, outChromosome1, outChromosome2);
    }
};

template<typename Gene>
class SinglePointCrossoverExecutor : public CrossoverExecutor<Gene> {
public:
    void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                 Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) override {
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
};

template<typename Gene>
class TwoPointCrossoverExecutor : public CrossoverExecutor<Gene> {
public:
    void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                 Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) override {
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
};

template<typename Gene>
class CrossoverExecutor : public EngineComponent {
public:
    static CrossoverExecutor<Gene> of(void (*executeFunc)(const Chromosome<Gene> &, const Chromosome<Gene> &,
                                                          Chromosome<Gene> &, Chromosome<Gene> &)) {
        CrossoverExecutorLambda<Gene> crossoverExecutor;
        crossoverExecutor.executeFunc = executeFunc;
        return crossoverExecutor;
    }

    static CrossoverExecutor<Gene> *getSinglePointCrossoverExecutor() {
        static SinglePointCrossoverExecutor<Gene> singlePointCrossoverExecutor;
        return &singlePointCrossoverExecutor;
    }

    static CrossoverExecutor<Gene> *getTwoPointCrossoverExecutor() {
        static TwoPointCrossoverExecutor<Gene> twoPointCrossoverExecutor;
        return &twoPointCrossoverExecutor;
    }

public:

    virtual void execute(const Chromosome<Gene> &inChromosome1, const Chromosome<Gene> &inChromosome2,
                         Chromosome<Gene> &outChromosome1, Chromosome<Gene> &outChromosome2) {
        outChromosome1 = inChromosome1;
        outChromosome2 = inChromosome2;
    }
};

/*
 * MUTATION
 */

template<typename Gene>
class MutatorLambda : public Mutator<Gene> {
public:
    Chromosome<Gene> (*executeFunc)(const Chromosome<Gene> &);

    Chromosome<Gene> execute(const Chromosome<Gene> &chromosome) override {
        return (*executeFunc)(chromosome);
    }
};

template<typename Gene>
class BitFlipMutator : public Mutator<Gene> {
public:
    BitFlipMutator() : Mutator<Gene>::Mutator() {}

    Chromosome<Gene> execute(const Chromosome<Gene> &chromosome) override {
        static std::random_device device;
        static std::mt19937 randomEngine(device());
        int genesCount = chromosome.getGenesCount();
        std::uniform_int_distribution<std::mt19937::result_type> geneDistribution(
                0, static_cast<unsigned int>(genesCount - 1));
        auto mutationIndex = static_cast<unsigned int>(geneDistribution(randomEngine));
        int geneBitSize = chromosome.getBitCount();
        std::uniform_int_distribution<std::mt19937::result_type> bitDistribution(
                0, static_cast<unsigned int>(geneBitSize - 1));
        auto bitIndex = static_cast<unsigned int>(bitDistribution(randomEngine));
        int bytesOffset = bitIndex / 8;
        int bitOffset = bitIndex % 8;
        Gene *genes = chromosome.getGenes();
        Gene *genesCopy = new Gene[chromosome.getGenesCount()];
        std::memcpy(genesCopy, genes, static_cast<size_t>(genesCount * sizeof(Gene)));
        char *castedGenes = reinterpret_cast<char *>(genesCopy);
        castedGenes[bytesOffset] ^= static_cast<unsigned char>(1 << bitOffset);
        return Chromosome<Gene>(genesCopy, chromosome.getGenesCount());
    }
};

template<typename Gene>
class Mutator : public EngineComponent {
public:
    static Mutator<Gene> of(Chromosome<Gene> (*executeFunc)(const Chromosome<Gene> &)) {
        MutatorLambda<Gene> mutator;
        mutator.executeFunc = executeFunc;
        return mutator;
    }

    static Mutator<Gene> *getBitFlipMutator() {
        static BitFlipMutator<Gene> bitFlipMutator;
        return &bitFlipMutator;
    }

public:

    /**
     * Default behaviour - no mutation
     * @param chromosome Source genes
     * @return new chromosome with mutation
     */
    virtual Chromosome<Gene> execute(const Chromosome<Gene> &chromosome) {
        return chromosome;
    }
};

class Stage {
public:
    static constexpr int INITIALIZATION = 0;
    static constexpr int GENERATION = 1;
    static constexpr int SELECTION = 2;
    static constexpr int BREEDING = 3;
    static constexpr int CROSSOVER = 4;
    static constexpr int MUTATION = 6;
    static constexpr int TERMINATION = 8;

    int operator()(const int &key) const {
        return key;
    }
};

template<typename Gene>
class Engine {
private:
    friend class ChromosomeGenerator<Gene>;

    EngineState<Gene> *state;

public:
    Engine() {
        this->state = new EngineState<Gene>();
    }

    ~Engine() {
        delete this->state;
    }

    Fitness getCurrentAverageFitness() {
        return state->currentFitness;
    }

    int getPopulationSize() {
        return state->populationSize;
    }

    void setFitnessFunction(FitnessFunction<Gene> *function) {
        this->state->fitnessFunction = function;
    }

    /**
     *
     * @tparam E Engine component for stage executing
     * @param stage Stage to set
     * @param component Pointer to component
     */
    template<typename E>
    void setStage(int stage, E *component) {
        auto engineComponent = static_cast<EngineComponent *>(component);
        engineComponent->attachTo(this);
        if (stage == Stage::INITIALIZATION) {
            state->stagesMap[Stage::INITIALIZATION] = engineComponent;
        } else if (stage == Stage::TERMINATION) {
            state->stagesMap[Stage::TERMINATION] = engineComponent;
        } else {
            state->stagesMap[stage] = engineComponent;
        }
    }

    void setPopulationSize(int size) {
        if (size % 2 != 0) {
            throw jl::IllegalArgumentException("population size should be multiple of 2");
        }
        state->populationSize = size;
    }

    __attribute_noinline__
    Chromosome<Gene> run() {
        finalizeEngineSetup(this);
        if (state->populationSize < 0) {
            throw jl::IllegalStateException("Invalid population size");
        }
        state->currentGeneration = new Chromosome<Gene>[state->populationSize];
        state->previousGeneration = nullptr;
        auto stageMap = state->stagesMap;
        auto initializer = static_cast<Initializer<Gene> *>(stageMap[Stage::INITIALIZATION]);
        auto terminator = static_cast<TerminationCondition<Gene> *>(stageMap[Stage::TERMINATION]);
        state->currentGeneration = initializer->init(state->populationSize);
        while (!terminator->terminate()) {
            computeFitness(this->state, state->fitnessFunction);
            auto selector = static_cast<Selector<Gene> *>(stageMap[Stage::SELECTION]);
            applySelector(this->state, selector);
            auto generator = static_cast<ChromosomeGenerator<Gene> *>(stageMap[Stage::GENERATION]);
            fillCurrentGeneration(this->state, generator);
            auto breedSelector = static_cast<BreedSelector<Gene> *>(stageMap[Stage::BREEDING]);
            applyBreedSelector(this->state, breedSelector);
            auto crossover = static_cast<CrossoverExecutor<Gene> *>(stageMap[Stage::CROSSOVER]);
            applyCrossoverExecutor(this->state, crossover);
            auto mutator = static_cast<Mutator<Gene> *>(stageMap[Stage::MUTATION]);
            //applyMutator(this->state, mutator);
        }
        auto bestFitIndex = static_cast<int>(0);
        auto bestFitness = 0;
        for (int i = 0; i < state->currentGenerationSize ; i++){
            if (state->currentGeneration[i].getFitness() > bestFitness){
                bestFitness = state->currentGeneration[i].getFitness();
                bestFitIndex = i;
            }
        }
        return state->currentGeneration[bestFitIndex];
    }

private:

    void finalizeEngineSetup(Engine<Gene> *engine) {
        auto stageMap = engine->state->stagesMap;
        if (stageMap[Stage::INITIALIZATION] == nullptr) {
            throw jl::IllegalStateException("Initialization stage is required");
        }
        if (stageMap[Stage::TERMINATION] == nullptr) {
            throw jl::IllegalStateException("Termination condition is required");
        }
        if (stageMap[Stage::GENERATION] == nullptr) {
            stageMap[Stage::GENERATION] = new ChromosomeGenerator<Gene>();
            stageMap[Stage::GENERATION]->attachTo(this);
        }
        if (stageMap[Stage::SELECTION] == nullptr) {
            stageMap[Stage::SELECTION] = Selector<Gene>::getAverageBoundSelector();
            stageMap[Stage::SELECTION]->attachTo(this);
        }
        if (stageMap[Stage::BREEDING] == nullptr) {
            stageMap[Stage::BREEDING] = BreedSelector<Gene>::getPanmixingSelector();
            stageMap[Stage::BREEDING]->attachTo(this);
        }
        if (stageMap[Stage::CROSSOVER] == nullptr) {
            stageMap[Stage::CROSSOVER] = CrossoverExecutor<Gene>::getSinglePointCrossoverExecutor();
            stageMap[Stage::CROSSOVER]->attachTo(this);
        }
        if (stageMap[Stage::MUTATION] == nullptr) {
            stageMap[Stage::MUTATION] = Mutator<Gene>::getBitFlipMutator();
            stageMap[Stage::MUTATION]->attachTo(this);
        }
        engine->state->stagesMap = stageMap;
    }

    void computeFitness(EngineState<Gene> *state, FitnessFunction<Gene> *fitnessFunction) {
        Fitness fitnessSum = 0;
        for (int i = 0; i < state->currentGenerationSize; i++) {
            Fitness fit = fitnessFunction->apply(state->currentGeneration[i]);
            state->currentGeneration[i].setFitness(fit);
            fitnessSum += fit;
        }
        state->currentFitness = fitnessSum / static_cast<Fitness>(state->currentGenerationSize);
    }

    int applySelector(EngineState<Gene> *state, Selector<Gene> *selector) {
        auto currentGeneration = state->currentGeneration;
        auto survivors = new Chromosome<Gene>[state->populationSize];
        auto lastIndex = 0;
        for (int i = 0; i < state->populationSize; i++) {
            if (selector->survive(currentGeneration[i]))
                survivors[lastIndex++] = currentGeneration[i];
        }
        delete[] state->currentGeneration;
        state->currentGeneration = survivors;
        state->currentGenerationSize = lastIndex;
        return lastIndex;
    }

    int fillCurrentGeneration(EngineState<Gene> *state, ChromosomeGenerator<Gene> *generator) {
        auto startIndex = state->currentGenerationSize;
        auto endIndex = state->populationSize;
        for (int i = startIndex; i < endIndex; i++) {
            state->currentGeneration[i] = generator->generate();
        }
        state->currentGenerationSize = endIndex;
        return endIndex - startIndex;
    }

    int applyBreedSelector(EngineState<Gene> *state, BreedSelector<Gene> *breedSelector) {
        breedSelector->sort(state->currentGeneration, state->currentGenerationSize);
        return state->currentGenerationSize;
    }

    int applyCrossoverExecutor(EngineState<Gene> *state, CrossoverExecutor<Gene> *crossoverExecutor) {
        auto currentGenSize = state->currentGenerationSize;
        auto currentGen = state->currentGeneration;
        auto newGen = new Chromosome<Gene>[currentGenSize];
        for (int i = 0; i < currentGenSize; i += 2) {
            crossoverExecutor->execute(currentGen[i], currentGen[i + 1], newGen[i], newGen[i + 1]);
        }
        state->previousGeneration = currentGen;
        state->previousGenerationSize = currentGenSize;
        state->currentGeneration = newGen;
        state->currentGenerationSize = currentGenSize;
        return currentGenSize;
    }

    int applyMutator(EngineState<Gene> *state, Mutator<Gene> *mutator) {
        auto currentGen = state->currentGeneration;
        for (int i = 0; i < state->currentGenerationSize; i++) {
            currentGen[i] = mutator->execute(currentGen[i]);
        }
        return state->currentGenerationSize;
    }


};

GA_NAMESPACE_END
#endif //EVOLVING_IMAGES_GAENGINE_H
