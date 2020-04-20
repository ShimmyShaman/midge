/* midge.h */

#ifndef MIDGE_H
#define MIDGE_H

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"

#include "opengl/gl_loop.h"

static cling::Interpreter *interp = nullptr;

std::vector<cling::Transaction *> transactions;
std::map<std::string, cling::Transaction *> definedTypes;
extern "C" void defineType(std::string typeName, std::string definition)
{
  std::map<std::string, cling::Transaction *>::iterator it = definedTypes.find(typeName);
  if (it != definedTypes.end())
  {
    std::cout << "type " << typeName << " already exists!" << std::endl;

    uint wastid = interp->getLatestTransaction()->getUniqueID();
    //std::cout << "current=" << interp->getLatestTransaction()->getUniqueID() << std::endl;
    //interp->getLatestTransaction()->dump();
    int iter = 0;
    while (interp->getLatestTransaction()->getUniqueID() > it->second->getUniqueID())
    {
      ++iter;
      interp->unload(1);
      //interp->getLatestTransaction()->dump();
    }

    if (interp->getLatestTransaction()->getUniqueID() != it->second->getUniqueID())
      throw 343;
    interp->unload(1);
  }

  cling::Transaction *transaction = nullptr;
  interp->declare(definition, &transaction);
  if (!transaction)
    throw 111;
  else
  {
    //std::cout << "Transaction:" << transaction->getUniqueID() << std::endl;
    //transaction->dump();
    transactions.push_back(transaction);
    definedTypes[typeName] = transaction;
  }
}

extern "C" void run()
{
  // Initialize data structures
  /*defineType("Node", "struct Node {"
                     "  Node *parent = nullptr;"
                     "};");

  interp->process("Node global");*/

  std::cout << "midge welcomes you" << std::endl;

  // Initiate GL Loop
  //interp->process("#include \"glfw/glfw_bindings.h\"");
}
#endif // MIDGE_H