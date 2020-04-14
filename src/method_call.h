/* method_call.h */

#ifndef METHOD_CALL_H
#define METHOD_CALL_H

#include <iostream>
#include <string>

#include "core_data.h"

class MidgeApp;

enum VariableScope
{
    _NULL,
    BLOCK,
    METHODCALL,
    METHODSTATIC,
    CLASSINSTANCE,
    CLASSSTATIC,
    GLOBAL,
};

class MethodCall
{
    friend class MethodCallStack;

private:
    DataValue *returnValue;
    std::map<std::string, DataValue *> *global;
    DataManager *dataManager;
    std::vector<std::map<std::string, DataValue *>> local;
    std::vector<std::vector<DataValue *>> localTemp;
    int localUsage;

protected:
    void clear();

public:
    MethodInfo *method;
    InstancedClass *instance;
    int statementProcessingIndex;

    void pushLocalMemoryBlock();
    void popLocalMemoryBlock();

    DataValue *getValue(std::string identifier);
    DataValue **getPointerToValue(std::string identifier);
    void instanceValue(std::string identifier, DataValue *dp, VariableScope scope);
    void assignValue(std::string identifier, DataValue *dp);

    void addBlockMemory(DataValue *value);

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

#endif // METHOD_CALL_H