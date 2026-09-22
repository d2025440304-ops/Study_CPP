#include <iostream>
using namespace std;


class A{
    public:
    void func()
    {
        cout << "回调成功" <<endl;
    }
};

typedef void (A::*pf) ();//函数指针，指向 A 作用域里一个返回类型 void 无参数的函数

int main()
{
    //void (A::*pf) () = nullptr;
    pf pff = nullptr;
    //C++规定在类里的成员函数必须要加&才能取到函数指针
    pff = &A::func;

    A aa;

    // .*操作符，用来回调类里的函数指针
    (aa.*pff)();
    return 0;
}