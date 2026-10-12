# 16장. C++ 고급기능-2 — 실습과제

## 실습과제 1

**아래는 std::bind 함수의 선언이다. 여기서 매개변수 선언에서 &&의 의미를 설명하라.**

```cpp
template< class F, class... Args >
/*unspecified*/ bind(F&& f, Args&&... args);
```

`F&&`와 `Args&&...`의 `&&`는 **포워딩 참조(forwarding reference, 보편 참조)** 이다. 강의노트에서는 `자료형&& x`를 우측값(상수, 임시객체) 참조변수라고 설명하는데, 이는 자료형이 `int&&`, `string&&`처럼 이미 정해진 경우의 의미이다. `F`와 `Args`처럼 **템플릿 매개변수로 추론되는 자료형에 `&&`가 붙은 경우**에는 인자로 넘어온 값의 종류에 따라 좌측값과 우측값을 모두 받을 수 있다.

| 구분 | 선언 예 | 받을 수 있는 인자 |
|---|---|---|
| 좌측값 참조 | `int& x` | 좌측값(변수)만 |
| 우측값 참조 | `int&& x` | 우측값(상수, 임시객체)만 |
| 포워딩 참조 | `template<class T> void f(T&& x)` | 좌측값과 우측값 모두 |

### 동작 원리

인자로 좌측값을 넘기면 `T`가 `int&`로 추론되고, 참조 축약(reference collapsing) 규칙에 따라 `int& &&`가 `int&`가 되어 좌측값 참조로 동작한다. 우측값을 넘기면 `T`가 `int`로 추론되어 `int&&`, 즉 우측값 참조로 동작한다.

```cpp
int a = 10;
auto f1 = std::bind(old_fn, a, 20, 30);
auto f2 = std::bind(old_fn, 10, 20, 30);
```

`f1`의 `a`는 좌측값이므로 `Args`의 첫 번째 자료형이 `int&`로 추론되고, `f2`의 `10`은 우측값이므로 `int`로 추론된다. 같은 `bind` 함수 하나로 두 경우를 모두 처리할 수 있다.

### 왜 &&를 쓰는가

`std::bind`는 받은 함수와 인자를 그대로 저장했다가 나중에 호출해야 한다. 인자가 좌측값이든 우측값이든 값의 종류를 유지한 채 내부로 전달(perfect forwarding)하기 위해 `std::forward<F>(f)`, `std::forward<Arg_i>(arg_i)` 형태로 넘기며, 그 전제가 포워딩 참조이다. `&`만 쓰면 `bind(old_fn, 10, 20, 30)`처럼 상수나 임시객체를 넘길 수 없고, `&&`만 있는 비템플릿 형태라면 변수를 넘길 수 없다. 또한 `bind`가 저장하는 객체는 `std::decay<F>::type`, `std::decay<Arg_i>::type` 형이므로 인자는 참조가 아니라 **복사 또는 이동되어 저장**되며, 참조로 넘기려면 `std::ref`, `std::cref`로 감싸야 한다(cppreference의 std::bind 문서 기준).

---

## 실습과제 2

**인터넷에서 함수의 인자로 std::bind 함수를 사용한 예제를 찾아서 실행해보고 소스코드를 자세히 설명하시오.**

C++ 알고리즘 함수(`count_if`, `for_each`, `sort`)가 요구하는 콜백함수의 형식과 기존 함수의 매개변수 형태가 맞지 않을 때, `std::bind`로 형태만 바꿔서 함수의 인자로 전달하는 예제이다.

### 소스코드 (bind_algorithm.cpp)

