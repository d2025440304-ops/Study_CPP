# C++ 类与对象：从"能跑"到"能上工程"

> **适合读者**：学过 C 语言、正在学 C++ 类和对象的同学。
> **本文目标**：回答一个问题 —— 一个类从"能跑"到"能上工程"，中间要过哪几关？
> **可信度声明**：文中所有代码都在 `g++/clang++ -std=c++17` 下实际编译运行过，报错信息、崩溃现场、内存报告全部是真实输出，原样贴出。

@[toc]

---

## 〇、先看一个"能跑"的类

很多同学（包括当年的我）对"类"的第一个直觉是：**把数据和函数包在一起，能跑就行**。

于是写出来的栈长这样：

```cpp
class MyStackRaw
{
public:
    MyStackRaw() : a(nullptr), top(0), capacity(0) {}
    ~MyStackRaw() { delete[] a; }

    void StackInit(int cap)          // 手动初始化
    {
        StackDestroy();
        if (cap <= 0) return;
        a = new int[cap];
        top = 0;
        capacity = cap;
    }
    void StackDestroy()              // 手动销毁
    {
        delete[] a;
        a = nullptr;
        top = 0;
        capacity = 0;
    }

    void push(int val)
    {
        if (top == capacity) { cout << "栈满" << endl; return; }   // 满了？打印一句就完事
        a[top++] = val;
    }

    int StackTop()
    {
        if (top == 0) { cout << "栈空" << endl; return -1; }        // 空了？返回 -1 当错误码
        return a[top - 1];
    }

private:
    int* a;
    int  top;
    int  capacity;
};
```

这段代码**能编译、能运行、能在作业里拿分**。但它离"工程代码"还差得远 —— 不是风格问题，是会出事故。我把它的三个典型场景跑了一遍：

```
== 现象 1：没初始化就 push ==
栈满                              ← 对象明明是空的，push 却打印"栈满"
push 之后 size 是多少？答案：0      ← 数据一个都没进去，调用者浑然不知

== 现象 2：-1 魔法值 ==
栈顶真的存了 -1，StackTop 返回：-1  ← 调用者无法区分"栈顶是 -1"和"栈空"
```

第三个现象不用跑就知道：`StackInit` / `StackDestroy` 这种**二段式初始化**，只要中途任何一条路径提前 `return`，`StackDestroy` 就被跳过，内存直接泄漏。

**问题出在哪？** 出在把 C 语言的思维原样搬进了 C++：

| C 的思维 | C++ 的答案 |
|---|---|
| 手动 `Init` / `Destroy` | 构造函数 / 析构函数（自动调用，退不掉的） |
| 用 `-1`、`NULL` 当错误码 | 返回值（`bool`）/ `std::optional` / 异常 |
| 打印一句话报告失败 | 让调用者**必须**面对失败 |
| 只管"能跑" | 还要管拷贝、赋值、扩容、异常 |

所以这篇文章就沿着这条线，把"能跑"的类一步步改成"能上工程"的类。改完的最终形态，是一个带**构造函数、析构函数、深拷贝、拷贝赋值、operator 重载、模板、异常安struc 整版本。

---

## 一、类的基础：封装

### 1.1 `class` 和 `struct` 的区别

一句话：**默认访问权限不同**。

```cpp
class A
{
    int x;      // 默认 private：类外访问不了
};

struct B
{
    int y;      // 默认 public：类外随便访问
};
```

其他能力（成员函数、构造析构、继承）两者完全等价。工程上通常这样分工：

- **`struct`**：纯数据聚合，比如链表结点、坐标点 —— 成员本来就是公开的；
- **`class`**：有封装需求的类型 —— 对外只暴露接口，数据藏起来。

### 1.2 访问限定符管的是"一段"，不是"一行"

```cpp
class A
{
    public:
        int x;      // public
        int y;      // public
    private:
        int z;      // private
        int w;      // private
struck    // 后面没写限定符了 → 仍然是 private
};
```

`public:` / `private:` 的作用范围是**从它开始、到下一个限定符之前**的所有成员。新手常见错误是以为它只修饰紧跟着的那一行。

### 1.3 封装到底封什么

```cpp
// 数据成员全公开的后果
MyStackRaw s;
s.top = 999;        // 外部随意篡改内部状态
s.capacity = -1;    // "容量为 -1 的栈"——后续所有逻辑全废
```

封装的意义是把**"能做什么"（接口）**和**"怎么做的"（实现）**分开：

| | public（对外） | private（对内） |
|---|---|---|
| 内容 | `push` / `pop` / `top` | `_a` / `_size` / `_capacity` |
| 意义 | **契约**：我保证提供这些能力 | **机密**：随时可以换实现 |
| 类比 | 遥控器上的按钮 | 电视内部的电路板 |

它有非常实在的回报：**只要接口不变，实现随便换**。

```
今天： int* _a; int _size; int _capacity;    // 裸指针
明天： std::vector<int> _v;                  // 换容器
调用方：s.push(1); s.pop();                    // 一个字都不用改 ✅
```

本文第九章把"手写指针版"重构成"模板版"、第十一章再重构成"vector 版"，靠的就是封装这层隔离。

---

## 二、this 指针

### 2.1 它从哪来

写成员函数时我们从来没写过 `this`，但它一直都在：

```cpp
class Date
{
public:
    void init(int year, int month, int day)
    {
        _year  = year;      // 等价于 this->_year = year;
        _month = month;
        _day   = day;
    }
private:
    int _year, _month, _day;
};
```

**编译器眼中真实的形态**是：

```cpp
void init(Date* const this, int year, int month, int day)
//        ↑ 指向的内容可以改，但 this 本身（指向哪个对象）不能改
```

### 2.2 为什么需要它

`d1.init(...)` 和 `d2.init(...)` 调用的是**同一份代码**，函数怎么知道该改哪个对象的 `_year`？答案就是调用时被隐式传入的 this：

```cpp
d1.init(2026, 9, 2);   // 编译器悄悄转成 init(&d1, 2026, 9, 2)
d2.init(2026, 9, 3);   // 编译器悄悄转成 init(&d2, 2026, 9, 3)
```

```
        ┌──────────────┐
   d1   │  2026  9  2  │ ← this 指向这里
        └──────────────┘
        ┌──────────────┐
   d2   │  2026  9  3  │ ← this 指向这里
        └──────────────┘
              ↑
        [ 同一份 init 代码 ]
```

### 2.3 三个要点

1. **`this` 只能在非静态成员函数里用**（静态成员函数没有 this）；
2. **不能显式定义 `this`** —— 它是编译器传的，不是普通变量；
3. **`*this` 就是对象本身**。`return *this;` 的意思是"返回当前对象"，这是后面 `operator=` 支持链式赋值的原理：

```cpp
a = b = c;
// a.operator=( b.operator=(c) );
//                 ↑ 返回 *this，才能接着赋给 a
```

> **一个前瞻**：`this` 的本质是"每个成员函数都带一个隐藏的对象指针"。
> 那这个指针本身能不能像普通函数指针一样存起来、传出去？
> 能 —— 那就是**成员函数指针**，本文第十三章有完整的演示。

---

## 三、六个默认成员函数

"默认成员函数"指的是**你不写、编译器也会自动生成**的成员函数。C++98 时代是 6 个：

| # | 函数 | 签名（以 `MyStack` 为例） | 作用 | 编译器默认行为 |
|---|---|---|---|---|
| 1 | 构造函数 | `MyStack()` | 初始化对象 | 逐成员默认初始化（**内置类型不初始化！**） |
| 2 | 析构函数 | `~MyStack()` | 清理资源 | 什么也不做 |
| 3 | **拷贝构造** | `MyStack(const MyStack&)` | 用已有对象**生**出新对象 | **逐成员浅拷贝** ⚠️ |
| 4 | **拷贝赋值重载** | `MyStack& operator=(const MyStack&)` | 两个**已存在**对象间赋值 | **逐成员浅拷贝** ⚠️ |
| 5 | 取地址重载 | `MyStack* operator&()` | 取普通对象地址 | 返回 this |
| 6 | const 取地址重载 | `const MyStack* operator&() const` | 取 const 对象地址 | 返回 this |

第 5、6 个几乎没人会重载，知道有这回事即可。

**C++11 又加了 2 个**（第十三章展开）：

| # | 函数 | 签名 |
|---|---|---|
| 7 | 移动构造 | `MyStack(MyStack&&)` |
| 8 | 移动赋值重载 | `MyStack& operator=(MyStack&&)` |

### 划重点：出问题的永远是第 3、4 个

第 1、5、6 个编译器生成的版本**基本不会出问题**。

**出问题的永远是第 3、4 个** —— 因为它们默认做的是"**逐成员浅拷贝**"，而浅拷贝遇到指针成员就是灾难：

```
一个类有指针成员（管理着堆内存）
        │
        ├── 需要"复制"语义（像 int 一样能拷贝）
        │       → 自己写深拷贝（第六、七章）      【本文主线：MyStack】
        │
        └── 资源天然独占，不该被复制
                → 用 = delete 明确禁止拷贝         【第十一章：文件句柄式资源】
```

这条岔路，是整个"类与对象"里最重要的一个决策。

---

## 四、构造函数

### 4.1 职责与特征

- **函数名与类名相同**，没有返回值（连 `void` 都不能写）
- **对象实例化时自动调用**，且一生只调用一次
- 可以重载（多个构造函数并存）

### 4.2 三种形式与"默认构造函数"

```cpp
class Date
{
public:
    Date() {}                                    // ① 无参构造
    Date(int y, int m, int d) { /* ... */ }      // ② 带参构造
    Date(int y = 1, int m = 1, int d = 1) { /* ... */ }   // ③ 全缺省构造（一顶两个）
};
```

**不用传参就能调用的构造函数**都叫"默认构造函数"，来源有三种：自己写的无参版、自己写的全缺省版、自己一个都不写（编译器生成）。

> ⚠️ **坑 1**：① 和 ③ 不能同时存在 —— `Date d;` 既匹配①也匹配③ → **二义性，编译报错**。
> ⚠️ **坑 2**：编译器自动生成的默认构造，对 `int`/`double` 等**内置类型成员不做初始化**，值是随机的。
> ```cpp
> class A { int x; };     // 忘了写构造函数
> A a;                    // a.x 是随机值，后面所有逻辑都建立在垃圾上
> ```
> **指针成员一定要在构造函数里初始化** —— 这是铁律。
> 因为析构函数迟早会 `delete[] 指针`，野指针必崩，而 `nullptr` 是安全的（见 5.3）。

### 4.3 初始化列表：推荐的写法

```cpp
class MyStack
{
public:
    explicit MyStack(int cap = 4)
        : _a(cap > 0 ? new int[cap] : nullptr),   // ← 初始化列表
          _size(0),
          _capacity(cap > 0 ? cap : 0)
    {
        // 函数体里再写 _size = 0; 就是"赋值"了：先默认初始化、再赋值，两次动作
    }
private:
    int* _a;
    int  _size;
    int  _capacity;
};
```

**有些成员只能在初始化列表里初始化**，写进函数体直接编译报错：

