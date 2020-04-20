/* midge_app.cpp */

#include <cstring>
#include <fstream>
#include <time.h>

#include "core_interpreter.h"
#include "midge_app.h"
#include "core_bindings.h"

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"
#include "cling/Interpreter/Value.h"
#include "cling/Utils/Casting.h"

using namespace std;

std::map<std::string, Thread *> MethodCallStack::threads;

void MethodCallStack::processMethod(MethodCall *methodCall)
{
  cout << "MethodCall:";
  if (methodCall->instance)
    cout << methodCall->instance->definition->type.name << "::";
  cout << methodCall->method->name << endl;
  processStatementBlock(methodCall, true);
}

void MethodCallStack::processStatementBlock(MethodCall *methodCall, bool skipBlockCheck)
{
  cout << "processStatementBlock(" << methodCall->method->statements.size() << " statements)" << endl;
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

void MethodCallStack::processStatement(MethodCall *methodCall)
{
  string statement = methodCall->method->statements[methodCall->statementProcessingIndex];
  cout << "processStatement(" << statement << ")" << endl;

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
    else if (methodName == "assign")
    {
      processCall_assign(methodCall, statement);
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
    else if (methodName == "createAttribute")
    {
      processCall_createAttribute(methodCall, statement);
      return;
    }
  }
  break;
  case 'i':
  {
    if (methodName == "initializeDefault")
    {
      processCall_initializeDefault(methodCall, statement);
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
  case 't':
  {
    if (methodName == "thread")
    {
      processCall_thread(methodCall, statement);
      return;
    }
  }
  break;
  default:
    break;
  }
  cout << "processStatement() UnhandledStatement:" << statement << endl;
  throw 1302;
}

void MethodCallStack::getCallArgsFromStatement(vector<string> *args, string &statement)
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
      else if (statement[i] == '{')
        ++blockDepth;
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

void MethodCallStack::processCall_addClassMethod(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 3)
    throw 2121;

  map<string, ClassDefinition *>::iterator classit = classDefinitions->find(args[0]);
  if (classit == classDefinitions->end())
    throw 2122;

  for (int i = 0; i < classit->second->methods.size(); ++i)
    if (classit->second->methods[i]->name == args[1])
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

  classit->second->methods.push_back(method);
}

void MethodCallStack::processCall_addClassMethodCode(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 3)
    throw 2131;

  if (classDefinitions->count(args[0]) != 1)
    throw 2132;
  ClassDefinition *classDefinition = (*classDefinitions)[args[0]];

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

  // argument may contain many statements
  {
    int t = 1;
    bool reading = false;
    int blockDepth = 0;
    string line = "";
    while (t < args[2].length() - 1)
    {
      char c = args[2][t++];
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
        cout << "methodStatement:" << line << endl;
        method->statements.push_back(line);
        line.clear();
        reading = false;
        continue;
      }
      if (!reading)
      {
        if (c == '{' || c == '}')
        {
          line.append(1, c);
          cout << "methodStatement:" << line << endl;
          method->statements.push_back(line);
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
  }
}

void MethodCallStack::processCall_assign(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 2 || args.size() > 3)
    throw 2151;

  DataValue *assignmentValue;
  if (args.size() == 2)
  {
    // assign(instanceName,valueInstanceNameOrLiteralValue)
    //   ~ c = a
    throw 2152;
  }
  else
  {
    // assign(instanceName,valueMemberName,valueInstanceName)
    //   ~ c = b.a
    DataValue **instancePtr = methodCall->getPointerToValue(args[2]);
    if (!instancePtr)
    {
      cout << "valueInstanceName:" << args[2] << " not found!" << endl;
      throw 2153;
    }
    if ((*instancePtr)->dataType().kind != DataType::Class)
      throw 2154;
    InstancedClass *instance = static_cast<InstancedClass *>((*instancePtr)->data());

    cout << "here2" << endl;
    map<string, DataValue *>::iterator it = instance->attributes.find(args[1]);
    if (it == instance->attributes.end())
      throw 2155;

    cout << "here3" << endl;
    DataValue *dv = dataManager->cloneData(it->second);
    if (args[0][0] >= '0' && args[0][0] <= '9')
    {
      for (int i = 1; i < args[0].length(); ++i)
        if (args[0][i] < '0' || args[0][i] > '9')
          throw 2157;

      cout << "here4" << endl;
      // Temporary Field
      methodCall->addBlockMemory(stoi(args[0]), dv);
    }
    else
    {
      methodCall->assignValue(args[0], dv);
    }
    cout << "here5" << endl;
  }
}

void MethodCallStack::processCall_bindingInvoke(MethodCall *methodCall, string &statement)
{
  // binding invoke
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 1)
    throw 1261;

  BoundMethodInfo *binding = Bindings::getMethod(args[0]);
  if (!binding)
    throw 1263;

  void **bindingArgs = new void *[args.size() - 1];
  int bindingArgCount = 0;
  if (args.size() > 1)
  {
    for (int i = 1; i < args.size(); ++i)
    {
      int p = i - 1;
      if (binding->argumentTypes.size() < p)
        throw 1264;
      switch (binding->argumentTypes[p]->dataType)
      {
      case DataType::String:
      {
        if (args[i].size() < 2 || args[i][0] != '"' || args[i][args[i].size() - 1] != '"')
          throw 1265;

        string *str = new string(args[i].substr(1, args[i].size() - 2));
        DataValue *dv = dataManager->createData(DataType::String, static_cast<void *>(str));
        methodCall->addBlockMemory(dv);

        bindingArgs[p] = dv->data();
        ++bindingArgCount;
      }
      break;
      case DataType::Int32:
      {
        for (int j = 0; j < args[i].size(); ++j)
          if (args[i][j] < '0' || args[i][j] > '9')
            throw 1266;

        int var = stoi(args[i]);
        DataValue *dv = dataManager->cloneData(DataType::Int32, static_cast<void *>(&var));
        methodCall->addBlockMemory(dv);

        bindingArgs[p] = dv->data();
        ++bindingArgCount;
      }
      break;
      case DataType::Int64:
      {
        for (int j = 0; j < args[i].size(); ++j)
          if (args[i][j] < '0' || args[i][j] > '9')
          {
            cout << "J:" << args[i][j] << endl;
            throw 1267;
          }

        long var = stol(args[i]);
        DataValue *dv = dataManager->cloneData(DataType::Int64, static_cast<void *>(&var));
        methodCall->addBlockMemory(dv);

        bindingArgs[p] = dv->data();
        ++bindingArgCount;
      }
      break;
      default:
        throw 1268;
      }
    }
  }

  // Invoke
  void *result = binding->method(bindingArgs, bindingArgCount);
  if (result)
    throw 1269; // NotYetImplemented

  delete (bindingArgs);

  // TODO return values
  //void *retValue = method(&bindingArgs[0], args.size() - 1);
}

