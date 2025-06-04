# 编译原理课程设计

本项目为编译原理课程设计的相关代码与文档。

## 语法

- <程序> $\rightarrow$ <定义语句列表>
- <定义语句列表> $\rightarrow$ <定义语句><定义语句列表> | <定义语句>
- <定义语句> $\rightarrow$ <函数定义> | <变量定义语句> | <常量定义语句>
- <函数定义> $\rightarrow$ $\text{fun}$ <标识符>(参数列表) : <类型> <代码块>
- <参数列表> $\rightarrow$ $\epsilon$ | <标识符> : <类型>, <参数列表>
- <代码块> $\rightarrow$ {<语句列表>}
- <语句列表> $\rightarrow$ $\epsilon$ | <语句> <语句列表> | <代码块><语句列表>
- <语句> $\rightarrow$ <变量定义语句> | <常量定义语句> | <表达式语句> | <if 语句> | <while 语句> | <返回语句>
- <变量定义语句> $\rightarrow$ $\text{var}$ <初始化列表> | <标识符列表> : <类型>;
- <常量定义语句> $\rightarrow$ $\text{val}$ <初始化列表>;
- <初始化列表> $\rightarrow$ <标识符> = <表达式> | <标识符> = <表达式>, <初始化列表>
- <标识符列表> $\rightarrow$ <标识符> | <标识符>, <标识符列表>
- <表达式语句> $\rightarrow$ <表达式>;
- <表达式> $\rightarrow$ <赋值表达式>;
- <赋值表达式> $\rightarrow$ <变量标识符> = <赋值表达式> | <逻辑表达式>
- <逻辑表达式> $\rightarrow$ <逻辑或表达式>
- <逻辑或表达式> $\rightarrow$ <逻辑与表达式> | <逻辑或表达式> || <逻辑与表达式>
- <逻辑与表达式> $\rightarrow$ <逻辑非表达式> | <逻辑与表达式> && <逻辑非表达达式>
- <逻辑非表达式> $\rightarrow$ !<关系表达式> | <关系表达式>
- <关系表达式> $\rightarrow$ <算术表达式> <关系运算符> <算术表达式> | <算术表达式>
- <算术表达式> $\rightarrow$ <项> | <算术表达式> <加减运算符> <项>
- <项> $\rightarrow$ <原子表达式> | <项> <乘除运算符> <原子表达式>
- <原子表达式> $\rightarrow$ <变量标识符> | <常量标识符> | <函数调用表达式> | <类型转换表达式> | <常数>
- <类型转换表达式> $\rightarrow$ <类型>(<表达式>)
- <函数调用表达式> $\rightarrow$ <函数标识符>(<实参列表>)
- <if 语句> $\rightarrow$ $\text{if}$ (<表达式>) <代码块> |  $\text{if}$ (<表达式>) <代码块> $\text{else}$ <代码块>
- <while 语句> $\rightarrow$ $\text{while}$ (<表达式>) <代码块>
- <返回语句> $\rightarrow$ $\text{return}$ <表达式>;
- <实参列表> $\rightarrow$ $\epsilon$ | <表达式>, <实参列表>
- <类型> $\rightarrow$ $\text{Int}$ | $\text{Float}$ | $\text{Char}$ | $\text{Bool}$
- <乘除运算符> $\rightarrow$ $*$ | $/$
- <加减运算符> $\rightarrow$ $+$ | $-$
- <关系运算符> $\rightarrow$ $==$ | $!=$ | $<$ | $<=$ | $>$ | $>=$


## 改造后语法

- <程序> $\rightarrow$ <定义语句列表>
- <定义语句列表> $\rightarrow$ <定义语句><定义语句列表'>
- <定义语句列表'> $\rightarrow$ <定义语句><定义语句列表'> | $\epsilon$
- <定义语句> $\rightarrow$ <函数定义> | <变量定义语句> | <常量定义语句>
- <函数定义> $\rightarrow$ $\text{fun}$ <标识符>(参数列表) : <类型> <代码块>
- <参数列表> $\rightarrow$ $\epsilon$ | <标识符> : <类型>, <参数列表>
- <代码块> $\rightarrow$ {<语句列表>}
- <语句列表> $\rightarrow$ $\epsilon$ | <语句> <语句列表> | <代码块><语句列表>
- <语句> $\rightarrow$ <变量定义语句> | <常量定义语句> | <表达式语句> | <if 语句> | <while 语句> | <返回语句>
- <变量定义语句> $\rightarrow$ $\text{var}$ <初始化列表>; | $\text{var}$ <标识符列表> : <类型>;
- <变量定义语句> $\rightarrow$ $\text{var}$ <标识符><变量定义后缀>
- <变量定义后缀> $\rightarrow$ <初始化列表后缀> | <标识符列表后缀>
- <初始化列表后缀> $\rightarrow$ = <表达式><初始化列表后缀'>
- <初始化列表后缀'> $\rightarrow$ ; | ,<标识符> = <表达式><初始化列表后缀'>
- <标识符列表后缀> $\rightarrow$ : <类型>; | ,<标识符><标识符列表后缀>
- <常量定义语句> $\rightarrow$ $\text{val}$ <初始化列表>;
- <初始化列表> $\rightarrow$ <标识符> = <表达式><初始化列表'>
- <初始化列表'> $\rightarrow$ $\epsilon$ | <标识符> = <表达式>, <初始化列表'>
- <表达式语句> $\rightarrow$ <表达式>;
- <表达式> $\rightarrow$ <赋值表达式>
- <赋值表达式> $\rightarrow$ <变量标识符> = <赋值表达式> | <逻辑表达式>
- <逻辑表达式> $\rightarrow$ <逻辑或表达式>
- <逻辑或表达式> $\rightarrow$ <逻辑与表达式> | <逻辑或表达式> || <逻辑与表达式>
- <逻辑与表达式> $\rightarrow$ <逻辑非表达式> | <逻辑与表达式> && <逻辑非表达式>
- <逻辑非表达式> $\rightarrow$ !<关系表达式> | <关系表达式>
- <关系表达式> $\rightarrow$ <算术表达式> <关系运算符> <算术表达式> | <算术表达式>
- <算术表达式> $\rightarrow$ <项> | <算术表达式> <加减运算符> <项>
- <项> $\rightarrow$ <原子表达式> | <项> <乘除运算符> <原子表达式>
- <原子表达式> $\rightarrow$ <变量标识符> | <常量标识符> | <函数调用表达式> | <类型转换表达式> | <常数> | (<表达式>)
- <类型转换表达式> $\rightarrow$ <类型>(<表达式>)
- <函数调用表达式> $\rightarrow$ <函数标识符>(<实参列表>)
- <if 语句> $\rightarrow$ $\text{if}$ (<表达式>) <代码块> |  $\text{if}$ (<表达式>) <代码块> $\text{else}$ <代码块>
- <while 语句> $\rightarrow$ $\text{while}$ (<表达式>) <代码块>
- <返回语句> $\rightarrow$ $\text{return}$ <表达式>;
- <实参列表> $\rightarrow$ $\epsilon$ | <表达式>, <实参列表>
- <类型> $\rightarrow$ $\text{Int}$ | $\text{Float}$ | $\text{Char}$ | $\text{Bool}$
- <乘除运算符> $\rightarrow$ $*$ | $/$
- <加减运算符> $\rightarrow$ $+$ | $-$
- <关系运算符> $\rightarrow$ $==$ | $!=$ | $<$ | $<=$ | $>$ | $>=$

