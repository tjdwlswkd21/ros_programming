#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    vector<int> vec = {5, 2, 9, 1, 7, 3};

    sort(vec.begin(), vec.end(), [](int a, int b) { return a > b; });
    for (auto& i : vec) cout << i << " ";
    cout << endl;

    int limit = 4;
    int cnt = count_if(vec.begin(), vec.end(), [limit](int num) { return num > limit; });
    cout << limit << "보다 큰 수: " << cnt << "개" << endl;

    int sum = 0;
    for_each(vec.begin(), vec.end(), [&sum](int num) { sum += num; });
    cout << "sum=" << sum << endl;
    return 0;
}
