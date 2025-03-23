#include "Filter.h"
#include <iostream>

Filter::Filter(int _dim)
{
  data[9] = 1;
  // Initialize all elements to 0, all matricies are 3x3
    for (int i = 0; i < 9; i++) {
        data[i] = 0;
    }
}

int Filter::get(int r, int c)
{
    return data[r * 3 + c];
}

void Filter::set(int r, int c, int value)
{
    data[r * 3 + c] = value;
}

int Filter::getDivisor()
{
    return data[9];
}

void Filter::setDivisor(int value)
{
    data[9] = value;
}

int Filter::getSize()
{
    return 3; // Always 3x3
}

void Filter::info()
{
    cout << "Filter is.." << endl;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            cout << get(row, col) << " ";
        }
        cout << endl;
    }
}
