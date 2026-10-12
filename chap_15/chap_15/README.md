# 15장. C++ 고급기능 — 실습과제

## 실습과제 1

**일반 함수와 람다식의 차이를 자세히 설명하시오. 일반 함수로 정의하는 대신에 람다식을 사용할 때 장점은 무엇인가?**

일반 함수와 람다식은 둘 다 호출할 수 있는 코드 묶음이지만, 이름의 유무, 정의하는 위치, 외부 변수에 접근하는 방식, 타입이 다르다. 람다식은 C++11에서 도입된 **익명(이름 없는) 함수**를 만드는 기능이다.

### 일반 함수와 람다식의 차이

| 구분 | 일반 함수 | 람다식 |
|---|---|---|
| 이름 | 함수 이름이 있고, 이름으로 호출한다. | 이름이 없다. `auto` 변수나 `std::function`에 저장하면 변수 이름으로 호출할 수 있다. |
| 정의 위치 | 함수 바깥(전역, 클래스 안)에서 정의하며, 다른 함수 안에서는 정의할 수 없다. | 함수 안, 함수의 인자 자리 등 필요한 곳에서 바로 정의한다. |
| 외부 변수 접근 | 다른 함수의 지역변수에 직접 접근할 수 없고, 매개변수나 전역변수를 통해야 한다. | 캡쳐 리스트로 외부 지역변수를 사용한다. `[x]`는 값 캡쳐(읽기만 가능), `[&x]`는 참조 캡쳐(값 변경 가능)이다. |
| 구성 | `반환형 함수명(매개변수) { 바디 }` | `[캡쳐 리스트](매개변수) -> 리턴타입 { 바디 }` 이며 리턴 타입은 생략할 수 있다. |
| 타입 | 함수 타입이며 함수 포인터로 가리킬 수 있다. | 컴파일러만 아는 고유한 타입이라 개발자가 변수 타입을 직접 쓸 수 없고 `auto`를 사용한다. |
| 호출 | `이름(인자)` | 선언과 동시에 `[](int x, int y) { return x + y; }(2, 3)`처럼 바로 호출하거나, 변수에 저장한 뒤 호출한다. |

### 람다식을 사용할 때의 장점

- **간결하고 읽기 쉽다.** 한두 줄짜리 동작을 쓰는 자리에 바로 적을 수 있어서 이름을 짓고 따로 선언·정의할 필요가 없고, 동작이 쓰이는 곳과 정의가 한곳에 모인다.
- **외부 지역변수를 바로 쓸 수 있다.** 캡쳐 리스트 덕분에 별도의 매개변수나 전역변수 없이 주변 변수를 사용한다. 예를 들어 `[&sum]`으로 합계를 지역변수에 누적할 수 있다.
- **함수의 인자로 즉시 전달할 수 있다.** `sort`의 비교 기준, `count_if`의 조건처럼 한 번만 쓰는 동작을 호출하는 자리에서 바로 넘긴다. 강의노트에서 `function` 객체를 인자로 받는 함수는 ROS 라이브러리에서 사용된다고 했으므로, 콜백을 넘기는 ROS 코드에서도 같은 방식이 쓰인다.
- **이름 공간을 어지럽히지 않는다.** 한 곳에서만 쓰는 함수를 전역 이름으로 만들지 않아도 되므로 이름 충돌이 줄어든다.

다만 동작이 길거나 여러 곳에서 재사용되는 경우에는 일반 함수로 정의하는 편이 읽기 좋고 관리하기 쉽다.

---

## 실습과제 2

**23페이지 예제처럼 함수에 인자로 람다식을 사용하는 예제를 인터넷에서 찾아서 실행해보고 소스코드를 자세히 설명하시오.**

`sort`, `count_if`, `for_each`는 마지막 인자로 호출 가능한 것을 받는 STL 알고리즘이다. 이 자리에 람다식을 넘기는 예제로, 소스파일은 [lambda_arg.cpp](./lambda_arg.cpp)이다.

### 소스코드

```cpp
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
```

### 줄 단위 코드 설명

