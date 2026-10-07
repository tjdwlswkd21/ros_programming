# 14장. ROS2 패키지 파일 — 실습과제

## 실습과제 1-1

**패키지를 구성하는 가장 중요한 필수 파일 2가지는 무엇인가?**

| 파일 | 역할 |
|---|---|
| `package.xml` | 패키지 설정 파일. 패키지 이름, 버전, 설명, 관리자(maintainer), 라이선스, 의존성 패키지 등 **패키지의 메타 정보**를 XML 형식으로 기술한다. 모든 ROS 패키지는 패키지당 반드시 1개를 포함해야 한다. |
| `CMakeLists.txt` | 빌드 설정 파일. C++ 패키지(`ament_cmake`)에서 CMake가 읽는 파일로, 실행 파일 생성(`add_executable`), 의존성 탐색(`find_package`), 링크, 설치(`install`) 등 **빌드 방법**을 기술한다. |

- 두 파일의 역할 분담: `package.xml`은 "이 패키지가 무엇이고 무엇에 의존하는가"를, `CMakeLists.txt`는 "소스 코드를 어떻게 빌드하고 어디에 설치하는가"를 정의한다.
- 두 파일에 적는 **패키지 이름은 서로 일치**해야 한다. `package.xml`의 `<name>`과 `CMakeLists.txt`의 `project()`가 다르면 빌드 시 에러가 발생한다.
- 참고: 파이썬 패키지(`ament_python`)는 `CMakeLists.txt` 대신 `setup.py`, `setup.cfg`가 빌드 설정을 맡는다. 그래도 `package.xml`은 모든 ROS 패키지에 공통으로 필요하다.

---

## 실습과제 1-2

**xml 파일형식에 대하여 조사하시오.**

### 1. XML이란

XML(eXtensible Markup Language, 확장 가능한 마크업 언어)은 W3C가 제정한 **데이터를 구조적으로 표현하기 위한 텍스트 기반 마크업 언어**이다. HTML처럼 태그(`<tag>`)를 사용하지만, HTML이 "화면에 어떻게 보일지"에 초점을 둔 것과 달리 XML은 **"데이터가 무엇인지"를 기술**하는 데 초점을 둔다. 태그 이름을 사용자가 직접 정의할 수 있어서 "확장 가능"이라고 한다.

### 2. 기본 문법 규칙

| 규칙 | 설명 |
|---|---|
| XML 선언 | 문서 맨 앞에 `<?xml version="1.0"?>`로 XML 버전을 선언한다. |
| 루트 요소 1개 | 문서 전체를 감싸는 최상위 요소가 반드시 하나만 있어야 한다. (package.xml에서는 `<package>`) |
| 시작/종료 태그 쌍 | `<name>...</name>`처럼 열면 반드시 닫아야 한다. 내용이 없으면 `<tag/>`로 자체 종료할 수 있다. |
| 올바른 중첩 | 태그는 열린 순서의 역순으로 닫아야 한다. |
| 대소문자 구분 | `<Name>`과 `<name>`은 서로 다른 태그이다. |
| 속성 | 시작 태그 안에 `이름="값"` 형태로 쓰며, 값은 반드시 따옴표로 감싼다. (예: `<package format="3">`) |
| 주석 | `<!-- 주석 -->` |
| 특수문자 | `<`, `>`, `&` 등은 `&lt;`, `&gt;`, `&amp;`로 표기한다. |

위 규칙을 지킨 문서를 **well-formed(올바르게 구성된)** XML이라고 한다.

### 3. 구조적 특징

- **계층(트리) 구조**: 요소가 부모-자식 관계로 중첩되어 트리를 이룬다.
- **자기 기술적(self-describing)**: 태그 이름만 보아도 데이터의 의미를 알 수 있다.
- **플랫폼·언어 독립적**: 단순 텍스트이므로 운영체제나 프로그래밍 언어에 상관없이 읽고 쓸 수 있다.
- **스키마 검증 가능**: DTD, XSD(XML Schema) 등으로 "어떤 태그가 어떤 순서로 와야 하는가"를 정의하고, 문서가 그 규칙을 만족하는지(**valid**) 검증할 수 있다.

