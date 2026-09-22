#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
using namespace std;
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

    //构造函数自动调用
    //类的默认成员函数
    Date(int year = 1,int month = 1,int day = 1)
    {
        if(month < 1 || month > 12 || day < 1 || day > 31)
        {
            _year = 1;
            _month = 1;
            _day = 1;
        }
        _year = year;
        _month = month;
        _day = day;
    }
    //不需要析构函数，没有内存的开辟

    //拷贝构造函数，拷贝构造函数的参数是一个引用类型的对象
    //拷贝构造函数的参数是一个常量引用类型的对象
    //防止在拷贝构造函数中修改传入的对象
    Date(const Date& d)
    {
        _year = d._year;
        _month = d._month;
        _day = d._day;
    }

    //析构运算符
    ~Date() = default;

    //赋值运算符
    Date& operator=(const Date&) = default;

    //辅助函数：判断是否是闰年
    bool isLeapYear()const
    {
        //是 400 的倍数的是闰年，是 4 的倍数但不是 100 的倍数的是闰年
        return (_year & 400 == 0) || (_year %4 ==0 && _year %100 != 0);
    }

    // 辅助函数：获取某个月的天数
    int getDaysInMonth() const
    {
        if (_month == 2) 
        {
            return isLeapYear() ? 29 : 28;
        } 
        else if(_month == 4 || _month == 6 ||_month == 9 || _month == 11) 
        {
            return 30;
        } 
        else 
        {
            return 31;
        }
    }

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
    // ================== 比较运算符 ==================
    
    //有隐式的 this 指针
    bool operator==(const Date& other) const
    {
        return this->_year == other._year
        && this->_month == other._month
        && this->_day == other._day;
    }

    bool operator!=(const Date& other) const{
        return !(*this == other);
    }

    int operator-(const Date&other)  const
    {
        return _year - other._year;
    }

    Date operator-(int days) const
    {
        Date result = *this;

        for(int i = 0;i < days ; i++)
        {
            result._day--;
            if(result._day <1)
            {
                result._month--;
                if(result._month < 1)
                {
                    result._year--;
                    result._month = 12;
                }
                result._day = result.getDaysInMonth();
            }
        }
        return result;
    }

    Date operator+(int days) const
    {
        Date result = *this;

        for(int i = 0;i<days;i++)
        {
            result._day++;
            if(result._day > result.getDaysInMonth())
            {
                result._day = 1;
                result._month++;
                if(result._month > 12)
                {
                    result._month = 1;
                    result._year++;
                }
            }
        }

        return result;
    }

private:
    //声明，实例化
        int  _year;
        int _month;
        int _day;
};
