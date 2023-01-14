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
int		i,j,b;

	/* tell user how to use program if incorrect arguments */
if (argc != 3)
  {
  printf("Usage:  brighten [filename] [amount]\n");
  exit(0);
  }
b=atoi(argv[2]);
if (b < 0  ||  b > 255)
  {
  printf("amount must be 0...255\n");
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
if (image == NULL)
  {
  printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
  exit(0);
  }
	/* read image data from file */
fread(image,1,ROWS*COLS,fpt);
fclose(fpt);

	/* simple CV operation to add b to every pixel */
for (i=0; i<ROWS*COLS; i++)
  {
  j=(int)(image[i])+b;
  if (j > 255)
    j=255;
  image[i]=(unsigned char)j;
  }

	/* write out brighter.ppm, the output result */
fpt=fopen("brighter.ppm","w");
if (fpt == NULL)
  {
  printf("Unable to open brighter.ppm for writing\n");
  exit(0);
  }
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(image,1,ROWS*COLS,fpt);
fclose(fpt);
}