### 4. ROS2 `package.xml`에서의 XML

```xml
<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd"
schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>my_first_ros_rclcpp_pkg</name>
  <version>0.0.0</version>
  <description>TODO: Package description</description>
  <maintainer email="pyo@robotis.com">pyo</maintainer>
  <license>TODO: License declaration</license>

  <buildtool_depend>ament_cmake</buildtool_depend>

  <depend>rclcpp</depend>
  <depend>std_msgs</depend>

  <test_depend>ament_lint_auto</test_depend>
  <test_depend>ament_lint_common</test_depend>

  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
```

| 줄 | XML 관점의 설명 |
|---|---|
| `<?xml version="1.0"?>` | XML 선언. 이 문서가 XML 1.0 문법을 따른다는 의미 |
| `<?xml-model href="...package_format3.xsd" ...?>` | 처리 명령(processing instruction). 이 문서를 검증할 **스키마(XSD) 위치**를 알려준다. |
| `<package format="3">` | **루트 요소**. `format="3"`은 속성으로, package.xml 형식의 버전이 3임을 뜻한다. |
| `<name>`, `<version>`, `<description>`, `<license>` | 자식 요소. 시작 태그와 종료 태그 사이에 텍스트 값을 가진다. |
| `<maintainer email="pyo@robotis.com">pyo</maintainer>` | 속성(`email`)과 텍스트 값(`pyo`)을 함께 가진 요소 |
| `<export>` 안의 `<build_type>` | 요소의 **중첩** 예. `export`의 자식으로 `build_type`이 들어간다. |

트리 구조로 나타내면 다음과 같다.

```
package (format="3")
├── name
├── version
├── description
├── maintainer (email="...")
├── license
├── buildtool_depend
├── depend  (rclcpp)
├── depend  (std_msgs)
├── test_depend (ament_lint_auto)
├── test_depend (ament_lint_common)
└── export
    └── build_type
```

---

## 실습과제 1-3

**ament_cmake와 CMake의 차이를 설명하시오.**

### 1. CMake

CMake(Cross-Platform Make)는 **범용 크로스 플랫폼 빌드 시스템 생성 도구**이다. `CMakeLists.txt`에 빌드 방법을 기술하면, 이를 읽어 각 환경에 맞는 빌드 파일(Makefile, Ninja 파일, Visual Studio 프로젝트 등)을 생성한다. ROS와는 무관하게 쓰이는 일반 도구이며, Make가 유닉스 계열 위주인 것과 달리 리눅스, BSD, macOS, 윈도우를 모두 지원한다. ROS가 CMake를 쓰는 이유도 패키지를 멀티 플랫폼에서 빌드할 수 있게 하기 위해서이다.

### 2. ament_cmake

ament_cmake는 ROS 2의 빌드 시스템 **ament** 안에서 **CMake 기반 패키지(주로 C/C++)를 위한 빌드 시스템**이다. 별도의 새로운 빌드 도구가 아니라, **CMake 위에 얹은 ROS 2 전용 매크로·함수 모음**이다. ROS 공식 문서도 ament_cmake를 "ROS 2에서 CMake 기반 패키지를 위한 빌드 시스템"이라고 설명하며, 사용 전에 CMake 기본을 알아야 한다고 안내한다. ROS Answers의 설명에 따르면 ament_cmake의 핵심 역할은 서로 의존하는 여러 패키지를 개발할 때 편하도록 CMake 매크로를 제공하는 것이고, 이 매크로들이 ROS 패키지를 CMake로 빌드하는 데 필수는 아니다.

### 3. 비교

