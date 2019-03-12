/**
 * Created by ilya on 3/12/19.
 */

#include "Bitmap.h"
#include <png.h>
#include <cstring>

namespace EvoImg {

Bitmap::Bitmap(UInt32 width, UInt32 height, Config config) {
    this->width = width;
    this->height = height;
    this->config = config;
    if (config == Config::RGB_888) {
        this->pixelDepth = 3;
    } else {
        this->pixelDepth = 4;
    }
}

Bitmap::~Bitmap() {
    delete[] this->data;
}

//unsafe
inline Pixel GetPixel(UByte *address, Bitmap::Config config) {
    Pixel px;
    UByte *i = address + (config == Bitmap::Config::ARGB_8888 ? 1 : 0);
    px.r = i[0];
    px.g = i[1];
    px.b = i[2];
    if (config == Bitmap::Config::ARGB_8888) {
        px.a = i[-1];
    } else if (config == Bitmap::Config::RGBA_8888) {
        px.a = i[3];
    } else {
        px.a = 255;
    }
    return px;
}

//unsafe
inline void SetPixel(UByte *address, Pixel color, Bitmap::Config config) {
    UByte *i = address + (config == Bitmap::Config::ARGB_8888 ? 1 : 0);
    i[0] = color.r;
    i[1] = color.g;
    i[2] = color.b;
    if (config == Bitmap::Config::ARGB_8888) {
        i[-1] = color.a;
    } else if (config == Bitmap::Config::RGBA_8888) {
        i[3] = color.a;
    }
}

Pixel Bitmap::getPixel(Int32 x, Int32 y) {
    checkPixelAccess(x, y);
    auto pixelAddr = this->data + y * this->width * this->pixelDepth + x;
    return GetPixel(pixelAddr, this->config);
}

void Bitmap::setPixel(Int32 x, Int32 y, Pixel color) {
    checkPixelAccess(x, y);
    auto pixelAddr = this->data + y * this->width * this->pixelDepth + x;
    SetPixel(pixelAddr, color, this->config);
}

void Bitmap::getPixels(Pixel *px, Int32 x, Int32 y, UInt32 width, UInt32 height) {
    checkPixelAccess(x, y);
    checkPixelAccess(x + width, y + height);
    auto pixels = this->data;
    for (auto j = y; j < y + height; ++j) {
        auto row = pixels + j * this->width * this->pixelDepth;
        auto offset = (j - y) * width;
        for (auto i = x; i < x + width; ++i) {
            px[offset + (i - x)] = GetPixel(row + i * this->pixelDepth, this->config);
        }
    }
}

void Bitmap::setPixels(Pixel *px, Int32 x, Int32 y, UInt32 width, UInt32 height) {
    checkPixelAccess(x, y);
    checkPixelAccess(x + width, y + height);
    auto pixels = this->data;
    for (auto j = y; j < y + height; ++j) {
        auto row = pixels + j * this->width * this->pixelDepth;
        auto offset = (j - y) * width;
        for (auto i = x; i < x + width; ++i) {
            SetPixel(row + i * this->pixelDepth, px[offset + (i - x)], this->config);
        }
    }
}

Bitmap *Bitmap::clone() {
    auto length = this->width * this->height;
    auto data = new UByte[length * this->pixelDepth];
    memcpy(data, this->data, length * this->pixelDepth);
    auto bitmap = new Bitmap(this->width, this->height, this->config);
    bitmap->data = data;
    return bitmap;
}

void DoWriteToStream(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto out = (OutputStream *) png_get_io_ptr(png_ptr);
    out->write((Byte *) data, length);
}

void CompressPng(Bitmap *bitmap, OutputStream &out) {
    //prepare
    auto width = bitmap->getWidth();
    auto height = bitmap->getHeight();
    auto bytes = bitmap->getData();
    auto config = bitmap->getConfig();
    auto png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) abort();
    auto info = png_create_info_struct(png);
    if (!info) abort();
    if (setjmp(png_jmpbuf(png))) abort();

    //write
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
    );
    png_set_write_fn(png, &out, DoWriteToStream, nullptr);
    png_write_info(png, info);
    auto stride = (config == Bitmap::Config::RGB_888 ? 3 : 4);
    for (auto y = 0; y < height; ++y) {
        auto row = bytes + y * width * stride;
        png_write_row(png, row);
    }

    //finish
    png_write_end(png, nullptr);
    png_destroy_write_struct(&png, &info);
}

void Bitmap::compress(CompressFormat format, int quality, OutputStream &out) {
    if (format == CompressFormat::PNG) {
        CompressPng(this, out);
    }
}

void Bitmap::checkPixelAccess(Int32 x, Int32 y) {
    if (x < 0)
        throw IllegalArgumentException("x can not be < 0");
    if (y < 0)
        throw IllegalArgumentException("y can not be < 0");
    if (x >= this->width)
        throw IllegalArgumentException("x should be < bitmap.getWidth()");
    if (y >= this->height)
        throw IllegalArgumentException("y should be < bitmap.getHeight()");
}


Bitmap *BitmapFactory::decodeFile(const char *filename) {
    //init
    auto file = std::fopen(filename, "rb");
    if (!file) return nullptr;
    auto png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) return nullptr;
    auto info = png_create_info_struct(png);
    if (!info) return nullptr;
    if (setjmp(png_jmpbuf(png))) return nullptr;
    png_init_io(png, file);

    //read info
    png_read_info(png, info);
    auto width = png_get_image_width(png, info);
    auto height = png_get_image_height(png, info);
    auto color_type = png_get_color_type(png, info);
    auto bit_depth = png_get_bit_depth(png, info);
    int stride;
    Bitmap::Config config;
    if (color_type == PNG_COLOR_TYPE_RGB) {
        stride = 3;
        config = Bitmap::RGB_888;
    } else {
        stride = 4;
        config = Bitmap::RGBA_8888;
    }

    //read
    png_read_update_info(png, info);
    auto row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = new png_byte[png_get_rowbytes(png, info) + stride];
    }
    png_read_image(png, row_pointers);
    auto data = new UByte[width * height * stride];
    for (auto j = 0; j < height; ++j) {
        auto row = row_pointers[j];
        auto offset = j * width * stride;
        for (auto i = 0; i < width; ++i) {
            auto px = &(row[i * stride]);
            auto pixel = (Pixel *) px;
            SetPixel(data + offset + i * stride, *pixel, config);
        }
    }

    //cleanup
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;
    fclose(file);
    png_destroy_read_struct(&png, &info, nullptr);
    auto bitmap = new Bitmap(width, height, config);
    bitmap->data = data;

    return bitmap;
}

} //EvoImg