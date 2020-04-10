/* core_interpreter.h */

#ifndef CORE_INTERPRETER_H
#define CORE_INTERPRETER_H

#include "core_data_structures.h"
#include "midge_app.h"

class CoreInterpreter
{
    struct Code
    {
        const char *text;
        int pos;
        int length;
    } code;

    void parsePast(char c);
    void parsePast(std::string s);
    char peekChar();
    char parseChar();
    std::string parseIdentifier();
    std::string parseStatement();

    void parseRoot();
    void parseClass();
    int parseClassMember(ClassDefinition *obj);
    int parseClassMethod(ClassDefinition *obj);
    int parseMethodDetails(Method *func);

    MidgeApp *app;

public:
    MidgeApp *interpret(const char *text);
};

#endif // CORE_INTERPRETER_H