#include <iostream>
#include <ctype.h>
#include <cassert>
using namespace std;

template <typename T>
void Swap(T &a, T &b)
{
    T tmp  = a;
    a = b;
    b = tmp;
}
int main()
{
    int i = 1,j = 2;
    double x = 2.2,y = 3.3;

    Swap(i,j);
    Swap(x,y);
    return 0;
}
