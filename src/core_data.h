/* core_data_structures.h */

#ifndef CORE_DATA_STRUCTURES_H
#define CORE_DATA_STRUCTURES_H

#include <map>
#include <string>
#include <vector>
enum DataType
{
    Null,
    Unknown,
    Void,
    Class,
    Pointer,
    Int32,
};

struct Type
{
    DataType kind;
    std::string name;

    Type(DataType pKind)
    {
        kind = pKind;
        name = "";
    }
    static bool isPrimitive(DataType kind);
    static DataType parseKind(std::string str);
    static std::string toString(DataType kind);
};

class DataValue
{
    friend class DataManager;

private:
    DataType _type;
    void *_data;

    DataValue(DataType pType, void *pData = nullptr)
        : _type(pType), _data(pData) {}

public:
    DataType type() { return _type; }
    void *data() { return _data; }
};

class DataManager
{
public:
    DataValue *createData(DataType pType, void *pData);
    DataValue *cloneData(DataValue *data);
    void deleteData(DataValue *data);
};

struct MethodInfo
{
    std::string name;
    std::vector<std::string> statements;
    Type *returnType;
};

struct ClassDefinition
{
    Type type;
    std::vector<Type *> attributes;
    std::vector<MethodInfo *> methods;
    ClassDefinition() : type(Type(DataType::Class)) {}
};

class InstancedClass
{
public:
    ClassDefinition *definition;

    std::map<std::string, DataValue *> attributes;
};

#endif // CORE_DATA_STRUCTURES_H