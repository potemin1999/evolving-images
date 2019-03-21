/**
 * Created by ilya on 3/17/19.
 */

#include <map>
#include <unordered_map>
#include <Exception.h>
#include "GAEngine.h"

using namespace jl;
using namespace ga;
using namespace std;

template<typename Gene>
void Initializer<Gene>::init() {

}

template<typename Gene>
bool Selector<Gene>::survive(const Chromosome<Gene> &chromosome) {
    return chromosome.getFitness() < getAttachedTo<Gene>()->getCurrentAverageFitness();
}

template<typename Gene>
class EngineState {
public:
    int populationSize = 0;
    Fitness currentFitness = 0;
    FitnessFunction<Gene> fitnessFunction;
    Initializer<Gene> initializer;
    unordered_map<int, EngineComponent*> stagesMap;
    TerminationCondition<Gene> terminationCondition;
};

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
    //TODO: return calculated fitness
    return 0;
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
    state->populationSize = size;
}

template<typename Gene>
void FinalizeEngineSetup(Engine<Gene>* engine){
    auto stageMap = static_cast<map<int,EngineComponent*>>(engine->state->stagesMap);
    if (stageMap[Stage::SELECTION] == nullptr){
        stageMap[Stage::SELECTION] = Selector<Gene>::getAverageBoundSelector();
    }
    //TODO: autosetup other parameters
}

template<typename Gene>
Chromosome<Gene> Engine<Gene>::run() {
    if (state->initializer != nullptr){
        state->initializer.init();
    }
    FinalizeEngineSetup(this);
    if (state->populationSize<0){
        throw IllegalStateException("Invalid population size");
    }
    auto currentGeneration = new Chromosome<Gene>[state->populationSize];
    auto previousPopulation = nullptr;
    auto stageMap = static_cast<map<int,EngineComponent*>>(state->stagesMap);
    while (!state->terminationCondition.terminate()){
        auto selector = static_cast<Selector<Gene>>(stageMap[Stage::SELECTION]);
        //TODO: make iterations
        auto breedSelector = static_cast<BreedSelector<Gene>>(stageMap[Stage::BREEDING]);
        auto crossover = static_cast<CrossoverExecutor<Gene>>(stageMap[Stage::CROSSOVER]);
        auto mutator = static_cast<Mutator<Gene>>(stageMap[Stage::MUTATION]);
    }
    return Chromosome<Gene>(nullptr, 0);
}
