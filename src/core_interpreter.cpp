/* core_interpreter.cpp */

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "core_interpreter.h"

using namespace std;

void CoreInterpreter::parsePast(char c)
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

void CoreInterpreter::parsePast(string s)
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

char CoreInterpreter::peekChar()
{
  if (code.pos >= code.length)
    throw 1005;
  return code.text[code.pos];
}

char CoreInterpreter::parseChar()
{
  if (code.pos >= code.length)
    throw 1006;
  return code.text[code.pos++];
}

string CoreInterpreter::parseIdentifier()
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

string CoreInterpreter::parseStatement()
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

void CoreInterpreter::parseRoot()
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
      //cout << "--class parsed" << endl;
    }
    break;
    case 'e':
    {
      // Entry MethodInfo
      parsePast("entry:");

      app->entryMethod = new MethodInfo();
      parseMethodDetails(app->entryMethod);
      //cout << "--entryMethod parsed" << endl;
    }
    break;
    default:
    {
      cout << "parseNext() : unhandled letter=" << peekChar() << endl;
      throw 1306;
    }
    }
  }
}

void CoreInterpreter::parseClass()
{
  ClassDefinition *obj = new ClassDefinition();
  obj->type = Type(DataType::Class);
  obj->type.name = parseIdentifier();
  app->classDefinitions[obj->type.name] = obj;

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
}

int CoreInterpreter::parseClassMember(ClassDefinition *obj)
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

int CoreInterpreter::parseClassMethod(ClassDefinition *obj)
{
  MethodInfo *method = new MethodInfo();
  parseMethodDetails(method);
  obj->methods.push_back(method);

  return 0;
}

int CoreInterpreter::parseMethodDetails(MethodInfo *func)
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
    func->returnType = new Type(DataType::Void);
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

MidgeApp *CoreInterpreter::interpret(const char *text)
{
  code.text = text;
  code.pos = 0;
  code.length = strlen(text);

  app = new MidgeApp();

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
    delete (app);
    app = nullptr;

    throw e;
  }
  return app;
}