1. `#include <iostream>`, `#include <vector>`, `#include <algorithm>`: 출력용 `cout`, 벡터 `vector`, 알고리즘 `sort`, `count_if`, `for_each`를 사용하기 위한 헤더이다.
2. `using namespace std;`: `std::`를 생략하고 쓰기 위한 선언이다.
3. `vector<int> vec = {5, 2, 9, 1, 7, 3};`: 정수 6개를 가진 벡터를 만든다.
4. `sort(vec.begin(), vec.end(), [](int a, int b) { return a > b; });`: `sort`는 `[begin, end)` 구간을 정렬하며, 세 번째 인자로 두 원소의 순서를 정하는 비교 기준을 받는다. 여기에 람다식을 바로 넘겼다. 캡쳐 리스트 `[]`는 비어 있고, 매개변수 `a`, `b`는 비교할 두 원소이며, `a > b`가 참이면 `a`를 앞에 놓는다. 따라서 내림차순으로 정렬되어 `vec`은 `9 7 5 3 2 1`이 된다.
5. `for (auto& i : vec) cout << i << " ";`와 `cout << endl;`: 범위 기반 for문으로 정렬된 벡터의 원소를 차례로 출력하고 줄을 바꾼다. `auto&`는 원소 타입(`int`)을 컴파일러가 추론하게 하고 참조로 받는다.
6. `int limit = 4;`: 람다식 바깥의 지역변수이다. 람다식은 이 변수를 직접 볼 수 없으므로 캡쳐해야 한다.
7. `count_if(vec.begin(), vec.end(), [limit](int num) { return num > limit; });`: `count_if`는 조건이 참인 원소의 개수를 반환한다. 조건으로 람다식을 넘겼고, 캡쳐 리스트 `[limit]`으로 `limit`의 **값을 캡쳐**했다. 값 캡쳐이므로 람다식 안에서 `limit`을 읽을 수만 있고 바꿀 수는 없다. `num > limit`을 만족하는 원소는 9, 7, 5이므로 `cnt`는 3이다.
8. `cout << limit << "보다 큰 수: " << cnt << "개" << endl;`: 조건 값과 개수를 출력한다.
9. `int sum = 0;`: 합계를 저장할 지역변수이다.
10. `for_each(vec.begin(), vec.end(), [&sum](int num) { sum += num; });`: `for_each`는 구간의 모든 원소에 대해 넘겨준 호출 가능 대상을 한 번씩 실행한다. 캡쳐 리스트 `[&sum]`은 `sum`의 **참조를 캡쳐**하므로 람다식 안에서 바깥 변수 `sum`의 값을 바꿀 수 있다. 원소를 하나씩 `num`으로 받아 `sum`에 더하므로 최종 `sum`은 27이다.
11. `cout << "sum=" << sum << endl;`: 누적된 합계를 출력한다.
12. `return 0;`: 프로그램을 정상 종료한다.

이 예제는 같은 구조의 호출에서 람다식의 캡쳐 방식만 바꿔 가며 쓰는 모습을 보여준다. 4번은 캡쳐 없이 인자만 사용하고, 7번은 값 캡쳐, 10번은 참조 캡쳐를 사용한다.

---

## 실습과제 3

**함수 포인터와 std::function 객체의 차이를 자세히 설명하시오.**

함수 포인터는 함수의 시작 주소를 저장하는 **포인터 변수**이고, `std::function`은 `<functional>` 헤더에 정의된 **템플릿 클래스**로 함수 포인터의 역할을 대신하는 클래스이다. 객체이지만 함수 포인터처럼 호출할 수 있고, 포인터 사용으로 인한 오류를 줄여 준다.

| 구분 | 함수 포인터 | `std::function` |
|---|---|---|
| 정체 | 함수의 시작 주소를 저장하는 포인터 변수 | 호출 가능한 대상을 감싸서 저장하는 템플릿 클래스의 객체 |
| 선언 | `void (*f1)(int, const string&) = func;` | `function<void(int, const string&)> f2 = func;` |
| 저장 가능한 대상 | 일반 함수(와 캡쳐 없는 람다식)만 가리킬 수 있다. | 함수 주소, 함수 객체, 람다식, 멤버함수 포인터 등 호출이 가능한 모든 것을 저장한다. |
| 캡쳐가 있는 람다식 | 저장할 수 없다. | 저장할 수 있다. |
| 멤버 함수 | 객체에 묶인 `a.show`를 그대로 저장할 수 없다(에러). | `function<void(A&)> f4 = &A::show;`처럼 `this` 역할의 매개변수를 추가해 저장할 수 있다. |
| 비어 있는 상태로 호출 | 널 포인터를 호출하면 정의되지 않은 동작이다. | 저장된 대상이 없는 채 호출하면 `std::bad_function_call` 예외가 발생한다. |
| 타입 표기 | `반환형 (*)(매개변수)` | `function<반환형(매개변수)>`로 템플릿 파라미터에 반환형과 매개변수를 쓴다. |
| 부담 | 주소 하나만 저장하므로 가볍다. | 객체이므로 호출 과정이 한 단계 더 있고, 구현에 따라 큰 호출 대상은 동적 메모리를 쓸 수 있다. |