```cpp
class Const
{
public:
    // ❌ 错误：const 成员和引用成员必须初始化
    // Const(int n) { _x = n; _ref = n; }

    // ✅ 正确
    Const(int n) : _x(n), _ref(n) {}

private:
    const int _x;      // ① const 成员
    int& _ref;         // ② 引用成员
    // ③ 没有默认构造函数的自定义类型成员，也必须走初始化列表
};
```

即使不报错，两者也有本质区别：

| | 初始化列表 | 函数体内赋值 |
|---|---|---|
| 动作 | **一次**：直接构造 | **两次**：先默认构造，再赋值 |
| 效率 | 高 | 低（对自定义类型成员） |
| 能力 | 能初始化一切成员 | 只能赋值 |

### 4.4 顺序陷阱

**成员的初始化顺序 = 在类里声明的顺序，而不是初始化列表里写的顺序！**

```cpp
class Bad
{
public:
    // 初始化列表写的是 b(a)、a(10)
    // 实际执行顺序是 a(10) → b(a)，因为声明顺序 a 在前
    Bad() : b(a), a(10) {}      // ⚠️ b 拿到的 a 是未初始化的！未定义行为

private:
    int a;      // ← 先声明
    int b;      // ← 后声明
};
```

编译器会给出 `-Wreorder` 警告。**养成"初始化列表顺序 = 声明顺序"的习惯**，本文的 `MyStack` 就是顺序一致的：

```cpp
    explicit MyStack(int cap = 4)
        : _a(...), _size(0), _capacity(...)      // 顺序与声明一致 ✅

private:
    int* _a;        // 声明顺序：_a
    int  _size;     //            _size
    int  _capacity; //            _capacity
```

### 4.5 `explicit`：拦住"隐式类型转换"

单参数（含全缺省）构造函数有个隐蔽能力：**隐式类型转换**。

```cpp
class D1 { public: D1(int y = 1, int m = 1, int d = 1) {} };

int main()
{
    D1 x = 5;    // 😱 居然能编译通过！
                 // 5 被隐式转换成 D1(5, 1, 1)，等价于 D1 x = D1(5);
}
```

实测输出：

```
D1 x = 5 编译通过，x.tag=1
```

**这类隐式转换几乎永远是 bug 的温床**（谁能想到 `x = 5` 是"用 5 构造一个对象"？）。修正方式是加 `explicit`：

```cpp
class D2 { public: explicit D2(int y = 1, int m = 1, int d = 1) {} };

int main()
{
    D2 y = 5;
    // error: no viable conversion from 'int' to 'D2'   ← 编译期直接拦下 ✅
    D2 z(5);    // 显式调用依然可以
}
```

> **工程规范**：除非你有意允许隐式转换，**单参数构造函数一律加 `explicit`**。
> 这是 Google C++ Style Guide 的明确要求，也是 Code Review 的高频意见。
> 反面参考：`std::vector` 的 `vector<int> v(10)` 是"创建 10 个元素"，而不是"把 10 转换成 vector"——
> 标准库自己就非常克制地使用隐式转换。

### 4.6 写了校验不等于校验生效（我踩过的坑）

这是我在写日期类时真实踩过的一个坑，而且非常隐蔽：

```cpp
Date(int year = 1, int month = 1, int day = 1)
{
    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        _year = 1; _month = 1; _day = 1;   // 想"修正"非法输入
    }
    _year = year;      // ⚠️ 问题在这：校验之后又无条件赋值，
    _month = month;    //    前面设置的值全被覆盖
    _day = day;        //    校验等于白写
}
```

跑一下：

```
Date d(2026,13,40) -> d.print() = 2026-13-40
```

`month = 13`、`day = 40` 原样通过。**控制流上"想拦却没拦住"**，这是新人最常见的一类 bug：判断写了、分支体也对，就是忘了让流程真的拐弯。

三种修正，按工程的严格程度递增：

```cpp
// 方案一：夹紧（把非法值改成 1 月 1 日，静默修正）
Date(int y = 1, int m = 1, int d = 1)
{
    if (!isValid(y, m, d)) { y = 1; m = 1; d = 1; }
    _year = y; _month = m; _day = d;      // ← 赋值放最后，用修正后的参数
}

// 方案二：断言（debug 期立刻暴露调用方的 bug）
explicit Date(int y = 1, int m = 1, int d = 1) : _year(y), _month(m), _day(d)
{
    assert(isValid(y, m, d));
}

// 方案三：抛异常（工程版）
explicit Date(int y = 1, int m = 1, int d = 1)
{
    if (!isValid(y, m, d)) throw std::invalid_argument("非法日期");
    _year = y; _month = m; _day = d;
}
```

另外注意：`day <= 31` 这种校验是**不完整**的——2 月没有 30 号、4 月没有 31 号。完整的校验要借助"某年某月有多少天"的辅助函数：

```cpp
static bool isValid(int y, int m, int d)
{
    return y >= 1 && m >= 1 && m <= 12 && d >= 1 && d <= getMonthDay(y, m);
}
```

---

## 五、析构函数

### 5.1 职责与特征

```cpp
~MyStack()
{
    delete[] _a;      // 释放构造函数申请的堆内存
}
```

- **函数名是 `~类名`**，没有参数，没有返回值
- **一个类只能有 1 个析构函数**（不能重载）
- **对象生命周期结束时自动调用**，一生只调用一次
- **后进先出**：后创建的对象先析构

析构顺序实测：

```cpp
Date d1(2026, 9, 2);
Date d2(2026, 9, 3);
Date d3(2026, 9, 4);
// 离开作用域 → d3 先析构，然后 d2，最后 d1（栈结构，后进先出）
```

### 5.2 什么时候需要写析构

判断标准就一句话：**类里有没有向系统"借"来的资源？**

| 资源 | 需要析构吗 |
|---|---|
| `new` / `new[]` 出来的堆内存 | ✅ 必须 `delete` / `delete[]` |
| `malloc` 出来的内存 | ✅ 必须 `free` |
| 打开的文件（`fopen`） | ✅ 必须 `fclose` |
| 加锁的互斥量 | ✅ 必须解锁 |
| 只是 `int` / `double` 等内置类型 | ❌ 不需要 |
| `std::string` / `std::vector` 成员 | ❌ 不需要（它们自己会管） |

对照两个例子：

- `MyStack` 里有 `int* _a`（`new[]` 来的）→ **必须写析构** ✅
- `Date` 只有三个 `int` → **不需要析构**（写了 `~Date() = default;` 也行，表达"我要默认语义"）

### 5.3 `delete[] nullptr` 安全吗？

```cpp
~MyStack()
{
    delete[] _a;   // 如果 _a 是 nullptr 呢？
}
```

**安全。** C++ 标准明确规定：**`delete` / `delete[]` 一个空指针是合法的空操作。**

```cpp
int* p = nullptr;
delete p;      // ✅ 合法，什么也不做
delete[] p;    // ✅ 合法，什么也不做
```

所以一个"容量为 0、`_a == nullptr`"的对象直接析构**不会崩**。

这也是为什么构造函数里要把指针初始化成 `nullptr`——不是可选项，是**保命措施**：

```cpp
explicit MyStack(int cap = 4)
    : _a(cap > 0 ? new int[cap] : nullptr),   // cap 为 0 时给 nullptr
      ...
```

> 但注意：`delete` 一个**野指针**（未初始化的随机值）或**重复 delete** 就是未定义行为，大概率崩溃。
> 后者正是浅拷贝的经典死法，第六章有完整的崩溃现场。

### 5.4 `new[]/delete[]` 与 `malloc/free` 不能混用

```cpp
// 各自配对：都对 ✅
int* p = new int[10];   delete[] p;
int* q = (int*)malloc(sizeof(int) * 10);   free(q);

// 交叉配对：都是未定义行为 💀
int* r = new int[10];   free(r);        // 💀
int* s = (int*)malloc(...);  delete[] s; // 💀
```

**为什么不能混？**（面试常考）

- `new[]` 会在返回的指针**前面**多分配一小块空间（俗称 cookie），记录"数组有多少个元素"，`delete[]` 要读它才知道调几次析构函数；
- `free` 不会往回偏移读这个数字 → 内存管理器直接崩溃；
- `new` 会调用构造函数，`malloc` 不会。

```cpp
std::string* p = (std::string*)malloc(sizeof(std::string) * 3);
// 💀 构造函数根本没被调用，p[0] 是垃圾内存；后面按 std::string 使用必崩
```

顺带说：C++ 里**优先用 `new[]/delete[]`**，因为失败行为更安全：

| | `malloc` | `new` |
|---|---|---|
| 失败时 | 返回 `NULL`（**容易忘记检查**） | 抛 `std::bad_alloc`（**无法忽略**） |
| 调用构造函数 | ❌ 不调用 | ✅ 调用 |
| 返回类型 | `void*`（需强转） | 正确类型 |
| 大小计算 | 手动 `sizeof(int) * n` | 编译器算 |

### 5.5 析构函数里不该做的两件事

**第一，不要写多余的"清场"代码。**

```cpp
~MyStack()
{
    delete[] _a;
    _a = nullptr;
    _size = _capacity = 0;    // ⚠️ 对象马上就要死了，这些赋值毫无意义
}
```

无害，但没必要。析构函数只需要**释放资源**。标准 C++ 里析构后的对象就"不存在"了，正常代码不会再碰它。

**第二，不要让析构函数抛异常。**

析构函数抛异常可能引发灾难（尤其是在栈展开过程中再次抛出 → `std::terminate` 直接终止程序）。
工程约定：析构函数里不做可能失败的操作，或者内部 `try/catch` 吞掉。这也是为什么标准容器的析构都不会抛异常、移动构造函数要标 `noexcept`（第十三章）。

---

## 六、拷贝构造函数

### 6.1 定义与特征

```cpp
// MyStack 的深拷贝构造（完整版，第七章有逐行解析）
MyStack(const MyStack& other)
    : _a(other._capacity > 0 ? new int[other._capacity] : nullptr),
      _size(other._size),
      _capacity(other._capacity)
{
    for (int i = 0; i < _size; ++i)
        _a[i] = other._a[i];
}
```

特征：

- 函数名 = 类名
- **参数必须是"同类型对象的引用"**，通常是 `const 类名&`
- 只有一个参数（否则就不是拷贝构造了）

### 6.2 为什么参数必须是引用 —— 否则无限递归

```cpp
// 假设写成按值传参（错误示范）
MyStack(MyStack other)    // 💀
{
    // ...
}

// 调用 MyStack s2(s1) 时：
//   1. 传参需要把 s1 拷贝一份给 other
//   2. 拷贝一个 MyStack 又要调用拷贝构造……
//   → 无限递归，栈溢出
```

**加 `const` 的理由**：既能接受 const 对象，又能接受临时对象（右值）。不加 `const` 的话，`MyStack s2 = s1 + s2;`（临时对象）就拷贝不了。

### 6.3 什么时候调用拷贝构造（最容易混的点）

