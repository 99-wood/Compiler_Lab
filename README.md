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
- <if 语句> $\rightarrow$ $\text{if}$ (<表达式>) <代码块>
- <while 语句> $\rightarrow$ $\text{while}$ (<表达式>) <代码块>
- <返回语句> $\rightarrow$ $\text{return}$ <表达式>;
- <实参列表> $\rightarrow$ $\epsilon$ | <表达式>, <实参列表>
- <类型> $\rightarrow$ $\text{Int}$ | $\text{Float}$ | $\text{Char}$ | $\text{Bool}$
- <乘除运算符> $\rightarrow$ $*$ | $/$
- <加减运算符> $\rightarrow$ $+$ | $-$
- <关系运算符> $\rightarrow$ $==$ | $!=$ | $<$ | $<=$ | $>$ | $>=$