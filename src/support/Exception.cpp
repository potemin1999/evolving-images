/**
 * Created by ilya on 3/12/19.
 */

#define ENABLE_STACK_TRACING

#ifdef ENABLE_STACK_TRACING
#define UNW_LOCAL_ONLY 1

#include <cxxabi.h>
#include <cstring>
#include <libunwind.h>

#endif

#include <csignal>
#include <vector>
#include "Exception.h"

JAVALIB_NAMESPACE_BEGIN

class StackFrameRecord {
public:

    unsigned long int offset;
    unsigned long int address;
    unsigned long int line;
    char *file;
    char *source;
    char *function;

    ~StackFrameRecord() {
        delete[] file;
        delete[] source;
        delete[] function;
    }
};

class StackTrace {
private:
    friend class Exception;

    Int32 size;

    std::vector<StackFrameRecord *> *data;

public:
    explicit StackTrace();

    ~StackTrace();

    inline Int32 getSize() { return size; }

    inline std::vector<StackFrameRecord *> *getData() { return data; }

    StackFrameRecord *popTop() {
        if (this->size == 0) return nullptr;
        auto ret = data->at(0);
        data->erase(data->begin(), data->begin() + 1);
        this->size--;
        return ret;
    }
};

#define DO_INIT_EXCEPTION(_exceptionName, _msg, _cause)  \
    this->name = _exceptionName;                         \
    this->message = _msg;                                \
    this->cause = _cause;                                \
    this->stack = new StackTrace();                      \
    delete stack->popTop();                              \
    delete stack->popTop();


Exception::Exception(const char *exceptionName, const char *msg, Exception *cause) :
        std::exception() {
    DO_INIT_EXCEPTION(exceptionName, msg, cause)
}

Exception::Exception(const char *exceptionName, const char *msg) :
        std::exception() {
    DO_INIT_EXCEPTION(exceptionName, msg, nullptr)
}

Exception::~Exception() {
    delete stack;
}

void Exception::printStackTrace(std::ostream &out) {
    out << "thrown " << this->name << " : " << this->message << std::endl;
    for (unsigned long i = 0; i < this->stack->getSize(); i++) {
        StackFrameRecord *record = this->stack->getData()->at(i);
        out << "\t\tat " << record->function << " : 0x" << record->address << "+" << record->offset << std::endl;
    }
    if (cause != nullptr) {
        out << " caused by ";
        cause->printStackTrace(out);
    }
}

#ifdef UNW_LOCAL_ONLY //equal to ENABLE_STACK_TRACING

StackFrameRecord *DoParseUnwindCursor(unw_cursor_t *cursor) {
    auto record = new StackFrameRecord();
    unw_word_t offset, pc;
    unw_get_reg(cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
        return nullptr;
    }
    char sym[256];
    record->address = pc;
    record->source = nullptr;
    record->line = 0;
    unw_proc_info_t info;
    unw_get_proc_info(cursor, &info);
    if (unw_get_proc_name(cursor, sym, sizeof(sym), &offset) == 0) {
        auto namePtr = sym;
        int status;
        auto demangledName = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
        if (status == 0) {
            namePtr = demangledName;
        }
        record->offset = offset;
        auto funcNameLen = strlen(namePtr);
        record->function = new char[funcNameLen + 1];
        std::strcpy(record->function, namePtr);
        std::free(demangledName);
    } else {
        record->offset = 0;
        auto error = "?unknown_function?";
        auto errorLen = strlen(error);
        record->function = new char[errorLen + 1];
        std::strcpy(record->function, error);
    }
    return record;
}

#endif

StackTrace::StackTrace() {
    data = new std::vector<StackFrameRecord *>();
#ifdef ENABLE_STACK_TRACING
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    while (unw_step(&cursor) > 0) {
        auto record = DoParseUnwindCursor(&cursor);
        if (record == nullptr)
            break;
        data->push_back(record);
    }
#endif
    this->size = data->size();
}

StackTrace::~StackTrace() {
    for (auto &i : *this->data) {
        delete i;
    }
    delete this->data;
}

typedef int (*SignalCallbackFunc)(int signal, StackTrace *stackTrace);

typedef Exception *(*ExceptionInstantiatorFunc)(const char *msg);

typedef struct {
    const char *signalName;
    SignalCallbackFunc signalCallbackFunc;
    ExceptionInstantiatorFunc exceptionInstantiatorFunc;
} SignalHandler;

SignalHandler signalHandlers[32];

void __ExceptImpl::processSignal(int signalNumber) {
    SignalHandler *handler = &signalHandlers[signalNumber];
    if (handler == nullptr)
        std::exit(signalNumber);
    int exitStatus = 0;
    SignalCallbackFunc callbackFunc = handler->signalCallbackFunc;
    auto signalNameLength = std::strlen(handler->signalName);
    auto buffer = new char[30 + signalNameLength];
    sprintf(buffer, "Unhandled %s signal received", handler->signalName);
    //const auto genericExceptionSize = sizeof(NullPointerException);
    auto exception = (handler->exceptionInstantiatorFunc)(buffer);
    //pop current function and killpg
    delete exception->stack->popTop();
    delete exception->stack->popTop();
    exception->printStackTrace();
    if (callbackFunc != nullptr)
        exitStatus = (*callbackFunc)(signalNumber, exception->stack);
    if (exitStatus == 0)
        exitStatus = signalNumber;
    delete exception;
    std::signal(signalNumber, SIG_DFL);
    std::exit(exitStatus);
}

void __ExceptImpl::doTerminate() {
    auto causePtr = std::current_exception();
    auto cause = *((Exception **) &causePtr);
    auto exception = InterruptedException("terminating", cause);
    //pop doTerminate and rethrow_exception
    delete exception.stack->popTop();
    delete exception.stack->popTop();
    exception.printStackTrace();
    std::exit(SIGTERM);
}

void *__ExceptImpl::allocateException(size_t size) {
    return abi::__cxa_allocate_exception(size);
}

void __ExceptImpl::freeException(void *ptr) {
    abi::__cxa_free_exception(ptr);
}

void __ExceptImpl::popExceptionStack(Exception *exception) {
    delete exception->stack->popTop();
}

//it is automatically called before main
//used to init exceptions handlers
__attribute__ ((unused))
__attribute__ ((constructor))
void init() {
    bzero(signalHandlers, 32 * sizeof(SignalHandler));
    signalHandlers[SIGABRT] = SignalHandler{"interrupt", nullptr, &InterruptedException::createNew};
    signalHandlers[SIGFPE] = SignalHandler{"arithmetic error", nullptr, &ArithmeticException::createNew};
    signalHandlers[SIGILL] = SignalHandler{"illegal instruction", nullptr, &IllegalInstructionException::createNew};
    signalHandlers[SIGSEGV] = SignalHandler{"segfault", nullptr, &SegfaultException::createNew};
    signalHandlers[SIGINT] = SignalHandler{"interrupt", nullptr, &InterruptedException::createNew};
    for (int i = 0; i < 32; i++) {
        std::signal(i, __ExceptImpl::processSignal);
    }
    std::set_terminate(__ExceptImpl::doTerminate);
}

JAVALIB_NAMESPACE_END