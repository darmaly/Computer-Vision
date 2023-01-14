/*  Deeb Armaly
    ECE 4310
    Fall 2022
    Lab 5

    To run code hawk.ppm and hawk_init.txt files must be in the current working folder
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define SQR(x) ((x) * (x))
int ROWS, COLS, BYTES;

void PaintContour(unsigned char *image, int r, int c, int count);

int main(int argc, char *argv[]) {

    FILE *fpt;
    unsigned char *image;
    char header[80];
    int window = 49;


    if(argc != 1) {
        printf("Usage: ./darmaly_lab5\n");
        exit(0);
    }

    if((fpt = fopen("hawk.ppm","rb")) == NULL) {
        printf("Unable to open %s for reading\n", argv[1]);
        exit(0);
    }

        /* Open image */
    fscanf(fpt, "%s %d %d %d", header, &COLS, &ROWS, &BYTES);
    if (strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("Must be a greyscale PPM 8-bit image\n");
        exit(0);
    }
    header[0] = fgetc(fpt); /* whitespace char after header*/
    image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    fread(image,1,ROWS*COLS,fpt);
    fclose(fpt);


        /* Read in initial contour points */
    if((fpt = fopen("hawk_init.txt","rb")) == NULL) {
        printf("Unable to open %s for reading\n", argv[2]);
        exit(0);
    }

        /* extract data from text file */
    int ptCount = 0;
    int i, row_tmp, col_tmp;
    while(fscanf(fpt,"%d %d ", &row_tmp, &col_tmp) == 2) {
        ptCount++;
    }
    rewind(fpt);
    
    int init_row[ptCount];
    int init_col[ptCount];
    i = 0;
    while(fscanf(fpt,"%d %d ", &row_tmp, &col_tmp) == 2) {
        init_row[i] = row_tmp;
        init_col[i] = col_tmp;
        i++;
    }
    fclose(fpt);

        /* Draw out original contour */
    unsigned char *tmp_img = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    for(i = 0; i < ROWS*COLS; i++) tmp_img[i] = image[i];
    fpt = fopen("hawk_contour_orig.ppm", "wb"); 
    for(i = 0; i < ptCount; i++){
        PaintContour(tmp_img, init_col[i], init_row[i], ptCount); 
    }
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(tmp_img,1,ROWS*COLS,fpt);
    fclose(fpt);
    free(tmp_img);

    int sobel_x[3][3] = { -1,0,1,-2,0,2,-1,0,1 };
    int sobel_y[3][3] = { -1,-2,-1,0,0,0,1,2,1 };
    
        /* Convolve original image with sobel x and y kernels */
    int* sobel_img = (int*)calloc(ROWS * COLS, sizeof(int));
    for (i = 0; i < ROWS * COLS; i++) sobel_img[i] = image[i]; // copy original image 
    int r, c, x, y, j;
    for (r = 1; r < ROWS - 1; r++) {
        for (c = 1; c < COLS - 1; c++) {
            x = 0;
            y = 0;
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    x += image[(r + i) * COLS + c + j] * sobel_x[i + 1][j + 1];
                    y += image[(r + i) * COLS + c + j] * sobel_y[i + 1][j + 1];
                }
            }
            sobel_img[r * COLS + c] = sqrt((SQR(x) + SQR(y)));
        }
    }

        /* Find min and max of sobel image */
    int min = 10000;
    int max = 0;
    for (r = 1; r < ROWS - 1; r++) {
        for (c = 1; c < COLS - 1; c++) {
            if (max < sobel_img[r * COLS + c]) max = sobel_img[r * COLS + c];
            if (min > sobel_img[r * COLS + c]) min = sobel_img[r * COLS + c];
        }
    }

        /* Normalize Sobel Image */
    unsigned char* normalized_sobel = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));
    for (i = 0; i < ROWS * COLS; i++) {
        normalized_sobel[i] = ((sobel_img[i] - min) * 255) / (max - min);
    }
    fpt = fopen("normalized_sobel.ppm", "wb");
    fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
    fwrite(normalized_sobel, 1, ROWS * COLS, fpt);
    fclose(fpt);


        /* Internal and External Energy calculations */
    float *internalE1, *internalE2;
    float *externalE;
    float *totalE;
    float avg_x, avg_y, avg_dist;
    int temp;
    float maxE1, minE1;
    float maxE2, minE2;
    float maxExt, minExt;
    int z;
    int move_to_row, move_to_col;

    externalE = (float *)calloc(ROWS*COLS, sizeof(float));

    for(r = 0; r < ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            externalE[r*COLS+c] = (-1)*(sobel_img[r*COLS+c]);
        }
    }
    maxExt = externalE[0];
    minExt = externalE[0];
    for(r = 0; r < ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            if(minExt > externalE[r*COLS+c]) minExt = externalE[r*COLS+c];
            if(maxExt < externalE[r*COLS+c]) maxExt = externalE[r*COLS+c];
        }
    }
    for(r = 0; r < ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            externalE[r*COLS+c] = (externalE[r*COLS+c] - minExt)/(maxExt - minExt);
        }
    }

    minE1 = 10000;
    maxE1 = 0;
    minE2 = 10000;
    maxE2 = 0;
    

    for (i = 0; i < 3; i++) {
        avg_x = 0.0;
        avg_y = 0.0;
        avg_dist = 0.0;

            /* Average distance between contour pts */
        for (j = 0; j < ptCount; j++) {
            if ((j + 1) < ptCount) {
                avg_x = SQR(init_col[j] - init_col[j + 1]);
                avg_y = SQR(init_row[j] - init_row[j + 1]);
            }
            else {
                avg_x = SQR(init_col[j] - init_col[0]);
                avg_y = SQR(init_row[j] - init_row[0]);
            }
            avg_dist += sqrt(avg_x + avg_y);
        }
        avg_dist = avg_dist / ptCount;
        temp = 0;
        j = 0;
        while (j < ptCount) {
            internalE1 = (float *)calloc(window, sizeof(float));
            internalE2 = (float *)calloc(window, sizeof(float));    
            totalE = (float *)calloc(window, sizeof(float));

            row_tmp = init_row[j];
            col_tmp = init_col[j];
            temp = 0;
            for(r = row_tmp - 3; r <= row_tmp + 3; r++) {
                for(c = col_tmp - 3; c <= col_tmp + 3; c++) {
                    if((j + 1) < ptCount) {
                        internalE1[temp] = SQR(r - init_row[j+1]) + SQR(c - init_col[j+1]);    
                    }
                    else {
                        internalE1[temp] = SQR(r - init_row[0]) + SQR(c - init_col[0]);
                    }
                    internalE2[temp] = SQR(sqrt(internalE1[temp]) - avg_dist);
                    temp++;
                }
            }
            
            for(z = 0; z < window; z++) {
                if(minE1 > internalE1[z]) minE1 = internalE1[z];
                if(maxE1 < internalE1[z]) maxE1 = internalE1[z];
            }
            
            for(z = 0; z < window; z ++) {
                internalE1[z] = (internalE1[z] - minE1)/(maxE1 - minE1);
            }
                
            
            for(z = 0; z < window; z++) {
                if(minE2 > internalE2[z]) minE2 = internalE2[z];
                if(maxE2 < internalE2[z]) maxE2 = internalE2[z];
            }
            

            for(z = 0; z < window; z ++) {
                internalE2[z] = (internalE2[z] - minE2)/(maxE2 - minE2);
            }


             
            z = 0;
            for(r = row_tmp - 3; r <= row_tmp + 3; r++) {
                for(c = col_tmp - 3; c <= col_tmp + 3; c++) {
                    totalE[z] = 100*internalE1[z] + 24*internalE2[z] + 17*externalE[r*COLS+c];
                    z++;
                }
            }

            
            
            min = 10000;
            temp = 0;
            for(r = row_tmp - 3; r <= row_tmp + 3; r++) {
                for(c = col_tmp - 3; c <= col_tmp + 3; c++) {
                    if(min > totalE[temp]) {
                        min = totalE[temp];
                        move_to_col = c;
                        move_to_row = r;
                    }
                    temp++;    
                }
            }

            init_col[j] = move_to_col;
            init_row[j] = move_to_row;

            free(internalE1);
            free(internalE2);
            free(totalE);
            j++;
        }

    }

    for(i = 0; i < ROWS*COLS; i++) tmp_img[i] = image[i];
    fpt = fopen("hawk_contour_final.ppm", "wb"); 
    for(i = 0; i < ptCount; i++){
        PaintContour(tmp_img, init_col[i], init_row[i], ptCount); 
    }
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(tmp_img,1,ROWS*COLS,fpt);
    fclose(fpt);
    free(tmp_img);

    fpt = fopen("final_pts.txt", "w");
    for(i = 0; i < ptCount; i++) {
        fprintf(fpt, "%d %d\n", init_row[i], init_col[i]);
    }
    fclose(fpt);
    

    free(externalE);

}


void PaintContour(unsigned char *image, int r, int c, int count) {
    int x, y;

    for(x = -3; x <= 3; x++) {
        image[(r+x)*COLS + c] = 0;
    }
    for(y = -3; y <= 3; y++) {
        image[(r)*COLS + c + y] = 0;
    }
}
