/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <iostream>
#include <string>

#include "core_data.h"
#include "method_call.h"

class MethodCallStack
{
private:
    std::vector<MethodCall> stack;
    int stackCapacity, stackUsage;

    DataManager *dataManager;
    std::map<std::string, DataValue *> *globalMemory;
    std::map<std::string, ClassDefinition *> *classDefinitions;

protected:
    struct Thread
    {
        pthread_t threadId;
        MethodCallStack *callStack;
    };
    static std::map<std::string, Thread *> threads;
    static void *execute(void *arg);

    void meaninglessPrint();

    void processMethod(MethodCall *methodCall);
    void processStatementBlock(MethodCall *methodCall, bool skipBlockCheck = false);
    void processStatement(MethodCall *memory);

    void getCallArgsFromStatement(std::vector<std::string> *args, std::string &statement);

    void processCall_addClassMethod(MethodCall *methodCall, std::string &statement);
    void processCall_addClassMethodCode(MethodCall *methodCall, std::string &statement);
    void processCall_bindingInvoke(MethodCall *methodCall, std::string &statement);
    void processCall_createAttribute(MethodCall *methodCall, std::string &statement);
    void processCall_createClass(MethodCall *methodCall, std::string &statement);
    void processCall_initializeDefault(MethodCall *methodCall, std::string &statement);
    void processCall_invoke(MethodCall *methodCall, std::string &statement);
    void processCall_print(MethodCall *methodCall, std::string &statement);
    void processCall_thread(MethodCall *methodCall, std::string &statement);

    MethodInfo *loadMethodFromFile(std::string filePath, std::string methodName);

public:
    MethodCall *incrementCallStack(MethodInfo *method, InstancedClass *instance);
    void decrementCallStack(MethodCall **finishedMethod);

    void initialize(DataManager *pDataManager, std::map<std::string, DataValue *> *pGlobalMemory,
                    std::map<std::string, ClassDefinition *> *pClassDefinitions);

    MethodCallStack();
};

class MidgeApp : public MethodCallStack
{
public:
    int run();

    MidgeApp();
    ~MidgeApp();
};

#endif // MIDGE_APP_H