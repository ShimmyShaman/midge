/* main.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <time.h>
#include <vector>
#include <unistd.h>

#include "cling/Interpreter/Interpreter.h"

using namespace std;

cling::Interpreter *clint;

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
      printf("Directory: %*s[%s]\n", indent, "", entry->d_name);
      loadSourceFiles(path, indent + 2);
    }
    else
    {
      if (!strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".h") || !strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".c"))
      {
        char path[1024];
        char *filePath = strcpy(path, name);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);
        clint->loadFile(path);
        printf("LoadedSrc: \"%*s- %s\"\n", indent, "", entry->d_name);
        continue;
      }
      printf("IgnoreSrc: %*s- %s\n", indent, "", entry->d_name);
    }
  }
  closedir(dir);
}

int main(int argc, const char *const *argv)
{
    char buffer[200];
    getcwd(buffer, 200);
    const char *LLVMDIR = "/home/jason/cling/inst";
    clint = new cling::Interpreter(argc, argv, LLVMDIR);

    clint->AddIncludePath("/home/jason/midge/src");
    clint->AddIncludePath("/home/jason/cling/inst/include");

    // Load App source
    printf("<AppSourceLoading>\n");
    loadSourceFiles("/home/jason/midge/src", 0);
    printf("</AppSourceLoading>\n\n");

    clint->process("clint = (cling::Interpreter *)" + std::to_string((long)clint) + ";");

    clint->process("run()");

    delete (clint);
}