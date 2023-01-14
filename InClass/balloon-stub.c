	/*
	** Coding challenge:  calculate balloon energy
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

int main(int argc, char *argv[])

{
FILE	*fpt;
int	px[100],py[100];
int	i,total_points;
int	window,x,y,move_x,move_y;
int	energy[19*19];

if (argc != 3)
  {
  printf("Usage:  balloon [points.txt] [window]\n");
  exit(0);
  }
window=atof(argv[2]);
if (window < 3  ||  window > 19)
  {
  printf("3 <= window <= 19\n");
  exit (0);
  }

	/* read contour points file */
if ((fpt=fopen(argv[1],"r")) == NULL)
  {
  printf("Unable to open %s for reading\n",argv[1]);
  exit(0);
  }
total_points=0;
while (1)
  {
  i=fscanf(fpt,"%d %d",&px[total_points],&py[total_points]);
  if (i != 2)
    break;
  total_points++;
  if (total_points > 100)
    break;
  }
fclose(fpt);

	/* calculate balloon energy around point 1 */
	/* hint:  you may want to calculate something before the loops ... */
    int centx = 0, centy = 0;
    int i;
    for(i = 0; i < total_points; i++) {
        centx += px[i];
        centy += py[i];
    }

    centx = centx / total_points;
    centy = centy / total_points;

for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    {
    energy[y*window+x]= .... YOUR CODE HERE (see hints for help)
    }
  }

for (y=0; y<window; y++)
  {
  for (x=0; x<window; x++)
    printf("%2d ",energy[y*window+x]);
  printf("\n");
  }


}
