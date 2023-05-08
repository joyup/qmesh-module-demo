# qmesh-module-demo

# 运行环境：
  UBuntu Linux

# 编译方法：
  cd src
  make clean
  make [arm|X86|rk3308] #默认不带参数是编译X86版本
  此时会在lib目录中生成对应的lib文件
  cd ..
  make
  没问题的话，会在output目录中生成qmesh_test可执行文件

# 运行
  cd output
  ./qmesh_test
  执行后，会等待用户的命令输入，输入对应的命令会执行PLC相关的操作，具体可参考qmesh_demo.c的设定
  