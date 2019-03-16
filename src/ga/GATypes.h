/**
 * Created by ilya on 3/16/19.
 */

#ifndef EVOLVING_IMAGES_GATYPES_H
#define EVOLVING_IMAGES_GATYPES_H

#define GA_NAMESPACE_BEGIN namespace ga{
#define GA_NAMESPACE_END }

GA_NAMESPACE_BEGIN

template<typename Gene>
class Chromosome{
private:
    Gene* genes;
    int genCount;
public:
    Chromosome(Gene* genes,int genesCount);
    virtual ~Chromosome();
    virtual void setGenes(Gene* gens,int genesCount);
    virtual const Gene* getGenes();
    virtual int getGenesCount();
    virtual int getBitCount();
};

GA_NAMESPACE_END

#endif //EVOLVING_IMAGES_GATYPES_H
