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
    std::vector<std::map<std::string, DataValue *>> local;
    std::vector<std::vector<DataValue *>> localTemp;
    int localUsage;

protected:
    void clear();

public:
    MethodInfo *method;
    InstancedClass *instance;

    void pushLocalMemoryBlock();
    void popLocalMemoryBlock();

    DataValue *getValue(std::string identifier);
    DataValue **getPointerToValue(std::string identifier);
    void instanceValue(std::string identifier, DataValue *dp);
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

class MidgeApp
{
private:
    MethodCallStack *callStack;

    void processMethod(MethodCall *methodCall);
    void processStatementBlock(MethodCall *methodCall, int nextStatementIndex,
                               bool skipBlockCheck = false);
    void processStatement(MethodCall *memory, int nextStatementIndex);

    void getCallArgsFromStatement(std::vector<std::string> *args, std::string &statement);

    void processCall_addClassMethod(MethodCall *methodCall, std::string &statement);
    void processCall_addClassMethodCode(MethodCall *methodCall, std::string &statement);
    void processCall_bindingInvoke(MethodCall *methodCall, std::string &statement);
    void processCall_createClass(MethodCall *methodCall, std::string &statement);
    void processCall_instance(MethodCall *methodCall, std::string &statement);
    void processCall_invoke(MethodCall *methodCall, std::string &statement);
    void processCall_print(MethodCall *methodCall, std::string &statement);

    int callMethodFromFile(std::string filePath, std::string methodName);

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