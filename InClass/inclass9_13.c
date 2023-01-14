#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main (int argc, char *argv[]) {

    FILE *fpt;
    char header1[25], header2[25];
    int i, nasal, saliva;
    int tp = 0;
    int tn = 0;
    int fp = 0;
    int fn = 0;

    fpt = fopen("covid_data.txt", "rb");
    if (fpt == NULL) {
        printf("Unable to open %s for reading\n",argv[1]);
        exit(0);
    }

    i = fscanf(fpt,"%s %s ", header1, header2);

    while(fscanf(fpt,"%d %d ", &nasal, &saliva) != EOF) {
        if(nasal == 1 && saliva == 1) {
            tp++;
        }
        else if(nasal == 1 && saliva == 0) {
            fn++;
        }
        else if(nasal == 0 && saliva == 1) {
            fp++;
        }
        else {
            tn++;
        }
        
    }

    fclose(fpt);

    printf("Result: True Pos = %d\tTrue Neg = %d\tFalse Pos = %d\tFalse Neg = %d\n", tp, tn, fp, fn);

}