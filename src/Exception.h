/**
 * Created by ilya on 3/12/19.
 */

#ifndef EVOLVING_IMAGES_EXCEPTION_H
#define EVOLVING_IMAGES_EXCEPTION_H

#include "Types.h"

class StackTrace;

class Exception : private std::exception {
protected:

    StackTrace *stack;

    Exception *cause;

    const char *name;

    const char *message;

    explicit Exception(const char *exceptionName, const char *msg, Exception *cause);

    explicit Exception(const char *exceptionName, const char *msg);

public:

    ~Exception() override;

    inline const char *getName() { return name; }

    inline const char *what() { return message; };

    inline const char *getMessage() { return message; }

    void printStackTrace();
};

#define GENERATE_EXCEPTION(name)                        \
    class name : public Exception {                     \
    public:                                             \
    explicit name(const char *msg, Exception *cause) :  \
        Exception(#name,msg,cause) {}                   \
    explicit inline name(const char *msg) :             \
        Exception(#name,msg) {}                         \
    };

GENERATE_EXCEPTION(ArithmeticException)

GENERATE_EXCEPTION(IllegalArgumentException)

GENERATE_EXCEPTION(IllegalInstructionException)

GENERATE_EXCEPTION(IllegalStateException)

GENERATE_EXCEPTION(InterruptedException)

GENERATE_EXCEPTION(NullPointerException)

GENERATE_EXCEPTION(SegfaultException)

#endif //EVOLVING_IMAGES_EXCEPTION_H
