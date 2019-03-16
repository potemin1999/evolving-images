/**
 * Created by ilya on 3/12/19.
 */

#ifndef EVOLVING_IMAGES_TYPES_H
#define EVOLVING_IMAGES_TYPES_H

#define JAVALIB_NAMESPACE_BEGIN namespace jl{
#define JAVALIB_NAMESPACE_END }

#include <fstream>

JAVALIB_NAMESPACE_BEGIN
typedef char Byte;
typedef unsigned char UByte;

typedef bool Bool;

typedef signed short int Int16;
typedef unsigned short int UInt16;
typedef signed long int Int32;
typedef unsigned long int UInt32;

typedef std::istream InputStream;
typedef std::ostream OutputStream;
typedef std::ifstream FileInputStream;
typedef std::ofstream FileOutputStream;

JAVALIB_NAMESPACE_END

#endif //EVOLVING_IMAGES_TYPES_H
