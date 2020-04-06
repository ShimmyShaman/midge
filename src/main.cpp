/* main.cpp */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

using namespace std;

const int BUILD_NUMBER = 0;

class DataPoint
{
public:
  int tc;
  void *p;
};

int main(void)
{
  

  cout << "> ";
  string str;
  getline(cin, str);

  if (str.substr(0, 3) != "ca ")
  {
    cout << "error1" << endl;
    return 1;
  }
  map<string, DataPoint *> data;

  int a = str.find(' ', 3);
  int b = str.find(' ', a + 1);

  if (str.substr(3, a - 3) == "int")
  {
    int *x = new int();
    *x = stoi(str.substr(b + 1, str.length() - b - 1));
    DataPoint *dp = new DataPoint();
    dp->tc = 1;
    dp->p = static_cast<void *>(x);
    data[str.substr(a + 1, b - a - 1)] = dp;
    cout << "int created" << endl;
  }
  else
  {
    cout << "error2" << endl;
    return 1;
  }

  cout << "> ";
  getline(cin, str);

  if (str.substr(0, 6) == "print ")
  {
    string s = str.substr(6, str.length() - 6);
    cout << s << "(" << data[s]->tc << ")"
         << "=" << *(static_cast<int *>(data[s]->p)) << endl;
    return 1;
  }

  cout << "error3" << endl;
  return 0;
}