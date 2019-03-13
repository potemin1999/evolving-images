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

typedef struct {
    unsigned long int offset;
    unsigned long int address;
    unsigned long int line;
    char *file;
    char *source;
    char *function;
} StackFrameRecord;

class StackTrace {
private:
    friend class Exception;
    Int32 size;
    std::vector<StackFrameRecord *> *data;

public:
    explicit StackTrace();

    ~ StackTrace();

    inline Int32 getSize() { return size; }

    inline std::vector<StackFrameRecord *> *getData() { return data; }
};

#ifdef ENABLE_STACK_TRACING
#define DO_REMOVE_EXCEPTION_FRAMES()             \
    auto data = this->stack->data;               \
    data->erase(data->begin(), data->begin()+2); \
    this->stack->size -= 2;
#else
#define DO_REMOVE_EXCEPTION_FRAMES()
#endif

#define DO_INIT_EXCEPTION(_exceptionName, _msg, _cause)  \
    this->name = _exceptionName;                         \
    this->message = _msg;                                \
    this->cause = _cause;                                \
    this->stack = new StackTrace();                      \
    DO_REMOVE_EXCEPTION_FRAMES()


Exception::Exception(const char *exceptionName, const char *msg, Exception *cause) :
        std::exception() {
    DO_INIT_EXCEPTION(exceptionName,msg,cause)
}

Exception::Exception(const char *exceptionName, const char *msg) :
        std::exception() {
    DO_INIT_EXCEPTION(exceptionName,msg,nullptr)
}

Exception::~Exception() {
    delete stack;
}

void Exception::printStackTrace() {
    std::printf("thrown %s : %s \n", this->name, this->message);
#ifdef ENABLE_STACK_TRACING
    for (unsigned long i = 0; i < this->stack->getSize(); i++) {
        StackFrameRecord *record = this->stack->getData()->at(i);
        std::printf("\t\tat %s : 0x%lx+%lx\n", record->function, record->address, record->offset);
    }
#endif
    if (cause != nullptr) {
        printf(" caused by ");
        cause->printStackTrace();
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
        delete[] i->function;
        delete i;
    }
    delete this->data;
}

#define GEN_SIG_HANDLER_CB(funcName, exClass, sigName, _callback) \
void funcName(int signalNumber) {                   \
    int exitStatus;                                 \
    SignalCallbackFunc callbackFunc = _callback;    \
    auto exceptionMessage = "Unhandled "            \
                             sigName                \
                            " signal received";     \
    auto exception = exClass(exceptionMessage);     \
    exception.printStackTrace();                    \
    auto stackTracePtr = ((StackTrace*)&exception); \
    if (callbackFunc != nullptr)                    \
        exitStatus = (*callbackFunc)                \
                (signalNumber,stackTracePtr);       \
    if (exitStatus == 0)                            \
        exitStatus = signalNumber;                  \
    std::signal(signalNumber,SIG_DFL);              \
    std::exit(signalNumber);                        \
}

#define GEN_SIG_HANDLER(funcName, exClass, sigName) \
    GEN_SIG_HANDLER_CB(funcName, exClass, sigName, nullptr)

typedef int (*SignalCallbackFunc)(int signal, StackTrace *stackTrace);

GEN_SIG_HANDLER(OnArithmeticErrorSignal, ArithmeticException, "arithmetic error")

GEN_SIG_HANDLER(OnIllegalInstructionSignal, IllegalInstructionException, "illegal instruction")

GEN_SIG_HANDLER(OnInterruptSignal, InterruptedException, "interrupt")

GEN_SIG_HANDLER(OnSegfaultSignal, SegfaultException, "segfault")

GEN_SIG_HANDLER(OnTerminationSignal, InterruptedException, "termination")

void __static_DoTerminate(){
    auto causePtr = std::current_exception();
    auto cause = *((Exception**) &causePtr);
    InterruptedException("terminating",cause).printStackTrace();
    std::exit(SIGTERM);
}

//it is automatically called before main
//used to init exceptions handlers
__attribute__ ((unused))
__attribute__ ((constructor))
void __static_ExceptionInit() {
    std::signal(SIGABRT, OnInterruptSignal);
    std::signal(SIGFPE, OnArithmeticErrorSignal);
    std::signal(SIGILL, OnIllegalInstructionSignal);
    std::signal(SIGSEGV, OnSegfaultSignal);
    std::signal(SIGINT, OnInterruptSignal);
    std::set_terminate(__static_DoTerminate);
}