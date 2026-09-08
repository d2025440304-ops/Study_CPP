#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
// int main()
// {
//     cout << "hello word" << endl;
//     return 0;
// }


//命名空间
// 域（命名空间）
// namespace xu
// {
//     int rand = 10; 
// }
// namespace math
// {
//     int pai = 3.14;
// }
// int a = 0;

// int main()
// {
//     int a = 1;
//     printf("%p\n",rand);
//     printf("%d\n",xu::rand);
//     printf("%d\n",a);
//     printf("%d\n",::a);//在：：前没有作用域限定符时，表示全局变量
//     //：：域作用限定符
//     cin >> math::pai;
//     cout << math::pai;
//     return 0;
// }

//命名空间
// namespace x
// {
//     int rand = 10;
    
//     double Add(double a, double b)
//     {
//         return a + b;
//     }

//     struct node
//     {
//         struct node * next;
//         int val;
//     };
// }

// int main()
// {
//     printf("%.2f\n",x::Add(2.6,2.6));
//     struct x::node * p;//node是 x 命名空间中的 node 结构体，定义一个指向 node 结构体的指针 p
//     p->next = NULL;
//     return 0;
// }

// namespace fclass
// {
//     namespace class1
//     {
//         int x = 1;
//         int y = 2;
//         int z = x+y;
//     }
//     namespace class2
//     {
//         int x = 1;
//         int y = 2;
//         int z = x+y;
//     }
// }
// int main()
// {
//     printf("%d\n",fclass::class1::z);
//     printf("%d\n",fclass::class2::z);
// }




// namespace fclass
// {
//     namespace class1
//     {
//         int x = 1;
//         int y = 2;
//         int z = x+y;
//     }
// }
// //全部展开命名空间---在项目中不推荐
// using namespace fclass::class1;
// int main()
// {
//    printf("%d\n",x);
// }


// //域，领域展开
// namespace fclass
// {
//     namespace class1
//     {
//         int x = 1;
//         int y = 2;
//         int z = x+y;
//     }
// }
// //部分展开命名空间---只展开 fclass::class1::x

// using fclass::class1::x;
// int main()
// {
//    printf("%d\n",x);
// }

// #include <iostream>
// //在iostream中的域中
// //istream cin 标准输入流
// //ostream cout 标准输出流
// using namespace std;//std 标准输入输出流
// int main()
// {
//     cout << "hello word" << endl;
//     //<< 流插入操作符 >> 流提取操作符
//     int a;
//     std::cin >> a;
// }

//优化输入输出流
// #include <iostream>
// using namespace std;
// int main()
// {
//     ios::sync_with_stdio(false);
//     cin.tie(0);
//     cout.tie(0);

// }




// #include <iostream>
// using namespace std;

// //缺省参数函数
// int func(int a = 10)
// {
//     return a;
// }

// //有多个参数时，缺省参数必须从右到左，不能间断
// int func1(int a,int b = 1,int c = 2)
// {
//     return a+b+c;
// }
// int main()
// {
//     cout << func() << endl;
//     cout << func(20) << endl;//缺省参数
//     cout << func1(1,2,3) << endl;
//     cout << func1(1,2) << endl;
//     cout << func1(1) << endl;
//     return 0;
// }



// // #include <iostream>
// // using namespace std;
// // typedef struct Stack
// // {
// //     int arr[];
// //     int top;
// //     int capacity;
// // }ST;


// // void InitStack(ST * stack,int capacity)
// // {
// //     stack = (ST *)malloc(sizeof(ST));
// //     stack->arr = (int *)malloc(sizeof(int)*capacity);
// //     stack->top = -1;
// //     stack->capacity = capacity;
// //     return;
// // }
// // void initStack(ST * stack,int capacity = 4);//缺省参数定义和声明不能同时存在
// // int main()
// // {
// //     InitStack(NULL,4);//缺省参数调用
// //     initStack(NULL);
// // }


// //函数重载

// //类型不同
// int Add(int a,int b)
// {
//     return a+b;
// }
// double Add(double a,double b)
// {
//     return a+b;
// }
// //参数个数不同
// int Add(int a,int b,int c)
// {
//     return a+b+c;
// }


// void Swap(int *a,int *b)
// {
//     int * p  = a;
//     a = b;
//     b = p;
// }
// void Swap(const char *a,const char *b)
// {
//     const char * p  = a;
//     a = b;
//     b = p;
// }


// //特殊的重载
// void f1()
// {
//     cout << "f1" << endl;
// }
// void f1(int a = 10)
// {
//     cout << "f1(a)" << a << endl;
// }

// int main()
// {
//     cout << Add(1,2) << endl;
//     cout << Add(1.1,2.2) << endl;
//     // cout << f1() << endl;
//     // cout << f1() << endl;
//     return 0;
// }


// #include <iostream>
// using namespace std;
// int main()
// {
//     int a = 10;
//     // 编 译报 错 ：“ra”: //int& ra;
//     // 必 须 初 始 化 引用
//     int& b = a;
//     int c = 20;
//     // 这里 并非 让b引用 c，
//     // 这里 是一个 赋值
//     b = c;
//     // 因为C++引用 不 能改变指 向 ，
//     cout << &a << endl;
//     cout << &b << endl;
//     cout << &c << endl;
//     cout << a << endl;
//     cout << b << endl;
//     cout << c << endl;
//     return 0;
// }





// #include <iostream>
// using namespace std;

// //函数的重载➕参数引用
// void swap(int &a,int &b)
// {
//     int tmp = a;
//     a = b;
//     b = tmp; 
// }
// void swap(string &a, string &b)
// {
//     string tmp = a;
//     a = b;
//     b = tmp;     
// }
// int& modify(int &a)
// {
//     a = 10;
//     return a;
// }

// //返回局部变量的引用,会警告，相当于返回了一个野指针
// int& func(int a)
// {
//     int a = 10;
//     return a;
// }
// int main()
// {
//     int x = 1,y = 2;
//     string a = "hello",b = "world";
//     swap(x,y);
//     swap(a,b);
//     modify(x) = 100;
//     cout << x << " " << y << endl;
//     cout << a << " " << b << endl;
// }



// int main()
// {
//     const int a = 0;
//     // int &ar = a;//编译会报错，因为引用放大了权限，原 a 只能读不能写，ar 可以写，所以编译器不允许

//     int b = 1;
//     const int & br = b;//可以，但 br 不能改 b 的值，br 只能读不能写。不影响 b 的变化

//     //权限可以缩小，但不能放大

//     const int& a = 10;//常量引用

//     const int& rc = a+b;


//     return 0; 
// } 


 
#define ADD(a,b)  ((a)+(b))
//内联函数，可以提升性能，减少函数调用的开销
inline int Add(int a,int b)
{
    return a+b;
}
int main()
{  
    int ret = ADD(1,2);
    cout << ret << endl; 
    cout << ADD(1,2) * 5 << endl;
    int x = 1,y = 2;
    ADD(x & y,x | y);

    int *p = nullptr;
    if(p == nullptr)
    {
        cout << "p is nullptr" << endl;
    }
    return 0; 
}