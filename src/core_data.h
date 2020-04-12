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
    DataType _dataType;
    void *_data;

    DataValue(DataType pType, void *pData = nullptr)
        : _dataType(pType), _data(pData) {}

public:
    DataType dataType() { return _dataType; }
    void *data() { return _data; }
};

class DataManager
{
private:
    int createdDataValues;

public:
    DataValue *createData(DataType pType, void *pData);
    DataValue *cloneData(DataValue *data);
    void deleteData(DataValue *data);
    ~DataManager();
};

struct MethodInfo
{
    std::string name;
    std::vector<std::string> statements;
    DataType *returnDataType;
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

#endif // CORE_DATA_H