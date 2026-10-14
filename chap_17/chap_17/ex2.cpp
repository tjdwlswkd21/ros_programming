#include<iostream>                                              // cout, endl 사용을 위한 헤더파일(원본에는 빠져 있어 추가)
#include<memory>                                                // 스마트 포인터가 정의된 헤더파일
using namespace std;                                            // std:: 생략
class CTest                                                     // 테스트용 클래스
{
    int test;                                                   // private 멤버변수
public:
    CTest(int value) { test = value; cout << "생성자호출" << endl; }  // 생성자: 값 저장 후 메시지 출력
    ~CTest() { cout << "소멸자호출" << endl; }                  // 소멸자: 객체가 해제될 때 자동 호출
    int GetTest() { return test; }                              // test 값을 반환하는 getter
    void SetTest(int value) { test = value; }                   // test 값을 변경하는 setter
};

int main()                                                      // 프로그램 시작 함수
{
    auto ptr1 = make_shared<CTest>(100);                        // CTest 객체를 생성자 인자 100으로 만들고 ptr1이 가리킴(생성자호출 출력)
    cout << ptr1->GetTest() << endl;                            // -> 연산자로 멤버함수 호출, 100 출력

    shared_ptr<CTest> ptr2 = ptr1;                              // ptr2도 같은 객체를 가리킴(참조 횟수 2)
    ptr2->SetTest(200);                                         // ptr2로 값을 200으로 변경
    cout << ptr2->GetTest() << endl;                            // 변경된 값 200 출력

    // delete ptr2;                                             // delete 필요 없음

    return 0;                                                   // 종료 시 참조 횟수 0 -> 객체 자동 해제(소멸자호출 출력)
}