```cpp
MyStack s1;
MyStack s2(s1);          // ① 拷贝构造 —— 括号形式
MyStack s3 = s1;         // ② 拷贝构造！⚠️ 虽然写着 =，但这是初始化不是赋值
s3 = s1;                 // ③ 拷贝赋值 —— 两个对象都已存在
void func(MyStack s);    // ④ 按值传参 → 拷贝构造
MyStack func2() { ... }  // ⑤ 按值返回 → 拷贝构造（可能被编译器优化掉）
```

**判断口诀**：

> **看左边：变量第一次出现（诞生）→ 拷贝构造；变量已经存在（活了）→ 拷贝赋值。**

```
MyStack s3 = s1;
       ↑
    第一次出现 → 诞生 → 拷贝构造

s3 = s1;
↑
   已经存在 → 存活 → 拷贝赋值
```

`MyStack s3 = s1;` 之所以不可能是 `operator=`，是因为 `operator=` 的前提是**两个对象都已经构造完毕**——而 `s3` 此刻还没有被构造，它是"正在出生"的对象。

**亲眼验证一下**（带上打印的 Demo 类）：

```cpp
class Demo
{
public:
    Demo()                       { cout << "  默认构造\n"; }
    Demo(const Demo&)            { cout << "  拷贝构造\n"; }
    Demo& operator=(const Demo&) { cout << "  拷贝赋值\n"; return *this; }
    ~Demo()                      { cout << "  析构\n"; }
};

Demo func(Demo d) { return d; }        // 按值传参 + 按值返回

int main()
{
    Demo d1;            // ①
    Demo d2(d1);        // ②
    Demo d3 = d1;       // ③ 观察它！
    d3 = d1;            // ④
    func(d1);           // ⑤
    return 0;
}
```

实测输出（`g++ -std=c++17 -O0`，未开优化）：

```
Demo d1;        ->  默认构造
Demo d2(d1);    ->  拷贝构造
Demo d3 = d1;   ->  拷贝构造      ← 确认：是拷贝构造
d3 = d1;        ->  拷贝赋值
func(d1);       ->  拷贝构造      ← 按值传参
                    拷贝构造      ← 按值返回（-O0 下未优化掉）
                    析构
                    析构
离开 main       ->  析构
                    析构
                    析构
```

> 我当年看这段输出的时候才真正记住"`=` 出现在初始化里是拷贝构造"这件事。
> 建议你也亲手跑一遍 —— 亲眼看到输出，比看十遍书都管用。

### 6.4 编译器默认生成的拷贝构造：逐成员浅拷贝

```cpp
// 编译器自动生成的版本（伪代码）
MyStack(const MyStack& other)
{
    _a = other._a;              // ← 只拷贝了指针的值（地址）！
    _size = other._size;
    _capacity = other._capacity;
}
```

**这就是"浅拷贝"**：对 `Date` 这种只有 `int` 的类完全够用，但 `MyStack` 里有个指针 → 灾难。

### 6.5 浅拷贝灾难现场（实测）

把深拷贝构造注释掉，让编译器用默认的浅拷贝，然后跑：

```cpp
MyStackRaw s1;
s1.StackInit(5);
s1.push(1); s1.push(2); s1.push(3);

MyStackRaw s2(s1);      // 浅拷贝：两个对象的 a 指向同一块内存
```

**内存状态与实测输出：**

```
   栈区                    堆区
┌─────────────┐
│ s1: a ──────┼────────┐
│     top=3   │        │
│     cap=5   │        ↓
├─────────────┤    ┌───────────┐
│ s2: a ──────┼───→│  1  2  3  │  ← 两个对象共享同一块内存
│     top=3   │    └───────────┘
│     cap=5   │        ↑
└─────────────┘        │
                    （同一块地址）
```

```
  s1.a = 0x1014790b0 (top=3, cap=5)
  s2.a = 0x1014790b0 (top=3, cap=5)        ← 地址完全相同！
  s2.push(99) 后 s1 的底层第 4 格: 99       ← 通过 s2 写，s1 也"看到"了
  s1: 1 2 3
  s2: 1 2 3 99
退出 main，开始析构 ->
  ~MyStackRaw(a=0x1014790b0)
  ~MyStackRaw(a=0x1014790b0)
Trace/BPT trap: 5                          ← 第二次 delete[] 同一块内存，进程崩溃
```

用 AddressSanitizer 跑同一个程序，报告更直白：

```
==47238==ERROR: AddressSanitizer: attempting double-free on 0x603000000fa0 in thread T0:
    #1 ... in MyStackRaw::~MyStackRaw() demo_shallow_copy.cpp:11
    #2 ... in main demo_shallow_copy.cpp:61

freed by thread T0 here:
    #1 ... in MyStackRaw::~MyStackRaw() demo_shallow_copy.cpp:11

previously allocated by thread T0 here:
    #1 ... in _Znam+0x74
```

两个后果，一个比一个严重：

1. **数据串改**：通过 `s2` 写，`s1` 会莫名其妙跟着变（它们根本就是同一块内存）；
2. **double free**：两个对象析构时各 `delete[]` 一次 → 同一块内存释放两次 → 崩溃。

### 6.6 深拷贝：把"拷贝"做成真正的拷贝

```cpp
MyStack(const MyStack& other)
    : _a(other._capacity > 0 ? new int[other._capacity] : nullptr),  // ① 申请自己的内存
      _size(other._size),
      _capacity(other._capacity)
{
    for (int i = 0; i < _size; ++i)
        _a[i] = other._a[i];                                        // ② 逐元素复制
}
```

```
   栈区                    堆区
┌─────────────┐
│ s1: a ──────┼───────→ ┌───────────┐
│     top=3   │         │  1  2  3  │  ← s1 专属
├─────────────┤         └───────────┘
│ s2: a ──────┼───────→ ┌───────────┐
│     top=3   │         │  1  2  3  │  ← s2 专属，互不干扰
└─────────────┘         └───────────┘
```

实测（修改 s2 完全不影响 s1）：

```
[深拷贝] s.top=60 s2.top=999
```

### 6.7 一句话总结

> **浅拷贝 = 只拷指针（地址）**
> **深拷贝 = 不只拷指针，还拷指针指向的那块内容**

```
浅拷贝：  [_a] ──┐
                 ├──→ [同一块数据]   💀 double free
          [_a] ──┘

深拷贝：  [_a] ──────→ [数据副本1]   ✅ 互不干扰
          [_a] ──────→ [数据副本2]
```

---

## 七、赋值运算符重载

### 7.1 `operator=` 的本质

`=` 原本只能用于内置类型。自定义类型怎么赋值？**把 `=` 当成一个函数名，重载它**：

```cpp
Date d1(2026, 9, 1);
Date d2(2026, 9, 2);
d1 = d2;
// 编译器眼中的形态：
d1.operator=(d2);      // ← 真正的调用形式
```

所有运算符重载都是这个套路——**给运算符起一个叫 `operator@` 的函数名**。

### 7.2 `MyStack::operator=` 逐行精读

```cpp
MyStack& operator=(const MyStack& other)
{
    if (this != &other)                          // ① 自赋值检查
    {
        MyStack tmp(other);                      // ② 深拷贝一份完整副本
        std::swap(_a, tmp._a);                   // ③ 交换指针
        std::swap(_size, tmp._size);
        std::swap(_capacity, tmp._capacity);
    }                                            // ④ tmp 析构，顺带释放旧内存
    return *this;                                // ⑤ 返回自身，支持链式赋值
}
```

#### ① 为什么返回 `MyStack&`？

支持**链式赋值**：

```cpp
a = b = c;
// 实际：a.operator=( b.operator=(c) );
// 若返回 void，a = (b = c) 就没法编译
```

返回类型也**不能是 `MyStack`（按值返回）**——那会再调用一次拷贝构造，且改的不是原对象。

#### ② `if(this != &other)` 是必需的吗？

**不是必需，是优化。**

这套"先拷贝、再交换"（copy-and-swap）的写法**本身就正确处理自赋值**：`tmp` 深拷贝一份自己，swap 后 `tmp` 持有原内存，析构释放 → 结果正确，只是白干一次。
加上这行判断，自赋值时直接跳过，省掉一次无用拷贝。

> 实测配套的一个小彩蛋：如果不加 `if`、直接写 `s = s;`，新版本的编译器甚至会警告：
> `warning: explicitly assigning value of variable ... to itself [-Wself-assign-overloaded]`。
> 编译器都在提醒你这里有猫腻。

#### ③ 为什么是"先拷贝、再交换"，而不是"先删后建"？

**对比错误写法：**

```cpp
// ❌ 错误示范：先删后建
MyStack& operator=(const MyStack& other)
{
    delete[] _a;                        // ① 先把旧内存释放了
    _a = new int[other._capacity];      // ② 万一这里抛异常（内存耗尽）→ 对象已废
    _size = other._size;
    _capacity = other._capacity;
    for (int i = 0; i < _size; ++i)
        _a[i] = other._a[i];
    return *this;
}
```

两个致命问题：

| 问题 | 说明 |
|---|---|
| **自赋值灾难** | `s = s;` 时，第 ① 步把自己的内存释放了，第 ② 步再去读 `other._a`（就是自己）→ **读已释放内存，未定义行为** |
| **异常不安全** | 第 ② 步 `new` 失败抛异常，`*this` 已经处于"半死不活"状态 |

**copy-and-swap 的妙处：**

```
执行前：  this->_a ──→ [旧内存]        tmp._a ──→ [新拷贝]
                               ↓ swap
执行后：  this->_a ──→ [新拷贝]        tmp._a ──→ [旧内存]
                                                 ↓ tmp 析构
                                             [旧内存被释放] ✅

关键：任何一步失败抛异常，*this 都丝毫未动 → 强异常安全
```

- **异常安全**：`new` 在 `tmp` 构造里完成，失败了 `*this` 原封不动 → **强异常保证**；
- **自赋值安全**：即使不加 `if` 检查也正确。

#### ④ 更简洁的惯用写法（按值传参版）

```cpp
// 参数本身就是副本：传进来的时候就已经拷贝好了
MyStack& operator=(MyStack other)   // ← 注意：没有 &，参数是值传递
{
    std::swap(_a, other._a);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    return *this;
}
```

少了一个临时变量、一个 `if` 判断，语义完全一样。代价是参数按值传递，即使是右值（临时对象）也会先拷贝一次——不过 C++11 有了移动语义之后，这个代价也被消除了（第十三章）。

### 7.3 拷贝构造 vs 拷贝赋值（对照表）

| | 拷贝构造 | 拷贝赋值 |
|---|---|---|
| **调用时机** | 对象**诞生**时 | 对象**已存在**时 |
| **典型写法** | `MyStack s2(s1);`<br>`MyStack s2 = s1;` | `s2 = s1;` |
| **当前对象状态** | 全新的，成员未初始化 | 已有资源，**必须先释放旧的** |
| **是否要自赋值检查** | 不需要（不可能是自己） | copy-and-swap 下不需要，但建议加 |
| **额外动作** | 无 | **释放旧资源** |

