#include <utility/ostream.h>
#include <time.h>

using namespace EPOS;

// template<int n>
// struct factorial {
//     enum { val = n * factorial<n-1>::val };
// };

// template<>
// struct factorial<0> {
//     enum { val = 1 };
// };

constexpr unsigned int factorial(unsigned int n) {
    return n == 0 ? 1 : n * factorial(n-1);
}

template<typename ...Args>
constexpr int sum(int first, Args ...more) {
    return first + sum(more...);
}
template<>
constexpr int sum(int first) { return first; }

constexpr int fib(int n) {
    return (n == 0) ? 0 : (n == 1 ? 1 : fib(n-1) + fib(n-2));
}

template<bool condition, typename Then, typename Else>
struct If
{ typedef Then Result; };

template<typename Then, typename Else>
struct If<false, Then, Else>
{ typedef Else Result; };

OStream cout;

int main()
{
    // cout << factorial<10>::val << endl;
    // cout << sum(1) << endl;
    // cout << sum(1,2) << endl;
    // cout << sum(1,2,3,4,5,6,7,8,9,10) << endl;
    // // cout << sum(1,"aa") << endl;
    // cout << fib(0+2) << endl;
    // cout << fib(1+2) << endl;
    // cout << fib(2+2) << endl;
    // cout << fib(3+2) << endl;
    // cout << fib(4+2) << endl;
    cout << "BEG:" << (If<false,char,unsigned>::Result)'a' << endl;

    return 0;
}
