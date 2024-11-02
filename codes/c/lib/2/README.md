# Command

- g++ --shared -fpic -o libbase.so libbase.cc
- g++ --shared -fpic -o liba.so liba.cc -L. -lbase
- g++ --shared -fpic -o libb.so libb.cc
- export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
- g++ -o main main.cc
- ./main

# Attentions

dlopen + RTLD_GLOBAL 会导致当前库以及间接依赖库的符号都添加到全局符号表中，
从而导致 libb.cc 中的 addBase 被覆盖
