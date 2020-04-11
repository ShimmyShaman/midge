/* midge_app.cpp */

#include "midge_app.h"

using namespace std;

void MethodCall::pushLocalMemoryBlock()
{
    ++localUsage;
    if (localUsage >= local.size())
        local.resize(localUsage + localUsage / 3 + 4);
}

void MethodCall::popLocalMemoryBlock()
{
    --localUsage;
    std::map<std::string, DataValue *>::iterator it = local[localUsage].begin();
    while (it != local[localUsage].end())
    {
        dataManager->deleteData(it->second);
        it++;
    }
    local[localUsage].clear();
}

DataValue *MethodCall::getValue(std::string identifier)
{
    DataValue **pdp = getPointerToValue(identifier);
    if (!pdp)
        return nullptr;
    return dataManager->cloneData(*pdp);
}

DataValue **MethodCall::getPointerToValue(std::string identifier)
{
    int i = localUsage - 1;
    std::map<std::string, DataValue *>::iterator it = local[i].find(identifier);
    while (true)
    {
        if (it != local[i].end())
            return &it->second;
        --i;
        if (i < 0)
            break;
        it = local[i].find(identifier);
    }

    if (instance)
    {
        it = instance->attributes.find(identifier);
        if (it != instance->attributes.end())
            return &it->second;
    }

    it = global->find(identifier);
    if (it == global->end())
        return nullptr;
    return &it->second;
}

void MethodCall::instanceValue(std::string identifier, DataValue *dp)
{
    DataValue **existing = getPointerToValue(identifier);
    if (existing)
        throw 3947;

    local[localUsage - 1][identifier] = dp;
}

void MethodCall::assignValue(std::string identifier, DataValue *dp)
{
    // Find the key
    int i = localUsage - 1;
    std::map<std::string, DataValue *>::iterator it = local[i].find(identifier);
    while (true)
    {
        if (it != local[i].end())
        {
            it->second = dp;
            return;
        }
        --i;
        if (i < 0)
            break;
        it = local[i].find(identifier);
    }

    if (instance)
    {
        it = instance->attributes.find(identifier);
        if (it != instance->attributes.end())
        {
            it->second = dp;
            return;
        }
    }

    it = global->find(identifier);
    if (it != global->end())
    {
        it->second = dp;
        return;
    }

    throw 3941;
}

DataValue *MethodCall::takeReturnValue()
{
    DataValue *ret = returnValue;
    returnValue = nullptr;
    return ret;
}

void MethodCall::setReturnValue(DataValue *value)
{
    returnValue = value;
}

void MethodCall::clear()
{
    if (returnValue)
    {
        dataManager->deleteData(returnValue);
        returnValue = nullptr;
    }

    method = nullptr;

    while (localUsage > 0)
        popLocalMemoryBlock();
    localUsage = 1;
}

MethodCall::MethodCall()
{
    // Permanent Parameters Slot
    pushLocalMemoryBlock();
}

MethodCall *MethodCallStack::increment(MethodInfo *method, InstancedClass *instance)
{
    if (stackUsage + 1 >= stackCapacity)
        throw 9989;

    MethodCall *methodCall = &stack[stackUsage];
    methodCall->method = method;
    methodCall->instance = instance;

    ++stackUsage;
    return methodCall;
}

void MethodCallStack::decrement(MethodCall **finishedMethod)
{
    (*finishedMethod)->clear();

    finishedMethod = nullptr;
}

MethodCallStack::MethodCallStack(MidgeApp *midgeApp)
{
    stackCapacity = 60;
    stack.resize(stackCapacity);
    stackUsage = 0;

    for (int i = 0; i < stackCapacity; ++i)
    {
        stack[i].global = &midgeApp->globalMemory;
        stack[i].dataManager = &midgeApp->dataManager;
    }
}

void MidgeApp::processMethod(MethodCall *methodCall)
{
    cout << "MethodCall:" << methodCall->method->name << endl;

    processStatementBlock(methodCall, 0);
}

void MidgeApp::processStatementBlock(MethodCall *methodCall, int nextStatementIndex)
{
    if (nextStatementIndex >= methodCall->method->statements.size())
        throw 845;
    if (methodCall->method->statements[nextStatementIndex] != "{")
        throw 846;
    ++nextStatementIndex;

    methodCall->pushLocalMemoryBlock();
    for (; nextStatementIndex < methodCall->method->statements.size(); ++nextStatementIndex)
        switch (methodCall->method->statements[nextStatementIndex][0])
        {
        case '}':
            methodCall->popLocalMemoryBlock();
            return;
        default:
            processStatement(methodCall, nextStatementIndex);
            break;
        }
    throw 847;
}