> **一句话记忆**：拷贝构造是"从零开始造一个"，拷贝赋值是"先把旧的拆了，再照着新的造"。
> 这也解释了为什么拷贝赋值要复杂得多——它比拷贝构造多一份"收拾旧摊子"的责任。

---

### 阶段小结（前七章）

到这里，`MyStack` 已经能通过大部分检查了：

```cpp
class MyStack
{
public:
    explicit MyStack(int cap = 4);                    // ① 构造：申请资源
    ~MyStack();                                       // ② 析构：释放资源
    MyStack(const MyStack& other);                    // ③ 深拷贝构造
    MyStack& operator=(const MyStack& other);         // ④ copy-and-swap 赋值
    void push(int val);                               // ⑤ 接口（错误处理见下章）
    // ...
private:
    int* _a;
    int  _size;
    int  _capacity;
};
```

**四个函数一套打包**（构造、析构、拷贝构造、拷贝赋值）。这个组合不是巧合——它就是第十一章要讲的**三法则**的雏形。

---

## 八、运算符重载全谱系

> 这一章拿"日期类"当例子，把 C++ 中最常用的一套运算符（比较、加减、自增自减、输入输出）全部过一遍。

### 8.1 本质：`operator@` 就是一个函数名

```
d2 == d3    ⟺  d2.operator==(d3)      // 成员函数形式
d2 == d3    ⟺  operator==(d2, d3)     // 全局函数形式
d1 += 5     ⟺  d1.operator+=(5)
cout << d1  ⟺  operator<<(cout, d1)   // 只能是全局形式（左操作数是 cout）
d1++        ⟺  d1.operator++(0)       // 后置：多一个 int 占位参数
```

`a + b` 能不能编译过，取决于能不能找到名为 `operator+` 的可用函数。就这么简单。

### 8.2 成员 vs 全局：判定规则

| 运算符 | 建议写成 | 原因 |
|---|---|---|
| `=` `+=` `-=` `[]` `()` `->` | **必须是成员** | C++ 语法强制 |
| `==` `!=` `<` `<=` `>` `>=` | 成员（习惯） | 左操作数通常是自己 |
| `+` `-` `*` `/`（双目算术） | 成员或全局都行 | 习惯写成员 |
| `<<` `>>`（流） | **必须是全局** | 左操作数是 `ostream/istream`，不是你的类 |
| `++` `--` | 成员 | — |

判定的思考起点就一句话：**"左操作数是谁？它是不是我的类？"**

- 左操作数是我的类 → 写成成员最自然：`d1 == d2` → `d1.operator==(d2)`
- 左操作数不是我的类（`cout << d`）→ 只能写成全局函数

### 8.3 比较运算符：只写两个，推六个

六个比较运算符不需要都从零写。`Date` 有天然的全序关系（先比年、再比月、再比日），所以：

1. **只写 `==` 和 `<`**（真正的核心逻辑，各一份）
2. 其余四个**全部复用**：

```cpp
bool operator==(const Date& o) const
{ return _year == o._year && _month == o._month && _day == o._day; }

bool operator!=(const Date& o) const { return !(*this == o); }           // 复用 ==
bool operator<=(const Date& o) const { return *this < o || *this == o; } // 复用 < 和 ==
bool operator>(const Date& o)  const { return !(*this <= o); }           // 复用 <=
bool operator>=(const Date& o) const { return !(*this <  o); }           // 复用 <
```

核心逻辑只有 `operator<` 这一份：

```cpp
bool operator<(const Date& o) const
{
    if (_year  != o._year)  return _year  < o._year;    // 年不同，比年
    if (_month != o._month) return _month < o._month;   // 年同月不同，比月
    return _day < o._day;                               // 年月都同，比日
}
```

> **这样写的好处**：比较逻辑只有一处。将来改规则（比如加入时区、支持公元前），只改 `<` 和 `==` 两处，六个函数自动同步。
> 反面教材是六个函数各抄一遍比较逻辑，改一处漏五处。
>
> C++20 更激进：`operator<=>`（三路比较）**一个顶六个**，编译器自动生成全部比较运算符。知道这个方向即可。

### 8.4 算术运算符：谁返回值、谁返回引用、const 放哪

把 `Date` 的算术家族排一张表：

| 运算符 | 返回类型 | 是否 const | 语义 |
|---|---|---|---|
| `operator+` | `Date`（新对象） | **const** | 不改自己，产生新值 |
| `operator-`(int) | `Date`（新对象） | **const** | 同上 |
| `operator-`(Date) | `int`（天数差） | **const** | 只读两个日期 |
| `operator+=` | `Date&`（自己） | 非 const | 就地修改 |
| `operator-=` | `Date&`（自己） | 非 const | 就地修改 |

规律一句话：

> **产生新值的运算符（`+` `-`）→ 返回新对象 + 加 const；就地修改的运算符（`+=` `-=`）→ 返回 `*this` 引用 + 不加 const。**

```cpp
// 产生新值：const（不动自己），返回新对象
Date operator+(int days) const
{
    Date r = *this;          // 在副本上操作
    r._day += days;
    // ... 处理进位 ...
    return r;                // 返回副本
}

// 就地修改：非 const（要改自己），返回 *this 引用
Date& operator+=(int days)
{
    *this = *this + days;    // 复用 +，消除重复逻辑
    return *this;
}
```

**`operator+` 忘了加 `const` 的后果**很实际：const 对象没法做加法。

```cpp
const Date d(2026, 9, 1);
Date x = d + 1;    // ❌ 编译错误：d 是 const，不能调用非 const 成员函数
```

### 8.5 自增自减：前置与后置

```cpp
// 前置 ++i：修改自己，返回自己（引用）—— 和 i += 1 一个意思
Date& operator++()
{
    *this += 1;
    return *this;
}

// 后置 i++：返回"修改前的旧值"，所以必须返回对象（副本）
Date operator++(int)      // ← 那个 int 是纯占位，没有任何用途！
{
    Date temp = *this;    // ① 先存档
    *this += 1;           // ② 再修改自己
    return temp;          // ③ 返回存档
}
```

| | 前置 `++d` | 后置 `d++` |
|---|---|---|
| 签名 | `operator++()` | `operator++(int)` ← 参数只是用来"占位区分" |
| 返回 | `Date&`（引用） | `Date`（值，副本） |
| 语义 | 先加，返回加完之后的值 | 先返回旧值，再加 |
| 效率 | 高（无拷贝） | 低（必须拷一份旧值） |

实测：

```cpp
Date g(2026, 9, 22);
cout << "++g = " << ++g << "，后置先返回旧值: " << g++ << "，g 现在是 " << g << endl;
```

```
++g = 2026-09-23，后置先返回旧值: 2026-09-23，g 现在是 2026-09-24
```

解读：`++g` 先加到 9-23 并返回 9-23；`g++` 返回当时的旧值 9-23，然后把 g 加到 9-24。

> **为什么后置要返回 `Date` 而不是 `Date&`？**
> 因为后置要返回的是一个"已经不存在于任何变量里的旧值"（`temp` 是局部变量）。
> 局部变量出函数就销毁，返回它的引用 = **悬垂引用** 💀。
> 这也是"能用前置就用前置"的性能原因——现代 C++ 规约：for 循环一律写 `++i`。

### 8.6 流运算符 `<<` / `>>`：为什么必须写在类外面

```cpp
class Date
{
    // ...
    friend std::istream& operator>>(std::istream& is, Date& date);   // >> 要写私有成员，需要 friend
};

std::ostream& operator<<(std::ostream& os, const Date& date)
{
    os << date.toString();
    return os;                  // ← 返回 os，支持 cout << d1 << d2 连续输出
}
```

**为什么不能写成成员函数？**

```cpp
// 假设写成成员函数
class Date {
    ostream& operator<<(ostream& os);   // ⚠️
};

d1 << cout;    // 😱 调用形式变成 d1.operator<<(cout)
               // 即"d1 往 cout 里输出"——顺序反了，完全不是 cout << d1
```

成员函数的第一个参数永远是隐含的 `this`（左操作数）。而 `cout << d1` 的左操作数是 `cout`，不是 `Date`。
**所以 `<<` 只能写成全局函数**，让左操作数成为第一个显式参数：

```cpp
ostream& operator<<(ostream& os, const Date& d);
//        ↑ 第一个参数接 cout，第二个参数接 d1  ← cout << d1 就能匹配
```

**链式输出的原理**：

```
cout << d1    返回 cout
     ↓
(cout << d1) << d2    就是  cout << d2
```

所以 `operator<<` 必须 `return os;`——返回 `void` 的话 `cout << d1 << endl` 直接编译不过。

> **`friend` 的定位**：给某个外部函数开一扇"进屋的门"。
> `operator<<` 如果只用 public 的 `toString()`，可以不加 friend；
> `operator>>` 需要逐字段读入私有成员 `_year/_month/_day`，所以必须 friend。
> 能不用就不用，但该用的时候（流输入这类"按部分读写内部状态"的场景）没必要绕。

### 8.7 写日期类时最常见的 5 个错误（我全踩过）

下面这 5 处，是我写日期类时真实写出来的 bug。跑一遍给它们"验尸"，都是非常有代表性的错误。

#### 错误 1：校验被后续赋值覆盖（4.6 节已讲）

```cpp
Date(int year = 1, int month = 1, int day = 1)
{
    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        _year = 1; _month = 1; _day = 1;
    }
    _year = year; _month = month; _day = day;   // 覆盖了上面的一切
}
```

```
Date d(2026,13,40) -> d.print() = 2026-13-40
```

#### 错误 2：`operator-(Date)` 返回的其实是"年份差"

```cpp
int operator-(const Date& other) const
{
    return _year - other._year;      // ⚠️ 这是年份差，不是天数差
}
```

```
2026-01-02 - 2026-01-01 = 0（天数差应为 1）
2026-09-01 - 2025-12-31 = 1（天数差应为 244）
```

两天差 1 天返回 0、差 244 天返回 1。**按这种数据做统计，全盘出错。**

修正：把两个日期都折算成"自某个纪元起的天数"，一减即可：

```cpp
int operator-(const Date& o) const
{
    return daysFromEpoch() - o.daysFromEpoch();     // 正数表示 this 在后
}

int daysFromEpoch() const      // 私有辅助：1970-01-01 起的天数
{
    int days = 0;
    for (int y = 1970; y < _year; ++y) days += isLeapYear(y) ? 366 : 365;
    for (int m = 1; m < _month; ++m) days += getMonthDay(_year, m);
    return days + _day - 1;
}
```

修正后实测：`2026-09-22 - 2024-01-01 = 995`（用 `python3` 对拍真值 995 ✅）。

#### 错误 3：`operator-=` 的三连错

```cpp
Date operator-=(int days)      // 错 ①：返回类型应为 Date&
{
    for (int i = 0; i < days; i++)
    {
        this->_day--;
        if (this->_day < 1)
        {
            this->_month--;
            if (this->_month < 1)
            {
                this->_year--;
                this->_year = 12;        // 错 ②：把"年"赋值成 12！应该是 this->_month = 12
            }
        }
        this->_day = getMonthDay(_year, _month);   // 错 ③：无论有没有跨月都重置
    }
    return *this;
}
```

