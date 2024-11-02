# Command

- g++ --shared -fpic -o libbase.so libbase.cc
- g++ --shared -fpic -o liba.so liba.cc -L. -lbase
- g++ --shared -fpic -o libb.so libb.cc
- export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
- g++ -o main main.cc -L. -la -lb
- ./main

# Attentions:
liba 和 libb 都依赖 libbase, 但是编译 libb 的时候没有显示指定依赖 libbase，
由于 liba 的直接依赖 libbase也被加载到全局符号表，从而 libb 的 addBase 可以正常调用
