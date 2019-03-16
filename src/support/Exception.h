/**
 * Created by ilya on 3/12/19.
 */

#ifndef EVOLVING_IMAGES_EXCEPTION_H
#define EVOLVING_IMAGES_EXCEPTION_H

#include <iostream>
#include "Types.h"

JAVALIB_NAMESPACE_BEGIN

class StackTrace;

class Exception;

class __ExceptImpl {
public:

    static void *allocateException(size_t size);

    static void freeException(void *ptr);

    static void popExceptionStack(Exception *exception);

    static void doTerminate();

    static void processSignal(int signalNumber);
};

class Exception : private std::exception {
protected:
    friend class __ExceptImpl;

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

    void printStackTrace(std::ostream &out);

    inline void printStackTrace() {
        printStackTrace(std::cout);
    }

};

#define GENERATE_EXCEPTION(name)                        \
    class name : public Exception {                     \
    public:                                             \
    __attribute__((noinline))                           \
    explicit name(const char *msg, Exception *cause) :  \
        Exception(#name,msg,cause) {}                   \
    __attribute__((noinline))                           \
    explicit inline name(const char *msg) :             \
        Exception(#name,msg) {}                         \
    inline static Exception *createNew(const char* msg){\
        auto newException = new name(msg);              \
        __ExceptImpl::popExceptionStack(newException);  \
        return newException;                            \
    }                                                   \
    void* operator new(size_t size){                    \
        return __ExceptImpl::allocateException(size);   \
    }                                                   \
    void operator delete(void* ptr){                    \
        __ExceptImpl::freeException(ptr);               \
    }                                                   \
};

GENERATE_EXCEPTION(ArithmeticException)

GENERATE_EXCEPTION(IllegalArgumentException)

GENERATE_EXCEPTION(IllegalInstructionException)

GENERATE_EXCEPTION(IllegalStateException)

GENERATE_EXCEPTION(InterruptedException)

GENERATE_EXCEPTION(NullPointerException)

GENERATE_EXCEPTION(SegfaultException)

JAVALIB_NAMESPACE_END

#endif //EVOLVING_IMAGES_EXCEPTION_H
