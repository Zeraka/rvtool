#include <stdio.h>

struct Global {
    Global () {
	printf ("Bug323: undefined advice definitions\n");
	printf ("====================================\n");
    }
    ~Global () {
	printf ("====================================\n");
    }
} global;

int main () {
    return 0;
}
