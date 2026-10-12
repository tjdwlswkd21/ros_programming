#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;
using namespace std::placeholders;

bool greater_than(int value, int limit)
{
    return value > limit;
}

void print_scaled(int value, int factor)
{
    cout << value * factor << " ";
}

int main()
{
    vector<int> v = {5, 12, 8, 20, 3, 15};

    int cnt = count_if(v.begin(), v.end(), bind(greater_than, _1, 10));
    cout << "10보다 큰 원소 개수: " << cnt << endl;

    cout << "3배 출력: ";
    for_each(v.begin(), v.end(), bind(print_scaled, _1, 3));
    cout << endl;

    sort(v.begin(), v.end(), bind(greater_than, _1, _2));
    cout << "내림차순 정렬: ";
    for (int n : v)
        cout << n << " ";
    cout << endl;

    return 0;
}
