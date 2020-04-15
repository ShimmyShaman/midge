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
    DataType type;
    string name;
    bool canDefault;

    Argument(DataType pType, string pName, bool pCanDefault = false)
    {
        type = pType;
        name = pName;
        pCanDefault = canDefault;
    }
};

struct BoundMethodInfo
{
    methodPtr method;
    vector<Argument *> *argumentTypes;
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
    static void addMethodBinding(std::string identity, methodPtr method, vector<Argument *> *argumentTypes)
    {
        if (getInstance().bindings.count(identity) > 0)
        {
            std::cout << "Method Binding for " << identity << " already exists!" << std::endl;
            throw 111;
        }

        BoundMethodInfo *info = new BoundMethodInfo();
        info->method = method;
        info->argumentTypes = argumentTypes;

        getInstance().bindings[identity] = info;
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
            throw 112;

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

        throw 112;
    }

public:
    static void bindFunctions()
    {
        vector<Argument *> *args = new vector<Argument *>();
        args->push_back(new Argument(DataType::String, "message"));
        Bindings::addMethodBinding("printCrap", &printCrap, args);

        args = new vector<Argument *>();
        args->push_back(new Argument(DataType::Long, "nanoseconds", true));
        Bindings::addMethodBinding("nanosleep", &_nanosleep, args);
    }
};

#endif // CORE_BINDINGS_H