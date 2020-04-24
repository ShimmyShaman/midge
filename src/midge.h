/* midge.h */

#ifndef MIDGE_H
#define MIDGE_H

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"

#include "rendering/renderer.h"

static cling::Interpreter *clint;

// std::vector<cling::Transaction *> transactions;
std::map<std::string, cling::Transaction *> definedTypes;
void defineType(std::string typeName, std::string definition)
{
  std::map<std::string, cling::Transaction *>::iterator it = definedTypes.find(typeName);
  if (it != definedTypes.end())
  {
    std::cout << "type " << typeName << " already exists!" << std::endl;

    uint wastid = clint->getLatestTransaction()->getUniqueID();
    //std::cout << "current=" << interp->getLatestTransaction()->getUniqueID() << std::endl;
    //interp->getLatestTransaction()->dump();
    int iter = 0;
    while (clint->getLatestTransaction()->getUniqueID() > it->second->getUniqueID())
    {
      ++iter;
      clint->unload(1);
      //interp->getLatestTransaction()->dump();
    }

    if (clint->getLatestTransaction()->getUniqueID() != it->second->getUniqueID())
      throw 343;
    clint->unload(1);
  }

  cling::Transaction *transaction = nullptr;
  clint->declare(definition, &transaction);
  if (!transaction)
    throw 111;
  else
  {
    //std::cout << "Transaction:" << transaction->getUniqueID() << std::endl;
    //transaction->dump();
    // transactions.push_back(transaction);
    definedTypes[typeName] = transaction;
  }
}

void run()
{
  // Initialize data structures
  /*defineType("Node", "struct Node {"
                     "  Node *parent = nullptr;"
                     "};");

  interp->process("Node global");*/

  //std::cout << "~~midge welcomes you~~" << std::endl;
  //std::cout << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

  try
  {
    // Libraries
    clint->loadLibrary("vulkan");

    beginRenderThread();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}
#endif // MIDGE_H