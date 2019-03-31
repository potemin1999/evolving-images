/**
 * Created by ilya on 3/12/19.
 */

#include <values.h>
#include "Main.h"

using namespace ga;
using namespace jl;

class PaletteGenerator : ChromosomeGenerator<Pixel> {
    virtual Chromosome<Pixel> generate() {

    }
};

class ImageFitnessFunction : public FitnessFunction<Pixel> {
private:
    Bitmap *ideal;
    Bitmap *last;
public:
    ImageFitnessFunction() : FitnessFunction<Pixel>() {
        ideal = nullptr;
        last = nullptr;
    }

    ~ImageFitnessFunction(){
        delete ideal;
        delete last;
    }

    void setIdeal(Bitmap *bitmap) {
        ideal = bitmap;
    }

    Bitmap *getLastImage() {
        return last;
    }

    Fitness apply(Chromosome<Pixel> &chromosome) override {
        if (last != nullptr){
            delete last;
        }
        auto width = ideal->getWidth();
        auto height = ideal->getHeight();
        auto paletted = ideal->clone();
        auto errorSum = static_cast<float>(0);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                auto px = paletted->getPixel(i, j);
                auto genesCount = chromosome.getGenesCount();
                auto sqrDistances = new float[genesCount];
                auto genes = chromosome.getGenes();
                for (int k = 0; k < genesCount; k++) {
                    auto gene = genes[k];
                    auto dr = static_cast<float>(px.r - gene.r);
                    auto dg = static_cast<float>(px.g - gene.g);
                    auto db = static_cast<float>(px.b - gene.b);
                    sqrDistances[k] = dr * dr + dg * dg + db * db;
                }
                auto minElemValue = static_cast<float>(MINFLOAT);
                auto minElemIndex = static_cast<Int32>(0);
                for (int k = 0; k < genesCount; k++) {
                    if (sqrDistances[k] < minElemValue) {
                        minElemValue = sqrDistances[k];
                        minElemIndex = k;
                    }
                }
                paletted->setPixel(i, j, genes[minElemIndex]);
                errorSum += minElemValue;
            }
        }
        const auto maxRandomError = static_cast<float>(3 * 128 * 128) * 512 * 512;
        return 1 - errorSum / maxRandomError;
    };
};

class ImageGenerationInitializer : public Initializer<Pixel> {
public:
    Chromosome<Pixel> * init(int required) override {
        auto chromosomes = new Chromosome<Pixel>[required];
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type>
                distribution(0, static_cast<UByte>(255));
        for (int i = 0; i < required; i++) {
            auto pixels = new Pixel[PALETTE_SIZE];
            for (int j = 0; j < PALETTE_SIZE; j++) {
                auto r = static_cast<UByte>(distribution(rng));
                auto g = static_cast<UByte>(distribution(rng));
                auto b = static_cast<UByte>(distribution(rng));
                pixels[j] = Pixel{.r = r, .g = g, .b = b};
            }
            chromosomes[i].setGenes(pixels, PALETTE_SIZE);
        }
        return chromosomes;
    }
};

class ImageGenerationTerminator : public TerminationCondition<Pixel> {
public:
    bool terminate() override {
        return getAttachedTo<Pixel>()->getCurrentAverageFitness() > 0.5;
    };
};

/*
int main(int argc, char **argv) {
    try {
        auto bitmap = BitmapFactory::decodeFile("./img.png");
        auto bitmap2 = bitmap->clone();
        delete bitmap;
        FileOutputStream out;
        out.open("./out.png");
        auto blackPixels = new Pixel[40 * 40];
        for (int i = 0; i < 40 * 40; i++) {
            blackPixels[i] = Pixel{.r=20, .g=20, .b=20, .a=255};
        }
        bitmap2->setPixels(blackPixels, 200, 200, 40, 40);
        delete[] blackPixels;
        bitmap2->compress(Bitmap::PNG, 100, out);
        out.close();
        delete bitmap2;
        return 0;
    } catch (Exception &e) {
        e.printStackTrace();
        throw IllegalStateException("Exception in function main()", &e);
    }
}
 */

int main(int argc, char **argv) {
    auto bitmap = BitmapFactory::decodeFile("./img.png");
    auto engine = new Engine<Pixel>();
    auto imageFitnessFunction = new ImageFitnessFunction();
    auto initializer = ImageGenerationInitializer();
    auto terminator = ImageGenerationTerminator();
    imageFitnessFunction->setIdeal(bitmap);
    engine->setPopulationSize(50);
    engine->setFitnessFunction(imageFitnessFunction);
    engine->setStage(Stage::INITIALIZATION, &initializer);
    //engine->setStage(Stage::GENERATION, )
    engine->setStage(Stage::TERMINATION, &terminator);
    Chromosome<Pixel> bestPalette = engine->run();
    imageFitnessFunction->apply(bestPalette);
    Bitmap* bestBitmap = imageFitnessFunction->getLastImage();
    FileOutputStream out;
    out.open("./out.png");
    bestBitmap->compress(Bitmap::PNG, 100, out);
    out.close();
    int i = 0;
}