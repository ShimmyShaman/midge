/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <iostream>
#include <string>

#include "core_data_structures.h"

class LocalMemory
{
public:
    LocalMemory *outer;
    std::map<std::string, DataPoint *> current;

    DataPoint *get(std::string identifier);

    LocalMemory() : outer(nullptr) {}
};

class MethodMemory
{
public:
    std::map<std::string, DataPoint *> *global;
    InstancedClass *instance;
    LocalMemory local;

    InstancedClass *getInstance(std::string identifier);
    DataPoint *getValue(std::string identifier);
    MethodMemory() {}
    MethodMemory(std::map<std::string, DataPoint *> *pGlobal,
                 InstancedClass *pInstance,
                 std::map<std::string, DataPoint *> *pLocal)
        : global(pGlobal), instance(pInstance), local(pLocal) {}
};

class MidgeApp
{
private:
    std::map<std::string, DataPoint *> global_memory;

    DataPoint *processMethod(Method *method, InstancedClass *instance,
                             LocalMemory *local);
    void processStatement(MethodMemory *memory, std::string &statement);

public:
    Method *entryMethod;
    std::map<std::string, ClassDefinition *> classDefinitions;

    int run();
};

#endif // MIDGE_APP_H