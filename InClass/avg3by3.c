	/*
	** Demonstrates how to load a PPM image from a file,
	** do some simple image processing, and then write out
	** the result to another PPM file.
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])

{
FILE		*fpt;
unsigned char	*image;
char		header[80];
int		ROWS,COLS,BYTES;
int		i,j,b,r,c;

	/* tell user how to use program if incorrect arguments */
if (argc != 4)
  {
  printf("Usage:  avg [filename] [r] [c]\n");
  exit(0);
  }
r = atoi(argv[2]);
c = atoi(argv[3]);
	/* open image for reading */
fpt=fopen(argv[1],"r");
if (fpt == NULL)
  {
  printf("Unable to open %s for reading\n",argv[1]);
  exit(0);
  }
	/* read image header (simple 8-bit greyscale PPM only) */
i=fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
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

	/* simple CV operation to add b to every pixel */
int sum = 0;
int avg;
int dr, dc;
for(dr = -1; dr <= 1; dr++){
    for(dc = -1; dc <= 1; dc++){
        sum += image[(r+dr)*COLS+(c+dc)];
    }
}
avg = sum/9;
printf("Average pixel value for 3x3 surrounding pixels from point [%d][%d]is: %d\n",r, c, avg);
return 0;
}



