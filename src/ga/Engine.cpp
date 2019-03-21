/**
 * Created by ilya on 3/17/19.
 */

#include <map>
#include <unordered_map>
#include <Exception.h>
#include <random>
#include "GAEngine.h"

using namespace jl;
using namespace ga;
using namespace std;

template<typename Gene>
void Initializer<Gene>::init(Chromosome<Gene> *chromosomes, int required) {

}

template<typename Gene>
bool Selector<Gene>::survive(const Chromosome<Gene> &chromosome) {
    return chromosome.getFitness() > getAttachedTo<Gene>()->getCurrentAverageFitness();
}

template<typename Gene>
Selector<Gene> *Selector<Gene>::getAverageBoundSelector() {
    Selector<Gene> averageBoundSelector;
    return averageBoundSelector;
}

GA_NAMESPACE_BEGIN

template<typename Gene>
class EngineState {
public:
    int generation = 0;
    int populationSize = 0;
    Fitness currentFitness = 0;
    FitnessFunction<Gene> fitnessFunction;
    unordered_map<int, EngineComponent *> stagesMap;
    TerminationCondition<Gene> terminationCondition;
    int currentGenerationSize = 0;
    Chromosome<Gene> *currentGeneration;
    int previousGenerationSize = 0;
    Chromosome<Gene> *previousGeneration;
};

GA_NAMESPACE_END

template<typename Gene>
Chromosome<Gene> ChromosomeGenerator<Gene>::generate() {
    static random_device randomDevice;
    static mt19937 randomEngine(randomDevice);
    auto engineState = getAttachedTo<Gene>()->state;
    auto hasPreviousGen = (engineState->previousGenerationSize == nullptr);
    auto selectionSet = hasPreviousGen ? engineState->previousGeneration : engineState->currentGeneration;
    auto upperBound = hasPreviousGen ? engineState->previousGenerationSize : engineState->currentGenerationSize;
    uniform_int_distribution<mt19937::result_type> geneDistribution(0, static_cast<unsigned long>(upperBound));
    auto selectionIndex = static_cast<int>(geneDistribution(randomEngine));
    return selectionSet[selectionIndex];
}

template<typename Gene>
Engine<Gene>::Engine() {
    this->state = new EngineState<Gene>();
}

template<typename Gene>
Engine<Gene>::~Engine() {
    delete this->state;
}

template<typename Gene>
Fitness Engine<Gene>::getCurrentAverageFitness() {
    return state->currentFitness;
}

template<typename Gene>
int Engine<Gene>::getPopulationSize() {
    return state->populationSize;
}

template<typename Gene>
void Engine<Gene>::setFitnessFunction(FitnessFunction<Gene> function) {
    this->state->fitnessFunction = function;
}

template<typename Gene>
template<typename E>
void Engine<Gene>::setStage(int stage, E &component) {
    static_assert(component != nullptr);
    auto engineComponent = static_cast<EngineComponent>(component);
    engineComponent.attachTo(this);
    if (stage == Stage::INITIALIZATION) {
        state->initializer = static_cast<Initializer<Gene>>(component);
    } else if (stage == Stage::TERMINATION) {
        state->terminationCondition = static_cast<TerminationCondition<Gene>>(component);
    }
    state->stagesMap.insert(stage, &component);
}

template<typename Gene>
void Engine<Gene>::setPopulationSize(int size) {
    if (size % 2 != 0) {
        throw IllegalArgumentException("population size should be multiple of 2");
    }
    state->populationSize = size;
}

template<typename Gene>
void FinalizeEngineSetup(Engine<Gene> *engine) {
    auto stageMap = static_cast<map<int, EngineComponent *>>(engine->state->stagesMap);
    if (stageMap[Stage::INITIALIZATION] == nullptr) {
        throw IllegalStateException("Initialization stage is required");
    }
    if (stageMap[Stage::TERMINATION] == nullptr) {
        throw IllegalStateException("Termination condition is required");
    }
    if (stageMap[Stage::GENERATION] == nullptr) {
        stageMap[Stage::GENERATION] = new ChromosomeGenerator<Gene>();
    }
    if (stageMap[Stage::SELECTION] == nullptr) {
        stageMap[Stage::SELECTION] = Selector<Gene>::getAverageBoundSelector();
    }
    if (stageMap[Stage::BREEDING] == nullptr) {
        stageMap[Stage::BREEDING] = BreedSelector<Gene>::getPanmixingSelector();
    }
    if (stageMap[Stage::CROSSOVER] == nullptr) {
        stageMap[Stage::CROSSOVER] = CrossoverExecutor<Gene>::getSinglePointCrossoverExecutor();
    }
    if (stageMap[Stage::MUTATION] == nullptr) {
        stageMap[Stage::MUTATION] = Mutator<Gene>::getBitFlipMutator();
    }
}

