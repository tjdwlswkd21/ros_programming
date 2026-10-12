#include <chrono>
#include <iostream>
#include <thread>
using namespace std;
using namespace std::chrono_literals;

int main()
{
    auto h = 1h;
    auto m = 5min;
    auto s = 20s;
    auto ms = 250ms;

    cout << h << " = " << h.count() << " hours" << endl;
    cout << m << " = " << m.count() << " minutes" << endl;
    cout << s << " = " << s.count() << " seconds" << endl;
    cout << ms << " = " << ms.count() << " milliseconds" << endl;

    auto total = 1h + 30min + 15s;
    cout << "total = " << total << endl;
    cout << "total in seconds = " << chrono::duration_cast<chrono::seconds>(total).count() << endl;

    cout << "500ms 대기 시작" << endl;
    auto start = chrono::steady_clock::now();
    this_thread::sleep_for(500ms);
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
    cout << "대기 완료: " << (elapsed >= 500ms ? "500ms 이상 경과" : "시간 부족") << endl;

    return 0;
}
