#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <list>
using namespace std;

string s1;



// int main()
// {
//     string s1;
//     string s2("hello_world");
//     string s3(s2);

//     cout << "s2:" << s2 << endl;
//     cout << "s3:" << s3 << endl;

//     //  截断拷贝，制定拷贝字符串，制定长度
//     //  npos 表示 int最大值 42 亿 9 千万
//     string s4(s2,6,string::npos);

//     cout << "s4:" << s4 << endl;

    
//     string s5("hello_world",5);
//     cout << "s5:" << s5 << endl;

//     string s6(10,'x');
//     cout << "s6:" << s6 << endl;

//     return 0;
// }




 ///  --------------------------auto---------------------------------
 std::map<std::string,std::string> dict 
    = 
 {{"apple","苹果"},{"orange","橙子"},{"pear","梨"}};    


void test1()
{

    auto it = dict.begin();
    while(it != dict.end())
    {
        cout << it->first << ":" << it->second << endl;
        it++;
    }
}


// ---------------------------范围 for----------------------------------

typedef std::map<std::string,std::string>::iterator map_it;
void test2()
{
    int array[] = {1,2,3,4,5};
    // C++98的遍历
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
    {
        array[i] *= 2;
    }
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
    {
        cout << array[i] << " ";
    }

    // C++11 遍历
    for(auto& x:array)
    {
        x *= 2;
    }

    for(auto x:array)
    {
        cout << x << " ";
    }

    string str  = "hello_world";
    for(auto ch: str)
    {
        cout << ch << " ";
    }
    cout << endl;
}

void test3()
{
    
    string s1("hello_world");
    s1[5] = 'x';
    for(int i = 0;i<s1.size();i++)
    {
        cout << s1[i] << " " ;
    }
    cout << endl;

    //字符赋值，自动迭代，自动判断结束
    for(auto e: s1)
    {
        //赋值给 e,再打印 e
        cout << e << " ";
    }
    cout << endl;

    for(auto it = s1.begin();it<s1.end();it++)
    {
        cout << *it << " ";
    }
    cout << endl;

    list<int> lt = {1,2,3,4,5,6,7};
    auto iit = lt.begin();
    list<int>::iterator it  = lt.begin();
    while(it != lt.end())
    {
        cout << *it << " ";
        ++it;
    }
    cout << endl;
}

void test4()
{
    int a = 1;
    char b = 'x';
    double x = 1.1;
    auto c = 3.14;

    cout << typeid(a).name() << endl;
    cout << typeid(b).name() << endl;
    cout << typeid(x).name() << endl;
    cout << typeid(c).name() << endl;
}


void test5()
{
    string s1("hello_world");
    for(auto it = s1.begin();it<s1.end();it++)
    {
        cout << *it << " ";
    }
    cout << endl;


    string::reverse_iterator itt = s1.rbegin();
    while(itt != s1.rend())
    {
        cout << *itt << endl;
        itt++; 
    }
    cout << endl;
}

void test6_string()
{
    string s1("hello_world");
    cout << s1 << endl;
    cout << s1.size() << endl;
    cout << s1.length() << endl;
    cout << s1.capacity() << endl;
    cout << s1.max_size() << endl;// 42亿9千万
    cout << s1.empty() << endl;// 0 false 1 true
    cout << s1[0] << endl;// 直接访问，越界不报错
    cout << s1.at(0) << endl;// 越界报错，抛出异常
    cout << s1.front() << endl;// 访问第一个元素
    cout << s1.back() << endl;// 访问最后一个元素
    cout << s1.c_str() << endl;// 以C风格的字符串形式返回
    cout << s1.data() << endl;// 以C风格的字符串形式返回
    cout << s1.find("world") << endl;// 查找子串,返回下标
    cout << s1.find("world",0) << endl;// 查找子串，从指定位置开始查找，返回下标
    cout << s1.find("world",0,3) << endl;// 查找子串，从指定位置开始查找，查找指定长度的子串，返回下标
    cout << s1.find("world",0,5) << endl;// 查找子串，从指定位置开始查找，查找指定长度的子串，返回下标

}


void test7_string()
{
    //reserve 保留，预留
    //reverse 反转，逆置
    string s1("hello_world");
    cout << s1 << endl;

    s1.reserve(100);//预留空间，避免频繁扩容，之后的插入不会再引起容量的变化
    
    size_t sz = s1.capacity();
    cout << "capacity change: " << sz << endl;

    cout << "making string bigger" << endl;
    for(int i = 0;i< 100;i++)
    {
        s1.push_back('x');
        if(sz != s1.capacity())
        {
            sz = s1.capacity();
            cout << "capacity change: " << sz << endl;
        }
    }
}


void test8_string()
{
    string s1("hello_world");
    cout << s1 << endl;

    s1.push_back(' ');
    s1.push_back('x');

    s1.append("yz");
    cou << s1 << endl;
}

int main() 
{
        // test1();
    // test2(); 
    // test3();
    // test4();
    // test5();
    // test6_string();
    test7_string();

    int *arr = new int[10]{1,2,3,4,5,6,7,8,9,10};
    return 0;
}   