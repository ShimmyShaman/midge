/* method_call.cpp */

#include <cstring>
#include <fstream>

#include "core_interpreter.h"
#include "method_call.h"
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

void MethodCall::instanceValue(std::string identifier, DataValue *dp, VariableScope scope)
{
  DataValue **existing = getPointerToValue(identifier);
  if (existing)
    throw 3947;

  switch (scope)
  {
  case VariableScope::BLOCK:
    local[localUsage - 1][identifier] = dp;
    break;
  case VariableScope::GLOBAL:
    (*global)[identifier] = dp;
    break;
  default:
    throw 6583;
  }
}

/* assigns value (does not create a clone of the value) */
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

void MethodCall::addBlockMemory(int slot, DataValue *dp)
{
  string slotIdentity("$" + to_string(slot));
  map<string, DataValue *>::iterator it = local[localUsage - 1].find(slotIdentity);
  if (it != local[localUsage - 1].end())
  {
    dataManager->deleteData(it->second);
  }

  local[localUsage - 1][slotIdentity] = dp;
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