```cpp
 1  #include <algorithm>
 2  #include <functional>
 3  #include <iostream>
 4  #include <vector>
 5  using namespace std;
 6  using namespace std::placeholders;
 7
 8  bool greater_than(int value, int limit)
 9  {
10      return value > limit;
11  }
12
13  void print_scaled(int value, int factor)
14  {
15      cout << value * factor << " ";
16  }
17
18  int main()
19  {
20      vector<int> v = {5, 12, 8, 20, 3, 15};
21
22      int cnt = count_if(v.begin(), v.end(), bind(greater_than, _1, 10));
23      cout << "10보다 큰 원소 개수: " << cnt << endl;
24
25      cout << "3배 출력: ";
26      for_each(v.begin(), v.end(), bind(print_scaled, _1, 3));
27      cout << endl;
28
29      sort(v.begin(), v.end(), bind(greater_than, _1, _2));
30      cout << "내림차순 정렬: ";
31      for (int n : v)
32          cout << n << " ";
33      cout << endl;
34
35      return 0;
36  }
```

### 라인단위 코드 설명

- **1~4행**: `count_if`, `for_each`, `sort`를 위한 `<algorithm>`, `std::bind`와 placeholders를 위한 `<functional>`, 입출력을 위한 `<iostream>`, 벡터를 위한 `<vector>`를 포함한다.
- **5행**: `std::` 접두어를 생략하기 위해 `std` 네임스페이스를 사용한다.
- **6행**: `_1`, `_2`를 `std::placeholders::` 없이 쓰기 위해 placeholders 네임스페이스를 사용한다.
- **8~11행**: 기존 함수 `greater_than`이다. 매개변수가 `value`, `limit` 두 개이며 `value`가 `limit`보다 크면 `true`를 리턴한다.
- **13~16행**: 기존 함수 `print_scaled`이다. `value`에 `factor`를 곱한 값을 공백과 함께 출력한다.
- **20행**: 정수 6개를 가진 벡터 `v`를 만든다.
- **22행**: `count_if`는 원소 하나를 받아 `bool`을 리턴하는 콜백함수를 요구하지만 `greater_than`은 매개변수가 두 개라 그대로 쓸 수 없다. `bind(greater_than, _1, 10)`은 `limit`을 10으로 묶고 `value`는 `_1`로 남겨 `bool (int)` 형태의 새 함수를 만든다. 따라서 `count_if`가 각 원소를 넘겨 호출하면 `greater_than(원소, 10)`이 실행되고, 10보다 큰 원소의 개수가 `cnt`에 저장된다.
- **23행**: `cnt`를 출력한다.
- **26행**: `for_each`는 원소 하나를 받는 콜백함수를 요구한다. `bind(print_scaled, _1, 3)`은 `factor`를 3으로 묶어서 `print_scaled(원소, 3)`을 호출하는 함수를 만들고, 모든 원소에 대해 3배한 값이 출력된다.
- **29행**: `sort`는 두 원소를 받아 `bool`을 리턴하는 비교함수를 요구한다. `bind(greater_than, _1, _2)`는 두 인자를 묶지 않고 그대로 전달하므로 `greater_than(a, b)`와 같다. 앞 원소가 뒤 원소보다 클 때 `true`이므로 내림차순으로 정렬된다.
- **30~33행**: 정렬된 벡터를 범위 기반 `for`문으로 출력한다.
- **35행**: 프로그램을 정상 종료한다.

### 실행 결과

```
$ g++ -std=c++20 -o bind_algorithm bind_algorithm.cpp
$ ./bind_algorithm
10보다 큰 원소 개수: 3
3배 출력: 15 36 24 60 9 45 
내림차순 정렬: 20 15 12 8 5 3 
```

`{5, 12, 8, 20, 3, 15}` 중 10보다 큰 값은 12, 20, 15의 3개이다. 3배 출력은 원래 순서(정렬 전)대로 15, 36, 24, 60, 9, 45이고, 마지막 줄은 `sort` 후의 내림차순 결과이다.

---

## 실습과제 3

**인터넷에서 std::chrono_literals 을 사용한 예제를 찾아서 실행해보고 소스코드를 자세히 설명하시오.**

시간 단위 리터럴(`h`, `min`, `s`, `ms`)로 `std::chrono::duration` 객체를 만들고, 값을 출력하거나 더하거나 `sleep_for`의 인자로 사용하는 예제이다.

### 소스코드 (chrono_literals_example.cpp)

