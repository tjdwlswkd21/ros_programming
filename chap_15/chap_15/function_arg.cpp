#include <functional>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

void process(const vector<int>& vec, function<void(int)> f)
{
    for (auto& i : vec) f(i);
}

void calculate(const string& name, int a, int b, function<int(int, int)> f)
{
    cout << name << ": " << f(a, b) << endl;
}

void print(int num)
{
    cout << num << " ";
}

int add(int a, int b)
{
    return a + b;
}

int main() {
    vector<int> vec = {1, 2, 3, 4, 5};

    process(vec, print);
    cout << endl;

    function<void(int)> f = print;
    process(vec, f);
    cout << endl;

    cout << "짝수: ";
    process(vec, [](int num) { if (num % 2 == 0) cout << num << " "; });
    cout << endl;

    calculate("덧셈(일반 함수)", 2, 3, add);
    calculate("곱셈(람다식)", 2, 3, [](int x, int y) { return x * y; });
    int base = 10;
    calculate("base+x+y(캡쳐)", 2, 3, [base](int x, int y) { return base + x + y; });
    return 0;
}
