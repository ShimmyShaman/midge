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

DataValue *DataManager::cloneData(DataValue *data)
{
    if (Type::isPrimitive(data->_type))
    {
        switch (data->_type)
        {
        default:
            std::cout << "No case to clone primitive:" << Type::toString(data->_type) << std::endl;
            throw 9998;
        }
    }
    else
    {
        switch (data->_type)
        {
        default:
            std::cout << "No case to clone class:" << Type::toString(data->_type) << std::endl;
            throw 9999;
        }
    }
}

void DataManager::deleteData(DataValue *data)
{
    --createdDataValues;
    delete (data);
}