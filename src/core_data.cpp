/* core_data.cpp */

#include "core_data.h"

bool Type::isPrimitive(DataType kind)
{
    switch (kind)
    {
    case DataType::Class:
    case DataType::Unknown:
        return false;
    case DataType::Int32:
    case DataType::String:
        return true;
    case DataType::Pointer:
    case DataType::Void:
    default:
        return true;
    }
}

DataType Type::parseKind(std::string str)
{
    if (str == "int")
        return DataType::Int32;
    else if (str == "void")
        return DataType::Void;
    else if (str == "string")
        return DataType::String;
    else
        return DataType::Unknown;
}

std::string Type::toString(DataType kind)
{
    switch (kind)
    {
    case DataType::Null:
        return "Null";
    case DataType::Unknown:
        return "Unknown";
    case DataType::Void:
        return "Void";
    case DataType::Class:
        return "Class";
    case DataType::Pointer:
        return "Pointer";
    case DataType::Int32:
        return "Int32";
    case DataType::String:
        return "String";
    default:
        return "unknown";
    }
}

DataValue *DataManager::createData(DataType pType, void *pData)
{
    DataValue *dv = new DataValue(pType, pData);
    ++createdDataValues;
    return dv;
}

DataValue *DataManager::cloneData(DataValue *dv)
{
    return cloneData(dv->dataType(), dv->data());
}

DataValue *DataManager::cloneData(DataType dataType, void *data)
{
    switch (dataType)
    {
    case DataType::String:
    {
        std::string *arg = static_cast<std::string *>(data);
        void *var = static_cast<void *>(new std::string(*arg));
        DataValue *dv = createData(DataType::String, var);
        return dv;
    }
    case DataType::Int32:
    {
        int *arg = static_cast<int *>(data);
        void *var = static_cast<void *>(new int(*arg));
        DataValue *dv = createData(DataType::Int32, var);
        return dv;
    }
    case DataType::Int64:
    {
        long *arg = static_cast<long *>(data);
        void *var = static_cast<void *>(new long(*arg));
        DataValue *dv = createData(DataType::Int64, var);
        return dv;
    }
    default:
        std::cout << "Unhandled cloneData Type:" << Type::toString(dataType) << std::endl;
        throw 9998;
    }
}

void DataManager::deleteData(DataValue *data)
{
    --createdDataValues;
    delete (data);
}

DataManager::~DataManager()
{
    if (createdDataValues)
        std::cout << "Undeleted DataValues:" << createdDataValues << std::endl;
}