template<typename Gene>
void ComputeFitness(EngineState<Gene> *state, FitnessFunction<Gene> &fitnessFunction) {
    Fitness fitnessSum = 0;
    for (int i = 0; i < state->currentGenerationSize; i++) {
        Fitness fit = fitnessFunction.apply(state->currentGeneration[i]);
        state->currentGeneration[i].setFitness(fit);
        fitnessSum += fit;
    }
    state->currentFitness = fitnessSum / static_cast<Fitness>(state->currentGenerationSize);
}

template<typename Gene>
int ApplySelector(EngineState<Gene> *state, Selector<Gene> *selector) {
    auto currentGeneration = state->currentGeneration;
    auto survivors = new Chromosome<Gene>[state->populationSize];
    auto lastIndex = 0;
    for (int i = 0; i < state->populationSize; i++) {
        if (selector->survive(currentGeneration[i]))
            survivors[lastIndex++] = currentGeneration[i];
    }
    delete state->currentGeneration;
    state->currentGeneration = survivors;
    state->currentGenerationSize = lastIndex;
    return lastIndex;
}

template<typename Gene>
int FillCurrentGeneration(EngineState<Gene> *state, ChromosomeGenerator<Gene> *generator) {
    auto startIndex = state->currentGenerationSize;
    auto endIndex = state->populationSize;
    for (int i = startIndex; i < endIndex; i++) {
        state->currentGeneration[i] = generator->generate();
    }
    state->currentGenerationSize = endIndex;
    return endIndex - startIndex;
}

template<typename Gene>
int ApplyBreedSelector(EngineState<Gene> *state, BreedSelector<Gene> *breedSelector) {
    breedSelector->sort(state->currentGeneration, state->currentGenerationSize);
    return state->currentGenerationSize;
}

template<typename Gene>
int ApplyCrossoverExecutor(EngineState<Gene> *state, CrossoverExecutor<Gene> *crossoverExecutor) {
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

template<typename Gene>
int ApplyMutator(EngineState<Gene> *state, Mutator<Gene> *mutator) {
    auto currentGen = state->currentGeneration;
    for (int i = 0; i < state->currentGenerationSize; i++) {
        currentGen[i] = mutator->execute(currentGen[i]);
    }
    return state->currentGenerationSize;
}

template<typename Gene>
Chromosome<Gene> Engine<Gene>::run() {
    FinalizeEngineSetup(this);
    if (state->populationSize < 0) {
        throw IllegalStateException("Invalid population size");
    }
    state->currentGeneration = new Chromosome<Gene>[state->populationSize];
    state->previousGeneration = nullptr;
    auto stageMap = static_cast<map<int, EngineComponent *>>(state->stagesMap);
    auto initializer = static_cast<Initializer<Gene>>(stageMap[Stage::INITIALIZATION]);
    auto terminator = static_cast<TerminationCondition<Gene>>(stageMap[Stage::TERMINATION]);
    initializer->init(state->currentGeneration, state->populationSize);
    while (terminator->terminate()) {
        ComputeFitness(this->state, state->fitnessFunction);
        auto selector = static_cast<Selector<Gene>>(stageMap[Stage::SELECTION]);
        ApplySelector(this->state, selector);
        auto generator = static_cast<ChromosomeGenerator<Gene>>(stageMap[Stage::GENERATION]);
        FillCurrentGeneration(this->state, generator);
        auto breedSelector = static_cast<BreedSelector<Gene>>(stageMap[Stage::BREEDING]);
        ApplyBreedSelector(this->state, breedSelector);
        auto crossover = static_cast<CrossoverExecutor<Gene>>(stageMap[Stage::CROSSOVER]);
        ApplyCrossoverExecutor(this->state, crossover);
        auto mutator = static_cast<Mutator<Gene>>(stageMap[Stage::MUTATION]);
        ApplyMutator(this->state, mutator);
    }
    return Chromosome<Gene>(nullptr, 0);
}
