/* core_interpreter.cpp */

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "core_interpreter.h"

using namespace std;

bool CoreInterpreter::update_core(const char *pCommandFilename, int pBuildNumber)
{
  int loaded_command_count = 0;
  size_t loaded_hash;
  hash<string> hasher;
  const char *filename = "word";
  filename = "eon";

  // Determine command count & hash
  fstream command_file;
  command_file.open(pCommandFilename, ios::in);
  if (command_file.is_open())
  {
    string tp;
    while (std::getline(command_file, tp))
    {
      if (tp == "word")
      {
      }
      cout << tp << "\n";
      ++loaded_command_count;
      loaded_hash *= hasher(tp);
    }
    command_file.close();
  }

  if (CoreInterpreter::PROCESSED_COMMAND_COUNT == loaded_command_count && CoreInterpreter::PROCESSED_HASH == loaded_hash)
    return false;

  // Redo the source
  // Redo this base executable
  // ~ main.cpp ~
  command_file.open("main.cpp", fstream::out);
  command_file << "/* main.cpp */" << endl;
  command_file << endl;
  command_file << "#include <stdlib.h>" << endl;
  command_file << "#include <iostream>" << endl;
  command_file << "#include <fstream>" << endl;
  command_file << "#include <string>" << endl;
  command_file << "#include <vector>" << endl;
  command_file << endl;
  command_file << "#include \"midge.h\"" << endl;
  command_file << endl;
  command_file << "using namespace std;" << endl;
  command_file << endl;
  command_file << "const int BUILD_NUMBER = " << std::to_string(pBuildNumber + 1) << ";" << endl;
  command_file << endl;
  command_file << "int main(void) {" << endl;
  command_file << "  CoreInterpreter interpret;" << endl;
  command_file << "  if (interpret.update_core(\"mdmd.txt\", BUILD_NUMBER))" << endl;
  command_file << "  {" << endl;
  command_file << "    cout << \"Executable updated to build:\" << BUILD_NUMBER + 1 << endl;" << endl;
  command_file << "    return 0" << endl;
  command_file << "  }" << endl;
  command_file << endl;
  command_file << "  Midge midge;" << endl;
  command_file << "  return midge.run();" << endl;
  command_file << "}" << endl;
  command_file.close();

  // ~core_interpreter.h~
  command_file << "/* core_interpreter.h */" << endl;
  command_file << endl;
  command_file << "#ifndef CORE_INTERPRETER_H" << endl;
  command_file << "#define CORE_INTERPRETER_H" << endl;
  command_file << endl;
  command_file << "class CoreInterpreter" << endl;
  command_file << "{" << endl;
  command_file << "public:" << endl;
  command_file << "  static const int PROCESSED_COMMAND_COUNT = " << std::to_string(loaded_command_count) << ";" << endl;
  command_file << "  static const int PROCESSED_HASH = " << std::to_string(loaded_hash) << ";" << endl;
  command_file << endl;
  command_file << "bool update_core(const char *pCommandFilename, int pBuildNumber);" << endl;
  command_file << "};" << endl;
  command_file << endl;
  command_file << "#endif // CORE_INTERPRETER_H" << endl;

  // ~ midge.h ~

  // ~ midge.cpp ~

  // ~ run.sh ~

  // Compile recreated source
  return true;
}