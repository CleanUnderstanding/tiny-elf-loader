<<<<<<< HEAD
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

void greet() {
    puts("Hello from the test program!");
}

int main() {
    greet();

    if (environ) {
        puts("Environment pointer is non-null. GLOB_DAT test passed.");
    } else {
        puts("Environment pointer is null. GLOB_DAT test failed.");
    }

=======
#include <stdio.h>     // printf
#include <math.h>      // cos
#include <dlfcn.h>     // dlopen

int main() {
    printf("Hello World\\n");   // forces libc
    double x = cos(0.0);        // forces libm
    void *handle = dlopen("libm.so.6", RTLD_LAZY); // forces libdl
>>>>>>> origin/main
    return 0;
}
