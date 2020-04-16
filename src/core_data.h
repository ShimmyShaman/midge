/* core_data.h */

#ifndef CORE_DATA_H
#define CORE_DATA_H

#include <iostream>
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
    String,
    Int64,
    Array,
};

struct Type
{
    DataType kind;
    std::string name;

    std::string toString();

    Type()
    {
        kind = DataType::Null;
        name = "";
    }
    Type(DataType pKind)
    {
        kind = pKind;
        name = "";
    }
    Type(DataType pKind, std::string pName)
    {
        kind = pKind;
        name = pName;
    }

    static bool isPrimitive(DataType kind);
    static DataType parseKind(std::string str);
    static std::string toString(DataType kind);
};

struct FieldInfo
{
    Type type;
    std::string name;

    FieldInfo(Type pType, std::string pName)
    {
        type = pType;
        name = pName;
    }
};

struct MethodInfo
{
    std::string name;
    DataType returnDataType;
    std::vector<Type *> arguments;
    std::vector<std::string> statements;
};

struct ClassDefinition
{
    Type type;
    std::vector<FieldInfo *> attributes;
    std::vector<MethodInfo *> methods;
    ClassDefinition() : type(Type(DataType::Class)) {}
    ~ClassDefinition()
    {
        // for (int i = 0; i < attributes.size(); ++i)
        //     delete (attributes[i]);
        // for (int i = 0; i < methods.size(); ++i)
        //     delete (methods[i]);
    }
};

class DataValue
{
    friend class DataManager;

private:
    Type _dataType;
    void *_data;

    DataValue(Type pType, void *pData = nullptr)
        : _dataType(pType), _data(pData) {}

public:
    Type dataType() { return _dataType; }
    void *data() { return _data; }

    int *int32()
    {
        if (_dataType.kind != DataType::Int32)
            throw 221;
        return static_cast<int *>(_data);
    }
};

class InstancedClass
{
public:
    ClassDefinition *definition;

    std::map<std::string, DataValue *> attributes;
};

class DataManager
{
private:
    int createdDataValues;
    std::map<std::string, ClassDefinition *> *classDefinitions;

public:
    DataValue *createData(Type pType, void *pData);
    DataValue *createClass(ClassDefinition *classDefinition);
    DataValue *cloneData(DataValue *data);
    DataValue *cloneData(Type pType, void *pData);
    void deleteData(DataValue *data);
    DataManager(std::map<std::string, ClassDefinition *> *pClassDefinitions)
    {
        classDefinitions = pClassDefinitions;
        createdDataValues = 0;
    }
    ~DataManager();
};

#endif // CORE_DATA_H