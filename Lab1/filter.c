/*
    Deeb Armaly
    ECE 4310
    Lab 1

    In this lab, a 7x7 mean filter will be applied to a greyscale PPM image of choice
    determined by the user input. The command to run this program properly follows the form
        ./filter [image file] [filter option]
    where the filter options should be either 1, 2, or 3 representing
            1 - basic 2D convolution smoothing
            2 - separable filters
            3 - serparable filters and sliding window   
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char *argv[]) {

    FILE		*fpt;
    unsigned char	*image;
    unsigned char	*smoothed;
    float   *filtered1;   //for step 1 of separable filters
    unsigned char   *filtered2;   //for step 2 of separable filters
    char		header[320];
    int		ROWS,COLS,BYTES;
    int		r,c,r2,c2,sum,option;
    struct timespec	tp1,tp2;

    //error checking for input
    if(argc != 3) {
        printf("Usage: filter [filename] [filtering (options are 1,2,3)]\n");
        exit(0);
    
    }

        /* set filtering type */
    option = atoi(argv[2]);
    
    if(option < 1 || option > 3) {
        printf("Usage: filter [filename] [filtering (options are 1,2,3)]\n");
        exit(0);
    }

   
        /* read image */
    if ((fpt=fopen(argv[1],"rb")) == NULL) {
        printf("Unable to open %s for reading\n", argv[0]);
        exit(0);
    }

    fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);

    if (strcmp(header,"P5") != 0  ||  BYTES != 255){
        printf("Not a greyscale 8-bit PPM image\n");
        exit(0);
    }

    image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    header[0]=fgetc(fpt);	/* read white-space character that separates header */
    fread(image,1,COLS*ROWS,fpt);
    fclose(fpt);

        /* allocate memory for smoothed version of image */
    smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

        /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp1);
    //printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);


        /* smooth image, skipping the border points using 2D convolution */
    if(option == 1) {
        fpt=fopen("smoothed1.ppm","w");
        for (r=3; r<ROWS-3; r++) {
            for (c=3; c<COLS-3; c++) {
                sum=0;
                for (r2=-3; r2<=3; r2++) {
                    for (c2=-3; c2<=3; c2++) {
                        sum+=image[(r+r2)*COLS+(c+c2)];
                    }
                    
                }
                smoothed[r*COLS+c]=sum/49;
            }
        }
        //printf("Pixel value option 1: %d\n", (int)smoothed[3*COLS + 267]);
    }

        /* Separable Filters smoothing option */
    float decimalSum = 0;
    if(option == 2) {

        fpt=fopen("smoothed2.ppm","w");
        filtered1 = (float *)calloc(ROWS*COLS,sizeof(float));
        filtered2 = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

        for (r=0; r<ROWS; r++) {
            for (c=3; c<COLS-3; c++) {
                decimalSum=0;
                for (c2=-3; c2<=3; c2++) {
                    decimalSum+=image[r*COLS+(c+c2)];
                }
                filtered1[r*COLS+c] = decimalSum;
            }
        }

        for (r=3; r<ROWS-3; r++) {
            for (c=0; c<COLS; c++) {
                decimalSum=0;
                for (r2=-3; r2<=3; r2++) {
                    decimalSum+=filtered1[(r + r2)*COLS+c];
                }
                filtered2[r*COLS+c] = (int) decimalSum/49.0;
            }
        }

        for (r=0; r<ROWS; r++) {
            for (c=0; c<COLS; c++) { 
                if((c < 3 || r < 3) || (c >= COLS-3 || r >= ROWS-3)) {
                    smoothed[r*COLS+c] = 0;
                }
                else {
                    smoothed[r*COLS+c] = filtered2[r*COLS+c];
                }
            }
        }

        free(filtered1);
        free(filtered2);
        //printf("Pixel value option 2: %d\n", (int)smoothed[3*COLS + 267]);
    }


        /* Separable Filters and Sliding Window smoothing option */
    if(option == 3) {

        float prev;
        fpt=fopen("smoothed3.ppm","w");
        filtered1 = (float *)calloc(ROWS*COLS,sizeof(float));
        filtered2 = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

        for(r = 0; r < ROWS; r++) {
            decimalSum = 0;
            for(c = 0; c < COLS; c++) {
                // calculate initial window for the horizontal filter
                if(c < 7) { 
                    decimalSum += image[r*COLS+c];
                    if(c == 6) {
                        filtered1[r*COLS+3] = decimalSum;
                    }
                }
                // once initial calulation is done, add the next pixel and subtract previous pixel value
                else {
                    prev = image[r*COLS+c-7];
                    decimalSum = decimalSum - prev + image[r*COLS+c];
                    filtered1[r*COLS+c-3] = decimalSum;
                }
            }
        }

        for(c = 0; c < COLS; c++) {
            decimalSum = 0;
            for(r = 0; r < ROWS; r++) {
                // calculate initial vertical filter window
                if(r < 7) {
                    decimalSum += filtered1[r*COLS+c];
                    if(r == 6) {
                        filtered2[3*COLS+c] = (int) decimalSum/49.0;
                    }
                }
                // sliding window implementation once initial window has been calculated
                else {
                    prev = filtered1[(r-7)*COLS+c];
                    decimalSum = decimalSum - prev + filtered1[r*COLS+c];
                    filtered2[(r-3)*COLS+c] = (int) decimalSum/49.0;
                }

            }
        }

        for (r=0; r<ROWS; r++) {
            for (c=0; c<COLS; c++) { 
                if((c < 3 || r < 3) || (c >= COLS-3 || r >= ROWS-3)) {
                    smoothed[r*COLS+c] = 0;
                }
                else {
                    smoothed[r*COLS+c] = filtered2[r*COLS+c];
                }
            }
        }

        free(filtered1);
        free(filtered2);
        //printf("Pixel value option 3: %d\n", (int)smoothed[3*COLS + 267]);
    }

        /* query timer */
    clock_gettime(CLOCK_REALTIME,&tp2);
    //printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

        /* report how long it took to smooth */
    printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);
    
        /* write out smoothed image to see result */
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(smoothed,COLS*ROWS,1,fpt);
    fclose(fpt);

        /* deallocate memory */
    free(image);
    free(smoothed);

}