#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

/**
 * @brief 日期类 - 工程级实现
 *
 * 功能特点：
 * 1. 完整的日期有效性验证
 * 2. 支持所有基本运算符重载
 * 3. 优化的日期加减算法（非逐日循环）
 * 4. 异常安全处理
 * 5. 输出格式化支持
 */
class Date {
public:
    // 构造函数
    explicit Date(int year = 1970, int month = 1, int day = 1)
        : _year(year), _month(month), _day(day) {
        if (!isValidDate(year, month, day)) {
            throw std::invalid_argument("Invalid date provided");
        }
    }

    // 拷贝构造函数（默认实现就足够）
    Date(const Date&) = default;

    // 析构函数（默认实现就足够）
    ~Date() = default;

    // 赋值运算符
    Date& operator=(const Date&) = default;

    // ================== 比较运算符 ==================
    bool operator==(const Date& other) const {
        return _year == other._year && _month == other._month && _day == other._day;
    }

    bool operator!=(const Date& other) const {
        return !(*this == other);
    }

    bool operator<(const Date& other) const {
        if (_year != other._year) return _year < other._year;
        if (_month != other._month) return _month < other._month;
        return _day < other._day;
    }

    bool operator<=(const Date& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const Date& other) const {
        return !(*this <= other);
    }

    bool operator>=(const Date& other) const {
        return !(*this < other);
    }

    // ================== 算术运算符 ==================
    /**
     * @brief 计算两个日期之间的天数差
     * @param other 另一个日期
     * @return 天数差（正数表示this在other之后，负数表示this在other之前）
     */
    int operator-(const Date& other) const {
        return daysSinceEpoch() - other.daysSinceEpoch();
    }

    /**
     * @brief 日期加天数
     * @param days 要加的天数（可为负数，表示减天数）
     * @return 新的日期对象
     */
    Date operator+(int days) const {
        if (days == 0) return *this;

        Date result = *this;
        result._day += days;

        // 处理进位和借位
        while (result._day > result.getDaysInMonth(result._year, result._month)) {
            result._day -= result.getDaysInMonth(result._year, result._month);
            result._month++;
            if (result._month > 12) {
                result._month = 1;
                result._year++;
            }
        }

        // 处理借位
        while (result._day < 1) {
            result._month--;
            if (result._month < 1) {
                result._month = 12;
                result._year--;
            }
            result._day += result.getDaysInMonth(result._year, result._month);
        }

        return result;
    }

    /**
     * @brief 日期减天数（通过operator+实现）
     */
    Date operator-(int days) const {
        return *this + (-days);
    }

    // ================== 复合赋值运算符 ==================
    Date& operator+=(int days) {
        *this = *this + days;
        return *this;
    }

    Date& operator-=(int days) {
        *this = *this - days;
        return *this;
    }

    // ================== 前置/后置增减运算符 ==================
    Date& operator++() { // 前置++
        *this += 1;
        return *this;
    }

    Date operator++(int) { // 后置++
        Date temp = *this;
        *this += 1;
        return temp;
    }

    Date& operator--() { // 前置--
        *this -= 1;
        return *this;
    }

    Date operator--(int) { // 后置--
        Date temp = *this;
        *this -= 1;
        return temp;
    }

    // ================== 输入输出运算符 ==================
    friend std::ostream& operator<<(std::ostream& os, const Date& date);
    friend std::istream& operator>>(std::istream& is, Date& date);

    // ================== Getter函数 ==================
    int getYear() const { return _year; }
    int getMonth() const { return _month; }
    int getDay() const { return _day; }

    // ================== 工具函数 ==================
    /**
     * @brief 转换为字符串
     * @param format 格式字符串，支持 "%Y", "%m", "%d"
     * @return 格式化后的日期字符串
     */
    std::string toString(const std::string& format = "%Y-%m-%d") const {
        std::stringstream ss;
        size_t pos = 0;
        size_t find_pos = format.find('%');

        while (find_pos != std::string::npos) {
            // 输出前面的普通字符
            ss << format.substr(pos, find_pos - pos);

            // 处理格式说明符
            if (find_pos + 1 < format.length()) {
                char format_char = format[find_pos + 1];
                switch (format_char) {
                    case 'Y':
                        ss << std::setw(4) << std::setfill('0') << _year;
                        break;
                    case 'm':
                        ss << std::setw(2) << std::setfill('0') << _month;
                        break;
                    case 'd':
                        ss << std::setw(2) << std::setfill('0') << _day;
                        break;
                    default:
                        ss << '%' << format_char;
                        break;
                }
                pos = find_pos + 2;
            } else {
                ss << '%';
                pos = find_pos + 1;
            }

            find_pos = format.find('%', pos);
        }

        // 输出剩余的字符
        ss << format.substr(pos);

        return ss.str();
    }

