/* midge.h */

#ifndef MIDGE_H
#define MIDGE_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/Transaction.h"

static cling::Interpreter *clint;

// std::vector<cling::Transaction *> transactions;
std::map<std::string, cling::Transaction *> definedTypes;
extern "C" void defineType(std::string typeName, std::string definition)
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

void loadSourceFiles(const char *name, int indent)
{
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
    return;

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_DIR)
    {
      if (!indent && !strcmp(entry->d_name, "main"))
      {
        printf("IgnoreDir: %*s[%s]\n", indent, "", entry->d_name);
        continue;
      }
      char path[1024];
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
      printf("%*s[%s]\n", indent, "", entry->d_name);
      loadSourceFiles(path, indent + 2);
    }
    else
    {
      if (!strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".h") || !strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".c"))
      {
        printf("LoadedSrc: %*s- %s\n", indent, "", entry->d_name);
        continue;
      }
      printf("IgnoreSrc: %*s- %s\n", indent, "", entry->d_name);
    }
  }
  closedir(dir);
}

extern "C" void run()
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
    clint->loadLibrary("vulkan");

    clint->loadFile("vulkanExp.c");

    //clint->process("beginRenderThread();");

    loadSourceFiles("/home/jason/midge/src", 0);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}
#endif // MIDGE_H