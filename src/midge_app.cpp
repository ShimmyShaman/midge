/* midge_app.cpp */

#include <cstring>
#include <fstream>

#include "core_interpreter.h"
#include "midge_app.h"
#include "core_bindings.h"

using namespace std;

void MethodCall::pushLocalMemoryBlock()
{
  ++localUsage;
  if (localUsage >= local.size())
  {
    local.resize(localUsage + localUsage / 3 + 4);
    localTemp.resize(local.size());
  }
}

void MethodCall::popLocalMemoryBlock()
{
  --localUsage;
  std::map<std::string, DataValue *>::iterator it = local[localUsage].begin();
  while (it != local[localUsage].end())
  {
    dataManager->deleteData(it->second);
    it++;
  }
  local[localUsage].clear();

  for (int i = 0; i < localTemp[localUsage].size(); ++i)
    dataManager->deleteData(localTemp[localUsage][i]);
  localTemp[localUsage].clear();
}

DataValue *MethodCall::getValue(std::string identifier)
{
  DataValue **pdp = getPointerToValue(identifier);
  if (!pdp)
    return nullptr;
  return dataManager->cloneData(*pdp);
}

DataValue **MethodCall::getPointerToValue(std::string identifier)
{
  int i = localUsage - 1;
  std::map<std::string, DataValue *>::iterator it = local[i].find(identifier);
  while (true)
  {
    if (it != local[i].end())
      return &it->second;
    --i;
    if (i < 0)
      break;
    it = local[i].find(identifier);
  }

  if (instance)
  {
    it = instance->attributes.find(identifier);
    if (it != instance->attributes.end())
      return &it->second;
  }

  it = global->find(identifier);
  if (it == global->end())
    return nullptr;
  return &it->second;
}

void MethodCall::instanceValue(std::string identifier, DataValue *dp)
{
  DataValue **existing = getPointerToValue(identifier);
  if (existing)
    throw 3947;

  local[localUsage - 1][identifier] = dp;
}

void MethodCall::assignValue(std::string identifier, DataValue *dp)
{
  // Find the key
  int i = localUsage - 1;
  std::map<std::string, DataValue *>::iterator it = local[i].find(identifier);
  while (true)
  {
    if (it != local[i].end())
    {
      it->second = dp;
      return;
    }
    --i;
    if (i < 0)
      break;
    it = local[i].find(identifier);
  }

  if (instance)
  {
    it = instance->attributes.find(identifier);
    if (it != instance->attributes.end())
    {
      it->second = dp;
      return;
    }
  }

  it = global->find(identifier);
  if (it != global->end())
  {
    it->second = dp;
    return;
  }

  throw 3941;
}

void MethodCall::addBlockMemory(DataValue *dp)
{
  localTemp[localUsage - 1].push_back(dp);
}

DataValue *MethodCall::takeReturnValue()
{
  DataValue *ret = returnValue;
  returnValue = nullptr;
  return ret;
}

void MethodCall::setReturnValue(DataValue *value)
{
  returnValue = value;
}

void MethodCall::clear()
{
  if (returnValue)
  {
    dataManager->deleteData(returnValue);
    returnValue = nullptr;
  }

  method = nullptr;

  while (localUsage > 0)
    popLocalMemoryBlock();
  localUsage = 1;
}

MethodCall::MethodCall()
{
  returnValue == nullptr;

  // Permanent Parameters Slot
  pushLocalMemoryBlock();
}

MethodCall *MethodCallStack::increment(MethodInfo *method, InstancedClass *instance)
{
  if (stackUsage + 1 >= stackCapacity)
    throw 9989;

  MethodCall *methodCall = &stack[stackUsage];
  methodCall->method = method;
  methodCall->instance = instance;
  methodCall->statementProcessingIndex = 0;

  ++stackUsage;
  return methodCall;
}

void MethodCallStack::decrement(MethodCall **finishedMethod)
{
  (*finishedMethod)->clear();

  finishedMethod = nullptr;
}

MethodCallStack::MethodCallStack(MidgeApp *midgeApp)
{
  stackCapacity = 60;
  stack.resize(stackCapacity);
  stackUsage = 0;

  for (int i = 0; i < stackCapacity; ++i)
  {
    stack[i].global = &midgeApp->globalMemory;
    stack[i].dataManager = &midgeApp->dataManager;
  }
}

