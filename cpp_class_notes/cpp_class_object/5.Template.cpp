#include <iostream>
#include <ctype.h>
#include <cassert>
#include <algorithm>
using namespace std;

// template <typename T>
// void Swap(T &a, T &b)
// {
//     T tmp  = a;
//     a = b;
//     b = tmp;
// }




//  -------------------------------------------------函数模板------------------------------------------


// //用函数模板生成
// template <typename T>
// //必须加 const,因为如果后面的参数是一个常量引用类型的对象，防止在函数中修改传入的对象
// //加 const 之后，即使后面的参数是一个右值，也不会报错出 bug
// T Add(const T & a,const T &b)
// {
//     return a + b;   // 修正：原写 rturn，拼写错误
// }

// template <typename T1,typename T2>
// T2 Add( const T1 & a,const T2 &b)
// {
//     return a + b;
// }


// template <typename T>
// T* func1(int n)
// {
//     return new T[n];//动态分配内存，给T 类型的数组在堆上开n 个空间
// }
// int main()
// {
//     int i = 1,j = 2;
//     double x = 2.2,y = 3.3;

//     cout << "i+j=" << Add(i,j) << endl;
//     cout << "x+y=" << Add(x,y) << endl;

//     //推导实例化
//     cout << "i+x=" << Add(i,(int)x) << endl;
//     cout << "x+i=" << Add((double)i,x) << endl;

//     //显示实例化
//     cout << "i+j=" << Add<double>(i,x) << endl;//隐式类型转换
//     cout << "x+i=" << Add<int>(x,i) << endl;
//     cout << "i+x=" << Add<int,double>(i,x) << endl;
//     // Swap(i,j);
//     // Swap(x,y);

//     // func1(10);//不知道T 的类型，所以必须显示实例化、


//     //告诉编译器T 的类型是 double,在堆上开辟10 个 double 类型的空间
//     double * p1 = func1<double>(10);
//     delete [] p1;


//     return 0;
// }




//  -------------------------------------------------类模板------------------------------------------




template <typename T>
class Stack
{
    public:
        // 构造函数：申请 _capacity 个 T 类型的堆空间
        Stack(int capacity = 10):
            _array(capacity > 0 ? new T[capacity] : nullptr),  // capacity <= 0 时给 nullptr，保命
            _size(0),
            _capacity(capacity)
        {
            assert(capacity > 0);
        }

        // 析构函数：释放堆空间
        ~Stack()
        {
            delete[] _array;   // delete[] nullptr 是合法空操作，安全
            _array = nullptr;
            _capacity = 0;
            _size = 0;
        }

        //深拷贝构造：新开一块自己的内存，逐元素复制
        //修正1：初始化列表必须用 other._capacity，不能用 _capacity（此时 _capacity 还没初始化）
        //修正2：_size 和 _capacity 必须从 other 拷贝，不能初始化为 0
        Stack(const Stack & other)
        :_array(other._capacity > 0 ? new T[other._capacity] : nullptr)
        ,_size(other._size)       // 修正：原写 _size(0)，应该拷贝 other._size
        ,_capacity(other._capacity) // 修正：原写 _capacity(0)，应该拷贝 other._capacity
        {
            //修正3：other 是引用不是指针，用 . 不用 ->
            for(int i = 0; i < other._size; i++)  // 修正4：原写 other.size()，没有这个方法
            {
                this->_array[i] = other._array[i];  // 修正：原写 other->_array[i]
            }
        }

        //拷贝赋值(copy-and-swap)：将 other 拷贝给新定义的栈
        Stack & operator=(const Stack & other)
        {
            if(this != &other)
            {
                Stack tmp(other);   // 先用拷贝构造造一个副本
                //修正5：原写 _a，类中成员名是 _array
                swap(_array, tmp._array);
                swap(_size, tmp._size);
                swap(_capacity, tmp._capacity);
                // tmp 析构时会自动释放旧内存，无需手动 delete
            }
            return *this;   // 修正6：原缺少 return，operator= 必须返回 *this 以支持链式赋值
        }
    private:
        T * _array;
        int _size;
        int _capacity;
};  // 修正7：类定义末尾必须加分号
