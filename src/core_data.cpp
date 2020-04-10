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
        return true;
    case DataType::Pointer:
    case DataType::Void:
    default:
        return true;
    }
}

DataType Type::parseKind(std::string str)
{
    switch (str[0])
    {
    case 'i':
        return DataType::Int32;
    case 'v':
        return DataType::Void;
    default:
        return DataType::Unknown;
    }
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
        return "ClassDefinition";
    case DataType::Pointer:
        return "Pointer";
    case DataType::Int32:
        return "Int32";
    default:
        return "unknown";
    }
}

DataValue *DataManager::createData(DataType pType, void *pData)
{
    throw 9997;
}

DataValue *DataManager::cloneData(DataValue *data)
{
    if (Type::isPrimitive(data->_type))
    {
        switch (data->_type)
        {
        default:
            throw 9998;
        }
    }
    else
    {
        switch (data->_type)
        {
        default:
            throw 9999;
        }
    }
}

void DataManager::deleteData(DataValue *data)
{
    throw 9997;
}