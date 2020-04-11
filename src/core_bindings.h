/* core_bindings.h */

#ifndef CORE_BINDINGS_H
#define CORE_BINDINGS_H

#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>

typedef void *(*methodPtr)(void **, int);

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
    std::map<std::string, methodPtr> bindings;

public:
    static void addMethodBinding(std::string identity, methodPtr method)
    {
        if (getInstance().bindings.count(identity) > 0)
        {
            std::cout << "Method Binding for " << identity << " already exists!" << std::endl;
            throw 111;
        }

        getInstance().bindings[identity] = method;
    }

    static methodPtr getMethod(std::string identity)
    {
        std::map<std::string, methodPtr>::iterator it = getInstance().bindings.find(identity);
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

        std::cout << crap << std::endl;
        return nullptr;
    }

public:
    static void bindFunctions()
    {
        Bindings::addMethodBinding("printCrap", &printCrap);
    }
};

#endif // CORE_BINDINGS_H