## 变元中英文对照（AI 生成）

| 中文变元         | 英文命名建议                    |
| ------------ | ------------------------- |
| `<程序>`       | `Program`                 |
| `<定义语句列表>`   | `DefStmtList`             |
| `<定义语句列表'>`  | `DefStmtListTail`         |
| `<定义语句>`     | `DefinitionStmt`          |
| `<函数定义>`     | `FunctionDef`             |
| `<变量定义语句>`   | `VarDefStmt`              |
| `<常量定义语句>`   | `ConstDefStmt`            |
| `<参数列表>`     | `ParamList`               |
| `<代码块>`      | `CodeBlock`               |
| `<语句列表>`     | `StmtList`                |
| `<语句>`       | `Stmt`                    |
| `<变量定义后缀>`   | `VarDefSuffix`            |
| `<初始化列表>`    | `InitList`                |
| `<初始化列表后缀>`  | `InitListSuffix`          |
| `<初始化列表后缀'>` | `InitListSuffixTail`      |
| `<标识符列表后缀>`  | `IdentListSuffix`         |
| `<标识符>`      | `Identifier`              |
| `<表达式语句>`    | `ExprStmt`                |
| `<表达式>`      | `Expression`              |
| `<赋值表达式>`    | `AssignExpr`              |
| `<逻辑表达式>`    | `LogicExpr`               |
| `<逻辑或表达式>`   | `LogicOrExpr`             |
| `<逻辑与表达式>`   | `LogicAndExpr`            |
| `<逻辑非表达式>`   | `LogicNotExpr`            |
| `<关系表达式>`    | `RelExpr`                 |
| `<算术表达式>`    | `ArithExpr`               |
| `<项>`        | `Term`                    |
| `<原子表达式>`    | `AtomExpr`                |
| `<类型转换表达式>`  | `TypeCastExpr`            |
| `<函数调用表达式>`  | `FuncCallExpr`            |
| `<类型>`       | `Type`                    |
| `<常数>`       | `ConstValue`（或 `Literal`） |
| `<变量标识符>`    | `VarIdentifier`           |
| `<常量标识符>`    | `ConstIdentifier`         |
| `<函数标识符>`    | `FuncIdentifier`          |
| `<if 语句>`    | `IfStmt`                  |
| `<while 语句>` | `WhileStmt`               |
| `<返回语句>`     | `ReturnStmt`              |
| `<实参列表>`     | `ArgList`                 |
| `<乘除运算符>`    | `MulDivOp`                |
| `<加减运算符>`    | `AddSubOp`                |
| `<关系运算符>`    | `RelOp`                   |

## 关键字编码

|  id | 内容 |
|:---:|:---:|
|  1  |  Int  |
|  2  |  Float  |
|  3  |  Char  |
|  4  |  Bool  |
|  5  |  Void  |
|  6  |  fun  |
|  7  |  var  |
|  8  |  val  |
|  9  |  if  |
|  10  |  else  |
|  11 |  while  |
|  12 |  return  |

## 界符

|  id | 内容 |
|:---:|:----:|
|  1  |  \(  |
|  2  |  \)  |
|  3  |  \{  |
|  4  |  \}  |
|  5  |  \[  |
|  6  |  \]  |
|  7  |  =   |
|  8  | \|\| |
|  9  |  &&  |
|  10 |  !   |
|  11 |  =   |
|  12 |  ==  |
|  13 |  !=  |
|  14 |  >   |
|  15 |  >=  |
|  16 |  <   |
|  17 |  <=  |
|  18 |  +   |
|  19 |  -   |
|  20 |  *   |
|  21 |  /   |

## 虚拟机支持指令

### STACK_ALLOC

语法：`STACK_ALLOC(SIZE)`

作用：分配 `SIZE` 字节空间

### STACK_FREE

语法：`STACK_FREE(SIZE)`

作用：释放 `SIZE` 字节空间

### JMP

语法：`JMP(POS)`

作用：跳转到第 `POS` 条指令执行

### 