错 ③ 的影响最直观——**每减一天都把"日"重置成"当月最后一天"**：

```
2026-09-15 -= 1 -> 2026-09-30      （应为 2026-09-14）
```

错 ② 影响"跨年减一天"：`_year = 12` 把年份毁掉，随后 `getMonthDay(12, 0)` 的月份参数是 0：

```
2026-01-01 -= 1 ->
Assertion failed: (month > 0 && month < 13), function getMonthDay
Abort trap: 6                              （exit code 134）
```

断言拦住了错误。**但如果关掉断言呢？** 我实际编译了 Release 版（`-DNDEBUG`）：

```
2026-01-01 -= 1 -> 0012-00--1        ← 年份变成 12，月份 0，日 -1
```

这就是**未定义行为**的真面目：debug 版崩溃、release 版静默产出垃圾数据。

#### 错误 4：跨年判断用错了对象

```cpp
Date operator+(int days)  
{
    Date result = *this;
    result._day += days;
    while (result._day > getMonthDay(result._year, result._month))
    {
        result._day -= getMonthDay(result._year, result._month);
        result._month++;
        if (_month > 12)                 // ⚠️ 用的是 this 的 _month，不是 result 的！
        {
            result._year++;
            result._month = 1;
        }
    }
    return result;
}
```

`this->_month` 从头到尾没变过（这个函数改的是 `result`），所以 `if (_month > 12)` **永远为假** → `result._month` 会涨到 13、14……

实测（debug 版）：

```
2026-12-25 + 10 ->
Assertion failed: (month > 0 && month < 13), function getMonthDay
Abort trap: 6                              （exit code 134）
```

实测（Release 版，断言被关掉，数组越界读出垃圾）：

```
2026-12-25 + 10 -> 2026-4097-04            （应为 2027-01-04）
```

**修正**：循环里每一步都只碰 `result`，一个变量都不能混：

```cpp
Date operator+(int days) const
{
    Date r = *this;
    r._day += days;
    while (r._day > getMonthDay(r._year, r._month))
    {
        r._day -= getMonthDay(r._year, r._month);
        if (++r._month > 12) { r._month = 1; ++r._year; }   // ← 全部是 r
    }
    while (r._day < 1)
    {
        if (--r._month < 1) { r._month = 12; --r._year; }   // ← 全部是 r
        r._day += getMonthDay(r._year, r._month);
    }
    return r;
}
```

修正后：`2026-12-25 += 10 -> 2027-01-04` ✅、`2026-01-01 -= 1 -> 2025-12-31` ✅。

#### 错误 5：闰年判断写成 `_year & 400 == 0`

```cpp
// 错误版本
return (_year & 400 == 0) || (_year % 4 == 0 && _year % 100 != 0);
```

两个错误叠在一起：

1. `&` 写成了按位与（想写取模 `%`）；
2. 即使真要写按位与，**`==` 的优先级高于 `&`** —— 表达式实际是 `_year & (400 == 0)` → `_year & 0` → **恒为 0（假）**。

正确写法：

```cpp
bool isLeapYear(int year)
{
    return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
}
```

> **教训**：位运算和比较混用时**永远加括号**：`(year & mask) == 0`。
> 编译器一般会警告 `&` 与 `==` 混用，但别赌它——`-Wall -Wextra` 是你的朋友。

### 8.8 完整修正版：可以用的 `Date`

把上面的修正全部落地，就是一个能上工程的 `Date`。这份代码我做过**批量对拍**：
**±400 天共 802 个日期，与 `python3` 的真值零误差**。

```cpp
class Date
{
    friend istream& operator>>(istream& is, Date& d);   // >> 写私有成员，需要 friend

public:
    explicit Date(int year = 1970, int month = 1, int day = 1)
    {
        if (!isValid(year, month, day))
            throw invalid_argument("非法日期");         // 工程做法：抛异常
        _year = year; _month = month; _day = day;
    }

    // ---- 比较：只写 == 和 <，其余推导 ----
    bool operator==(const Date& o) const
    { return _year == o._year && _month == o._month && _day == o._day; }
    bool operator!=(const Date& o) const { return !(*this == o); }
    bool operator<(const Date& o) const
    {
        if (_year  != o._year)  return _year  < o._year;
        if (_month != o._month) return _month < o._month;
        return _day < o._day;
    }
    bool operator<=(const Date& o) const { return *this < o || *this == o; }
    bool operator>(const Date& o)  const { return !(*this <= o); }
    bool operator>=(const Date& o) const { return !(*this <  o); }

    // ---- 日期 + 天数（处理进位与借位）----
    Date operator+(int days) const
    {
        Date r = *this;
        r._day += days;
        while (r._day > getMonthDay(r._year, r._month))
        {
            r._day -= getMonthDay(r._year, r._month);
            if (++r._month > 12) { r._month = 1; ++r._year; }
        }
        while (r._day < 1)
        {
            if (--r._month < 1) { r._month = 12; --r._year; }
            r._day += getMonthDay(r._year, r._month);
        }
        return r;
    }
    Date& operator+=(int days) { *this = *this + days; return *this; }

    // ---- 日期 - 天数（复用 +）----
    Date operator-(int days) const { return *this + (-days); }
    Date& operator-=(int days) { *this = *this - days; return *this; }

    // ---- 日期 - 日期 = 天数差 ----
    int operator-(const Date& o) const { return daysFromEpoch() - o.daysFromEpoch(); }

    // ---- 前置/后置 ++/-- ----
    Date& operator++()    { *this += 1; return *this; }
    Date  operator++(int) { Date t = *this; *this += 1; return t; }
    Date& operator--()    { *this -= 1; return *this; }
    Date  operator--(int) { Date t = *this; *this -= 1; return t; }

    int    getYear()  const { return _year;  }
    int    getMonth() const { return _month; }
    int    getDay()   const { return _day;   }
    string toString() const { /* stringstream + setw/setfill 格式化 YYYY-MM-DD */ }

private:
    int _year, _month, _day;

    static bool isLeapYear(int y) { return (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0); }
    static int  getMonthDay(int y, int m)
    {
        static const int t[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (m == 2 && isLeapYear(y)) return 29;
        return t[m];
    }
    static bool isValid(int y, int m, int d)
    { return y >= 1 && m >= 1 && m <= 12 && d >= 1 && d <= getMonthDay(y, m); }

    int daysFromEpoch() const
    {
        int days = 0;
        for (int y = 1970; y < _year; ++y) days += isLeapYear(y) ? 366 : 365;
        for (int m = 1; m < _month; ++m) days += getMonthDay(_year, m);
        return days + _day - 1;
    }
};

ostream& operator<<(ostream& os, const Date& d) { return os << d.toString(); }

istream& operator>>(istream& is, Date& d)
{
    int y, m, dd; char s1, s2;
    is >> y >> s1 >> m >> s2 >> dd;
    if (!is || s1 != '-' || s2 != '-') { is.setstate(ios::failbit); return is; }
    try { d = Date(y, m, dd); }        // 借构造函数做校验，非法输入置 failbit
    catch (...) { is.setstate(ios::failbit); }
    return is;
}
```

实测输出节选：

```
d1 + 15 = 2022-09-16 (2022-09-16)
d1 - 1  = 2022-08-31 (2022-08-31)
2026-09-22 - 2024-01-01 = 995 天
2026-12-25 += 10 -> 2027-01-04
2026-01-01 -= 1  -> 2025-12-31
2026-09-15 -= 1  -> 2026-09-14
++g = 2026-09-23 (后置先返回旧值: 2026-09-23)，g 现在是 2026-09-24
Date(2026,13,40) 被拦下: 非法日期
```

### 8.9 运算符重载的边界

- **不能发明新运算符**：两个星号连写、`<->` 这类符号都不存在，不能凭空创造；
- **不能改变操作数个数和优先级**：`a + b * c` 永远先算 `*`；
- **不能重载 `::`、`.`、`.*`、`?:`、`sizeof`**；
- **`&&` `||` 重载后失去短路特性**——业务上几乎永远不该重载它们；
- **语义要符合直觉**：`operator+` 就该"相加"，别拿它做删除。

> 运算符重载的目的是让自定义类型的用法**向内置类型看齐**（`d1 + 1` 像 `i + 1`），不是为了炫技。
> 用得克制，代码才像人写的。

---

## 九、模板化：MyStack 泛型版

### 9.1 为什么需要模板

前面的 `MyStack` 里存的是 `int`。如果明天要存 `double`、存 `string`、存 `Date` 呢？

```cpp
// 没有模板的世界：复制粘贴出无数份
class StackInt    { int*    _a;  /* 一模一样的管理逻辑 */ };
class StackDouble { double* _a;  /* 一模一样的管理逻辑 */ };
class StackDate   { Date*   _a;  /* 一模一样的管理逻辑 */ };
// …… 每来一个新类型就抄一遍，改一个 bug 要改 N 个文件 💀
```

**类模板 = 把"类型"本身变成参数**：

```cpp
template<typename T>
class MyStack { T* _a; /* 管理逻辑只写一遍，T 由使用者决定 */ };

MyStack<int>    s1;    // T = int
MyStack<double> s2;    // T = double
MyStack<Date>   s3;    // T = Date（任何类型，包括自定义类）
```

> 模板和函数重载解决的是同一类问题——**"相同逻辑、不同数据类型"**。
> 重载是"手写 N 份，编译器帮你选"；模板是"写 1 份，编译器帮你生成 N 份"（实例化）。
> 代价也在这里：`MyStack<int>` 和 `MyStack<double>` 在二进制里是**两份独立的代码**。

### 9.2 类模板的三个语法坑

```cpp
template<typename T>        // ① 模板头：T 是占位符类型
class MyStack
{
public:
    // 类内定义：T 直接当类型用
    explicit MyStack(size_t cap = 4)
        : _a(cap > 0 ? new T[cap] : nullptr), _size(0), _capacity(cap) {}

private:
    T*     _a;              // ② 成员的类型可以依赖 T
    size_t _size;
    size_t _capacity;
};

// 类外定义成员函数：每个函数前面都要补上 template<typename T>
template<typename T>
void MyStack<T>::someFunc() { /* ... */ }
```

1. **类外写成员函数时**，`template<typename T>` 不能省，且类名要写成 `MyStack<T>`（不是 `MyStack`）；
2. **`.h` / `.cpp` 分离对模板不适用**：模板的定义必须在实例化的翻译单元里可见，所以模板代码通常全部写在头文件——这是模板与普通类最大的工程差异之一；
3. **实例化**：`MyStack<int> s;` 写出来的那一刻，编译器用 `int` 替换 `T`，生成一份完整的类。

### 9.3 `MyStack<T>` 逐段精读

