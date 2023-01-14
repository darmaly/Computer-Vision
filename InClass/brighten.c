	/*
	** Demonstrates how to load a PPM image from a file,
	** do some simple image processing, and then write out
	** the result to another PPM file.
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main (int argc, char *argv[])

{
FILE		*fpt;
unsigned char	*image;
char		header[80];
int		ROWS,COLS,BYTES;
int		i,j,deg,rad;
unsigned char *output;


	/* tell user how to use program if incorrect arguments */
if (argc != 3)
  {
  printf("Usage:  brighten [filename] [amount]\n");
  exit(0);
  }
deg=atoi(argv[2]);
if (deg < 0  ||  deg > 360)
  {
  printf("amount must be 0...360\n");
  exit(0);
  }
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
output=(unsigned char *)calloc(ROWS*COLS,1);
if (image == NULL || output == NULL)
  {
  printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
  exit(0);
  }
	/* read image data from file */
fread(image,1,ROWS*COLS,fpt);
fclose(fpt);

rad = deg * (M_PI/180);
int x0, y0;
int x1, y1;
int x2, y2;

x0 = COLS/2;
y0 = ROWS/2;


	/* simple CV operation to add b to every pixel */
for (x1=0; x1<ROWS; x1++) {
  for(y1 = 0; y1 < COLS; y1++){
    y2 = cos(rad)*(x1-x0)-sin(rad)*(y1-y0)+x0;
    x2=sin(rad)*(x1-x0)+cos(rad)*(y1-y0)+y0;
    output[y1*COLS+x1] = image[y2*COLS+x2];
  }
}

	/* write out brighter.ppm, the output result */
fpt=fopen("rotate.ppm","w");
if (fpt == NULL)
  {
  printf("Unable to open brighter.ppm for writing\n");
  exit(0);
  }
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(output,1,ROWS*COLS,fpt);
fclose(fpt);
}
