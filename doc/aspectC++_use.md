# AspectC++用法

详细教程见
`http://www.aspectc.org/doc/ac-languageref.xhtml`
## 实例
example文件里, 有一个随机事件发生程序——SimpleEventLoop, 
aspectC++最适合的是插装事件日志

## 编译和使用
见AutoMonitor/lib/aspectC++/examples/ 里面编写的示例, acc的文件后缀名为.ah,本质是一种特殊的C++头文件。
ac++是编译AspectC++程序的编译器。   
实现将AspectC++代码转化为C++代码。  在ac++翻译代码之后，源代码可以被g++等编译器编译。   
分为两种编译方式，WPT 和 STU ， 
WPT 模式 把所有代码文件编译然后保存为另一个目录树。 
STU模式把目前是最为常用的。
在STU模式下，每个翻译单元必须像普通单元一样调用一次ac++
c++编译器。这使得将ac++集成到makefile或ide中变得更加容易。作为
无法在此模式下保存被操纵的头文件，因为未更改
中的所有#include指令，都是下一个翻译单元所需的头文件
引用项目目录树的头文件的翻译单元将展开
因此，与被操纵的翻译单元一起保存。生成的文件
可以直接输入到c++编译器中。它们不依赖于任何其他文件
这个项目了。

### 编译一个实例
ag++的用法类似于g++。
在aspect/example/helloworld文件夹里
```
ag++ -o hello  main.cc
```
而以.ah后缀命名的文件，如同头文件一样，要被包含才能够使用。

详细编译教程参考
`http://www.aspectc.org/doc/ag-man.pdf`

### 使用aspectC++之前和之后的代码对比


## 基础知识

### 匹配表达式
#### 举例    
"int C::%(...)",匹配名称为C的类的所有返回类型为int的函数        
"%List",    匹配任何以List结尾的名字的名称空间、class、struct、union或者enum。  
"% printf(const char *, ...)"匹配函数printf()  
且其参数至少为一个const char* 并且其返回类型为任意。       
"const %& ...::%(...)" 匹配所有返回类型有const的 引用类型的函数，且参数任意。

##  


### pointcut表达式
pointcut    使用 &&，||，！操作符； 
#### 举例
"%List" && !derived("Queue")    除了Queue类之外的“%Lish”
call("void draw()") && within("Shape")  在Shape类内部的方法，调用了 void draw()

### pointcut的声明
pointcut,即切点，是AOP范式中的一种类型，
```
pointcut lists() = derived("List");
```
意思是

### Advice code
advice可被理解为一个切片Aspect激发出的行为。    

#### 把函数作为切入点，并且传递参数。
```
pointcut new_user(const char*name) = execution("void login(...)")&& args(name);

advice new_user(name) : before(const char *name){
    cout << "User" << " is logging in." <<endl;
}
```
这段代码，第一部分指， 声明一个切入点——new_user, 每当被插装程序到达new_user所描绘的连接点时， 都会提供一个const char*类型的值，切入点表达式中使用的切入点函数，args(name), 是将参数name传递给 void login(...)的。

advice 切入点 ： 后面接的是相应的操作。
这里意思是在 传递 const char * name 之前，先打印 字符串。

#### 以类作为切入点
```
pointcut shapes() = "Circle" || "Polygon";
advice shapes() : slice class {
  bool m_shaded;
  void shaded(bool state) {
    m_shaded = state;
  }
};
```
该段代码， 声明了一个pointcut——shapes(), 意思是将 “Circle” || "Polygon"作为一个切入点。 后者是类的名称。    
advice 后的意思是，在切入的类中加入方括号内的声明。
#### 关于执行函数的advice
有 call, execution,两种主要的方式，call的意思就是该函数被调用时，而execution意思就是该函数被执行时。
例如
```
advice call("% send(...)") : order("Encrypt", "Log");
```
意思是，在调用
### 什么是join？

## 在函数执行之后，打印出该函数相应的日志。
## tjp 对象
tjp对象，用于表示被插入部分的代码， 可通过该对象得到被插入代码所在的函数名、行数、时间等基本性质。