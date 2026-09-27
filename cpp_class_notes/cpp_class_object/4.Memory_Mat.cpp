#include <iostream>
using namespace std;

// class A
// {
//     public:
//     void func ()
//     {
//         cout << "yes" << endl;
//     }

//     A(int x = 1): _x(x)
//     {

//     }

//     private:
//     int _x;
// };


// int main()
// {
//     // int * p0  = new int;
//     // int * p1  = new int[10];

//     // delete p0;
//     // delete[] p1;

//     // int * p2 = new int(0);
//     // int * p3 = new int[10]{0};
//     // int * p4 = new int[10]{1,2,3,4,5};

//     // delete p2;
//     // delete[] p3;
//     // delete[] p4;

//new int 可以直接申请类的内存
//     A* p1 = new A();
//     A* p2 = new A(1);



//     return 0;
// }


class A
{
public:
	A(int a1 = 0, int a2 = 0)
		:_a1(a1)
		,_a2(a2)
	{
		cout << "A(int a1 = 0, int a2 = 0)" << endl;
	}

	A(const A& aa)
		:_a1(aa._a1)
	{
		cout << "A(const A& aa)" << endl;
	}

	A& operator=(const A& aa)
	{
		cout << "A& operator=(const A& aa)" << endl;
		if (this != &aa)
		{
			_a1 = aa._a1;
		}
		return *this;
	}

	~A()
	{
		//delete _ptr;
		cout << "~A()" << endl;
	}

	void Print()
	{
		cout << "A::Print->" << _a1 << endl;
	}

	A& operator++()
	{
		_a1 += 100;

		return *this;
	}
private:
	int _a1 = 1;
	int _a2 = 1;
};

//int main()
//{
//	A aa1 = 1;
//	const A& aa2 = 1;
//
//	return 0;
//}

void f1(A aa)
{}

// int main()
// {
// 	A aa1(1);
// 	f1(aa1);
// 	cout << endl;

// 	// Ż
// 	f1(A(1));
// 	cout << endl;

// 	// Ż
// 	f1(1);
// 	cout << endl;

// 	return 0;
// }

A f2()
{
	A aa(1);
	++aa;

	return aa;
}

// int main()
// {
// 	f2().Print();
// 	cout <<"*********"<< endl << endl;

// 	return 0;
// }

//A f2()
//{
//	A aa(1);
//	++aa;
//
//	return aa;
//}


struct Listnode{
    int _val;
    Listnode * next;
    Listnode(int val = 1): _val(val),next(nullptr)
    {

    }
};

// int main()
// {
//     Listnode * p1 = new Listnode();
//     Listnode * p2 = new Listnode(2);
//     Listnode * p3 = new Listnode(3);
//     Listnode * p4 = new Listnode(4);

//     p1->next = p2;
//     p2->next = p3;
//     p3->next = p4;
//     p4->next = nullptr;
// }

void func()
{
	int n = 1;
	while(1)
	{
		void * p1 = new char [1024*1024];
		cout << p1 << "->" << n << endl;
		++n;
	}
} 

int main()
{
	try
	{
		// void * p1 = new char [1024*1024*1024];
		// cout << p1 << endl;
		// void * p2 = new char [1024*1024*1024];
		// cout << p1 << endl;
		// void * p3 = new char [1024*10 24*1024];
		// cout << p1 << endl;
		func();
	}
	catch(const exception & e)
	{
		cout << e.what() << endl;
	}

	return 0;
}