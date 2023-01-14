#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


int main(int argc, char *argv[]) {

    FILE		*fpt;
    int i, total, start, end, t1, t2;
    float data[500];


        /* read data */
    if ((fpt=fopen(argv[1],"rb")) == NULL) {
        printf("Unable to open %s for reading\n", argv[0]);
        exit(0);
    }

    fscanf(fpt,"%f ", data);

    total = sizeof(data);

    printf("data size = %d", total);
}

