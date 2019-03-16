/**
 * Created by ilya on 3/12/19.
 */

#ifndef EVOLVING_IMAGES_BITMAP_H
#define EVOLVING_IMAGES_BITMAP_H

#include "Exception.h"
#include "Types.h"

JAVALIB_NAMESPACE_BEGIN

/**
 * 4 byte bitmap pixel representation
 */
typedef struct {
    UByte r;
    UByte g;
    UByte b;
    UByte a;
} Pixel;

/**
 * Bitmap
 */
class Bitmap {

public:

    enum CompressFormat {
        PNG
    };

    enum Config {
        RGB_888,
        RGBA_8888,
        ARGB_8888
    };

protected:

    friend class BitmapFactory;

    Int32 pixelDepth;
    Config config;
    UByte *data;

    UInt32 width;
    UInt32 height;

    Bitmap(UInt32 width, UInt32 height, Config config);

public:

    ~Bitmap();

    inline Config getConfig() { return config; }

    inline UByte *getData() { return data; }

    inline UInt32 getWidth() { return width; }

    inline UInt32 getHeight() { return height; }

    Pixel getPixel(Int32 x, Int32 y);

    void getPixels(Pixel *pixels, Int32 x, Int32 y, UInt32 width, UInt32 height);

    void setPixel(Int32 x, Int32 y, Pixel color);

    void setPixels(Pixel *pixels, Int32 x, Int32 y, UInt32 width, UInt32 height);

    Bitmap *clone();

    void compress(CompressFormat format, int quality, OutputStream &out);

private:

    void checkPixelAccess(Int32 x, Int32 y);

};

class BitmapFactory {

public:

    static Bitmap *decodeFile(const char *filename);

};

JAVALIB_NAMESPACE_END

#endif //EVOLVING_IMAGES_BITMAP_H