void MethodCallStack::processCall_createAttribute(MethodCall *methodCall, string &statement)
{
  // create attribute
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 3)
    throw 2141;

  map<string, ClassDefinition *>::iterator it = classDefinitions->find(args[0]);
  if (it == classDefinitions->end())
    throw 2142;

  Type type(Type::parseKind(args[2]));
  if (type.kind == DataType::Unknown)
  {
    type.kind = DataType::Class;
    type.name = args[2];
  }
  FieldInfo *attribute = new FieldInfo(type, args[1]);

  it->second->attributes.push_back(attribute);
}

void MethodCallStack::processCall_createClass(MethodCall *methodCall, string &statement)
{
  // create class
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 1)
    throw 2111;

  if (classDefinitions->count(args[0]))
    throw 2112;

  ClassDefinition *classDefinition = new ClassDefinition();
  classDefinition->type.kind = DataType::Class;
  classDefinition->type.name = args[0];
  (*classDefinitions)[classDefinition->type.name] = classDefinition;
}

void MethodCallStack::processCall_initializeDefault(MethodCall *methodCall, string &statement)
{
  // initialize to default value
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() < 2 || args.size() > 3)
    throw 2111;

  DataValue *dp = nullptr;
  DataType dataType = Type::parseKind(args[0]);
  if (dataType == DataType::Unknown)
  {
    // Assume class-name specified
    dataType = DataType::Class;
  }
  if (!Type::isPrimitive(dataType))
  {
    map<string, ClassDefinition *>::iterator it = classDefinitions->find(args[0]);
    if (it == classDefinitions->end())
    {
      cout << "processStatement() NonPrimitiveUnspecifiedType:" << args[0] << endl;
      throw 1202;
    }
    ClassDefinition *definition = it->second;

    dp = dataManager->createClass(definition);
  }
  else
  {
    switch (dataType)
    {
    case DataType::Int32:
    {
      int *value = new int();
      dp = dataManager->createData(dataType, static_cast<void *>(value));
    }
    break;
    default:
      cout << "processStatement() Unexpected Primitive Type:" << dp->dataType().toString() << endl;
      throw 1205;
      break;
    }
  }

  if (args.size() == 2)
  {
    // Default to local block initialization
    methodCall->instanceValue(args[1], dp, VariableScope::BLOCK);
    return;
  }

  if (args[2][0] != '%')
    throw 1206; // FormatException

  switch (args[2][1])
  {
  case 'g': //global
    methodCall->instanceValue(args[1], dp, VariableScope::GLOBAL);
    break;
  case 'i':     // class instance
  case 's':     // class static
  case 'm':     // method scope
  case 't':     // method static
  case 'b':     // current block
    throw 1207; // NotYetImplemented
  default:
    throw 1208;
  }
}

