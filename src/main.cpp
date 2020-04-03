/* main.cpp */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "midge.h"

using namespace std;

const int BUILD_NUMBER = 0;

const int PROCESSED_COMMAND_COUNT = 0;
const int PROCESSED_HASH = 0;

void init();
void process_commands();

int main(void)
{
  initialize();
  process_commands();

  Midge midge;
  return midge.run();
}

void initialize()
{
  // Empty
}

void process_commands()
{
  int loaded_command_count = 0;
  size_t loaded_hash;
  hash<string> hasher;
  fstream command_file;

  // Determine command count & hash
  command_file.open("mcmd.txt", ios::in);
  if (command_file.is_open())
  {
    string tp;
    while (std::getline(command_file, tp))
    {
      cout << tp << "\n";
      ++loaded_command_count;
      loaded_hash *= hasher(tp);
    }
    command_file.close();
  }

  if (PROCESSED_COMMAND_COUNT == loaded_command_count && PROCESSED_HASH == loaded_hash)
    return;

  // Redo this base executable
  // ~ main.cpp ~
  command_file.open("mcmd.txt", fstream::out);
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
  command_file << "const int BUILD_NUMBER = " << std::to_string(BUILD_NUMBER + 1) << ";" << endl;
  command_file << endl;
  command_file << "const int PROCESSED_COMMAND_COUNT = " << std::to_string(loaded_command_count) << ";" << endl;
  command_file << "const int PROCESSED_HASH = " << std::to_string(loaded_hash) << ";" << endl;
  command_file << endl;
  command_file << "void initialize();" << endl;
  command_file << "void process_commands();" << endl;
  command_file << endl;
  command_file << "int main(void) {" << endl;
  command_file << "  initialize();" << endl;
  command_file << "  process_commands();" << endl;
  command_file << endl;
  command_file << "  Midge midge;" << endl;
  command_file << "  return midge.run();" << endl;
  command_file << "}" << endl;

  command_file.close();

  // ~ midge.h ~

  // ~ midge.cpp ~

  // ~ run.sh ~

  // Compile recreated source

  // Execute newly created base app. Close this one.
}