핵심 차이는 저장할 수 있는 범위이다. 함수 포인터는 시그니처가 맞는 일반 함수만 가리키지만, `std::function`은 시그니처(반환형과 매개변수)만 같으면 일반 함수든 람다식이든 함수 객체든 같은 타입으로 다룬다. 그래서 함수의 매개변수 타입으로 `std::function`을 쓰면 일반 함수와 캡쳐가 있는 람다식을 모두 받을 수 있다. 강의노트 20페이지처럼 멤버 함수는 `this`가 첫 매개변수로 필요하므로, `function<void(A*)>`나 `function<void(A&)>`처럼 클래스 타입의 매개변수를 추가해야 하고, `std::mem_fn`을 사용하면 타입을 자동으로 추론할 수 있다.

---

## 실습과제 4

**23페이지 예제처럼 인터넷에서 std::function 클래스를 함수에 매개변수에 사용한 예제를 찾아서 실행해보고 소스코드를 자세히 설명하시오.**

`std::function`을 함수의 매개변수 타입으로 사용하는 예제로, 소스파일은 [function_arg.cpp](./function_arg.cpp)이다. 매개변수가 `std::function`이므로 같은 함수에 일반 함수, `function` 객체, 람다식을 모두 넘길 수 있음을 보여준다.

### 소스코드

```cpp
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
```

### 줄 단위 코드 설명

1. `#include <functional>`: `std::function`이 정의된 헤더이다. 나머지 헤더는 출력, 문자열, 벡터를 위한 것이다.
2. `void process(const vector<int>& vec, function<void(int)> f)`: 벡터와 함수를 받는 함수이다. `function<void(int)>`는 "`int` 하나를 받고 아무것도 반환하지 않는 호출 가능한 대상"을 뜻하는 타입이다. 벡터는 `const` 참조로 받아 복사를 막고 수정도 막았다.
3. `for (auto& i : vec) f(i);`: 벡터의 원소를 하나씩 꺼내 매개변수로 받은 `f`를 호출한다. `f`가 무엇인지는 호출하는 쪽이 정한다.
4. `void calculate(const string& name, int a, int b, function<int(int, int)> f)`: 연산 이름, 두 정수, 연산 방법을 받는다. `function<int(int, int)>`는 템플릿 파라미터 `R(Args...)`에서 반환형 `R`이 `int`, 매개변수 `Args`가 `int, int`인 경우이다.
5. `cout << name << ": " << f(a, b) << endl;`: 전달받은 연산 `f`를 `a`, `b`에 적용한 결과를 이름과 함께 출력한다.
6. `void print(int num)`: 숫자 하나와 공백을 출력하는 일반 함수이다. `process`에 넘길 대상으로 쓴다.
7. `int add(int a, int b)`: 두 수를 더해 반환하는 일반 함수이다. `calculate`에 넘길 대상으로 쓴다.
8. `vector<int> vec = {1, 2, 3, 4, 5};`: 1부터 5까지 가진 벡터를 만든다.
9. `process(vec, print);`: 함수 이름 `print`는 함수의 시작 주소이므로, 이것이 `function<void(int)>` 객체로 변환되어 전달된다. `1 2 3 4 5`가 출력된다.
10. `function<void(int)> f = print;`와 `process(vec, f);`: `print`를 `function` 객체 `f`에 저장한 뒤 그 객체를 전달한다. 결과는 9번과 같이 `1 2 3 4 5`이다. 함수 포인터처럼 쓰지만 포인터가 아니라 객체이다.
11. `process(vec, [](int num) { if (num % 2 == 0) cout << num << " "; });`: 이름 있는 함수를 만들지 않고 람다식을 인자로 바로 넘긴다. 짝수일 때만 출력하므로 `2 4`가 출력된다.
12. `calculate("덧셈(일반 함수)", 2, 3, add);`: 일반 함수 `add`를 `function<int(int, int)>`로 전달한다. `2 + 3`이므로 5가 출력된다.
13. `calculate("곱셈(람다식)", 2, 3, [](int x, int y) { return x * y; });`: 곱셈 람다식을 전달한다. 반환형은 `return x * y;`에서 `int`로 추론되며 결과는 6이다.
14. `int base = 10;`: 람다식에서 캡쳐해 쓸 지역변수이다.
15. `calculate("base+x+y(캡쳐)", 2, 3, [base](int x, int y) { return base + x + y; });`: `base`를 값 캡쳐한 람다식을 전달한다. 캡쳐가 있는 람다식은 함수 포인터에 저장할 수 없지만 `std::function` 매개변수로는 받을 수 있다. 결과는 `10 + 2 + 3`으로 15이다.
16. `return 0;`: 프로그램을 정상 종료한다.

`process`와 `calculate`는 함수 안쪽 코드를 바꾸지 않고, 호출하는 쪽에서 무엇을 넘기느냐에 따라 동작이 달라진다. 함수의 매개변수 타입을 `std::function`으로 두면 일반 함수, 함수 객체, 캡쳐가 있는 람다식을 같은 방식으로 받을 수 있다는 점이 이 예제의 핵심이다.
