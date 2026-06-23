# Mini-Lisp

这是一个使用 C++20 实现的 Mini-Lisp / Scheme 子集解释器。项目包含词法分析、语法分析、值表示、求值环境、特殊形式和内建过程，可以以交互式 REPL 或脚本文件两种方式运行 Lisp 程序。

当前解释器支持数字、布尔值、字符串、符号、空表、点对、列表、变量绑定、词法作用域闭包、条件表达式、局部绑定、引用、准引用以及常用的数值和列表操作。

## 项目结构

```text
.
├── CMakeLists.txt        # CMake 构建配置，生成 bin/mini_lisp
├── README.md             # 项目说明
├── bin/                  # 可执行文件输出目录
├── build/                # CMake 构建目录
├── lv7-answer.scm        # 一个可直接运行的 quick-sort 示例程序
└── src/
    ├── main.cpp          # 程序入口、REPL、文件运行模式
    ├── tokenizer.*       # 词法分析：源码字符串 -> Token 序列
    ├── token.*           # Token 类型定义
    ├── parser.*          # 语法分析：Token 序列 -> Value AST
    ├── value.*           # Lisp 值类型：数字、布尔、字符串、符号、点对等
    ├── eval_env.*        # 求值环境、变量查找、过程调用
    ├── forms.*           # 特殊形式：define、if、lambda、let 等
    ├── builtins.*        # 内建过程：+、car、map、display 等
    ├── error.h           # 解释器异常类型
    └── rjsj_test.hpp     # 课程测试辅助头文件
```

## 环境要求

- C++20 编译器，例如 Apple Clang、Clang、GCC、MSVC。
- CMake 3.24.2 或更高版本。
- 推荐使用 VS Code + CMake Tools，也可以直接在终端中使用 CMake。

## 构建

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

构建成功后，可执行文件会生成到：

```text
bin/mini_lisp
```

## 运行方式

### 交互式 REPL

不传参数时进入 REPL：

```bash
./bin/mini_lisp
```

示例：

```scheme
>>> (+ 1 2 3)
6
>>> (define x 10)
()
>>> (* x 2)
20
```

REPL 会对每一行输入进行一次解析和求值，并打印表达式结果。遇到运行时错误时会输出 `Error: ...`，然后继续等待下一行输入。

### 执行脚本文件

传入文件路径时，解释器会读取整个文件，依次解析并求值其中的所有顶层表达式：

```bash
./bin/mini_lisp lv7-answer.scm
```

文件模式不会自动打印每个表达式的结果。如果需要看到输出，请在 Lisp 程序中使用 `print`、`display`、`displayln` 或 `newline`。

## 快速示例

### 变量与函数

```scheme
(define radius 3)
(define pi 3.14159)

(define (area r)
  (* pi r r))

(print (area radius))
```

### 递归

```scheme
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(print (factorial 5))
```

### 列表与高阶函数

```scheme
(define xs '(1 2 3 4 5))

(print (map (lambda (x) (* x x)) xs))
(print (filter odd? xs))
(print (reduce + xs))
```

### `let` 局部绑定

```scheme
(let ((x 2)
      (y 3))
  (print (+ x y)))
```

`let` 会先在外层环境中求出所有绑定右侧的值，再创建局部环境执行主体表达式。

### 引用与准引用

```scheme
(print '(1 2 3))

(define x 10)
(print `(1 ,x 3))
```

`'expr` 会被解析为 `(quote expr)`，反引号会被解析为 `(quasiquote expr)`，逗号会被解析为 `(unquote expr)`。

## 支持的语法和值类型

### 字面量

| 类型 | 示例 | 说明 |
| --- | --- | --- |
| 数字 | `42`, `3.14`, `-7` | 内部使用 `double` 保存；整数值打印时不带小数部分 |
| 布尔值 | `#t`, `#f` | 只有 `#f` 被条件判断视为假 |
| 字符串 | `"hello"`, `"a\nb"` | 支持反斜杠转义，`\n` 会变成换行 |
| 符号 | `x`, `+`, `hello-world` | 未求值时是符号，求值时会查找变量绑定 |
| 空表 | `'()` | 空表本身必须通过引用等方式作为数据使用 |
| 点对 | `'(1 . 2)` | 使用 `PairValue` 表示 |
| 列表 | `'(1 2 3)` | 本质上是以 `()` 结尾的点对链 |

### 注释

分号 `;` 开始的内容会被词法分析器视为注释，直到当前行结束：

```scheme
; this is a comment
(+ 1 2) ; this is also a comment
```

