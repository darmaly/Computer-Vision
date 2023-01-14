#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {
    FILE		*fpt, *fpt2;
    unsigned char	*image;
    char		header[80], letter, gt_letter;
    int		ROWS,COLS,BYTES;
    int		i,j,b,r,c,letter_r,letter_c;
    int *intensity;

        /* tell user how to use program if incorrect arguments */
    if (argc != 2) {
        printf("Usage:  outline [Letter]\n");
        exit(0);
    }

    letter = argv[1][0];

    fpt = fopen("parenthood.ppm", "rb");
    	/* read image header (simple 8-bit greyscale PPM only) */
    i=fscanf(fpt,"%s %d %d %d ",header,&COLS,&ROWS,&BYTES);
    if (i != 4  ||  strcmp(header,"P5") != 0  ||  BYTES != 255)
    {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n",argv[1]);
        fclose(fpt);
        exit(0);
    }
        /* allocate dynamic memory for image */
    image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    if (image == NULL)
    {
        printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
        exit(0);
    }
        /* read image data from file */
    fread(image,1,ROWS*COLS,fpt);
    fclose(fpt);

        /* open GT file for reading */
    fpt=fopen("parenthood_gt.txt","r");


}