void MethodCallStack::processCall_invoke(MethodCall *methodCall, string &statement)
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
  MethodCall *subcall = incrementCallStack(method, instance);
  processMethod(subcall);
  decrementCallStack(&subcall);
}

void MethodCallStack::processCall_print(MethodCall *methodCall, string &statement)
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
    switch ((*valueInstance)->dataType().kind)
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
      cout << (*valueInstance)->dataType().toString();
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

void MethodCallStack::processCall_thread(MethodCall *methodCall, string &statement)
{
  vector<string> args;
  getCallArgsFromStatement(&args, statement);

  // argument check
  if (args.size() != 3)
    throw 1371;

  if (threads.count(args[0]))
    throw 1372;

  InstancedClass *instance = nullptr;
  {
    DataValue **pInstance = methodCall->getPointerToValue(args[2]);
    if (!pInstance)
      throw 1373;
    instance = static_cast<InstancedClass *>((*pInstance)->data());
  }

  if (!instance)
    throw 1374;

  if (args[1] == "null")
    throw 1375; // Implement global static methods?

  MethodInfo *method = nullptr;
  for (int i = 0; i < instance->definition->methods.size(); ++i)
    if (instance->definition->methods[i]->name == args[1])
    {
      method = instance->definition->methods[i];
      break;
    }
  if (!method)
    throw 1376;

  // Invoke
  MethodCallStack *anotherCallStack = new MethodCallStack();
  anotherCallStack->initialize(dataManager, globalMemory, classDefinitions);

  Thread *thread = new Thread();
  thread->callStack = anotherCallStack;
  threads[args[0]] = thread;

  void **threadArgs = new void *[4];
  threadArgs[0] = static_cast<void *>(new string(args[0]));
  threadArgs[1] = static_cast<void *>(anotherCallStack);
  threadArgs[2] = static_cast<void *>(instance);
  threadArgs[3] = static_cast<void *>(method);

  int err = pthread_create(&(thread->threadId), NULL, &MethodCallStack::threadStart, threadArgs);
  if (err)
  {
    cout << "pthread_create error=" << err << endl;
    threads.erase(args[0]);
    delete (thread);
    throw 1377;
  }
}

void MethodCallStack::meaninglessPrint()
{
  cout << "meaninglessprint()" << endl;
}

void *MethodCallStack::threadStart(void *arg)
{
  string *threadName = nullptr;
  MethodCallStack *callStack = nullptr;
  DataValue *result = nullptr;
  try
  {
    // TODO -- this doesn't allow forced cancellation of this thread.
    // Have to find out what does.
    pthread_setcanceltype(PTHREAD_CANCEL_DISABLE, NULL);

    // State
    void **pThreadArgs = static_cast<void **>(arg);
    threadName = static_cast<string *>(pThreadArgs[0]);
    callStack = static_cast<MethodCallStack *>(pThreadArgs[1]);
    InstancedClass *instance = static_cast<InstancedClass *>(pThreadArgs[2]);
    MethodInfo *method = static_cast<MethodInfo *>(pThreadArgs[3]);
    delete (pThreadArgs);

    // Register
    cout << "Thread Began:" << *threadName << endl;

    // Invoke
    MethodCall *methodCall = callStack->incrementCallStack(method, instance);
    callStack->processMethod(methodCall);
    result = methodCall->takeReturnValue();
    callStack->decrementCallStack(&methodCall);
  }
  catch (int e)
  {
    std::cout << "Exception(thread:" << *threadName << "):" << e << std::endl;

    // Deregister
    Thread *threadHandle = threads[*threadName];
    threads.erase(*threadName);
    if (threadHandle)
      delete (threadHandle);
    cout << "Thread Ended:" << *threadName << endl;

    // Cleanup
    if (threadName)
      delete (threadName);
    if (callStack)
      delete (callStack);
  }

  // Return
  if (result)
  {
    void *returnValue = result->data();
    delete (result);
    return returnValue;
    pthread_exit(returnValue);
  }

  pthread_exit(nullptr);
}