### 点对与列表

解析器支持普通列表和点对：

```scheme
'(a b c)      ; => (a b c)
'(a b . c)    ; => (a b . c)
(cons 1 2)    ; => (1 . 2)
(cons 1 '())  ; => (1)
```

需要注意：很多内建过程和特殊形式内部会把参数转换成“正规列表”，也就是必须以 `()` 结尾的列表。对点对调用这些过程可能会触发 `Cannot convert dotted pair to vector` 或相关错误。

## 特殊形式

特殊形式不会像普通函数那样先求值所有参数，而是由解释器按各自规则处理。

| 形式 | 示例 | 行为 |
| --- | --- | --- |
| `quote` | `(quote (1 2))`, `'(1 2)` | 原样返回表达式，不求值 |
| `define` | `(define x 1)` | 在当前环境中绑定变量，返回 `()` |
| `define` 函数简写 | `(define (square x) (* x x))` | 创建并绑定一个闭包 |
| `if` | `(if test then else)` | 必须提供 3 个参数；只有 `#f` 进入 else 分支 |
| `and` | `(and a b c)` | 从左到右短路求值；无参数时返回 `#t` |
| `or` | `(or a b c)` | 从左到右短路求值；无参数时返回 `#f` |
| `lambda` | `(lambda (x) (+ x 1))` | 创建捕获当前环境的闭包 |
| `cond` | `(cond ((< x 0) -1) (else 1))` | 按顺序测试分支；`else` 必须放在最后 |
| `begin` | `(begin expr1 expr2)` | 顺序求值，返回最后一个表达式的值 |
| `let` | `(let ((x 1)) (+ x 2))` | 创建局部环境并执行主体 |
| `quasiquote` | `` `(1 ,x 3) `` | 返回准引用结构，其中 `unquote` 部分会被求值 |

## 内建过程

### 数值运算

| 过程 | 参数 | 说明 |
| --- | --- | --- |
| `+` | 任意个数字 | 求和；无参数返回 `0` |
| `-` | 至少 1 个数字 | 一个参数时取相反数，多个参数时连续相减 |
| `*` | 任意个数字 | 求积；无参数返回 `1` |
| `/` | 至少 1 个数字 | 一个参数时取倒数，多个参数时连续相除 |
| `abs` | 1 个数字 | 绝对值 |
| `expt` | 2 个数字 | 幂运算 |
| `quotient` | 2 个整数 | 整数商 |
| `remainder` | 2 个整数 | C++ `%` 风格的余数 |
| `modulo` | 2 个整数 | 结果符号跟除数一致的模 |

除法、`quotient`、`remainder` 和 `modulo` 遇到除数为 `0` 时会抛出错误。

### 比较与逻辑

| 过程 | 参数 | 说明 |
| --- | --- | --- |
| `=` | 任意个数字 | 检查相邻数字是否都相等 |
| `<` | 任意个数字 | 检查是否严格递增 |
| `>` | 任意个数字 | 检查是否严格递减 |
| `<=` | 任意个数字 | 检查是否非递减 |
| `>=` | 任意个数字 | 检查是否非递增 |
| `not` | 1 个值 | 只有参数为 `#f` 时返回 `#t` |
| `even?` | 1 个整数 | 判断偶数 |
| `odd?` | 1 个整数 | 判断奇数 |
| `zero?` | 1 个数字 | 判断是否为 0 |

### 类型谓词

| 过程 | 说明 |
| --- | --- |
| `atom?` | 不是点对、内建过程或 lambda 时返回真 |
| `boolean?` | 判断布尔值 |
| `integer?` | 判断数值是否为整数 |
| `list?` | 判断是否为正规列表 |
| `number?` | 判断数字 |
| `null?` | 判断空表 |
| `pair?` | 判断点对 |
| `procedure?` | 判断内建过程或 lambda |
| `string?` | 判断字符串 |
| `symbol?` | 判断符号 |

### 列表操作

| 过程 | 参数 | 说明 |
| --- | --- | --- |
| `car` | 1 个点对 | 返回左值 |
| `cdr` | 1 个点对 | 返回右值 |
| `cons` | 2 个值 | 创建点对 |
| `length` | 1 个正规列表 | 返回列表长度 |
| `list` | 任意个值 | 创建正规列表 |
| `append` | 任意个列表/尾值 | 拼接列表；最后一个参数作为尾部保留 |
| `map` | 过程、列表 | 对列表每个元素应用过程，返回新列表 |
| `filter` | 谓词过程、列表 | 保留谓词结果不为 `#f` 的元素 |
| `reduce` | 二元过程、非空列表 | 从右向左归约列表 |

