#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char *argv[]) {

    int i;
    int sigma; 
    int window;
    sigma = atoi(argv[2]);
    window = atoi(argv[1]);

    for(i = -(window/2); i <= window/2; i++) {
        printf("%f ", pow(M_E,(0.0-(i*i))/(2.0*sigma*sigma)));
    }
     printf("\n");
}