```cpp
 1  #include <chrono>
 2  #include <iostream>
 3  #include <thread>
 4  using namespace std;
 5  using namespace std::chrono_literals;
 6
 7  int main()
 8  {
 9      auto h = 1h;
10      auto m = 5min;
11      auto s = 20s;
12      auto ms = 250ms;
13
14      cout << h << " = " << h.count() << " hours" << endl;
15      cout << m << " = " << m.count() << " minutes" << endl;
16      cout << s << " = " << s.count() << " seconds" << endl;
17      cout << ms << " = " << ms.count() << " milliseconds" << endl;
18
19      auto total = 1h + 30min + 15s;
20      cout << "total = " << total << endl;
21      cout << "total in seconds = " << chrono::duration_cast<chrono::seconds>(total).count() << endl;
22
23      cout << "500ms 대기 시작" << endl;
24      auto start = chrono::steady_clock::now();
25      this_thread::sleep_for(500ms);
26      auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
27      cout << "대기 완료: " << (elapsed >= 500ms ? "500ms 이상 경과" : "시간 부족") << endl;
28
29      return 0;
30  }
```

### 라인단위 코드 설명

- **1~3행**: 시간 라이브러리 `<chrono>`, 입출력 `<iostream>`, `sleep_for`가 있는 `<thread>`를 포함한다.
- **4행**: `std` 네임스페이스를 사용한다.
- **5행**: 시간 리터럴 연산자가 선언된 `std::chrono_literals` 네임스페이스를 사용한다. 이 줄이 없으면 `1h`, `250ms` 같은 표기를 쓸 수 없다.
- **9~12행**: `1h`는 `operator""h(1)`, `5min`은 `operator""min(5)`, `20s`는 `operator""s(20)`, `250ms`는 `operator""ms(250)`을 호출하며, 각각 `std::chrono::hours`, `minutes`, `seconds`, `milliseconds` 형의 객체를 리턴한다. `auto`가 이 자료형을 추론한다.
- **14~17행**: `duration` 객체를 `cout`으로 출력하면 값과 단위 기호가 함께 출력되고(`1h`, `5min`), `count()`는 단위를 뺀 숫자만 리턴한다. `duration`의 `<<` 출력은 C++20에서 지원되므로 `-std=c++20`으로 컴파일해야 한다.
- **19행**: 서로 다른 단위의 duration을 더하면 더 작은 단위로 자동 변환되어 `seconds` 형이 된다. 1시간 30분 15초는 5415초이다.
- **20~21행**: `total`을 출력하고, `duration_cast<chrono::seconds>`로 초 단위로 명시적으로 변환한 숫자를 `count()`로 출력한다.
- **23~24행**: 대기 시작을 알리고, 단조 증가하는 시계 `steady_clock`으로 현재 시각을 `start`에 저장한다.
- **25행**: `sleep_for`는 duration 형을 인자로 받으므로 `500ms` 리터럴을 바로 넘겨 현재 스레드를 500밀리초 동안 멈춘다. `sleep_for(500)`처럼 단위 없는 숫자보다 단위가 코드에 드러난다.
- **26행**: 현재 시각에서 `start`를 뺀 경과 시간을 밀리초 단위로 변환해 `elapsed`에 저장한다.
- **27행**: `elapsed`와 `500ms`를 직접 비교하여 500ms 이상 경과했는지 출력한다. `sleep_for`는 지정한 시간 이상 대기하므로 조건이 참이 된다.
- **29행**: 프로그램을 정상 종료한다.

### 실행 결과

```
$ g++ -std=c++20 -o chrono_literals_example chrono_literals_example.cpp
$ ./chrono_literals_example
1h = 1 hours
5min = 5 minutes
20s = 20 seconds
250ms = 250 milliseconds
total = 5415s
total in seconds = 5415
500ms 대기 시작
대기 완료: 500ms 이상 경과
```

각 리터럴이 출력될 때는 단위 기호(`h`, `min`, `s`, `ms`)가 붙고 `count()`는 숫자만 출력한다. `total`은 `1h + 30min + 15s`가 초 단위로 합쳐져 `5415s`로 출력된다.
