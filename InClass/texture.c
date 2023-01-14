	/*
	** Calculate the grey-level cooccurence matrix for a given
	** d-vector (distance) and window size.
	** Calculate entropy and output a normalized greyscale
	** of entropy for display.
	**
	** Run on zebra.ppm using f=0 d=1 and w=11.
	** Compare to w=3 to see effect of window size.
	** Compare to f=1 to see effect of different formulas.
	**
	** Coding challenge:  Try to find f/d/w for eggs-grey.ppm
	** that help make eggs stand out compared to Sobel gradients.
	*/

#define	GRAYS 16	/* number of grayscale ranges to use for GLCM */
			/* this downsamples the #bits per pixel */
			/* must be a power of 2 */
#define	DEBUG 0		/* turn on to print out GLCM to see example */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main (int argc, char *argv[])

{
FILE		*fpt;
unsigned char	*image,*image_g,*image_g_scaled,*output;
char		header[80];
int		ROWS,COLS,BYTES;
int		i,j,d,w,r,c,r2,c2;
int		*glcm;
double		*texture,min,max;
int		formula;


	/* tell user how to use program if incorrect arguments */
if (argc != 5)
  {
  printf("Usage:  texture [filename] [formula] [d] [window]\n");
  exit(0);
  }
formula=atoi(argv[2]);
if (formula < 0  ||  formula > 3)
  {
  printf("formula must be 0...3\n");
  exit(0);
  }
d=atoi(argv[3]);
w=atoi(argv[4]);

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

	/* calculate downsampled image (has only GRAYS pixel values) */
image_g=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
image_g_scaled=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
for (i=0; i<ROWS*COLS; i++)
  {
  image_g[i]=image[i]/(256/GRAYS);
  image_g_scaled[i]=image_g[i]*(256/GRAYS);
  }
	/* output downsampled image so we can check grey-resolution */
fpt=fopen("greys.ppm","wb");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(image_g_scaled,ROWS*COLS*1,1,fpt);
fclose(fpt);

texture=(double *)calloc(ROWS*COLS,sizeof(double));
glcm=(int *)calloc(GRAYS*GRAYS,sizeof(int));

	/* calculate glcm at each pixel */
for (r=0; r<ROWS-w-d; r++)
  for (c=0; c<COLS-w-d; c++)
    {
    for (i=0; i<GRAYS*GRAYS; i++)
      glcm[i]=0;
    for (r2=r; r2<r+w; r2++)
      for (c2=c; c2<c+w; c2++)
        {
        i=r2*COLS+c2;
        j=(r2+d)*COLS+(c2+d);
        glcm[image_g[i]*GRAYS+image_g[j]]++;
        }
    if (DEBUG)
      {
      for (i=0; i<GRAYS; i++)
        {
        for (j=0; j<GRAYS; j++)
          {
          printf("%2d ",glcm[i*GRAYS+j]);
          }
        printf("\n");
        }
      exit(0);
      }
    texture[r*COLS+c]=0.0;
    for (i=0; i<GRAYS; i++)
      for (j=0; j<GRAYS; j++)
        {
	if (formula == 0)	/* calculate entropy */
          {
          if (glcm[i*GRAYS+j] > 0)
            texture[r*COLS+c]-=(double)glcm[i*GRAYS+j]*
			log((double)glcm[i*GRAYS+j]);
          }
	else if (formula == 1)	/* calculate max */
          {
          if ((double)glcm[i*GRAYS+j] > texture[r*COLS+c])
            texture[r*COLS+c]=(double)glcm[i*GRAYS+j];
          }
	else if (formula == 2)	/* calculate homogeneity */
          {
          texture[r*COLS+c]+=(double)glcm[i*GRAYS+j]/(1.0+fabs(i-j));
          }
	else if (formula == 3)	/* calculate energy */
          {
          texture[r*COLS+c]+=(double)glcm[i*GRAYS+j]*(double)glcm[i*GRAYS+j];
          }
        }
    if (r == 0  &&  c == 0)
      min=max=texture[r*COLS+c];
    if (texture[r*COLS+c] < min)
      min=texture[r*COLS+c];
    if (texture[r*COLS+c] > max)
      max=texture[r*COLS+c];
    }
if (DEBUG)
  printf("%lf to %lf\n",min,max);

	/* convert texture image to greyscale display */
output=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
for (i=0; i<ROWS*COLS; i++)
  output[i]=(unsigned char)((texture[i]-min)*(255.0/(max-min)));

	/* write out texture.ppm, the output result */
fpt=fopen("texture.ppm","w");
if (fpt == NULL)
  {
  printf("Unable to open texture.ppm for writing\n");
  exit(0);
  }
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(output,1,ROWS*COLS,fpt);
fclose(fpt);
}