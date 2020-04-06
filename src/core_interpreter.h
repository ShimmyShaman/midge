/* core_interpreter.h */

#ifndef CORE_INTERPRETER_H
#define CORE_INTERPRETER_H

class CoreInterpreter
{
public:
    static const int PROCESSED_COMMAND_COUNT = 0;
    static const int PROCESSED_HASH = 0;

public:
    bool update_core(const char *pCommandFilename, int pBuildNumber);
};

#endif // CORE_INTERPRETER_H