MethodInfo *MethodCallStack::loadMethodFromFile(string filePath, std::string methodName)
{
  MethodInfo *fileMethod = new MethodInfo();
  fileMethod->name = methodName;
  fileMethod->returnDataType = DataType::Int32;

  ifstream file;
  try
  {
    file.open(filePath);
    if (!file.is_open())
    {
      cout << "Failure to open file:" << filePath << endl;
      delete (fileMethod);
      throw 897;
    }
  }
  catch (const std::exception &e)
  {
    cout << "ERROR failed to open file:" << filePath << endl;
    std::cerr << e.what() << '\n';
    delete (fileMethod);
    throw 898;
  }

  try
  {
    string line;
    bool reading = false;
    int blockDepth = 0;
    char p = '\0';
    int inComment = 0;
    while (file.good())
    {
      char c = static_cast<char>(file.get());
      if (reading)
      {
        switch (c)
        {
        case '{':
          ++blockDepth;
          break;
        case '}':
          --blockDepth;
          break;
        }
      }
      else
      {
        // Comments
        if (c == '/')
        {
          if (p = '*')
          {
            inComment = 0;
          }
          else if (p = '/')
            inComment = 2;
        }
        else if (c == '*')
        {
          if (p == '/')
            inComment = 1;
        }
        else if (c == '\n')
        {
          if (inComment == 2)
            inComment = false;
        }

        p = c;
        if (inComment)
          continue;
      }

      if (!blockDepth && c == ')')
      {
        line.append(1, c);
        if (line.length() == 0)
          continue;
        //cout << "pushback line:" << line << endl;
        fileMethod->statements.push_back(line);
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
          fileMethod->statements.push_back(line);
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
    delete (fileMethod);
    throw 899;
  }

  return fileMethod;
}

MethodCall *MethodCallStack::incrementCallStack(MethodInfo *method, InstancedClass *instance)
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

void MethodCallStack::decrementCallStack(MethodCall **finishedMethod)
{
  (*finishedMethod)->clear();

  finishedMethod = nullptr;
}

void MethodCallStack::initialize(DataManager *pDataManager, std::map<std::string, DataValue *> *pGlobalMemory,
                                 std::map<std::string, ClassDefinition *> *pClassDefinitions)
{
  dataManager = pDataManager;
  globalMemory = pGlobalMemory;
  classDefinitions = pClassDefinitions;

  stackCapacity = 60;
  stack.resize(stackCapacity);
  stackUsage = 0;

  for (int i = 0; i < stackCapacity; ++i)
  {
    stack[i].global = globalMemory;
    stack[i].dataManager = dataManager;
  }
}

MethodCallStack::MethodCallStack()
{
}
#include <unistd.h>
int MidgeApp::run(int argc, const char *const *argv)
{
  try
  {
    // Create the Interpreter. LLVMDIR is provided as -D during compilation.
    const char *LLVMDIR = "/home/daniel/cling/obj";
    cling::Interpreter interp(argc, argv, LLVMDIR);

    // char buffer[200];
    // getcwd(buffer, 200);
    // cout << "cwd:" << std::string(buffer) << endl;

    interp.process("#include <iostream>");

    interp.process("class Node {"
                   "public:"
                   "  Node *parent;"
                   "  Node() {"
                   "    parent = nullptr;"
                   "    std::cout << \"Node()\" << std::endl;"
                   "  }"
                   "  ~Node() {"
                   "    std::cout << \"~Node()\" << std::endl;"
                   "  }"
                   "};");
    const cling::Transaction *nodeDeclTransaction = interp.getLatestTransaction();

    interp.process("Node *rootNode = new Node();");

    void *rootNode = interp.getAddressOfGlobal("rootNode");
    cout << "rootNodeGetAddress=" << rootNode << endl;

    cout << "nodeDeclTransaction=" << nodeDeclTransaction->getUniqueID() << endl;
    cout << "latestTransaction=" << interp.getLatestTransaction()->getUniqueID() << endl;

    interp.unload(1 + interp.getLatestTransaction()->getUniqueID() - nodeDeclTransaction->getUniqueID());

    rootNode = interp.getAddressOfGlobal("rootNode");
    cout << "rootNodeGetAddress=" << rootNode << endl;

    cout << "latestTransaction=" << interp.getLatestTransaction()->getUniqueID() << endl;

    interp.process("#include <string>");
    interp.process("class Node {"
                   "public:"
                   "  Node *parent;"
                   "  std::string name;"
                   "  Node(std::string pName) {"
                   "    name = pName;"
                   "    std::cout << \"Node()\" << std::endl;"
                   "  }"
                   "  ~Node() {"
                   "    std::cout << \"~Node()\" << std::endl;"
                   "  }"
                   "};");

    interp.process("Node *rootNode = new Node(\"global\");");

    rootNode = interp.getAddressOfGlobal("rootNode");
    cout << "rootNodeGetAddress=" << rootNode << endl;

    // interp.unload()

    // interp.AddIncludePath("src/glfw/glfw_bindings.h");
    // interp.process("#include <iostream>");
    // interp.process("#include <unistd.h>");
    // interp.process("chdir(\"/home/daniel/midge/src\");");
    // interp.process("char buffer[200];");
    // interp.process("getcwd(buffer, 200);");
    // interp.process("std::cout << \"cwd:\" << std::string(buffer) << std::endl;");
    // interp.AddIncludePath("/home/daniel/midge/src");
    // interp.process("#include \"glfw/glfw_bindings.h\"");
    // // interp.declare("class GLFWBindings;");
    // // interp.declare("void *GLFWBindings::runTriangle(void **args, int argCount);");
    // interp.process("GLFWBindings::runTriangle(nullptr, 0);");
    return 0;

    /*MethodInfo *method = loadMethodFromFile("./mcmd.txt", "entryMethod");

    MethodCall *methodCall = incrementCallStack(method, nullptr);
    processMethod(methodCall);
    DataValue *result = methodCall->takeReturnValue();
    decrementCallStack(&methodCall);

    if (result)
    {
      int returnValue = *result->int32();
      delete (result);
      return returnValue;
    }*/

    if (threads.size())
    {
      // Elegant Shutdown
      cout << "Issuing Shutdown order to " << threads.size() << " active threads." << endl;
      map<string, Thread *>::iterator it = threads.begin();
      while (it != threads.end())
      {
        try
        {
          it->second->shouldExit = true;
        }
        catch (const std::exception &e)
        {
          std::cerr << e.what() << endl;
        }
        ++it;
      }

      int threadWaitingTime = 0;
      const long ONE_MILLISECOND = 1000000L;
      while (threads.size() && threadWaitingTime < 1000)
      {
        ++threadWaitingTime;
        nanosleep((const struct timespec[]){{0, ONE_MILLISECOND}}, NULL);
      }

      // Brute Shutdown
      cout << "Forcing Shutdown of " << threads.size() << " active threads." << endl;
      it = threads.begin();
      while (it != threads.end())
      {
        try
        {
          int result = pthread_cancel(it->second->threadId);
          cout << "thread_cancel=" << result << endl;
        }
        catch (const std::exception &e)
        {
          std::cerr << e.what() << endl;
        }
        ++it;
      }
      threadWaitingTime = 0;
      while (threads.size() && threadWaitingTime < 1000)
      {
        ++threadWaitingTime;
        nanosleep((const struct timespec[]){{0, ONE_MILLISECOND}}, NULL);
      }

      // Concern message
      if (threads.size())
        cout << "Concern! Active Threads At Shutdown=" << threads.size() << endl;
    }

    return 0;
  }
  catch (int e)
  {
    std::cout << "Exception:" << e << std::endl;
    return e;
  }
}

MidgeApp::MidgeApp()
{
  map<string, ClassDefinition *> *cd = new map<string, ClassDefinition *>();
  initialize(new DataManager(cd), new map<string, DataValue *>(), cd);
}

MidgeApp::~MidgeApp()
{
  // map<string, ClassDefinition *>::iterator it = classDefinitions->begin();
  // while (it != classDefinitions->end())
  // {
  //   delete(it->second);
  // }
}