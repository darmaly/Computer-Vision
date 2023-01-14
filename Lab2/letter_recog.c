/*
    Deeb Armaly
    ECE 4310
    Lab 2
    Fall 2022
    C17525380

    This lab will implement a matched filter in order to recognize letters in an 
    image of text. To run the program simply run the executable and ensure that
    the three files: 
        parenthood.ppm
        parenthood_e_template.ppm
        parenthood_gt.txt
    are in the directory the executable will be run on.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    /* Function declarations */
int *zero_mean_template(unsigned char *template, int r, int c);
int *msf_image_create(unsigned char *image, int ri, int ci, int *zm_template, int rt, int ct);
unsigned char *normalize(int *msf_image, int r, int c);

int main (int argc, char *argv[]) {

    FILE		    *fpt;
    unsigned char	*image, *template;
    char		    header[80], header_t[80];
    int		        ROWS,COLS,BYTES;
    int		        ROWS_t,COLS_t,BYTES_t;
    int		        i;

        /* Open image with text to be scanned */
    fpt = fopen("parenthood.ppm", "rb");
    if(fpt == NULL) {
        printf("Unable to open %s for reading\n", "parenthood.ppm");
        exit(0);
    }

    	/* read image header (simple 8-bit greyscale PPM only) */
    i = fscanf(fpt,"%s %d %d %d ",header,&COLS,&ROWS,&BYTES);
    if (i != 4  ||  strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n", "parenthood.ppm");
        fclose(fpt);
        exit(0);
    }
        /* allocate dynamic memory for image */
    image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    if (image == NULL) {
        printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
        exit(0);
    }
        /* read image data from file */
    fread(image,1,ROWS*COLS,fpt);
    fclose(fpt);

        /* open template for reading */
    fpt = fopen("parenthood_e_template.ppm", "rb");
    if(fpt == NULL) {
        printf("Unable to open %s for reading\n", "parenthood_e_template.ppm");
        exit(0);
    }

    i = fscanf(fpt, "%s %d %d %d ", header_t, &COLS_t, &ROWS_t, &BYTES_t);
    if (i != 4  ||  strcmp(header,"P5") != 0  ||  BYTES != 255) {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n", "parenthood_e_template.ppm");
        fclose(fpt);
        exit(0);
    }

    template = (unsigned char *)calloc(ROWS_t*COLS_t, sizeof(unsigned char));
    fread(template, 1, ROWS_t*COLS_t, fpt);
    fclose(fpt);

        /* open ground truth file for reading */
    fpt = fopen("parenthood_gt.txt","r");
    if(fpt == NULL) {
        printf("Unable to open %s for reading\n", "parenthood_gt.txt");
        exit(0);
    }

        /* First get the zero mean template */
    int *zm_temp; 
    zm_temp = zero_mean_template(template,ROWS_t,COLS_t);

        /* Get MSF image */
    int *msf_image;
    msf_image = msf_image_create(image, ROWS, COLS, zm_temp, ROWS_t, COLS_t);

        /* Normalize MSF image */
    unsigned char *normalized;
    normalized = normalize(msf_image, ROWS, COLS);

        /* Write out normalized image */
    FILE *fpt2;
    fpt2 = fopen("msf_normalized.ppm","w");
    fprintf(fpt2,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(normalized,COLS*ROWS,1,fpt2);
    fclose(fpt2);

        /* Open ground truth file */
    fpt2 = fopen("parenthood_gt.txt", "rb");

    FILE *result;
    result = fopen("results.txt", "w");

    unsigned char *thresh;
    int t, x, y, row, col, tp, fp, m, temp, tn, fn;
    float tpr, fpr;
    char gt_char;
    thresh = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    for(t = 0; t <= 255; t++) {
            /* threshold image */
        for(x = 0; x < ROWS; x++){
            for(y = 0; y < COLS; y++) {
                if(normalized[x*COLS+y] > t) {
                    thresh[x*COLS+y] = 255;
                }
                else {
                    thresh[x*COLS+y] = 0;    
                }
            }
        }

        tp = 0;
        fp = 0;
        tn = 0;
        fn = 0;
        tpr = 0.0;
        fpr = 0.0;
          
        while(1) {
            temp = 0;
            m = fscanf(fpt2,"%c %d %d ", &gt_char, &col, &row);
            if(m != 3){
                break;
            }
            for(x = (row - 7); x <= (row + 7); x++) {
                for(y = (col - 4); y <= (col + 4); y++) {
                    if(thresh[x*COLS+y] == 255) {
                        temp = 1;
                    }
                }
            }  
            if(temp == 1){
                if(gt_char == 101) {
                    tp++;
                }
                else {
                    fp++;
                }
            }
            else if(temp == 0) {
                if(gt_char == 101) {
                    fn++;
                }
                else {
                    tn++;
                }
            }
        }
        tpr =(float) tp/(tp + fn);
        fpr =(float) fp/(fp + tn);

        fprintf(result,"Threshold = %d\t\tTP = %d\t\tFP = %d\t\tFN = %d\t\tTN = %d\t\tTPR = %0.3f\t\tFPR = %0.3f\n", t, tp, fp, fn, tn, tpr, fpr);
        rewind(fpt2);
    }

    fclose(fpt2);
    fclose(result);
    free(thresh);
    free(image);
    free(template);
    free(zm_temp);
    free(msf_image);
    free(normalized);

}


/*
The zero_mean_template function creates a zero mean template by taking the sum
of all the pixels in the template, dividing the sum by the number of pixels to
get the mean, and then subtracting the mean from each of the original template
pixel values.
Input:
    pointer to template image data
    template row count
    template column count

Output:
    pointer to newly created zero mean template
*/
int *zero_mean_template(unsigned char *template, int r, int c) {

    int *zm_temp;
    int i,j,mean,sum;
    sum = 0;
    mean = 0;

        /* Calculate total sum of pixel values in template */
    for(i = 0; i < r; i++){
        for(j = 0; j < c; j++){
            sum += (int) template[i*c+j];
        }
    }
        /* Get the mean pixel value */
    mean = sum/(r*c);

        /* Get the zero mean template */
    zm_temp = (int *)calloc(r*c, sizeof(int));
    for(i = 0; i < r; i++){
        for(j = 0; j < c; j++) {
            zm_temp[i*c+j] = (int) (template[i*c+j] - mean);
        }
    }

    return zm_temp;
}



/*
msf_image_create function convolutes the zero mean template with the original 
image and produces a new matched spatial filtered image.
Input:
    pointer to original image data
    image rows
    image columns
    pointer to zero mean template data
    template rows
    template columns
Output:
    pointer to matched spatial filtered image data
*/
int *msf_image_create(unsigned char *image, int ri, int ci, int *zm_template, int rt, int ct) {
    int *msf_image;
    int r,c,r2,c2;
    int sum = 0;

    msf_image = (int *)calloc(ri*ci, sizeof(int));
    for(r = 7; r < (ri - 7); r++) {
        for(c = 4; c < (ci - 4); c++) {
            sum = 0;
            for(r2 = -(rt/2); r2 <= rt/2; r2++){
                for(c2 = -(ct/2); c2 <= ct/2; c2++) {
                    sum +=((int) image[(r+r2)*ci+(c+c2)])*zm_template[(r2+rt/2)*ct+(c2+ct/2)];
                }
            }
            msf_image[r*ci+c] = sum;
        }
    }

    return msf_image;
}




unsigned char *normalize(int *msf_image, int r, int c) {
    int max;
    int min;
    int norm;
    int i, j;
    unsigned char *normalized;
    max = msf_image[0];
    min = msf_image[0];

    normalized = (unsigned char *)calloc(r*c, sizeof(unsigned char));

        /* Find max and min values of MSF image */
    for(i = 0; i < r; i++){
        for(j = 0; j < c; j++){
            if(msf_image[i*c+j] > max){
                max = msf_image[i*c+j];
            }
            if(msf_image[i*c+j] < min){
                min = msf_image[i*c+j];
            }
        }
    }

        /* Normalize MSF pixel values */
    for(i = 0; i < r; i++){
        for(j = 0; j < c; j++){
            norm = ((msf_image[i*c+j] - min)*255)/(max-min);
            normalized[i*c+j] = (unsigned char) norm;
        }
    }

    return normalized;
}