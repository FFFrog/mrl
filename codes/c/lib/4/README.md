# Command

- g++ -fPIC -shared lib_global.cpp -o lib_global.so
- g++ -fPIC -shared lib_local.cpp -o lib_local.so
- g++ -fPIC -shared lib.cpp -o lib.so -L. -l_local
- g++ -o main main.cpp

# Attentions

运行时(dlopen 以及 ld.so)符号查找顺序优先级：
全局作用域 -> 当前动态库(待符号重定位的库) -> 当前动态库递归依赖(DFS)

