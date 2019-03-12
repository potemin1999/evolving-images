/**
 * Created by ilya on 3/12/19.
 */

#ifndef EVOLVING_IMAGES_EXCEPTION_H
#define EVOLVING_IMAGES_EXCEPTION_H

#include "Types.h"

typedef struct {
    char *function;
} StackFrameRecord;

class StackTrace {
private:
    Int32 size;
    StackFrameRecord *data;

public:
    explicit StackTrace();

    ~ StackTrace();

    inline Int32 getSize() { return size; }

    inline StackFrameRecord *getData() { return data; }
};

class Exception : private std::exception {
protected:

    StackTrace *stack;

    const char *name;

    const char *message;

    explicit Exception(const char *exceptionName, const char *msg);

public:
    ~Exception() override;

    inline const char *getName() { return name; }

    inline const char *what() { return message; };

    inline const char *getMessage() { return message; }

    void printStackTrace();
};

#define GENERATE_EXCEPTION(name) \
    class name : public Exception { \
    public: \
    explicit name(const char *msg) : Exception(#name,msg) {} \
    };

GENERATE_EXCEPTION(NullPointerException)

GENERATE_EXCEPTION(IllegalArgumentException)

GENERATE_EXCEPTION(IllegalStateException)

#endif //EVOLVING_IMAGES_EXCEPTION_H
