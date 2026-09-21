#include <iostream>
#include <cstddef>
#include <assert.h>
using namespace std;

template<typename T>
class Stack{

    public:
    //构造函数：创建一个容量为 cap 的空栈
    //explicit 防止“stack s =4"这种隐式转换

    explicit Stack(size_t cap = 4)
    : _a(cap > 0 ? new T[cap] : nullptr),
    _size(0),
    _capacity(cap)
    {

    }
    //析构函数
    ~Stack()
    {
        delete[] _a;//与 new[] 配对；delete[]nullptr 是安全的
    }

    //拷贝构造
    Stack(const Stack& other)
        :   _a(other._capacity > 0 ? new T [other._capacity] : nullptr)
           ,_size(other._size)
           ,_capacity(other._capacity)
    {
        for(size_t i = 0;i<other._size;i++)
        {
            _a[i] = other._a[i];
        }
    }

    //拷贝赋值
    Stack& operator=(const Stack& other)
    {
        if(this != &other)
        {
            Stack tmp(other);//拷贝构造，用 other 来初始化 tmp，同一块地址
            std::swap(_a,tmp._a);
            std::swap(_size,tmp._size);
            std::swap(_capacity,tmp._capacity);
        }
        return *this;
    }


    void push(const T& val)
    {
        if(_size == _capacity )
        {
            size_t newcap = _capacity== 0 ? 4 : 2 * _capacity;
            T* _aa = new T [newcap];

            for(size_t i = 0;i<_size;i++)
            {
                _aa[i] = _a[i];
            }
            delete[] _a;
            _a = _aa;
            _capacity = newcap;

        }
        _a[_size++] = val;
    }

     size_t size() const
    {
        return _size;
    }

    size_t capacity() const
    {
        return _capacity;
    }

    bool empty() const
    {
        return _size == 0;
    }

    const T& top() const
    {
        assert(_size  > 0);
        return _a[_size-1];
    }

    T& top()
    {
        assert(_size>0);
        return _a[_size-1];
    }

    void pop()
    {
        assert(_size>0);
        _size--;
    }

    void clear()
    {
        _size = 0;
    }

    private:
    T*  _a;
    size_t _size;
    size_t _capacity;
};

int main()
{
    Stack<int> s;
    Stack<int> s2(8);
    cout << "栈创建成功" << endl;

    // 测试 empty()
    cout << "空栈? " << (s.empty() ? "是" : "否") << endl;
    // 期望：是

    // 测试 push()
    s.push(10);
    s.push(20);
    s.push(30);
    cout << "size=" << s.size() << endl;         // 期望：3
    cout << "capacity=" << s.capacity() << endl; // 期望：4
    cout << "空栈? " << (s.empty() ? "是" : "否") << endl;
    // 期望：否

    // 测试 top()
    cout << "栈顶: " << s.top() << endl;         // 期望：30

    // 测试 pop()
    s.pop();
    cout << "pop 后，栈顶: " << s.top() << endl;  // 期望：20
    cout << "size: " << s.size() << endl;         // 期望：2

    // 先测试拷贝（s 此时是非空栈）
    Stack<int> s3 = s;  // 调用拷贝构造
    cout << "拷贝构造后，s3 的栈顶: " << s3.top() << endl;  // 期望：20
    cout << "s3.size: " << s3.size() << endl;              // 期望：2

    // 测试拷贝赋值
    Stack<int> s4;
    s4 = s;  // 调用拷贝赋值
    cout << "拷贝赋值后，s4 的栈顶: " << s4.top() << endl;  // 期望：20
    cout << "s4.size: " << s4.size() << endl;              // 期望：2

    // 验证深拷贝：修改 s，s3 和 s4 应该不受影响
    s.pop();
    cout << "s.pop() 后：" << endl;
    cout << "s.top: " << s.top() << endl;           // 期望：10
    cout << "s3.top (应该不变): " << s3.top() << endl;  // 期望：20
    cout << "s4.top (应该不变): " << s4.top() << endl;  // 期望：20

    // 测试自拷贝
    cout << "\n=== 测试自拷贝 ===" << endl;
    Stack<int> s5;
    s5.push(100);
    s5.push(200);
    cout << "s5.top (拷贝前): " << s5.top() << endl;  // 期望：200

    s5 = s5;  // 自拷贝
    cout << "s5.top (自拷贝后): " << s5.top() << endl;  // 期望：200（应该不变）

    // 最后再 clear
    s.clear();
    cout << "\nclear 后，空栈? " << (s.empty() ? "是" : "否") << endl;  // 期望：是
    cout << "size: " << s.size() << endl;                            // 期望：0
    return 0;
}