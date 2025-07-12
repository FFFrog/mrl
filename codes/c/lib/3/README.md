# Command

- g++ -fPIC -shared libC.cpp -o libC.so
- g++ -fPIC -shared libB.cpp -o libB.so -L. -lC
- g++ -fPIC -shared libA.cpp -o libA.so -L. -lB
- g++ -o main main.cpp

# Attentions

dlopen + RTLD_LOCAL 调用方法时会进行递归查找，而不仅仅是直接依赖(DT_NEEDED)