```cpp
// ---- 构造：explicit + 初始化列表 + 边界处理 ----
explicit MyStack(size_t cap = 4)
    : _a(cap > 0 ? new T[cap] : nullptr),
      _size(0),
      _capacity(cap)
{
}

// ---- 析构：一行 ----
~MyStack() { delete[] _a; }          // delete[] nullptr 安全；与 new[] 配对

// ---- 深拷贝构造：与 int 版逻辑完全一样，只是把 int 换成 T ----
MyStack(const MyStack& other)
    : _a(other._capacity > 0 ? new T[other._capacity] : nullptr),
      _size(other._size),
      _capacity(other._capacity)
{
    for (size_t i = 0; i < _size; ++i)
        _a[i] = other._a[i];
}

// ---- 拷贝赋值：copy-and-swap，一字未变 ----
MyStack& operator=(const MyStack& other)
{
    if (this != &other)
    {
        MyStack tmp(other);
        std::swap(_a, tmp._a);
        std::swap(_size, tmp._size);
        std::swap(_capacity, tmp._capacity);
    }
    return *this;
}

// ---- 查询接口 + const 重载的 top ----
size_t size()     const { return _size;     }
size_t capacity() const { return _capacity; }
bool   empty()    const { return _size == 0; }

const T& top() const { assert(_size > 0); return _a[_size - 1]; }   // const 对象：只读
T&       top()       { assert(_size > 0); return _a[_size - 1]; }   // 非 const 对象：可写
```

注意三点：

1. **`explicit` 挡住了 `MyStack<int> s = 10;` 这种隐式转换**；
2. **`top()` 提供了 const / 非 const 两个版本**——标准容器的标准做法（`vector::front` 同款），第十章细讲；
3. **`assert(_size > 0)` 表达前置条件**：STL 风格——"调用者必须保证非空，否则未定义行为"。
   对比前面 C 风格版的"返回 -1 + 打印"，这是一种**把契约写进代码**的方式。

还有一个深层的约束值得知道：`_a[i] = other._a[i]` 要求 `T` 支持赋值；`new T[cap]` 要求 `T` 有**默认构造函数**（数组每个元素都要先默认构造）。
所以 `MyStack<Date>` 能用（Date 有全缺省构造），但 `MyStack<没有默认构造的类>` 会编译失败。
`std::vector` 通过"分离内存分配与对象构造"（allocator + placement new）绕开了这个限制——知道这个差异，聊"手写 vector"时用得上。

### 9.4 push 与成倍扩容

```cpp
void push(const T& val)
{
    if (_size == _capacity)
    {
        size_t newCap = _capacity == 0 ? 4 : _capacity * 2;   // ← 成倍扩容
        T* fresh = new T[newCap];
        for (size_t i = 0; i < _size; ++i)
            fresh[i] = _a[i];       // 搬旧数据
        delete[] _a;                // 释放旧内存
        _a = fresh;
        _capacity = newCap;
    }
    _a[_size++] = val;
}
```

**为什么成倍扩容，而不是每次 +1？** 算一笔账：

```
每次 +1 扩容：插入 n 个元素 → 扩容 n 次，第 k 次搬 k 个元素
             总搬运次数 = 1 + 2 + ... + n = O(n²)   💀

成倍扩容：    容量 1→2→4→8→16...
             第 k 次扩容搬 2^(k-1) 个，总搬运 < 2n = O(n)（等比级数求和）
             均摊到每次 push → O(1)   ✅
```

实测（默认容量 4，push 6 个元素）：

```
[扩容] size=6 capacity=8 top=60        ← 4 装不下，翻倍到 8
```

这就是 `std::vector` 也采用成倍扩容（通常 1.5 或 2 倍）的原因。

### 9.5 ⚠️ 自引用陷阱：`s.push(s.top())`

这是我踩过的最隐蔽的一个坑，用 AddressSanitizer 才现出原形：

```cpp
MyStack<int> t(2);
t.push(1); t.push(2);     // 满了：_size == _capacity == 2
t.push(t.top());          // 把"栈顶元素的引用"传给了 push
```

`t.top()` 返回 `T&`——指向 `_a[1]` 的**引用**，不是值！
`push` 收到 `const T& val`，这个引用在扩容时**指向的内存被 `delete[] _a` 释放了**：

```cpp
void push(const T& val)          // val 直接指向 _a[1]
{
    if (_size == _capacity)
    {
        T* fresh = new T[newCap];
        for (...) fresh[i] = _a[i];
        delete[] _a;             // 💀 val 指向的内存在这里被释放
        _a = fresh;
        _capacity = newCap;
    }
    _a[_size++] = val;           // 💀 读已释放内存（use-after-free）
}
```

ASan 报告原文：

```
==47154==ERROR: AddressSanitizer: heap-use-after-free on address 0x6020000009b4
READ of size 4 at 0x6020000009b4 thread T0
    #0 MyStack<int>::push(int const&) demo_my_stack_template.cpp:58     ← 读悬垂引用
    #1 main demo_my_stack_template.cpp:100
```

debug 版可能"碰巧正常"（释放的内存还没被覆盖），release 版数据错乱——**又一次：行为随环境随机变化的未定义行为**。

**怎么修？** 三个层次：

```cpp
// 方案一（推荐）：按值传参 —— 进函数第一件事就是拷贝，与内部内存无关
void push(T val)                     // 对 int 等小类型本来就该按值传
{
    if (_size == _capacity) { /* 扩容 */ }
    _a[_size++] = std::move(val);    // C++11 移动，零多余拷贝
}

// 方案二：保持 const T&，但扩容前先把 val 复制出来
void push(const T& val)
{
    if (_size == _capacity)
    {
        T copied = val;              // ← 扩容前先复制一份
        // ... 扩容 ...
        _a[_size++] = copied;
        return;
    }
    _a[_size++] = val;
}

// 方案三：把陷阱留给调用方（不推荐）
```

同类问题在更早的 `int` 版本里也埋着（`push(const int& val)`）——**对内置类型，`push(int val)` 本来就是更优解**，还顺带躲开这个陷阱。

### 9.6 模板 + 三法则 = 一个"资源管理模板"

逐段对比会发现，`MyStack<int>` 和 `MyStack<T>` 的**资源管理代码逐行对应**：

| 职责 | 手写 int 版 | 泛型模板版 |
|---|---|---|
| 获取资源 | `explicit MyStack(int cap)` | `explicit MyStack(size_t cap)` |
| 释放资源 | `~MyStack() { delete[] _a; }` | `~MyStack() { delete[] _a; }` |
| 深拷贝 | `MyStack(const MyStack&)` | `MyStack(const MyStack&)` |
| 赋值 | `operator=`（copy-and-swap） | `operator=`（copy-and-swap） |
| 扩容 | 同样成倍扩容 | 同样成倍扩容 |

> **这就是模板的意义**：把"资源管理"这个模式**写一次**，对任意 `T` 生效。
> 反过来说也成立：**会写 `MyStack<T>`，你就已经理解了"所有容器"的骨架**——
> 自己实现 `vector` / `string` 的难度主要不在模板语法，而在异常安全、迭代器、allocator 这些边角。

---

## 十、const 成员函数与 mutable

### 10.1 语法与本质

```cpp
int getYear() const      // ← const 写在参数列表后面
{
    return _year;
}
```

**它修饰的其实是 this 指针**：

```cpp
int getYear() const
// 编译器眼中的形态：
int getYear(const Date* const this)
//           ↑ 指向的内容也不能改了
```

对比非 const 成员函数：

```cpp
void setYear(int y)      { _year = y; }
// void setYear(Date* const this, int y)   ← 指向的内容可以改
```

**所以：const 成员函数内不能修改任何成员变量**（除非该成员被 `mutable` 修饰，10.4 节）。

### 10.2 为什么必须加 —— 一个会真实咬人的错误

```cpp
class MyStackRaw
{
public:
    int StackTop()      // ← 没有 const
    {
        if (top == 0) return -1;
        return a[top - 1];
    }
    // ...
};

void printStack(const MyStackRaw& s)    // 接收 const 引用
{
    s.StackTop();     // ❌ 编译报错！
    // error: passing 'const MyStackRaw' as 'this' argument discards qualifiers
}
```

**错误原因**：`const MyStackRaw&` 承诺"我不修改你"，但 `StackTop()` 的 this 类型是 `MyStackRaw* const`（可修改）。
编译器不允许把"只读权限"的对象传给"可写权限"的函数——**权限不能放大**。

> 这和引用绑定的规则是**同一条原理**，两个场景：
> ```cpp
> const int a = 0;
> int& ar = a;         // ❌ 权限放大，编译报错
> const int& br = a;   // ✅ 权限缩小，可以
> ```

**判定方法**：**这个函数会修改对象的状态吗？不会 → 加 `const`。**

```cpp
int  top() const;       // ✅ 只读
bool empty() const;     // ✅ 只读
size_t size() const;    // ✅ 只读
void push(int val);     // 不加 —— 它会修改对象
void pop();             // 不加 —— 它会修改 _size
```

### 10.3 const 重载：两个版本共存

```cpp
const T& top() const { assert(_size > 0); return _a[_size - 1]; }   // ① const 版
T&       top()       { assert(_size > 0); return _a[_size - 1]; }   // ② 非 const 版
```

**重载决议规则**（实测过）：

| 调用者 | 选中版本 | 效果 |
|---|---|---|
| 非 const 对象 `s.top()` | ② 非 const 版 | 返回 `T&`，可以改：`s.top() = 99;` ✅ |
| const 对象 `cs.top()` | ① const 版 | 返回 `const T&`，只读 |

```
[const] cs.top()=60                       ← const 对象正常读取
[可写 top] s.top=12345                    ← 非 const 对象透过引用修改
```

**为什么两个都要？** 只有 const 版的话，非 const 对象也无法通过 `top()` 修改元素（表达力丢失）；只有非 const 版的话，`const MyStack&` 参数完全无法访问栈顶。这就是标准库容器（`vector::front`、`operator[]`）都提供 const/非 const 成对接口的原因。

### 10.4 mutable：打破 const 的特例

```cpp
class Counter
{
public:
    int getCount() const
    {
        ++count_;      // ✅ 合法：count_ 被 mutable 修饰
        return count_;
    }
private:
    mutable int count_ = 0;   // 允许在 const 函数中修改
};
```

**用途**：缓存、统计计数、互斥锁。典型生产场景：

```cpp
class Cache
{
public:
    string get(const string& key) const
    {
        lock_guard<mutex> lk(mtx_);      // 加锁要改 mutex → mutex 得是 mutable
        // ...
    }
private:
    mutable mutex mtx_;                  // 加锁不算"修改对象的逻辑状态"
    map<string, string> map_;
};
```

> **原则**：`mutable` 用在"物理上要改、逻辑上算 const"的成员上（缓存、锁）。
> 拿它来绕过 const 检查改业务数据 = 埋雷。

---

## 十一、三法则、五法则与零法则（RAII）

### 11.1 三法则（Rule of Three）

> **如果你需要自己写析构函数，那么你几乎一定也需要自己写拷贝构造和拷贝赋值。**

推理链：

```
需要写析构函数
    ↓
说明类里管理着"需要手动释放的资源"（指针/句柄）
    ↓
那么默认的浅拷贝一定会出问题（double free，第六章）
    ↓
所以必须自己写拷贝构造 + 拷贝赋值，改为深拷贝
```

反过来也成立：

