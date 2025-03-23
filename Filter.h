//-*-c++-*-
#ifndef _Filter_h_
#define _Filter_h_

using namespace std;

class Filter {
  //all data stored in 1 array, data[9] is divisor, makes sure memory is grouped idk if its any faster that way
  int data[10]; // Fixed 3x3 size

public:
  Filter(int _dim);
  int get(int r, int c);
  void set(int r, int c, int value);
  int getDivisor();
  void setDivisor(int value);
  int getSize();
  void info();
};

#endif
