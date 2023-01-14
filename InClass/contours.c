	/*
	** Coding challenge:  calculate even-spacing energy
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

int main(int argc, char *argv[])

{
int	px[3],py[3];
int	window,x,y;
int	energy[19*19];

if (argc != 2)
  {
  printf("Usage:  spacing [window]\n");
  exit(0);
  }
window=atof(argv[1]);
if (window < 3  ||  window > 19)
  {
  printf("3 <= window <= 19\n");
  exit (0);
  }

	/* three points, 2nd point is closer to 3rd point than to 1st point */
px[0]=20; py[0]=32;
px[1]=28; py[1]=39;
px[2]=29; py[2]=42;

	/* calculate even-spacing energy around point 1 */
for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    {
	/* YOUR CODE HERE.  See "hints" for suggestions. */
    energy[y*window+x]= 
    }
  }

for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    printf("%2d ",energy[y*window+x]);
  printf("\n");
  }


}
