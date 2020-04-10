/* core_data_structures.h */

#ifndef CORE_DATA_STRUCTURES_H
#define CORE_DATA_STRUCTURES_H

#include <map>
#include <string>
#include <vector>

struct Type
{
    enum Kind
    {
        Null,
        Unknown,
        Void,
        Class,
        Pointer,
        Int32,
    } kind;
    std::string name;

    Type(Kind pKind)
    {
        kind = pKind;
        name = "";
    }

    static bool isPrimitive(Type::Kind kind)
    {
        switch (kind)
        {
        case Class:
        case Unknown:
            return false;
        case Int32:
            return true;
        case Pointer:
        case Void:
        default:
            return true;
        }
    }

    static Type::Kind parseKind(std::string str)
    {
        switch (str[0])
        {
        case 'i':
            return Int32;
        case 'v':
            return Void;
        default:
            return Unknown;
        }
    }

    static std::string toString(Type::Kind kind)
    {
        switch (kind)
        {
        case Null:
            return "Null";
        case Unknown:
            return "Unknown";
        case Void:
            return "Void";
        case Class:
            return "ClassDefinition";
        case Pointer:
            return "Pointer";
        case Int32:
            return "Int32";
        default:
            return "unknown";
        }
    }
};

class DataPoint
{
public:
  Type::Kind type;
  void *data;
  DataPoint() : type(Type::Kind::Unknown) {}
  DataPoint(Type::Kind pType, void *pData = nullptr)
      : type(pType), data(pData) {}
};

struct Method
{
    std::string name;
    std::vector<std::string> statements;
    Type *returnType;
};

struct ClassDefinition
{
  Type type;
  std::vector<Type *> attributes;
  std::vector<Method *> methods;
  ClassDefinition() : type(Type(Type::Class)) {}
};

class InstancedClass
{
public:
  ClassDefinition *definition;

  std::map<std::string, DataPoint *> attributes;
};

#endif // CORE_DATA_STRUCTURES_H