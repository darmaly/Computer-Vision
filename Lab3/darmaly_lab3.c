/*
    Deeb Armaly
    ECE 4310
    Lab 2
    Fall 2022
    C17525380
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {

    FILE            *fpt;
    unsigned char   *image, *msf;
    char            header[80];
    int             i;
    int		        ROWS,COLS,BYTES;
    int		        ROWS_m,COLS_m,BYTES_m;

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

        /* open msf image */
    fpt = fopen("msf_e.ppm", "rb");
    if(fpt == NULL) {
        printf("Unable to open %s for reading\n", "parenthood.ppm");
        exit(0);
    }

    	/* read image header (simple 8-bit greyscale PPM only) */
    i = fscanf(fpt,"%s %d %d %d ",header,&COLS_m,&ROWS_m,&BYTES_m);
    if (i != 4  ||  strcmp(header,"P5") != 0  ||  BYTES_m != 255) {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n", "msf_e.ppm");
        fclose(fpt);
        exit(0);
    }

        /* allocate dynamic memory for msf image */
    msf = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    if (msf == NULL) {
        printf("Unable to allocate %d x %d memory\n",COLS_m,ROWS_m);
        exit(0);
    }
        /* read image data from file */
    fread(msf,1,ROWS_m*COLS_m,fpt);
    fclose(fpt);

        /* open ground truth file for reading */
    fpt = fopen("parenthood_gt.txt","rb");
    if(fpt == NULL) {
        printf("Unable to open %s for reading\n", "parenthood_gt.txt");
        exit(0);
    }

    
    unsigned char *thresh, *pixel_image, *marked_img, *points_image;
    int t, x, y, row, col, tp, fp, m, temp, tn, fn, z, f;
    int edge_trans, edge_neighbors, pass_test;
    int N, E, S, W;
    int thin_count;
    int branch_pt, end_pt;
    float tpr, fpr;
    char gt_char;
    thresh = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    pixel_image = (unsigned char *)calloc(9*15,sizeof(unsigned char));
    marked_img = (unsigned char *)calloc(9*15,sizeof(unsigned char));
    points_image = (unsigned char *)calloc(9*15,sizeof(unsigned char));


    FILE *result, *pix_img;
    result = fopen("results.txt", "w");

    for(t = 0; t <= 255; t++) {

            /* threshold image */
        for(x = 0; x < ROWS; x++){
            for(y = 0; y < COLS; y++) {
                if(msf[x*COLS+y] > t) {
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
            m = fscanf(fpt,"%c %d %d ", &gt_char, &col, &row);

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

            if(temp == 0) {
                if(gt_char == 101) {
                    fn++;
                }
                else {
                    tn++;
                }
                continue;
            }

                /* pixel image that is a copy of the area centered at the ground truth location from the original image */
            for(x = -7; x <= 7; x++){
                for(y = -4; y <= 4; y++) {
                    pixel_image[(x+7)*9+(y+4)] = image[(row+x)*COLS+(col+y)];
                }
            }

            /*
            pix_img = fopen("pixel_image.ppm", "w");
            fprintf(pix_img,"P5 %d %d 255\n",9,15);
            fwrite(pixel_image,9*15,1,pix_img);
            fclose(pix_img);
            */

                /* threshold pixel image at 128 */
            for(x = 0; x < 15; x++){
                for(y = 0; y < 9; y++) {
                    if(pixel_image[x*9+y] > 128) {
                        pixel_image[x*9+y] = 255;
                    }
                    else {
                        pixel_image[x*9+y] = 0;
                    }
                }
            }
            /*
            pix_img = fopen("pixel_image_binary.ppm", "w");
            fprintf(pix_img,"P5 %d %d 255\n",9,15);
            fwrite(pixel_image,9*15,1,pix_img);
            fclose(pix_img);
            */

            
            while(1) {
                    /* thin pixel image */
                for(x = 0; x < 15; x++){
                    for(y = 0; y < 9; y++) {
                        marked_img[x*9+y] = pixel_image[x*9+y];
                    }
                }
                thin_count = 0;
                    /* Determine edge to non-edge transitions */           
                for(x = 1; x < 14; x++){
                    for(y = 1; y < 8; y++) {
                        edge_neighbors = 0;
                        pass_test = 0;
                        edge_trans = 0;
                        if(marked_img[x*9+y] == 0) {
                                /* Find edge neighbors */
                            for(z = -1; z <= 1; z++) {
                                for(f = -1; f <= 1; f++) {
                                    if(z == 0 && f == 0) {
                                        continue;
                                    }
                                    else if(marked_img[(x+z)*9+(y+f)] == 0) {
                                        edge_neighbors++;
                                    }
                                }
                                
                            }

                                /* get edge transitions */
                            if(marked_img[(x-1)*9+(y-1)] == 0 && marked_img[(x-1)*9+y] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x-1)*9+(y)] == 0 && marked_img[(x-1)*9+(y+1)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x-1)*9+(y+1)] == 0 && marked_img[(x)*9+(y+1)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x)*9+(y+1)] == 0 && marked_img[(x+1)*9+(y+1)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x+1)*9+(y+1)] == 0 && marked_img[(x+1)*9+(y)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x+1)*9+(y)] == 0 && marked_img[(x+1)*9+(y-1)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x+1)*9+(y-1)] == 0 && marked_img[(x)*9+(y-1)] == 255) {
                                edge_trans++;
                            }
                            if(marked_img[(x)*9+(y-1)] == 0 && marked_img[(x-1)*9+(y-1)] == 255) {
                                edge_trans++;
                            }

                            

                                /* North East South West edge pixel check */
                            N = marked_img[(x-1)*9+y];
                            E = marked_img[(x)*9+(y+1)];
                            S = marked_img[(x+1)*9+y];
                            W = marked_img[x*9+(y-1)];
                            if(N != 0 || E != 0 || (S != 0 && W != 0)) {
                                pass_test = 1;
                            }

                            if(edge_trans == 1 && 2 <= edge_neighbors && edge_neighbors <= 6 && pass_test == 1) {
                                pixel_image[x*9+y] = 255;
                                thin_count++;
                            }

                        }
                    }
                }
                if(thin_count == 0) {
                    break;
                }
            }

            /*
            pix_img = fopen("pixel_image_thinned.ppm", "w");
            fprintf(pix_img,"P5 %d %d 255\n",9,15);
            fwrite(pixel_image,9*15,1,pix_img);
            fclose(pix_img);         
            */

            for(x = 0; x < 15; x++){
                for(y = 0; y < 9; y++) {
                    points_image[x*9+y] = pixel_image[x*9+y];
                }
            }

            branch_pt = 0;
            end_pt = 0;
            for(x = 1; x < 14; x++){
                for(y = 1; y < 8; y++) {
                    edge_trans = 0;
                    if(pixel_image[x*9+y] == 0) {
                            /* get edge transitions */
                        if(pixel_image[(x-1)*9+(y-1)] == 0 && pixel_image[(x-1)*9+y] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x-1)*9+(y)] == 0 && pixel_image[(x-1)*9+(y+1)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x-1)*9+(y+1)] == 0 && pixel_image[(x)*9+(y+1)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x)*9+(y+1)] == 0 && pixel_image[(x+1)*9+(y+1)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x+1)*9+(y+1)] == 0 && pixel_image[(x+1)*9+(y)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x+1)*9+(y)] == 0 && pixel_image[(x+1)*9+(y-1)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x+1)*9+(y-1)] == 0 && pixel_image[(x)*9+(y-1)] == 255) {
                            edge_trans++;
                        }
                        if(pixel_image[(x)*9+(y-1)] == 0 && pixel_image[(x-1)*9+(y-1)] == 255) {
                            edge_trans++;
                        }
                        
                        if (edge_trans == 1) {
                            end_pt++;
                            points_image[x*9+y] = 100;
                        }
                        else if(edge_trans > 2) {
                            branch_pt++;
                            points_image[x*9+y] = 150;
                        }
                    }

                    
                }
            }

            /*
            pix_img = fopen("branch_and_end_points.ppm", "w");
            fprintf(pix_img,"P5 %d %d 255\n",9,15);
            fwrite(points_image,9*15,1,pix_img);
            fclose(pix_img);

            if(gt_char == 'e') {
                printf("For letter %c, the number of branch points is %d and endpoints is %d\n", gt_char, branch_pt, end_pt);
                exit(0);
            }
            */
            

            if(branch_pt != 1 || end_pt != 1) {
                if(gt_char == 101) {
                    fn++;
                    /*
                    pix_img = fopen("branch_and_end_points.ppm", "w");
                    fprintf(pix_img,"P5 %d %d 255\n",9,15);
                    fwrite(points_image,9*15,1,pix_img);
                    fclose(pix_img);

                    pix_img = fopen("pixel_image_thinned.ppm", "w");
                    fprintf(pix_img,"P5 %d %d 255\n",9,15);
                    fwrite(pixel_image,9*15,1,pix_img);
                    fclose(pix_img);
                    exit(0);
                    */
                }
                else {
                    tn++;
                }
                continue;
            }

            if(gt_char == 101) {
                tp++;
                /*
                pix_img = fopen("branch_and_end_points.ppm", "w");
                fprintf(pix_img,"P5 %d %d 255\n",9,15);
                fwrite(points_image,9*15,1,pix_img);
                fclose(pix_img);

                pix_img = fopen("pixel_image_thinned.ppm", "w");
                fprintf(pix_img,"P5 %d %d 255\n",9,15);
                fwrite(pixel_image,9*15,1,pix_img);
                fclose(pix_img);
                exit(0);
                */
            }
            else {
                fp++;
            }

        }
        tpr =(float) tp/(tp + fn);
        fpr =(float) fp/(fp + tn);

        //fprintf(result,"Threshold = %d\t\tTP = %d\t\tFP = %d\t\tFN = %d\t\tTN = %d\t\tTPR = %0.3f\t\tFPR = %0.3f\n", t, tp, fp, fn, tn, tpr, fpr);
        fprintf(result,"%0.4f\n",fpr);
        rewind(fpt);
    }

    fclose(result);
    fclose(fpt);


    // Free dynamic memory
    free(image);
    free(msf);
    free(pixel_image);
    free(marked_img);
}