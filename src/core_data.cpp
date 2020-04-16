/* core_data.cpp */

#include "core_data.h"

bool Type::isPrimitive(DataType kind)
{
  switch (kind)
  {
  case DataType::Array:
  case DataType::Class:
  case DataType::Unknown:
    return false;
  case DataType::Int32:
  case DataType::String:
  case DataType::Pointer:
  case DataType::Void:
    return true;
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
  else if (str == "array")
    return DataType::Array;
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
  case DataType::Array:
    return "Array";
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

std::string Type::toString()
{
  return Type::toString(kind) + (name.length() > 0 ? name : "");
}

DataValue *DataManager::createData(Type pType, void *pData)
{
  DataValue *dv = new DataValue(pType, pData);
  ++createdDataValues;
  return dv;
}

DataValue *DataManager::createClass(ClassDefinition *definition)
{
  InstancedClass *obj = new InstancedClass();
  obj->definition = definition;
  for (int i = 0; i < definition->attributes.size(); ++i)
  {
    DataType attrType = definition->attributes[i]->type.kind;
    void *attrData = nullptr;
    switch (definition->attributes[i]->type.kind)
    {
    case DataType::Class:
    {
      std::map<std::string, ClassDefinition *>::iterator it = classDefinitions->find(definition->attributes[i]->type.name);
      if (it == classDefinitions->end())
      {
        std::cout << "DataManager::createClass() Could not find attribute class:" << definition->attributes[i]->type.name << std::endl;
        throw 1990;
      }

      attrData = createClass(it->second);
    }
    break;
    case DataType::Int32:
    {
      int *value = new int();
      attrData = static_cast<void *>(value);
    }
    break;
    case DataType::String:
    {
      std::string *value = new std::string("");
      attrData = static_cast<void *>(value);
    }
    break;
    case DataType::Array:
    {
      DataValueArray *ary = new DataValueArray();
      attrData = static_cast<void *>(ary);
    }
    break;
    default:
      std::cout << "DataManager::createClass() UnexpectedAttributeType:" << Type::toString(attrType) << std::endl;
      throw 1991;
      break;
    }
    obj->attributes[definition->attributes[i]->name] = createData(definition->attributes[i]->type.kind,
                                                                  attrData);
  }
  return createData(DataType::Class, static_cast<void *>(obj));
}

DataValue *DataManager::cloneData(DataValue *dv)
{
  return cloneData(dv->dataType(), dv->data());
}

DataValue *DataManager::cloneData(Type dataType, void *data)
{
  switch (dataType.kind)
  {
  case DataType::String:
  {
    std::string *arg = static_cast<std::string *>(data);
    void *var = static_cast<void *>(new std::string(*arg));
    DataValue *dv = createData(dataType, var);
    return dv;
  }
  case DataType::Int32:
  {
    int *arg = static_cast<int *>(data);
    void *var = static_cast<void *>(new int(*arg));
    DataValue *dv = createData(dataType, var);
    return dv;
  }
  case DataType::Int64:
  {
    long *arg = static_cast<long *>(data);
    void *var = static_cast<void *>(new long(*arg));
    DataValue *dv = createData(dataType, var);
    return dv;
  }
  default:
    std::cout << "Unhandled cloneData Type:" << Type::toString(dataType.kind);
    if (dataType.kind == DataType::Class)
      std::cout << ":" << dataType.name;
    std::cout << std::endl;
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