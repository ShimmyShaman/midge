/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <iostream>
#include <string>

#include "core_data.h"

class MidgeApp;

class MethodCall
{
    friend class MethodCallStack;

private:
    DataValue *returnValue;
    std::map<std::string, DataValue *> *global;
    DataManager *dataManager;
    InstancedClass *instance;
    std::vector<std::map<std::string, DataValue *>> local;
    int localUsage;

protected:
    void clear();

public:
    MethodInfo *method;

    void pushLocalMemoryBlock();
    void popLocalMemoryBlock();

    DataValue *getValue(std::string identifier);
    DataValue **getPointerToValue(std::string identifier);
    void instanceValue(std::string identifier, DataValue *dp);
    void assignValue(std::string identifier, DataValue *dp);

    DataValue *takeReturnValue();
    void setReturnValue(DataValue *value);

    MethodCall();
};

class MethodCallStack
{
private:
    std::vector<MethodCall> stack;
    int stackCapacity, stackUsage;

public:
    MethodCall *increment(MethodInfo *method, InstancedClass *instance);
    void decrement(MethodCall **finishedMethod);

    MethodCallStack(MidgeApp *midgeApp);
};

class MidgeApp
{
private:
    MethodCallStack *callStack;

    void processMethod(MethodCall *methodCall);
    void processStatementBlock(MethodCall *methodCall, int nextStatementIndex);
    void processStatement(MethodCall *memory, int nextStatementIndex);

public:
    DataManager dataManager;
    std::map<std::string, DataValue *> globalMemory;

    MethodInfo *entryMethod;
    std::map<std::string, ClassDefinition *> classDefinitions;

    int run();
    MidgeApp();
    ~MidgeApp();
};

#endif // MIDGE_APP_H