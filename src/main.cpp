/* main.cpp */

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

using namespace std;

const int BUILD_NUMBER = 0;

const char *fileText =
    "[class:Midge{"
    "[method:print:void:(){"
    "print(Hello Universe\n);"
    "}]"
    "}]"
    "[entry:run:void:(){"
    "instance(Midge,midge);"
    "invoke(print,midge);"
    "}]";

struct Code
{
  const char *text;
  int pos;
  int length;
} code;

void parsePast(char c)
{
  if (code.pos >= code.length)
    throw 1001;

  if (code.text[code.pos] != c)
  {
    cout << "1002: [" << code.pos << "] expected:" << c << " actual:" << code.text[code.pos] << endl;
    throw 1002;
  }

  ++code.pos;
}

void parsePast(string s)
{
  for (int i = 0; i < s.length(); ++i)
  {
    if (code.pos >= code.length)
      throw 1003;
    if (code.text[code.pos] != s[i])
      throw 1004;

    ++code.pos;
  }
}

char peekChar()
{
  if (code.pos >= code.length)
    throw 1005;
  return code.text[code.pos];
}

char parseChar()
{
  if (code.pos >= code.length)
    throw 1006;
  return code.text[code.pos++];
}

string parseIdentifier()
{
  if (code.pos >= code.length)
    throw 1007;

  int i = code.pos;
  while (code.pos < code.length)
  {
    if (code.text[code.pos] >= 'a' && code.text[code.pos] <= 'z' || code.text[code.pos] >= 'A' && code.text[code.pos] <= 'z')
    {
      code.pos++;
      continue;
    }
    if (code.pos > i && code.text[code.pos] >= '0' && code.text[code.pos] <= '9')
    {
      code.pos++;
      continue;
    }

    // Unacceptable character
    if (code.pos - i == 0)
      // Zero-Length string
      throw 1008;

    return string(code.text, i, code.pos - i);
  }

  throw 1009;
}

string parseStatement()
{
  if (code.pos >= code.length)
    throw 1010;

  int i = code.pos;
  while (code.pos < code.length)
  {
    if (code.text[code.pos] == ';')
    {
      if (code.pos - i == 0)
        // Zero-Length string
        throw 1011;

      code.pos++;
      return string(code.text, i, code.pos - i - 1);
    }
    code.pos++;
  }

  throw 1012;
}

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
  string name;

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

  static Type::Kind parseKind(string str)
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

  static string toString(Type::Kind kind)
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

struct Method
{
  string name;
  vector<string> statements;
  Type *returnType;
};

class DataPoint;

struct ClassDefinition
{
  Type type;
  vector<Type *> attributes;
  vector<Method *> methods;
  ClassDefinition() : type(Type(Type::Class)) {}
};

int findDefinitionPartEnd(const char *fileText, int *startIndex)
{
  int i = *startIndex;
  while (fileText[i] != '{' && fileText[i] != ':')
    ++i;

  return i;
}

int parseMethodDetails(Method *func)
{
  func->name = parseIdentifier();
  parsePast(':');

  // Parse return type
  switch (peekChar())
  {
  case 'v':
  {
    // Void
    parsePast("void:");
    func->returnType = new Type(Type::Void);
  }
  break;
  default:
  {
    cout << "parseMethodDetails() switch error=" << peekChar() << endl;
    return 1;
  }
  break;
  }

  // Parse Arguments
  parsePast('(');
  parsePast(')');
  parsePast('{');

  // Parse statements
  while (peekChar() != '}')
    func->statements.push_back(parseStatement());
  parsePast('}');
  parsePast(']');
  return 0;
}

int parseClassMethod(ClassDefinition *obj)
{
  Method *method = new Method();
  parseMethodDetails(method);
  obj->methods.push_back(method);

  return 0;
}

int parseClassMember(ClassDefinition *obj)
{
  switch (peekChar())
  {
  case 'm':
  {
    // method
    parsePast("method:");
    parseClassMethod(obj);
  }
  break;
  default:
  {
    cout << "parseClassMember() switch error=" << peekChar() << endl;
    throw 1100;
  }
  break;
  }

  return 0;
}

Method *entryMethod;
map<string, ClassDefinition *> classDefinitions;