void MidgeApp::processMethod(MethodCall *methodCall)
{
  cout << "MethodCall:";
  if (methodCall->instance)
    cout << methodCall->instance->definition->type.name << "::";
  cout << methodCall->method->name << endl;
  processStatementBlock(methodCall, true);
}

void MidgeApp::processStatementBlock(MethodCall *methodCall, bool skipBlockCheck)
{
  //cout << "processStatementBlock(" << methodCall->method->statements.size() << " statements)" << endl;
  if (methodCall->statementProcessingIndex >= methodCall->method->statements.size())
    throw 845;
  if (!skipBlockCheck)
  {
    if (methodCall->method->statements[methodCall->statementProcessingIndex] != "{")
      throw 846;
    ++methodCall->statementProcessingIndex;
  }

  methodCall->pushLocalMemoryBlock();
  for (; methodCall->statementProcessingIndex < methodCall->method->statements.size();
       ++methodCall->statementProcessingIndex)
    switch (methodCall->method->statements[methodCall->statementProcessingIndex][0])
    {
    case '{':
      processStatementBlock(methodCall);
      --methodCall->statementProcessingIndex;
      break;
    case '}':
      methodCall->popLocalMemoryBlock();
      ++methodCall->statementProcessingIndex;
      return;
    default:
      processStatement(methodCall);
      break;
    }

  if (skipBlockCheck && methodCall->statementProcessingIndex == methodCall->method->statements.size())
    return;
  throw 847;
}

void MidgeApp::processStatement(MethodCall *methodCall)
{
  string statement = methodCall->method->statements[methodCall->statementProcessingIndex];
  //cout << "processStatement(" << statement << ")" << endl;

  // By Method-Name
  string methodName = statement.substr(0, statement.find('('));
  switch (statement[0])
  {
  case 'a':
  {
    if (methodName == "addClassMethod")
    {
      processCall_addClassMethod(methodCall, statement);
      return;
    }
    else if (methodName == "addClassMethodCode")
    {
      processCall_addClassMethodCode(methodCall, statement);
      return;
    }
  }
  case 'b':
  {
    if (methodName == "bindingInvoke")
    {
      processCall_bindingInvoke(methodCall, statement);
      return;
    }
  }
  break;
  case 'c':
  {
    if (methodName == "createClass")
    {
      processCall_createClass(methodCall, statement);
      return;
    }
  }
  break;
  case 'i':
  {
    if (methodName == "instanceData")
    {
      processCall_instance(methodCall, statement);
      return;
    }
    else if (methodName == "invoke")
    {
      processCall_invoke(methodCall, statement);
      return;
    }
  }
  break;
  case 'p':
  {
    if (methodName == "print")
    {
      processCall_print(methodCall, statement);
      return;
    }
    else if (methodName == "printLine")
    {
      processCall_print(methodCall, statement);
      cout << endl;
      return;
    }
  }
  break;
  default:
    break;
  }
  cout << "processStatement() UnexpectedStatement:" << statement << endl;
  throw 1302;
}

void MidgeApp::getCallArgsFromStatement(vector<string> *args, string &statement)
{
  bool argsRemain = true;
  int ix = statement.find('(', 0) + 1;

  int blockDepth = 0;
  for (int i = ix; i < statement.length(); ++i)
  {
    if (blockDepth)
    {
      if (statement[i] == '}')
        --blockDepth;
      if (blockDepth)
        continue;

      ++i;
      if (statement[i] != ',' && statement[i] != ')')
      {
        throw 1965;
      }

      args->push_back(statement.substr(ix, i - ix));
      ix = i;
      if (statement[i] == ')')
        return;
    }

    switch (statement[i])
    {
    case '{':
      ++blockDepth;
      break;
    case ',':
      args->push_back(statement.substr(ix, i - ix));
      ix = i + 1;
      break;
    case ')':
      args->push_back(statement.substr(ix, i - ix));
      return;
    default:
      break;
    }
  }

  throw 1977;
}

void MidgeApp::processCall_addClassMethod(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 3)
    throw 2121;

  if (classDefinitions.count(args[0]) != 1)
    throw 2122;
  ClassDefinition *classDefinition = classDefinitions[args[0]];

  for (int i = 0; i < classDefinition->methods.size(); ++i)
    if (classDefinition->methods[i]->name == args[1])
    {
      cout << "Error! Cannot have two methods with the same name in class spec" << endl;
      throw 2124;
    }

  int argCount = (args.size() - 3) / 2;
  MethodInfo *method = new MethodInfo();
  method->name = args[1];
  method->returnDataType = Type::parseKind(args[2]);
  for (int i = 0; i < argCount; ++i)
  {
    DataType dataType = Type::parseKind(args[3 + i * 2]);
    method->arguments.push_back(new Type(dataType, args[3 + i * 2 + 1]));
  }

  classDefinition->methods.push_back(method);
}

