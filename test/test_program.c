#include <stdio.h>     // printf
#include <math.h>      // cos
#include <dlfcn.h>     // dlopen

int main() {


    printf("Hello World\\n");   // forces libc
    double x = cos(0.0);        // forces libm
    void *handle = dlopen("libm.so.6", RTLD_LAZY); // forces libdl
    return 0;
    
}
