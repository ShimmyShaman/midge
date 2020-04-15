/* core_bindings.h */

#ifndef CORE_BINDINGS_H
#define CORE_BINDINGS_H

#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <time.h>

using namespace std;

typedef void *(*methodPtr)(void **, int);

struct Argument
{
    DataType dataType;
    string name;
    bool canDefault;

    Argument(DataType pDataType, string pName, bool pCanDefault = false)
    {
        dataType = pDataType;
        name = pName;
        pCanDefault = canDefault;
    }
};

struct BoundMethodInfo
{
    methodPtr method;
    vector<Argument *> argumentTypes;
};

struct SleepTimeSpec
{
    long seconds;
    long nanoseconds;
};

class Bindings
{
private:
    static Bindings &getInstance()
    {
        static Bindings instance;
        return instance;
    }
    Bindings() {}
    Bindings(Bindings const &);       // Don't Implement.
    void operator=(Bindings const &); // Don't implement

protected:
    std::map<std::string, BoundMethodInfo *> bindings;

public:
    static void addMethodBinding(std::string identity, BoundMethodInfo *methodInfo)
    {
        if (getInstance().bindings.count(identity) > 0)
        {
            std::cout << "Method Binding for " << identity << " already exists!" << std::endl;
            throw 111;
        }

        getInstance().bindings[identity] = methodInfo;
    }

    static BoundMethodInfo *getMethod(std::string identity)
    {
        std::map<std::string, BoundMethodInfo *>::iterator it = getInstance().bindings.find(identity);
        if (it == getInstance().bindings.end())
            return nullptr;
        return it->second;
    }
};

class CoreBindings
{
protected:
    static void *printCrap(void **args, int argCount)
    {
        if (argCount != 1)
            throw - 1;

        std::string crap = *static_cast<std::string *>(args[0]);

        std::cout << "> " << crap << std::endl;
        return nullptr;
    }

    static void *_nanosleep(void **args, int argCount)
    {
        if (argCount == 0)
        {
            nanosleep((const struct timespec[]){{0, 10000000000L}}, NULL);
            return nullptr;
        }

        timespec time;
        time.tv_sec = 0;
        if (argCount == 0)
            time.tv_nsec = 1000000L; // 1 millisecond
        else
            time.tv_nsec = *static_cast<long *>(args[0]);
        nanosleep(&time, NULL);

        throw - 1;
    }

public:
    static void bindFunctions()
    {
        BoundMethodInfo *method = new BoundMethodInfo();
        method->method = &printCrap;
        method->argumentTypes.push_back(new Argument(DataType::String, "message"));
        Bindings::addMethodBinding("printCrap", method);

        method = new BoundMethodInfo();
        method->method = &_nanosleep;
        method->argumentTypes.push_back(new Argument(DataType::Int64, "nanoseconds", true));
        Bindings::addMethodBinding("nanosleep", method);
    }
};

#endif // CORE_BINDINGS_H