```
不需要写析构函数（如 Date）
    ↓
说明类里没有需要手动释放的资源
    ↓
默认的逐成员拷贝就是正确的
    ↓
所以不该自己写拷贝构造/拷贝赋值
```

本文的几个类正好演示了这个规律：

| 类 | 有资源吗 | 需要析构？ | 做法 | 符合三法则吗 |
|---|---|---|---|---|
| `Date` | 无（3 个 int） | ❌ | 什么都不用写（或 `= default`） | ✅ |
| `MyStack` | ✅ `new[]` 的 `_a` | ✅ | 深拷贝 + copy-and-swap | ✅ |
| `MyStack<T>` | ✅ `new[]` 的 `_a` | ✅ | 泛化的深拷贝 + copy-and-swap | ✅ |

### 11.2 另一种选择：`= delete` 禁止拷贝

如果一个类**不该被复制**（文件句柄、互斥锁、线程），与其写一份"危险的浅拷贝"，不如**直接禁止**：

```cpp
class FileHandle
{
public:
    explicit FileHandle(const char* path);
    ~FileHandle();                              // 析构：fclose

    FileHandle(const FileHandle&) = delete;             // 删除拷贝构造
    FileHandle& operator=(const FileHandle&) = delete;  // 删除拷贝赋值
};
```

`= delete` 是 C++11 的**删除函数**语法：明确告诉编译器"这两个函数不许存在"。

```cpp
FileHandle f1("a.txt");
FileHandle f2(f1);        // ❌ 编译报错：use of deleted function
```

**代价**：对象不能按值传递、不能返回、不能放进容器。
**适用场景**：独占型资源（文件句柄、互斥锁、`std::thread`、`std::unique_ptr`）。

> **两条路怎么选？** 取决于**语义**：
> `MyStack<int>` 应该像 `int` 一样能复制（值语义）→ 深拷贝；
> "一个文件句柄"复制不出第二个句柄（所有权语义）→ 禁止拷贝。
> 没有标准答案，只有语义判断。`std::unique_ptr` 走的就是禁止拷贝这条路的正典。

### 11.3 五法则（Rule of Five，C++11）

有了移动语义后，三法则扩展为五法则：

```
析构函数 + 拷贝构造 + 拷贝赋值 + 【移动构造】 + 【移动赋值】
                                    ↑ 新增（第十三章给最小示例）
```

**判断**：如果你写了三法则中的任意一个，就应该考虑是否需要移动语义。

> **一个坑**：如果你只实现了拷贝构造，**编译器不会自动生成移动构造**
> （拷贝构造的存在说明这个类有特殊逻辑，编译器不敢擅自"偷"资源）。
> 所以 `std::move` 会退化成拷贝构造——你以为在移动，其实在拷贝。

### 11.4 零法则（Rule of Zero）—— 现代 C++ 的推荐做法

> **最好的做法是：一个都不写。**

怎么做到？——**用已经管好资源的成员，而不是裸指针。**

```cpp
// 用 vector 重写整个栈：资源管理代码一行都不用写
class Stack
{
public:
    void push(int v)       { v_.push_back(v); }
    void pop()             { v_.pop_back(); }
    int&       top()       { return v_.back(); }
    const int& top() const { return v_.back(); }
    bool empty()  const    { return v_.empty(); }
    size_t size() const    { return v_.size(); }
private:
    std::vector<int> v_;   // 析构/拷贝/扩容/异常安全——全是 vector 的事
};
// 析构、拷贝构造、拷贝赋值：编译器默认生成的全都正确！
```

**为什么默认生成的就对？** 因为 `std::vector` 内部实现了三法则。
编译器为 `Stack` 生成的浅拷贝，实际调用的是 `vector` 的**深拷贝构造**——深拷贝顺着成员的类型"传递"下去了。

对比一下工作量：

```
手写版：构造 + 析构 + 深拷贝 + copy-and-swap + 扩容 ≈ 60 行内存管理代码
vector 版：                                    ≈  7 行业务代码 + 1 个成员
```

> **Rule of Zero 的核心思想**：让每个类只负责一件事。
> **资源管理交给专门的类**（`std::vector` / `std::string` / `std::unique_ptr` / `std::shared_ptr`），
> 你自己的业务类就不需要写任何析构/拷贝/移动函数。
>
> 但注意：**理解三法则仍然极其重要**——你迟早要写那个"管理资源的类"（自己实现 vector、实现连接池），
> 而且这是面试必考。**写容器是三法则的活，用容器是零法则的活。**

### 11.5 RAII：三法则背后的思想

> **RAII = Resource Acquisition Is Initialization，资源获取即初始化**
> **资源的申请绑定到构造函数，释放绑定到析构函数。**

`MyStack` 从写出来那天起就是 RAII 的：

```cpp
MyStack(...)   // ← 构造函数负责申请（new int[cap]）
~MyStack()     // ← 析构函数负责释放（delete[] _a）
```

**RAII 的价值：即使中途抛异常或提前 return，析构函数也一定会被调用。**

```cpp
void func()
{
    MyStack s(100);
    s.push(1);
    riskyOperation();     // ← 这里抛异常了
    return;               // ← 或者提前 return 了
}                         // ✅ s 的析构函数依然会被调用，内存不会泄漏
```

对比 C 风格的二段式——**任何一个提前 return 都会漏掉 `Destroy`**：

```cpp
void func_c_style()
{
    MyStackRaw s;
    s.StackInit(100);
    if (!someCondition()) return;   // 💀 内存泄漏：StackDestroy 永远不会被调用
    // ...
    s.StackDestroy();
}
```

这正是文章开头那个 C 风格栈的"现象 3"——**二段式把一个不可分割的"获取-释放"拆成了两半，中间任何退出路径都是泄漏点**。

**C++11 的智能指针就是 RAII 的标准化产物：**

| 智能指针 | 语义 | 对应本文的哪个类 |
|---|---|---|
| `std::unique_ptr` | 独占所有权，**禁止拷贝** | 11.2 节的 `FileHandle`（`= delete` 的思路） |
| `std::shared_ptr` | 共享所有权，引用计数 | — |
| `std::vector` | 动态数组，**深拷贝** | `MyStack` / `MyStack<T>`（深拷贝的思路） |

```cpp
// 裸指针（需要手动管理）
int* p = new int(10);
// ... 中间 return / 抛异常 → 内存泄漏
delete p;

// unique_ptr（RAII，自动释放）
auto p = std::make_unique<int>(10);
// ... 中间 return / 抛异常都没事
// 出作用域自动 delete ✅
```

---

## 十二、常见陷阱清单（15 条，附实测证据）

> 这一章把全文的坑集中成一份 checklist。每条都是【现象 → 为什么 → 怎么修】。
> 我在学习阶段把它们**全部亲手踩过一遍**——踩过一遍的 bug，比读十遍的"注意事项"记得牢。

### A. 资源管理类

#### 12.1 二段式初始化：对象"活着"不等于"可用"

```cpp
MyStackRaw s;          // 构造完成，对象"活着"
s.push(1);             // 但没调 StackInit —— 会发生什么？
```

实测：

```
栈满                     ← 打印的是"栈满"！
push 之后 size 是多少？答案：0    ← 数据一个都没进去
```

`top == capacity`（0 == 0）被误判为"栈满"，**调用者以为 push 成功了**。
再加上"提前 return 就泄漏"的问题（11.5 节），二段式应彻底淘汰。

**修正**：容量交给构造函数，补一个 `clear()` 用于逻辑清空：

```cpp
explicit MyStack(int cap = 0);
void clear() { _size = 0; }            // 清空数据，但不释放内存
```

#### 12.2 魔法值错误码：`-1` 和"真的存了 -1"无法区分

```cpp
if (top == 0) return -1;     // 栈空用 -1 表示
```

实测：

```
栈顶真的存了 -1，StackTop 返回：-1      ← 和"栈空"一模一样
```

**修正**（三选一）：

```cpp
// ① 输出参数：bool 报告成功与否
bool top(int& out) const { if (_size == 0) return false; out = _a[_size-1]; return true; }

// ② std::optional（C++17，语义最清晰）
std::optional<int> top() const;

// ③ 前置条件契约（STL 的做法）：调用者保证非空，否则未定义行为
int top() const { assert(_size > 0); return _a[_size - 1]; }
```

#### 12.3 静默失败：`push` 满了只打印一句

```cpp
if (top == capacity) { cout << "栈满" << endl; return; }   // 调用者不知道失败了
```

**修正**：返回 `bool`、自动扩容（`MyStack` 的做法）、或抛异常——**总之让失败"可见"**。

#### 12.4 `new[]/delete[]` 与 `malloc/free` 交叉配对

```cpp
int* p = new int[10];
free(p);                    // 💀 未定义行为
```

原因见 5.4 节（数组 cookie）。**修正**：各用各的，C++ 优先 `new[]/delete[]`。

### B. 构造函数 / 析构函数

#### 12.5 校验被后续赋值覆盖

见 4.6 节，实测 `Date(2026,13,40)` 原样通过。**修正**：让非法流程真的拐弯（夹紧/断言/抛异常）。

#### 12.6 初始化列表顺序 ≠ 声明顺序

```cpp
Bad() : b(a), a(10) {}      // ⚠️ 实际先初始化 a 再 b —— 如果反过来依赖，就是 UB
private:
    int a;      // 先声明
    int b;      // 后声明
```

**修正**：把初始化列表写成声明顺序（编译器 `-Wreorder` 会提醒）。

#### 12.7 析构函数里做多余的事 / 危险的事

```cpp
~MyStack()
{
    delete[] _a;
    _a = nullptr; _size = _capacity = 0;   // 多余：对象马上死了
}
```

**修正**：析构只释放资源；**不要抛异常**（栈展开中再次抛出 → `std::terminate`）。

### C. 拷贝控制

#### 12.8 浅拷贝：double free 的标准死法

见 6.5 节，实测两对象指针地址相同（`0x1014790b0`）+ `Trace/BPT trap: 5`。
**修正**：写深拷贝（三法则），或者 `= delete` 禁止拷贝。

#### 12.9 赋值"先删后建"：自赋值时读已释放内存

```cpp
delete[] _a;                      // 如果 other 就是自己 → 内存已释放
_a = new int[other._capacity];    // 再读 other._a → 💀
```

**修正**：copy-and-swap（7.2 节），或至少加自赋值检查。

#### 12.10 手写"和编译器默认版本完全一样"的拷贝构造

```cpp
// 对只有 int 成员的类，这个函数编译器生成的版本逐字等价
Date(const Date& d) { _year = d._year; _month = d._month; _day = d._day; }
```

学习阶段写一遍很有价值（知道默认版本长什么样），生产代码里属于冗余。
**修正**：删掉，或写 `Date(const Date&) = default;` 明确表达意图。

### D. 运算符重载

#### 12.11 返回值 / 引用选错

| 症状 | 修正 |
|---|---|
| `operator+=` 返回 `Date`（值） | 返回 `Date&`（就地修改，返回自己） |
| `operator+` 返回 `Date&`（引用） | 返回 `Date`（新对象；返回局部变量的引用 = 悬垂） |
| 后置 `operator++` 返回 `Date&` | 返回 `Date`（旧值副本是局部变量） |