void MidgeApp::processCall_addClassMethodCode(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 3)
    throw 2131;

  if (classDefinitions.count(args[0]) != 1)
    throw 2132;
  ClassDefinition *classDefinition = classDefinitions[args[0]];

  MethodInfo *method = nullptr;
  for (int i = 0; i < classDefinition->methods.size(); ++i)
    if (classDefinition->methods[i]->name == args[1])
    {
      method = classDefinition->methods[i];
      break;
    }
  if (method == nullptr)
  {
    cout << "addClassMethodCode] method not found:" << args[0] << "::" << args[1] << endl;
    throw 2133;
  }

  if (args[2][0] != '{' && args[2][args[2].length() - 1] != '}')
    throw 2135;

  method->statements.push_back(args[2].substr(1, args[2].length() - 2));
}

void MidgeApp::processCall_bindingInvoke(MethodCall *methodCall, string &statement)
{
  // binding invoke
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 1)
    throw 1246;

  bool argsRemain = true;
  int ix = statement.find('(', 0) + 1;
  int iy = statement.find(',');
  if (iy == string::npos)
  {
    argsRemain = false;
    iy = statement.find(')', ix);
  }
  string methodIdentity = statement.substr(ix, iy - ix);

  void *bindingArgs[args.size() - 1];
  for (int i = 1; i < args.size(); ++i)
  {
    switch (args[i][0])
    {
    case '"':
    {
      // TODO find the end of literal string

      string *stringLiteral = new string(args[i].substr(1, args[i].length() - 2));
      DataValue *dv = dataManager.createData(DataType::String,
                                             static_cast<void *>(stringLiteral));
      methodCall->addBlockMemory(dv);
      bindingArgs[i - 1] = dv->data();
    }
    break;
    default:
      throw 1246;
    }
  }

  methodPtr method = Bindings::getMethod(methodIdentity);
  if (!method)
    throw 1249;

  // Invoke
  // TODO return values
  void *retValue = method(&bindingArgs[0], args.size() - 1);
}

void MidgeApp::processCall_createClass(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 1)
    throw 2111;

  if (classDefinitions.count(args[0]))
    throw 2112;

  ClassDefinition *classDefinition = new ClassDefinition();
  classDefinition->type.kind = DataType::Class;
  classDefinition->type.name = args[0];
  classDefinitions[classDefinition->type.name] = classDefinition;
}

void MidgeApp::processCall_instance(MethodCall *methodCall, string &statement)
{
  // instance
  int ix = statement.find('(', 0) + 1;
  int iy = statement.find(',');
  string dataTypeStr = statement.substr(ix, iy - ix);
  string instanceName = statement.substr(iy + 1, statement.size() - 1 - iy - 1);

  DataValue *dp = nullptr;
  DataType dataType = Type::parseKind(dataTypeStr);
  if (dataType == DataType::Unknown)
  {
    // Assume class-name specified
    dataType = DataType::Class;
  }
  if (!Type::isPrimitive(dataType))
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
      DataType attrType = definition->attributes[i]->kind;
      void *attrData = nullptr;
      switch (definition->attributes[i]->kind)
      {
      case DataType::Class:
      {
        attrData = static_cast<void *>(nullptr);
      }
      break;
      case DataType::Int32:
      {
        int *value = new int();
        attrData = static_cast<void *>(value);
      }
      break;
      default:
        cout << "processStatement() UnexpectedAttributeType:" << Type::toString(attrType) << endl;
        throw 1203;
        break;
      }
      obj->attributes[definition->attributes[i]->name] = dataManager.createData(definition->attributes[i]->kind,
                                                                                attrData);
    }
    dp = dataManager.createData(dataType, static_cast<void *>(obj));
  }
  else
  {
    switch (dataType)
    {
    case DataType::Int32:
    {
      int *value = new int();
      dp = dataManager.createData(dataType, static_cast<void *>(value));
    }
    break;
    default:
      cout << "processStatement() Unexpected Primitive Type:" << Type::toString(dp->dataType()) << endl;
      throw 1204;
      break;
    }
  }
  methodCall->instanceValue(instanceName, dp);
}