void MidgeApp::processStatement(MethodCall *methodCall, int nextStatementIndex)
{
    string &statement = methodCall->method->statements[nextStatementIndex];
    switch (statement[0])
    {
    case '{':
        processStatementBlock(methodCall, nextStatementIndex);
        break;
    case 'i':
    {
        if (statement[1] != 'n')
            throw 1201;
        switch (statement[2])
        {
        case 's':
        {
            cout << "here1" << endl;
            // instance
            int ix = statement.find('(', 0) + 1;
            int iy = statement.find(',');
            string dataTypeStr = statement.substr(ix, iy - ix);
            string instanceName = statement.substr(iy + 1, statement.size() - 1 - iy - 1);

            cout << "here2" << endl;
            DataValue *dp = nullptr;
            DataType dataType = Type::parseKind(dataTypeStr);
            if (dataType == DataType::Unknown)
            {
                // Assume class-name specified
                dataType = DataType::Class;
            }
            if (!Type::isPrimitive(dataType))
            {
                map<string, ClassDefinition *>::iterator it = classDefinitions.find(dataTypeStr);
                if (it == classDefinitions.end())
                {
                    cout << "processStatement() NonPrimitiveUnspecifiedType:" << dataTypeStr << endl;
                    throw 1202;
                }

                cout << "here3" << endl;
                ClassDefinition *definition = it->second;
                InstancedClass *obj = new InstancedClass();
                obj->definition = definition;
                for (int i = 0; i < definition->attributes.size(); ++i)
                {
                    DataType attrType = definition->attributes[i]->kind;
                    void *attrData = nullptr;
                    switch (definition->attributes[i]->kind)
                    {
                    case DataType::Class:
                    {
                        attrData = static_cast<void *>(nullptr);
                    }
                    break;
                    case DataType::Int32:
                    {
                        int *value = new int();
                        attrData = static_cast<void *>(value);
                    }
                    break;
                    default:
                        cout << "processStatement() UnexpectedAttributeType:" << Type::toString(attrType) << endl;
                        throw 1203;
                        break;
                    }
                    obj->attributes[definition->attributes[i]->name] = dataManager.createData(definition->attributes[i]->kind,
                                                                                              attrData);
                }
                cout << "here4" << endl;
                dp = dataManager.createData(dataType, static_cast<void *>(obj));
            }
            else
            {
                switch (dataType)
                {
                case DataType::Int32:
                {
                    int *value = new int();
                    dp = dataManager.createData(dataType, static_cast<void *>(value));
                }
                break;
                default:
                    cout << "processStatement() Unexpected Primitive Type:" << Type::toString(dp->type()) << endl;
                    throw 1204;
                    break;
                }
            }
            cout << "here5" << endl;
            methodCall->instanceValue(instanceName, dp);
        }
        break;
        /*case 'd':
    {
      // indent
    }
    break;*/
        /*case 'i':
    {
      // init
    }
    break;*/
        case 'v':
        {
            // invoke
            bool argsRemain = true;
            int ix = statement.find('(', 0) + 1;
            int iy = statement.find(',');
            if (iy == string::npos)
            {
                argsRemain = false;
                iy = statement.find(')', ix);
            }
            string memberName = statement.substr(ix, iy - ix);

            InstancedClass *instance = nullptr;
            if (argsRemain)
            {
                ix = iy + 1;
                iy = statement.find(',', ix);
                if (iy == string::npos)
                {
                    argsRemain = false;
                    iy = statement.find(')', ix);
                }
                string instanceName = statement.substr(ix, iy - ix);

                //cout << "memberName:" << memberName << " instanceName:" << instanceName << endl;

                if (instanceName.length() == 0)
                    throw 1241; // TODO use indents

                DataValue **pInstance = methodCall->getPointerToValue(instanceName);
                if (!pInstance)
                    throw 1242;
                instance = static_cast<InstancedClass *>((*pInstance)->data());
            }
            else
                throw 1243;

            if (!instance)
                throw 1244;

            MethodInfo *method = nullptr;
            for (int i = 0; i < instance->definition->methods.size(); ++i)
                if (instance->definition->methods[i]->name == memberName)
                {
                    method = instance->definition->methods[i];
                    break;
                }
            if (!method)
                throw 1245;

            map<string, DataValue *> parameters;
            while (argsRemain)
            {
                ix = iy + 1;
                iy = statement.find(',', ix);
                if (iy == string::npos)
                {
                    argsRemain = false;
                    iy = statement.find(')', ix);
                }

                string parameterString = iy < 0 ? "" : statement.substr(ix, iy - ix);

                throw 1246; // TODO
            }

            // Invoke
            MethodCall *subcall = callStack->increment(method, instance);
            processMethod(subcall);
            callStack->decrement(&subcall);

            // TODO -- indent?
        }
        break;
        default:
        {
            cout << "processStatement() UnexpectedStatement beginsWithI:" << statement << endl;
            throw 1296;
        }
        break;
        }
    }
    break;
    case 'p':
    {
        // print
        bool argsRemain = true;
        int ix = statement.find('(', 0) + 1;
        int iy = statement.find(',');
        if (iy == string::npos)
        {
            argsRemain = false;
            iy = statement.find(')', ix);
        }
        string text = statement.substr(ix, iy - ix);

        if (text[0] != '"')
            throw 1321;
        else
        {
            if (text[text.length() - 1] != '"')
                throw 1322;
            text = text.substr(1, text.length() - 2);
        }

        map<string, DataValue *> parameters;
        while (argsRemain)
        {
            ix = iy + 1;
            iy = statement.find(',', ix);
            if (iy == string::npos)
            {
                argsRemain = false;
                iy = statement.find(')', ix);
            }

            string parameterString = iy < 0 ? "" : statement.substr(ix, iy - ix);

            throw 1323; // TODO
        }

        cout << text;
    }
    break;
    default:
    {
        cout << "processStatement() UnexpectedStatement:" << statement << endl;
        throw 1302;
    }
    break;
    }
}

int MidgeApp::run()
{
    try
    {
        MethodCall *methodCall = callStack->increment(entryMethod, nullptr);
        processMethod(methodCall);
        DataValue *result = methodCall->takeReturnValue();
        callStack->decrement(&methodCall);
    }
    catch (int e)
    {
        std::cout << "Exception:" << e << std::endl;
        return e;
    }
    return 0;
}

MidgeApp::MidgeApp()
{
    callStack = new MethodCallStack(this);
}

MidgeApp::~MidgeApp()
{
    if (callStack)
        delete callStack;
}