#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
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
int main()
{
    test1();
    test2();
    return 0;
}