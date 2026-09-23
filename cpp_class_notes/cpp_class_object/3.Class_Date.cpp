#include <iostream>
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

    // //辅助函数：判断是否是闰年
    // bool isLeapYear()const
    // {
    //     //是 400 的倍数的是闰年，是 4 的倍数但不是 100 的倍数的是闰年
    //     return (_year & 400 == 0) || (_year %4 ==0 && _year %100 != 0);
    // }

    // // 辅助函数：获取某个月的天数
    // int getDaysInMonth() const
    // {
    //     if (_month == 2) 
    //     {
    //         return isLeapYear() ? 29 : 28;
    //     } 
    //     else if(_month == 4 || _month == 6 ||_month == 9 || _month == 11) 
    //     {
    //         return 30;
    //     } 
    //     else 
    //     {
    //         return 31;
    //     }
    // }
    int GetMonthDay(int year, int month)
    {
        assert(month > 0 && month < 13);
        static int monthDayArray[13] = { -1, 31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31 };
            // 365天 5h +
        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year
        % 400 == 0)))
        {
            return 29;
        }
        else
        {
            return monthDayArray[month];    
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

    // 将日期转换为字符串，格式为 YYYY-MM-DD
    string toString() const
    {
        stringstream ss;
        ss << setw(4) << setfill('0') << _year << "-"
           << setw(2) << setfill('0') << _month << "-"
           << setw(2) << setfill('0') << _day;
        return ss.str();
    }

    // 打印日期到控制台
    void print() const
    {
        cout << toString() << endl;
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
    
    Date operator-=(int days) 
    {
        for(int i = 0;i < days; i++)
        {
            this->_day--;
            if(this->_day <1)
            {
                this->_month--;
                if(this->_month <1)
                {
                    this->_year--;
                    this->_year = 12;
                }
            }
            this->_day = GetMonthDay(_year,_month);
        }
        return *this;
    }


    Date operator-(int days) const//const Date * const this
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
                result._day = result.GetMonthDay(_year,_month);
            }
        }
        return result;
    }


    Date operator+=(int days) 
    {
        this->_day += days;
        while(_day > GetMonthDay(_year,_month))
        {
            _day -= GetMonthDay(_year,_month);
            _month++;
            if(_month > 12)
            {
                _year++;
                _month = 1;
            }
        }
        return *this;
    }


    Date operator+(int days)  
    {
        Date result = *this;
        result._day += days;
        while(result._day > GetMonthDay(result._year,result._month))
        {
            result._day -= GetMonthDay(result._year,result._month);
            result._month++;
            if(_month > 12)
            {
                result._year++;
                result._month = 1;
            }
        }
        return result;
    }


        // for(int i = 0;i<days;i++)
        // {
        //     result._day++;
        //     if(result._day > result.getDaysInMonth())
        //     {
        //         result._day = 1;
        //         result._month++;
        //         if(result._month > 12)
        //         {
        //             result._month = 1;
        //             result._year++;
        //         }
        //     }
        // }

private:
    //声明，实例化
        int  _year;
        int _month;
        int _day;
};

void test1()
{
    Date d1(2022,9,1);
    Date d2 = d1+15;
    d2 += 1;
    // 测试print函数
    cout << "测试print函数：" << endl;
    cout << "d1: ";
    d1.print();  // 应该输出 2022-09-01
    cout << "d2: ";
    d2.print();  // 应该输出 2022-09-16

    // 测试边界情况
    Date d3(2024,2,29);  // 闰年
    Date d4(2023,1,1);   // 普通日期
    cout << "闰年2月29日: ";
    d3.print();
    cout << "普通日期: ";
    d4.print();
}

void testPrintFunction()
{
    cout << "\n=== 测试print函数 ===" << endl;

    // 测试不同日期
    Date dates[] = {
        Date(2023, 1, 1),    // 2023-01-01
        Date(2023, 12, 31),  // 2023-12-31
        Date(2024, 2, 29),   // 闰年
        Date(2023, 2, 28),   // 平年
        Date(2024, 10, 8),   // 当前日期附近
        Date(1, 1, 1)        // 最小日期
    };

    for (int i = 0; i < sizeof(dates)/sizeof(dates[0]); i++) {
        cout << "日期" << i+1 << ": ";
        dates[i].print();
    }
}
 

int main()
{
    test1();
    testPrintFunction();
    return 0;
}