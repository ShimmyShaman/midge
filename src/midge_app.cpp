/* midge_app.cpp */

#include "midge_app.h"

using namespace std;

DataPoint *LocalMemory::get(std::string identifier)
{
    std::map<std::string, DataPoint *>::iterator it = current.find(identifier);
    if (it == current.end())
    {
        if (outer)
            return outer->get(identifier);
        return nullptr;
    }

    return it->second;
}

InstancedClass *MethodMemory::getInstance(std::string identifier)
{
    DataPoint *dp = getValue(identifier);
    if (!dp)
        return nullptr;

    // Check
    if (dp->type != Type::Class)
    {
        std::cout << "RequestedClass:" << identifier << " Got:" << Type::toString(dp->type) << endl;
        throw 1070;
    }

    // Return
    return static_cast<InstancedClass *>(dp->data);
}

DataPoint *MethodMemory::getValue(std::string identifier)
{
    DataPoint *dp = local.get(identifier);
    if (dp)
        return dp;

    if (instance)
    {
        std::map<std::string, DataPoint *>::iterator it = instance->attributes.find(identifier);
        if (it != instance->attributes.end())
            return it->second;
    }

    std::map<std::string, DataPoint *>::iterator it = global->find(identifier);
    if (it == global->end())
        return nullptr;
    return it->second;
}

DataPoint *MidgeApp::processMethod(Method *method, InstancedClass *instance,
                                   LocalMemory *local)
{
    cout << "MethodCall:" << method->name << endl;

    // Set Functional Memory
    MethodMemory memory;
    memory.global = &global_memory;
    memory.instance = instance;
    memory.local.outer = local;

    for (int i = 0; i < method->statements.size(); ++i)
        processStatement(&memory, method->statements[i]);

    if (method->returnType->kind == Type::Void)
        return new DataPoint(Type::Void);

    return memory.local.get("return");
}

void MidgeApp::processStatement(MethodMemory *memory, string &statement)
{
    switch (statement[0])
    {
    case 'i':
    {
        if (statement[1] != 'n')
            throw 1201;
        switch (statement[2])
        {
        case 's':
        {
            // instance
            int ix = statement.find('(', 0) + 1;
            int iy = statement.find(',');
            string dataTypeStr = statement.substr(ix, iy - ix);
            string instanceName = statement.substr(iy + 1, statement.size() - 1 - iy - 1);

            DataPoint *dp = new DataPoint();
            dp->type = Type::parseKind(dataTypeStr);
            if (dp->type == Type::Unknown)
            {
                // Assume class-name specified
                dp->type = Type::Class;
            }
            if (!Type::isPrimitive(dp->type))
            {
                map<string, ClassDefinition *>::iterator it = classDefinitions.find(dataTypeStr);
                if (it == classDefinitions.end())
                {
                    cout << "processStatement() NonPrimitiveUnspecifiedType:" << dataTypeStr << endl;
                    throw 1202;
                }

                ClassDefinition *definition = it->second;
                InstancedClass *obj = new InstancedClass();
                obj->definition = definition;
                for (int i = 0; i < definition->attributes.size(); ++i)
                {
                    DataPoint *attdp = new DataPoint();
                    attdp->type = definition->attributes[i]->kind;
                    switch (attdp->type)
                    {
                    case Type::Kind::Class:
                    {
                        attdp->data = static_cast<void *>(nullptr);
                    }
                    break;
                    case Type::Kind::Int32:
                    {
                        int *value = new int();
                        attdp->data = static_cast<void *>(value);
                    }
                    break;
                    default:
                        cout << "processStatement() UnexpectedAttributeType:" << Type::toString(attdp->type) << endl;
                        throw 1203;
                        break;
                    }
                    obj->attributes[definition->attributes[i]->name] = attdp;
                }
                dp->data = static_cast<void *>(obj);
            }
            else
            {
                switch (dp->type)
                {
                case Type::Kind::Int32:
                {
                    int *value = new int();
                    dp->data = static_cast<void *>(value);
                }
                break;
                default:
                    cout << "processStatement() Unexpected Primitive Type:" << Type::toString(dp->type) << endl;
                    throw 1204;
                    break;
                }
            }

            map<string, DataPoint *>::iterator it = memory->local->find(instanceName);
            if (it != memory->local->end())
            {
                cout << "processStatement() OverwritingPreviousMemoryWithoutDeleting:" << dataTypeStr << endl;
                throw 1205;
            }
            (*memory->local)[instanceName] = dp;
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

                instance = memory->getInstance(instanceName);
            }
            else
                throw 1242;

            if (!instance)
                throw 1243;

            Method *method = nullptr;
            for (int i = 0; i < instance->definition->methods.size(); ++i)
                if (instance->definition->methods[i]->name == memberName)
                {
                    method = instance->definition->methods[i];
                    break;
                }
            if (!method)
                throw 1244;

            map<string, DataPoint *> parameters;
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

                throw 1245; // TODO
            }

            // Invoke
            DataPoint *result = processMethod(method, instance, nullptr);
            (*memory->local)["return"] = result;
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

        map<string, DataPoint *> parameters;
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

            throw 1321; // TODO
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
        DataPoint *result = processMethod(entryMethod, nullptr, nullptr);
    }
    catch (int e)
    {
        std::cout << "Exception:" << e << std::endl;
        return e;
    }
    return 0;
}