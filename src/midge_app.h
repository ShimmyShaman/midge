/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <iostream>
#include <string>

#include "core_data.h"
#include "method_call.h"

class MidgeApp
{
private:
    MethodCallStack *callStack;

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