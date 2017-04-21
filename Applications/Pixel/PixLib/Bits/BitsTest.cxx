#include "Bits.h"

#include <iostream>

using namespace std;
using namespace PixLib;

int main() {
  Bits a(20);
  Bits b("3x0123456789");
  Bits c;

  a = a + b + Bits("2x1122");
  a |= Bits("0xfff");
  cout << a << endl;
  cout << ~a << endl;

  c = Bits("0xff00ff00ff00ff");
  if (c.divide(4)) cout << c << endl;

  Bits d(4);
  d[0] = true;
  d[2] = true;
  cout << d << endl;

  Bits e(4);
  e[0] = d[2];
  cout << e << endl;
  return 0;
}

