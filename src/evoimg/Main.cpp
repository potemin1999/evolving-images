/**
 * Created by ilya on 3/12/19.
 */

#include "Main.h"

using namespace ga;
using namespace jl;

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
typedef UInt32 Gene;
template class ga::Engine<Gene>;
/*
template class ga::Chromosome<Gene>;
template class ga::Initializer<Gene>;
template class ga::TerminationCondition<Gene>;
*/
int main(int argc, char **argv) {
    Engine<UInt32>* engine = new Engine<UInt32>();
    auto initializer = Initializer<UInt32>::of([](auto chromosomes,auto required){});
    engine->setStage(Stage::INITIALIZATION,&initializer);
    engine->run();
}