void MidgeApp::processCall_invoke(MethodCall *methodCall, string &statement)
{
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 2)
    throw 1241;

  InstancedClass *instance = nullptr;
  {
    DataValue **pInstance = methodCall->getPointerToValue(args[1]);
    if (!pInstance)
      throw 1242;
    instance = static_cast<InstancedClass *>((*pInstance)->data());
  }

  if (!instance)
    throw 1243;

  if (args[1] == "null")
    throw 1244; // Implement global static methods?

  MethodInfo *method = nullptr;
  for (int i = 0; i < instance->definition->methods.size(); ++i)
    if (instance->definition->methods[i]->name == args[0])
    {
      method = instance->definition->methods[i];
      break;
    }
  if (!method)
    throw 1245;

  // Invoke
  MethodCall *subcall = callStack->increment(method, instance);
  processMethod(subcall);
  callStack->decrement(&subcall);
}

void MidgeApp::processCall_print(MethodCall *methodCall, string &statement)
{
  // print
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 1)
    throw 1321;

  if (args[0][0] != '"')
  {
    DataValue **valueInstance = methodCall->getPointerToValue(args[0]);
    if (!valueInstance)
      throw 1322; // Can't find it
    switch ((*valueInstance)->dataType())
    {
    case DataType::Int32:
      cout << *(*valueInstance)->int32();
      break;
    case DataType::String:
      cout << *static_cast<string *>((*valueInstance)->data());
      break;
    case DataType::Class:
    {
      InstancedClass *instancedClass = static_cast<InstancedClass *>((*valueInstance)->data());
      cout << "<Class=" << instancedClass->definition->type.name << ">";
    }
    break;
    default:
      cout << Type::toString((*valueInstance)->dataType());
      break;
    }
  }
  else
  {
    if (args[0][args[0].length() - 1] != '"')
      throw 1323;
    if (args.size() > 1)
      throw 1324; // NotYetImplemented
    cout << args[0].substr(1, args[0].length() - 2);
  }
}

int MidgeApp::callMethodFromFile(string filePath, std::string methodName)
{
  MethodInfo fileMethod;
  fileMethod.name = methodName;
  fileMethod.returnDataType = DataType::Int32;

  ifstream file;
  try
  {
    file.open(filePath);
    if (!file.is_open())
    {
      cout << "Failure to open file:" << filePath << endl;
      throw 244;
    }
  }
  catch (const std::exception &e)
  {
    cout << "ERROR failed to open file:" << filePath << endl;
    std::cerr << e.what() << '\n';
    return -1;
  }

  try
  {
    string line;
    bool reading = true;
    int blockDepth = 0;
    while (file.good())
    {
      char c = static_cast<char>(file.get());
      if (reading)
      {
        if (c == '{')
          ++blockDepth;
        else if (c == '}')
          --blockDepth;
      }
      if (!blockDepth && c == ')')
      {
        line.append(1, c);
        if (line.length() == 0)
          continue;
        //cout << "pushback line:" << line << endl;
        fileMethod.statements.push_back(line);
        line.clear();
        reading = false;
        continue;
      }
      if (!reading)
      {
        if (c == '{' || c == '}')
        {
          line.append(1, c);
          //cout << "pushback line:" << line << endl;
          fileMethod.statements.push_back(line);
          line.clear();
          continue;
        }

        if (c >= 'a' && c <= 'z')
        {
          reading = true;
        }
        else
          continue;
      }

      line.append(1, c);
    }
    file.close();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
    return -2;
  }

  MethodCall *methodCall = callStack->increment(&fileMethod, nullptr);
  processMethod(methodCall);
  DataValue *result = methodCall->takeReturnValue();
  callStack->decrement(&methodCall);

  if (result)
  {
    int returnValue = *result->int32();
    delete (result);
    return returnValue;
  }

  return 0;
}

int MidgeApp::run()
{
  try
  {
    return callMethodFromFile("./mcmd.txt", "entryMethod");
  }
  catch (int e)
  {
    std::cout << "Exception:" << e << std::endl;
    return e;
  }
}

MidgeApp::MidgeApp()
{
  callStack = new MethodCallStack(this);
}

MidgeApp::~MidgeApp()
{
  if (callStack)
    delete callStack;

  // map<string, ClassDefinition *>::iterator it = classDefinitions.begin();
  // while (it != classDefinitions.end())
  // {
  //   delete(it->second);
  // }
}