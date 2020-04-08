/* main.cpp */

#include <stdlib.h>
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
    "printLine(Hello Universe);"
    "}]"
    "}]"
    "[entry:run:void:(){"
    "instance(Midge, midge)"
    "invoke(midge, print)"
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
    Void,
    Class,
    Int32,
  } kind;
  string name;

  Type(Kind pKind)
  {
    kind = pKind;
    name = "";
  }
};

struct Method
{
  string name;
  vector<string> statements;
  Type *returnType;
};

struct Object
{
  Type type;
  vector<Method *> methods;
  Object() : type(Type(Type::Class)) {}
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

int parseClassMethod(Object *obj)
{
  Method *method = new Method();
  parseMethodDetails(method);
  obj->methods.push_back(method);

  return 0;
}

int parseClassMember(Object *obj)
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

int parseClass()
{
  Object *obj = new Object();
  obj->type = Type(Type::Class);
  obj->type.name = parseIdentifier();

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
    }
    break;
    case 'e':
    {
      // Entry Method
      parsePast("entry:");

      entryMethod = new Method();
      parseMethodDetails(entryMethod);
    }
    break;
    default:
    {
      cout << "parseNext() : unhandled letter=" << peekChar() << endl;
    }
    }
  }
}

class InstancedObject
{
public:
  Object *object;

  map<string, void *> attributes;
};

class HeirarchicalMemory
{
public:
  HeirarchicalMemory *parent;
  map<string, InstancedObject *> data;
  HeirarchicalMemory(HeirarchicalMemory *pParent = nullptr)
      : parent(pParent) {}
};

Method *entryMethod;
map<string, Object *> dataTypes;

void processMethod(HeirarchicalMemory *memory, Method *method);

int main(void)
{
  code.text = fileText;
  code.pos = 0;
  code.length = 122;

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

  HeirarchicalMemory memory;
  processMethod(&memory, entryMethod);

  // cout << "> ";
  // string str;
  // getline(cin, str);

  // if (str.substr(0, 3) != "ca ")
  // {
  //   cout << "error1" << endl;
  //   return 1;
  // }
  // map<string, DataPoint *> data;

  // int a = str.find(' ', 3);
  // int b = str.find(' ', a + 1);

  // if (str.substr(3, a - 3) == "int")
  // {
  //   int *x = new int();
  //   *x = stoi(str.substr(b + 1, str.length() - b - 1));
  //   DataPoint *dp = new DataPoint();
  //   dp->tc = 1;
  //   dp->p = static_cast<void *>(x);
  //   data[str.substr(a + 1, b - a - 1)] = dp;
  //   cout << "int created" << endl;
  // }
  // else
  // {
  //   cout << "error2" << endl;
  //   return 1;
  // }

  // cout << "> ";
  // getline(cin, str);

  // if (str.substr(0, 6) == "print ")
  // {
  //   string s = str.substr(6, str.length() - 6);
  //   cout << s << "(" << data[s]->tc << ")"
  //        << "=" << *(static_cast<int *>(data[s]->p)) << endl;
  //   return 1;
  // }

  // cout << "error3" << endl;
  return 0;
}

void processMethod(HeirarchicalMemory *memory, Method *method)
{
  HeirarchicalMemory localMemory;
  localMemory.parent = memory;


}