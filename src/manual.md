## 原理
运用了spot库中Monitor生成算法，将buchi自动机转化为能够接受一连串有限字的Monitor自动机，从而可以监控运行时系统。


## 安装spot库
spot库可以通过apt命令安装。如果通过apt命令安装，就使用

## 编译命令
`g++ std=c++11 hello.cc -lspot -o hello`

如果发生错误，可以尝试以下命令  
`g++ -std=c++14 -I/usr/local/include hello.cc -L/usr/local/lib -lspot -lbddx -o hello`         
编译出现错误，往往是include 文件夹里缺少spot的头文件，或者说lib文件夹里缺少 libspot相关的文件。


### 用命令行创建确定性Monitor自动机
`ltl2tgba -D -M '!F(red & yellow)'`

## 注意事项
编译时,g++版本要5.0以后。

