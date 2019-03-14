/**
 * Created by ilya on 3/12/19.
 */

#include "Main.h"

using namespace EvoImg;

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