#### 12.12 只读运算符忘了 `const`

```cpp
Date operator+(int days)        // ⚠️ 忘了 const
const Date d(2026, 9, 1);
Date x = d + 1;                 // ❌ 编译错误：const 对象不能调用非 const 成员函数
```

**修正**：`+` `-`（Date-Date）这类只读运算一律加 `const`。

#### 12.13 位运算与比较混用不加括号

```cpp
return (_year & 400 == 0) || ...;    // 实际是 _year & (400==0) → _year & 0 → 恒假
```

**修正**：写取模 `%`；任何位运算混比较都加括号 `(a & b) == 0`。

### E. 其它

#### 12.14 引用传参的别名陷阱：`s.push(s.top())`

见 9.5 节，ASan 实测 `heap-use-after-free`。
**修正**：`push(T val)` 按值传参；或扩容前先保存副本。

#### 12.15 交换两个指针：传的是"指针的副本"

```cpp
void SwapPtr(int* a, int* b) { int* p = a; a = b; b = p; }   // ⚠️ 只改了形参副本
```

实测：

```
SwapPtr 后 pa==&m? 1 pb==&n? 1  (*pa=1 *pb=2)      ← 外部指针纹丝不动
```

**修正**：要交换"指针本身"，得传指针的引用或二级指针：

```cpp
void SwapPtr(int*& a, int*& b) { int* p = a; a = b; b = p; }      // 引用版
void SwapPtr(int** a, int** b) { int* p = *a; *a = *b; *b = p; }  // 二级指针版
```

> 顺带附赠一个真实的编译期"二义性"错误（很多人写重载时踩到）：
> ```cpp
> void f1() {}
> void f1(int a = 10) {}
> int main() { f1(); }      // error: call to 'f1' is ambiguous
> ```
> `f1()` 没有参数时，两个候选都可行且不分优劣 → 编译报错。
> **缺省参数的重载组合要小心**：一个函数能"无参调用"时，不能再有一个真正的无参重载。

---

## 十三、延伸：成员函数指针与移动语义

### 13.1 成员函数指针：类的"回调"怎么写

普通函数指针可以直接 `pf = &func`。那**类的成员函数**呢？
成员函数多了一个隐含的 `this` 参数——所以它的指针类型和调用方式都不一样：

```cpp
class A
{
public:
    void func() { cout << "回调成功" << endl; }
};

typedef void (A::*pf)();      // 指向 A 的、返回 void 无参的成员函数

int main()
{
    pf pff = nullptr;
    pff = &A::func;           // 注意：取成员函数地址必须写 &

    A aa;
    (aa.*pff)();              // 对象 + .* 调用

    A* pa = &aa;
    (pa->*pff)();             // 指针 + ->* 调用
    return 0;
}
```

**三个语法要点（都实测过）**：

| 要点 | 说明 |
|---|---|
| 类型写法 | `void (A::*pf)()` —— 必须带 `A::` 限定，表示"这是 A 的成员" |
| 取地址 | **必须写 `&A::func`**。省略 `&` 实测报错：`error: call to non-static member function without an object argument` |
| 调用 | 对象用 `(aa.*pff)()`；指针用 `(pa->*pff)()` —— **括号不能省**（`.*` 优先级低于 `()`） |

带参数、批量调度、数据成员指针，同一个套路：

```cpp
int (A::*pm)(int) = &A::add;
cout << (aa.*pm)(5) << endl;                 // 105

void (A::*table[2])() = {&A::func, &A::func};   // 成员版"转移表"
for (auto fp : table) (aa.*fp)();

int A::*pdata = &A::base;                    // 成员数据指针（对比记忆）
cout << aa.*pdata << endl;                   // 100
```

> **和 C 语言的接口**：这就是 C 语言里"转移表 / 回调函数"在 C++ 里的形态。
> 区别是成员函数指针**必须绑定一个对象**才能调用（这就是 `this` 的代价）。

**现代 C++ 的替代品**（工程里更常用）：

```cpp
#include <functional>
std::function<void()> f = [&aa]{ aa.func(); };   // lambda 捕获对象
f();

auto g = std::mem_fn(&A::func);                  // 或 std::mem_fn
g(aa);      // 用对象调用
g(&aa);     // 用指针调用
```

事件回调、定时器、观察者模式的主流写法是 `std::function` + lambda；`.*` 更多出现在理解底层机制或写反射式代码时。

### 13.2 移动语义与 `std::move`（最小示例）

**问题**：

```cpp
MyStack<int> createStack() { MyStack<int> s(1000); /* 塞 1000 个元素 */ return s; }

MyStack<int> s = createStack();
// 拷贝构造：把 1000 个 int 又复制一遍，然后销毁临时的那个——
// 明明它马上就要死了，为什么不直接把它的指针"偷"过来?
```

移动语义就是干这个的：**不复制资源，直接转移所有权**。

```cpp
// 移动构造（C++11）
MyStack(MyStack&& other) noexcept           // 参数是右值引用 &&
    : _a(other._a), _size(other._size), _capacity(other._capacity)   // ① 直接偷指针
{
    other._a = nullptr;                     // ② 源对象置空，防止它析构时释放
    other._size = other._capacity = 0;      // ③ 源对象处于"有效但未指定"状态
}                                           // ④ 全程没有 new / 拷贝！
```

```
拷贝构造：               移动构造：
[src] ──→ [数据]         [src] ──→ nullptr        源对象被"掏空"
                            ↘
[新对象] ──→ [新副本]    [新对象] ──→ [原数据]    指针直接转移

代价：O(n) 拷贝          代价：几个指针赋值，O(1)
```

**`noexcept` 很关键**：移动构造要声明不抛异常，否则 `std::vector` 扩容时不敢用它
（一旦移动中途抛异常，原数据就被破坏了）。`std::vector` 在"移动构造是 noexcept"时才会优先移动——所以给自己的类加移动构造时顺手写 `noexcept`。

```cpp
// std::move：把左值转成右值，从而触发移动构造
MyStack<int> a(1000);
MyStack<int> b(a);                 // 左值 → 拷贝构造
MyStack<int> c(std::move(a));      // std::move(a) 是右值 → 移动构造

// ⚠️ std::move 本身不移动任何东西！
// 它只是一个类型转换（static_cast<MyStack<int>&&>），真正干活的是移动构造函数。
// 移动之后 a 处于"有效但未指定"的状态，不应该再使用（可以安全地析构/重新赋值）。
```

**五法则完整版**：

```cpp
class MyStack
{
public:
    ~MyStack();                              // ① 析构
    MyStack(const MyStack&);                 // ② 拷贝构造
    MyStack& operator=(const MyStack&);      // ③ 拷贝赋值
    MyStack(MyStack&&) noexcept;             // ④ 移动构造 (C++11)
    MyStack& operator=(MyStack&&) noexcept;  // ⑤ 移动赋值 (C++11)
};
```

---

## 附录：速查卡

```
┌─────────────────────────────────────────────────────────────┐
│  对象的一生                                                  │
│                                                             │
│   诞生 ──→ [构造函数] ──→ ...使用... ──→ [析构函数] ──→ 死亡   │
│              ↑                              ↑               │
│          自动调用                        自动调用             │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  拷贝的两种形态                                              │
│                                                             │
│   MyStack s2(s1);   ┐                                       │
│   MyStack s2 = s1;  ┴─→ 拷贝构造（对象诞生）                  │
│                                                             │
│   s2 = s1;             ─→ 拷贝赋值（对象已存在）               │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  浅拷贝 vs 深拷贝                                            │
│                                                             │
│   浅: [_a]──┐                                               │
│            ├──→ [同一块数据]  💀 double free                 │
│       [_a]──┘                                               │
│                                                             │
│   深: [_a]──────→ [副本1]     ✅ 互不干扰                     │
│       [_a]──────→ [副本2]                                   │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  运算符重载返回值速查                                        │
│                                                             │
│   + - * /（产生新值）  → 返回【新对象】+ 标 const             │
│   += -= ++ --（改自己）→ 返回【*this 引用】+ 不标 const       │
│   后置 i++/i--         → 返回【旧值副本】（不能返回引用）      │
│   << >>                → 返回【流引用】，必须是全局/friend     │
│   == <                 → 只写这两个，其余四个推导             │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  三法则                                                      │
│                                                             │
│   写了析构函数 → 也要写拷贝构造 + 拷贝赋值                     │
│   不需要析构   → 拷贝构造/赋值都别写（编译器默认的就对）        │
│                                                             │
│   终极方案：Rule of Zero —— 用 vector/string/智能指针，        │
│             自己一个都不写                                    │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  写类的 checklist                                            │
│                                                             │
│   □ 指针成员在构造函数里初始化了吗？（nullptr 保命）            │
│   □ 单参数构造加 explicit 了吗？                              │
│   □ 初始化列表顺序 = 成员声明顺序了吗？                        │
│   □ 只读函数加 const 了吗？                                   │
│   □ new[] 配 delete[] / malloc 配 free，配对了吗？            │
│   □ 有堆资源的类：写深拷贝，还是 = delete？（三法则）           │
│   □ 拷贝赋值用 copy-and-swap 了吗？（强异常安全）              │
│   □ += 返回引用、+ 返回新对象 + const 了吗？                   │
│   □ 扩容前的自引用（push(s.top())）防住了吗？                  │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  一个类要过关，问自己四个问题                                 │
│                                                             │
│   1. 它拥有资源吗？        → 有 → 动脑子写析构                │
│   2. 它能被复制吗？        → 值语义深拷贝 / 独占语义 =delete   │
│   3. 它的接口是 const 的吗？→ 不改对象就标 const              │
│   4. 它的运算符符合直觉吗？→ 语义怪就别重载                    │
└─────────────────────────────────────────────────────────────┘
```

---

## 结语

回头看这篇文章的路线：

```
"能跑"的 C 风格栈
   ├── 二段式初始化 → 构造函数 / 析构函数（自动调用，退不掉）
   ├── 浅拷贝 → 拷贝构造 + copy-and-swap（资源转移要显式声明）
   ├── 魔法值 / 静默失败 → 返回值 / 断言 / 异常（让失败可见）
   ├── 手写类 → 类模板（同一套逻辑对任意类型生效）
   └── 裸指针 → RAII / Rule of Zero（让资源的生命周期自动跟着对象走）
= "能上工程"的 MyStack<T>
```

这一路走下来，其实只围绕**一个问题**：

> **这个对象"拥有"什么资源？谁来管它的生老病死？**

构造函数管出生，析构函数管死亡，拷贝构造和拷贝赋值决定它能不能被复制、怎么复制。
想清楚这个问题，三法则、深拷贝、copy-and-swap、Rule of Zero 全都是自然结论——**不需要背条款**。

最后送一句我自己踩坑总结的话：

> **写代码时想想"我的对象里有没有借来的资源"：**
> 有资源 → 三法则（构造申请、析构释放、拷贝要深拷贝；或者干脆禁止拷贝）
> 没资源 → 什么都别写，编译器比你写得好

（完）