| 구분 | CMake | ament_cmake |
|---|---|---|
| 성격 | 범용 빌드 시스템 생성기 | ROS 2용 CMake 확장(매크로·함수 모음) |
| 사용 범위 | ROS와 무관하게 모든 C/C++ 프로젝트 | ROS 2의 C/C++ 패키지 |
| 기본 설정 파일 | `CMakeLists.txt` | 같은 `CMakeLists.txt`에 `find_package(ament_cmake REQUIRED)`로 불러와 사용 |
| 패키지 메타정보 | 없음 (프로젝트 이름, 버전 정도만 `project()`에 기술) | `package.xml`과 연동하여 이름·의존성·빌드 타입 관리 |
| 패키지 등록 | 없음 | `ament_package()`가 ament 인덱스에 패키지를 등록하고, 다른 패키지가 `find_package`로 찾을 수 있도록 설정 파일을 생성 |
| 의존성 지정 | `target_link_libraries`, `target_include_directories` 등을 직접 작성 | `ament_target_dependencies()` 한 줄로 헤더, 라이브러리, 하위 의존성까지 처리 |
| 테스트·린트 | 직접 구성 | `ament_lint_auto` 등 ROS 2 표준 린트·테스트 도구 연동 |

### 4. 수업 예제 코드로 본 차이

```cmake
cmake_minimum_required(VERSION 3.5)      # (순수 CMake) 최소 CMake 버전
project(my_first_ros_rclcpp_pkg)         # (순수 CMake) 프로젝트 이름

find_package(ament_cmake REQUIRED)       # (ament_cmake) ament_cmake 매크로 사용 선언
find_package(rclcpp REQUIRED)            # (CMake 명령, ROS 패키지 탐색)
find_package(std_msgs REQUIRED)

add_executable(${PUBLISHER_NODE_NAME} src/publisher/main.cpp src/publisher/counter.cpp)
                                         # (순수 CMake) 실행 파일 정의
ament_target_dependencies(${PUBLISHER_NODE_NAME} ${dependencies})
                                         # (ament_cmake) 의존성을 한 번에 연결
install(TARGETS ${PUBLISHER_NODE_NAME} DESTINATION lib/${PROJECT_NAME})
                                         # (순수 CMake) 설치 경로 지정

ament_package()                          # (ament_cmake) 패키지 등록, 항상 마지막에 1회 호출
```

- `ament_package()`는 패키지당 **정확히 한 번만** 호출해야 하며, `CMakeLists.txt`의 정보를 모으기 때문에 **가장 마지막에 호출**하는 것이 권장된다. 이 호출이 `package.xml`을 설치하고 패키지를 ament 인덱스에 등록한다.
- 이름이 `ament_`로 시작하는 `ament_target_dependencies`, `ament_package`, `ament_lint_auto` 등이 ament_cmake가 제공하는 기능이고, `project`, `add_executable`, `install`, `find_package` 등은 CMake 본래의 명령이다.

### 5. 빌드 흐름에서의 위치

```
colcon build  →  ament_cmake (CMake 기반 패키지)  →  CMake  →  Make/Ninja  →  컴파일러(g++ 등)
(워크스페이스 전체 빌드 툴)   (ROS 2 매크로·규약)       (빌드 파일 생성)
```

- `colcon`은 여러 패키지를 의존성 순서대로 빌드하는 **빌드 툴**이고, ament_cmake는 개별 CMake 패키지가 따르는 **빌드 시스템(규약)** 이며, CMake는 그 밑에서 실제 빌드 파일을 만드는 **범용 도구**이다.

---

## 참고 자료

- ROS 2 Documentation, *ament_cmake user documentation*: https://docs.ros.org/en/jazzy/How-To-Guides/Ament-CMake-Documentation.html
- CMake Documentation: https://cmake.org/cmake/help/latest/
- ROS Answers, *the main role of ament_cmake*: https://answers.ros.org/answers/349871/revisions/
- 강의자료: 24장 ROS2 패키지 파일 (컴퓨터소프트웨어학부 임베디드SW전공, 이성렬)
