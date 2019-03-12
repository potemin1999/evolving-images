/**
 * Created by ilya on 3/12/19.
 */

#include <execinfo.h>
#include <cxxabi.h>
#include <cstring>
#include "Exception.h"

Exception::Exception(const char *exceptionName, const char *msg) :
        std::exception() {
    this->name = exceptionName;
    this->message = msg;
    this->stack = new StackTrace();
}

Exception::~Exception() {
    delete stack;
}

void Exception::printStackTrace() {
    printf("%s : %s \n", this->name, this->message);
    for (int i = 0; i < this->stack->getSize(); i++) {
        StackFrameRecord &record = this->stack->getData()[i];
        printf("    at %s\n", record.function);
    }
}

StackTrace::StackTrace() {
    void *addressList[1024];
    auto addressLength = backtrace(addressList, sizeof(addressList) / sizeof(void *));
    if (addressLength == 0) {
        return;
    }

    char **symbolList = backtrace_symbols(addressList, addressLength);
    size_t functionNameSize = 256;
    this->size = addressLength - 1;
    char *functionNameBuffer = new char[functionNameSize];
    auto stack = new StackFrameRecord[addressLength - 1];

    for (int i = 1; i < addressLength; i++) {
        char *begin_name = nullptr;
        char *begin_offset = nullptr;
        char *end_offset = nullptr;
        for (char *p = symbolList[i]; *p; ++p) {
            if (*p == '(') begin_name = p;
            else if (*p == '+') begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }
        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';
            int status;
            auto ret = abi::__cxa_demangle(begin_name, functionNameBuffer, &functionNameSize, &status);
            auto functionName = (status == 0) ? ret : begin_name;
            auto funcNameLen = strlen(functionName);
            stack[i - 1].function = new char[funcNameLen + 1];
            strcpy(stack[i - 1].function, functionName);
        } else {
            printf("  %s\n", symbolList[i]);
            auto functionName = symbolList[i];
            auto funcNameLen = strlen(functionName);
            stack[i - 1].function = new char[funcNameLen + 1];
            strcpy(stack[i - 1].function, functionName);
        }
    }
    delete[] functionNameBuffer;
    free(symbolList);
    this->data = stack;
}

StackTrace::~StackTrace() {
    delete[] this->data;
}
