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
};

struct Type
{
    DataType kind;
    std::string name;

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

class DataValue
{
    friend class DataManager;

private:
    DataType _dataType;
    void *_data;

    DataValue(DataType pType, void *pData = nullptr)
        : _dataType(pType), _data(pData) {}

public:
    DataType dataType() { return _dataType; }
    void *data() { return _data; }

    int *int32()
    {
        if (_dataType != DataType::Int32)
            throw 221;
        return static_cast<int *>(_data);
    }
};

class DataManager
{
private:
    int createdDataValues;

public:
    DataValue *createData(DataType pType, void *pData);
    DataValue *cloneData(DataValue *data);
    DataValue *cloneData(DataType pType, void *pData);
    void deleteData(DataValue *data);
    DataManager() { createdDataValues = 0; }
    ~DataManager();
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
    std::vector<Type *> attributes;
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

class InstancedClass
{
public:
    ClassDefinition *definition;

    std::map<std::string, DataValue *> attributes;
};

#endif // CORE_DATA_H