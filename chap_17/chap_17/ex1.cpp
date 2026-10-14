#include<iostream>                          // cout, endl 사용을 위한 헤더파일
#include<memory>                            // 스마트 포인터(shared_ptr, make_shared)가 정의된 헤더파일
using namespace std;                        // std:: 를 생략하기 위한 네임스페이스 선언
int main()                                  // 프로그램 시작 함수
{
    auto ptr1 = make_shared<int>(5);        // int 동적객체를 5로 초기화하여 만들고 ptr1이 가리킴(참조 횟수 1)
    cout << *ptr1 << endl;                  // 포인터처럼 * 연산자로 값을 읽어 5 출력

    shared_ptr<int> ptr2 = ptr1;            // ptr2도 같은 동적객체를 가리킴(참조 횟수 2)
    *ptr2 = 10;                             // ptr2로 값을 10으로 변경(ptr1이 가리키는 객체와 동일)
    cout << *ptr2 << endl;                  // 변경된 값 10 출력

    // delete ptr1;                         // 스마트 포인터는 delete 필요 없음(main 종료 시 자동 해제)

    return 0;                               // 종료 시 ptr2, ptr1 소멸 -> 참조 횟수 0 -> 메모리 자동 해제
}
