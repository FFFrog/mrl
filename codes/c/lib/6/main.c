// https://blog.csdn.net/Programmer_acu/article/details/52017234

#include <stdio.h>

extern void addvec(int *, int *, int *, int);
extern void multvec(int *, int *, int *, int);
 
int main(void) {
 
    int x[2] = {1, 2};
    int y[2] = {3, 4};
    int z[2] = {0};
 
    addvec(x, y, z, 2);
    printf("%d  %d\n", z[0], z[1]);
    return 0;
}