int parseClass()
{
  ClassDefinition *obj = new ClassDefinition();
  obj->type = Type(Type::Class);
  obj->type.name = parseIdentifier();
  classDefinitions[obj->type.name] = obj;

  parsePast('{');

  bool loop = true;
  while (loop)
    switch (peekChar())
    {
    case '[':
    {
      parsePast("[");
      parseClassMember(obj);
    }
    break;
    case '}':
    {
      loop = false;
    }
    break;
    default:
    {
      cout << "parseClass() switch error=" << peekChar() << endl;
      throw 1110;
    }
    break;
    }

  parsePast('}');
  parsePast(']');

  return 0;
}

void parseRoot()
{
  while (code.pos < code.length)
  {
    if (peekChar() != '[')
      break;

    // Find the begin of the next section
    parsePast('[');

    // Find the type of the next section
    switch (peekChar())
    {
    case 'c':
    {
      // Class
      parsePast("class:");
      parseClass();
      //cout << "--class parsed" << endl;
    }
    break;
    case 'e':
    {
      // Entry Method
      parsePast("entry:");

      entryMethod = new Method();
      parseMethodDetails(entryMethod);
      //cout << "--entryMethod parsed" << endl;
    }
    break;
    default:
    {
      cout << "parseNext() : unhandled letter=" << peekChar() << endl;
      throw 1306;
    }
    }
  }
}

class DataPoint
{
public:
  Type::Kind type;
  void *data;
  DataPoint() : type(Type::Kind::Unknown) {}
  DataPoint(Type::Kind pType, void *pData = nullptr)
      : type(pType), data(pData) {}
};

class InstancedClass
{
public:
  ClassDefinition *definition;

  map<string, DataPoint *> attributes;
};

class MethodMemory
{
public:
  map<string, DataPoint *> *global, *local;
  InstancedClass *instance;

  InstancedClass *getInstance(string identifier)
  {
    DataPoint *dp = getValue(identifier);
    if (!dp)
      return nullptr;

    // Check
    if (dp->type != Type::Class)
    {
      cout << "RequestedClass:" << identifier << " Got:" << Type::toString(dp->type) << endl;
      throw 1070;
    }

    // Return
    return static_cast<InstancedClass *>(dp->data);
  }
  DataPoint *getValue(string identifier)
  {
    map<string, DataPoint *>::iterator it = local->find(identifier);
    if (it == local->end())
    {
      if (instance)
        it = instance->attributes.find(identifier);
      if (!instance || it == instance->attributes.end())
      {
        it = global->find(identifier);
        if (it == global->end())
          return nullptr;
      }
    }
    // Return
    return it->second;
  }
  MethodMemory(map<string, DataPoint *> *pGlobal, InstancedClass *pInstance,
               map<string, DataPoint *> *pLocal)
      : global(pGlobal), instance(pInstance), local(pLocal) {}
};

DataPoint *processMethod(Method *method, map<string, DataPoint *> *global, InstancedClass *instance,
                         map<string, DataPoint *> *local);
void processStatement(MethodMemory *memory, string &statement);

int main(void)
{
  code.text = fileText;
  code.pos = 0;
  code.length = strlen(fileText);

  try
  {
    parseRoot();
  }
  catch (int e)
  {
    cout << "Exception:" << e << endl;
    string s;
    for (int i = -5; i < 6; ++i)
    {
      if (i < 0)
      {
        s += "-";
        continue;
      }
      if (i == 0)
        s += "|";

      s += code.text[code.pos + i];

      if (i == 0)
        s += "|";
    }
    cout << "Code:" << s << endl;
    return e;
  }

  try
  {
    map<string, DataPoint *> global, local;
    DataPoint *result = processMethod(entryMethod, &global, nullptr, &local);
  }
  catch (int e)
  {
    cout << "Exception:" << e << endl;
    return e;
  }
  return 0;
}

DataPoint *processMethod(Method *method, map<string, DataPoint *> *global, InstancedClass *instance,
                         map<string, DataPoint *> *local)
{
  cout << "MethodCall:" << method->name << endl;
  MethodMemory memory(global, instance, local);

  for (int i = 0; i < method->statements.size(); ++i)
    processStatement(&memory, method->statements[i]);

  if (method->returnType->kind == Type::Void)
    return new DataPoint(Type::Void);

  map<string, DataPoint *>::iterator it = memory.local->find("return");
  if (it == memory.local->end())
    return nullptr;

  return it->second;
}

void processStatement(MethodMemory *memory, string &statement)
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
      DataPoint *result = processMethod(method, memory->global, instance, &parameters);
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
  default:
  {
    cout << "processStatement() UnexpectedStatement:" << statement << endl;
    throw 1302;
  }
  break;
  }
}