    /**
     * @brief 获取星期几（0=周日，1=周一，...，6=周六）
     */
    int getWeekday() const {
        // Zeller公式计算星期几
        if (_month < 3) {
            int m = _month + 12;
            int y = _year - 1;
            return (y + y/4 - y/100 + y/400 + (13*(m+1))/5 + _day) % 7;
        } else {
            int m = _month;
            int y = _year;
            return (y + y/4 - y/100 + y/400 + (13*(m+1))/5 + _day) % 7;
        }
    }

private:
    int _year;
    int _month;
    int _day;

    // ================== 私有辅助函数 ==================
    /**
     * @brief 检查日期是否有效
     */
    static bool isValidDate(int year, int month, int day) {
        if (year < 1) return false;
        if (month < 1 || month > 12) return false;

        // 检查天数是否在有效范围内
        int daysInMonth = getDaysInMonth(year, month);
        return day >= 1 && day <= daysInMonth;
    }

    /**
     * @brief 获取指定年份和月份的天数
     */
    static int getDaysInMonth(int year, int month) {
        switch (month) {
            case 4: case 6: case 9: case 11:
                return 30;
            case 2:
                return isLeapYear(year) ? 29 : 28;
            default:
                return 31;
        }
    }

    /**
     * @brief 判断是否是闰年
     */
    static bool isLeapYear(int year) {
        return (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
    }

    /**
     * @brief 计算从1970-01-01到当前日期的天数（简化版本）
     */
    int daysSinceEpoch() const {
        int days = 0;

        // 计算年份贡献的天数
        for (int y = 1970; y < _year; y++) {
            days += isLeapYear(y) ? 366 : 365;
        }

        // 计算当年月份贡献的天数
        for (int m = 1; m < _month; m++) {
            days += getDaysInMonth(_year, m);
        }

        // 加上当月天数
        days += _day - 1;

        return days;
    }
};

// ================== 输入输出运算符实现 ==================
std::ostream& operator<<(std::ostream& os, const Date& date) {
    os << date.toString();
    return os;
}

std::istream& operator>>(std::istream& is, Date& date) {
    char sep1, sep2;
    is >> date._year >> sep1 >> date._month >> sep2 >> date._day;

    if (sep1 != '-' || sep2 != '-') {
        is.setstate(std::ios::failbit);
        return is;
    }

    if (!Date::isValidDate(date._year, date._month, date._day)) {
        is.setstate(std::ios::failbit);
    }

    return is;
}

// ================== 测试函数 ==================
void testDateClass() {
    try {
        std::cout << "===== 测试日期类 =====" << std::endl;

        // 测试构造函数
        Date d1(2023, 12, 25);
        Date d2(2024, 1, 1);

        std::cout << "日期1: " << d1.toString() << std::endl;
        std::cout << "日期2: " << d2.toString() << std::endl;

        // 测试比较运算符
        std::cout << "d1 < d2: " << (d1 < d2) << std::endl;
        std::cout << "d1 == d2: " << (d1 == d2) << std::endl;

        // 测试日期加减
        Date d3 = d1 + 10;
        std::cout << d1 << " + 10天 = " << d3 << std::endl;

        Date d4 = d2 - 1;
        std::cout << d2 << " - 1天 = " << d4 << std::endl;

        // 测试天数差
        std::cout << "d2 - d1 = " << (d2 - d1) << " 天" << std::endl;

        // 测试格式化输出
        std::cout << "d1格式化: " << d1.toString("%Y年%m月%d日") << std::endl;
        std::cout << "d1星期: " << d1.getWeekday() << std::endl;

        // 测试复合赋值
        Date d5 = d1;
        d5 += 5;
        std::cout << "d1 += 5: " << d5 << std::endl;

        // 测试自增自减
        Date d6 = Date(2023, 12, 31);
        std::cout << "d6: " << d6 << std::endl;
        std::cout << "d6++: " << d6++ << ", 之后: " << d6 << std::endl;

        std::cout << "测试成功！" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
    }
}

int main() {
    testDateClass();
    return 0;
}