### 相等性

| 过程 | 说明 |
| --- | --- |
| `eq?` | 比较对象身份；数字、布尔、符号和空表按值比较 |
| `equal?` | 递归比较数字、布尔、字符串、符号、空表和点对结构 |

### 输入输出和控制

| 过程 | 说明 |
| --- | --- |
| `display` | 连续输出参数；字符串输出原始内容，不带引号 |
| `displayln` | 输出 1 个参数并换行 |
| `newline` | 输出一个换行 |
| `print` | 每个参数调用 `toString()` 后单独输出一行 |
| `error` | 抛出 Lisp 运行时错误 |
| `eval` | 在当前环境中求值一个值 |
| `apply` | 调用过程，第二个参数必须是参数列表 |
| `exit` | 退出解释器；可选整数退出码 |

## 求值模型

解释器的核心求值逻辑在 `EvalEnv::eval` 中：

1. 数字、布尔、字符串、内建过程和 lambda 是自求值对象，直接返回自身。
2. 符号会在当前环境中查找绑定；当前环境没有时继续向父环境查找。
3. 空表 `()` 不能直接求值，直接求值会报错。
4. 列表求值时，若第一个元素是特殊形式名称，则交给对应特殊形式处理。
5. 否则先求值列表第一个元素得到过程，再从左到右求值参数，最后调用过程。

lambda 会捕获创建时的环境，因此支持词法作用域：

```scheme
(define (make-adder x)
  (lambda (y) (+ x y)))

(define add10 (make-adder 10))
(print (add10 5)) ; 15
```

## 错误处理

解释器使用 `LispError` 和 `SyntaxError` 表示运行时错误和语法错误。REPL 模式会捕获错误并继续运行；文件模式遇到错误会输出错误信息并以非零状态码退出。

常见错误包括：

- 引用未定义变量：`Variable x not defined.`
- 参数数量错误：`foo expects n argument(s).`
- 对非过程值进行调用：`Cannot apply non-procedure.`
- 对非正规列表执行需要列表的操作：`Cannot convert dotted pair to vector`
- 除零：`Division by zero.`

## 测试与示例程序

项目根目录的 `lv7-answer.scm` 是一个 quick-sort 示例：

```bash
./bin/mini_lisp lv7-answer.scm
```

输出为排序后的列表。

`src/main.cpp` 中保留了课程测试入口：

```cpp
// RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6, Lv7, Lv7Lib, Sicp);
```

如果需要运行课程测试，可以按需要取消注释并重新构建。

## 扩展指南

### 添加新的内建过程

1. 在 `src/builtins.h` 中声明函数。
2. 在 `src/builtins.cpp` 中实现函数，参数类型通常为 `const std::vector<ValuePtr>&`。
3. 如果过程需要访问当前求值环境，签名使用：

   ```cpp
   ValuePtr yourProc(const std::vector<ValuePtr>& params, EvalEnv& env);
   ```

4. 在 `builtins` 映射中注册名称和函数。

普通内建过程会在调用前先求值所有实参。如果你需要自己控制参数是否求值，应添加特殊形式而不是内建过程。

### 添加新的特殊形式

1. 在 `src/forms.h` 中声明函数。
2. 在 `src/forms.cpp` 中实现函数，签名为：

   ```cpp
   ValuePtr yourForm(const std::vector<ValuePtr>& args, EvalEnv& env);
   ```

3. 在 `SPECIAL_FORMS` 映射中注册名称。

特殊形式接收到的是未求值参数，是否求值、何时求值、求值几次都由特殊形式自己决定。

### 添加新的值类型

1. 在 `src/value.h` 中继承 `Value`。
2. 在 `src/value.cpp` 中实现 `toString()` 等接口。
3. 如果它可以自求值，重写 `isSelfEvaluating()`。
4. 如果它需要参与解析、打印或比较，还需要同步修改 `parser.cpp`、`builtins.cpp` 或相关逻辑。

## 当前实现边界

这个解释器是课程规模的 Mini-Lisp，不是完整 Scheme 实现。当前代码中没有实现宏系统、尾调用优化、变长参数 lambda、`set!`、`let*`、`letrec`、向量、字符、端口、模块系统等完整 Scheme 特性。

另外，`if` 当前固定要求 `test`、`then`、`else` 三个部分；准引用实现也更接近简单递归替换，没有完整处理嵌套准引用的深度规则。
