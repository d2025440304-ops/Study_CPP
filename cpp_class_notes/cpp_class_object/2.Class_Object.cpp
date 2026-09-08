#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <utility>
using namespace std;
//类的定义
class Stacka
{
    public:
        Stacka()
            : a(nullptr), top(0), capacity(0)
        {
        }

        ~Stacka()
        {
            delete [] a;
        }

        Stacka(const Stacka& other)
            : a(other.capacity > 0 ? new int[other.capacity] : nullptr),
              top(other.top),
              capacity(other.capacity)
        {
            for(int i = 0; i < top; i++)
            {
                a[i] = other.a[i];
            }
        }

        Stacka& operator=(const Stacka& other)
        {
            if(this != &other)
            {
                Stacka temp(other);
                std::swap(a, temp.a);
                std::swap(top, temp.top);
                std::swap(capacity, temp.capacity);
            }
            return *this;
        }

        //void push(Stacka* const this,int &val)
        void push(const int &val)
        {
            if(top == capacity)
            {
                cout << "栈满" << endl;
                return;
            }
            a[top++] = val;
        }
        void pop()
        {
            if(top == 0)
            {
                cout << "栈空" << endl;
                return;
            }
            top--;
        }
        void StackInit(int cap)
        {
            StackDestroy();
            if(cap <= 0)
            {
                return;
            }
            a = new int[cap];
            top = 0;
            capacity = cap;
        }
        void StackDestroy()
        {
            delete [] a;
            a = NULL;
            top = 0;
            capacity = 0;
        }
        int StackTop()
        {
            if(top == 0)
            {
                cout << "栈空" << endl;
                return -1;
            }
            return a[top-1];
        }
        void StackPrint()
        {
            for(int i = 0;i < top;i++)
            {
                cout << a[i] << " ";
            }
            cout << endl;
        }
        void StackClear()
        {
            top = 0;
        }
        void StackSize()
        {
            cout << "栈的大小为：" << top << endl;
        }
    private:
        int *a;
        int top;
        int capacity;
};

//类可以实例化为对象，类是抽象的，类是一个类型，类是一个模板
// class Date
// {
//     public:
//         void DateInit(int year,int month,int day);
//     private:
//     //声明，实例化
//         int _year;
//         int _month;
//         int _day;
// };
//  void Date::DateInit(int year,int month,int day)
//         {
//             _year = year;
//             _month = month;
//             _day = day;
//         }
//class 默认的访问权限是 private
//struct 默认的访问权限是 public

// struct Listnode
// {
//     int val;
//     Listnode *next;//Listnode是类，不需要 struct 关键字
// };






// class Date
// {
//     public:
//         //类的成员函数可以在类的外部定义

//         //this 指针是一个隐式参数，指向调用对应成员函数的对象的指针
//         //void DateInit(Date* const this,int year,int month,int day)
//         void DateInit(int year,int month,int day)
//         {
//             this->_year = year;
//             this->_month = month;
//             this->_day = day;
//         }
//     private:
//     //声明，实例化
//         int _year;
//         int _month;
//         int _day;
// };
        

// int main()
// {
//     Date d1;
//     //d1.DateInit(&d1,2026,9,2);
//     d1.DateInit(2026,9,2);   

//     Date d2;
//     //d2.DateInit(&d2,2026,9,3);
//     d2.DateInit(2026,9,3);

//     return 0;
// }






class stack_
{ 
    public:
    //构造函数
    stack_(int n = 4)//全缺省函数
        : _a(nullptr), _capacity(0), _top(0)
    {
        if(n <= 0)
        {
            return;
        }
        _a = static_cast<int *>(malloc(sizeof(int) * n));
        if( nullptr == _a)
        {
            perror("malloc fail");
            return;
        }
        _capacity = n;
        _top = 0;
    }
    stack_(const stack_&) = delete;
    stack_& operator=(const stack_&) = delete;
    //析构函数：无参数
    ~stack_()
    {
        free(_a);
        _a = nullptr;
        _capacity = _top = 0;
    }

    private:
    int* _a;
    int _capacity;
    int _top;
};



class Date
{
    public:
    // 构造函数：分为无参构造函数，全缺省构造函数，编译器自动生成的构造函数
    // Date()
    // {
    //     _year = 1;
    //     _month = 1;
    //     _day = 1;
    // }

    // Date(int year,int month,int day)
    // {
    //     _year = year;
    //     _month = month;
    //     _day = day;
    // }

    //结构函数自动调用
    Date(int year = 1,int month = 1,int day = 1)
    {
        _year = year;
        _month = month;
        _day = day;
    }
    //不需要析构函数，没有内存的开辟

    int Getyear() const
    {
        return _year;
    }

    int Getmonth() const
    {
        return _month;
    }

    int Getday() const
    {
        return _day;
    }

    //有隐式的 this 指针
    bool operator==(const Date& d2) const
    {
        return _year == d2._year
        && _month == d2._month
        && _day == d2._day;
    }
private:
    //声明，实例化
        int  _year;
        int _month;
        int _day;
};

//因为无法访问 class 内部的成员，所以需要采取一些方案

//方案一，将 class 里的成员改为共有
// bool operator==(Date d1,Date d2)
// {
//     return d1._year == d2._year
//         && d1._month == d2._month
//         && d1._day == d2._day;
// }

//方案二，在 class 里添加获取成员函数

// bool operator==(Date d1,Date d2)
// {
//     return d1.Getyear() == d2.Getyear()
//         && d1.Getmonth() == d2.Getmonth()
//         && d1.Getday() == d2.Getday();
// }

//方案三，直接将运算符重载写进成员函数



int main()
{
    Date d1;
    Date d2(2026,9,2);
    Date d3(2026,9,3);

    cout << (d2 == d3) << endl;

    return 0;
}