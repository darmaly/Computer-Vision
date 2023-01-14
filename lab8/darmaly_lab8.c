/*
    Deeb Armaly
    ECE 4310
    Fall 2022
    Lab 8
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x) * (x))
#define THRESHOLD 165
#define ANGLTHRESH 0.63
#define MINREG 70
#define MAX_QUEUE 10000 /* max perimeter size (pixels) of border wavefront */

int main(int argc, char *argv[])
{

    FILE *fpt;
    int i, j;
    char header[80];
    int ROWS, COLS, BYTES;
    unsigned char *image, *thresh_img;

    /* Get Image Data */
    if ((fpt = fopen("chair-range.ppm", "rb")) == NULL)
    {
        printf("Unable to open chair-range.ppm for reading\n");
        exit(0);
    }

    fscanf(fpt, "%s %d %d %d", header, &COLS, &ROWS, &BYTES);
    if (strcmp(header, "P5") != 0 || BYTES != 255)
    {
        printf("cahir-range.ppm must be a greyscale PPM 8-bit image\n");
        exit(0);
    }

    header[0] = fgetc(fpt); /* whitespace char after header */
    image = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));
    fread(image, 1, ROWS * COLS, fpt);
    fclose(fpt);

    /* Mask Range Image By Thresholding */
    thresh_img = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));

    if ((fpt = fopen("range_img_threshold.ppm", "wb")) == NULL)
    {
        printf("Unable to range_img_threshold.ppm for writing\n");
        exit(0);
    }

    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLS; j++)
        {
            if (image[i * COLS + j] > THRESHOLD)
            {
                thresh_img[i * COLS + j] = 255;
            }
            else
            {
                thresh_img[i * COLS + j] = image[i * COLS + j];
            }
        }
    }
    fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
    fwrite(thresh_img, ROWS * COLS, sizeof(unsigned char), fpt);
    fclose(fpt);

    /* Calculate 3D Coords */
    double *X, *Y, *Z;
    int r, c;
    double cp[7];
    double xangle, yangle, dist;
    double ScanDirectionFlag, SlantCorrection;
    int ImageTypeFlag;
    ScanDirectionFlag = 1; /* assumed to be scan-direction downward */

    X = (double *)calloc(ROWS * COLS, sizeof(double));
    Y = (double *)calloc(ROWS * COLS, sizeof(double));
    Z = (double *)calloc(ROWS * COLS, sizeof(double));

    cp[0] = 1220.7;           /* horizontal mirror angular velocity in rpm */
    cp[1] = 32.0;             /* scan time per single pixel in microseconds */
    cp[2] = (COLS / 2) - 0.5; /* middle value of columns */
    cp[3] = 1220.7 / 192.0;   /* vertical mirror angular velocity in rpm */
    cp[4] = 6.14;             /* scan time (with retrace) per line in milliseconds */
    cp[5] = (ROWS / 2) - 0.5; /* middle value of rows */
    cp[6] = 10.0;             /* standoff distance in range units (3.66cm per r.u.) */

    cp[0] = cp[0] * 3.1415927 / 30.0; /* convert rpm to rad/sec */
    cp[3] = cp[3] * 3.1415927 / 30.0; /* convert rpm to rad/sec */
    cp[0] = 2.0 * cp[0];              /* beam ang. vel. is twice mirror ang. vel. */
    cp[3] = 2.0 * cp[3];              /* beam ang. vel. is twice mirror ang. vel. */
    cp[1] /= 1000000.0;               /* units are microseconds : 10^-6 */
    cp[4] /= 1000.0;                  /* units are milliseconds : 10^-3 */

    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            SlantCorrection = cp[3] * cp[1] * ((double)c - cp[2]);
            xangle = cp[0] * cp[1] * ((double)c - cp[2]);
            yangle = (cp[3] * cp[4] * (cp[5] - (double)r)) + SlantCorrection * ScanDirectionFlag;
            dist = (double)image[r * COLS + c] + cp[6];
            Z[r * COLS + c] = sqrt((dist * dist) / (1.0 + (tan(xangle) * tan(xangle)) + (tan(yangle) * tan(yangle))));
            X[r * COLS + c] = tan(xangle) * Z[r * COLS + c];
            Y[r * COLS + c] = tan(yangle) * Z[r * COLS + c];
        }
    }

    /* Calculate Surface Normals */
    double x0, y0, z0, x1, y1, z1;
    double *Xsn, *Ysn, *Zsn;
    int width = 3;

    Xsn = (double *)calloc(ROWS * COLS, sizeof(double));
    Ysn = (double *)calloc(ROWS * COLS, sizeof(double));
    Zsn = (double *)calloc(ROWS * COLS, sizeof(double));

    for (r = 0; r < (ROWS - width); r++)
    {
        for (c = 0; c < (COLS - width); c++)
        {
            /* Vector a */
            x0 = X[r * COLS + c + width] - X[r * COLS + c];
            y0 = Y[r * COLS + c + width] - Y[r * COLS + c];
            z0 = Z[r * COLS + c + width] - Z[r * COLS + c];

            /* Vector b */
            x1 = X[(r + width) * COLS + c] - X[r * COLS + c];
            y1 = Y[(r + width) * COLS + c] - Y[r * COLS + c];
            z1 = Z[(r + width) * COLS + c] - Z[r * COLS + c];

            /* Cross Product of vectors a and b */
            Xsn[r * COLS + c] = (y0 * z1) - (z0 * y1);
            Ysn[r * COLS + c] = (x0 * z1) - (z0 * x1);
            Zsn[r * COLS + c] = (x0 * y1) - (y0 * x1);
        }
    }

    /* Region Grow */
    unsigned char *region_img;
    int TotalRegions;
    int check = 0;
    int count = 0;
    double avgx, avgy, avgz;
    double dotprod;
    double angle;
    double orientation_a, orientation_b;
    double totalx, totaly, totalz;
    int r2, c2;
    int queue[MAX_QUEUE], qh, qt;
    int new_label;

    avgx = avgy = avgz = 0;
    totalx = totaly = totalz = 0;
    orientation_a = orientation_b = 0;

    region_img = (unsigned char *)calloc(ROWS * COLS, sizeof(unsigned char));

    TotalRegions = 0;
    for (i = width; i < ROWS - width; i++)
    {
        for (j = width; j < COLS - width; j++)
        {
            check = 1;
            /* Check 5x5 window */
            for (r = -2; r <= 2; r++)
            {
                for (c = -2; c <= 2; c++)
                {
                    if (region_img[(i + r) * COLS + j + c] != 0 || thresh_img[(i + r) * COLS + j + c] == 255)
                        check = 0;
                }
            }

            /* Paint Fill */
            if (check == 1)
            {
                count = 0;
                queue[0] = i * COLS + j;
                qh = 1; /* queue head */
                qt = 0; /* queue tail */
                count = 1;

                /* Initial orientation of pixels */
                avgx = Xsn[i * COLS + j];
                avgy = Ysn[i * COLS + j];
                avgz = Zsn[i * COLS + j];

                totalx = Xsn[i * COLS + j];
                totaly = Ysn[i * COLS + j];
                totalz = Zsn[i * COLS + j];

                new_label = rand() % (210 - 20) + 20;

                while (qt != qh)
                {
                    for (r2 = -1; r2 <= 1; r2++)
                    {
                        for (c2 = -1; c2 <= 1; c2++)
                        {
                            if (region_img[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] != 0)
                            {
                                continue;
                            }

                            if (r2 == 0 && c2 == 0)
                            {
                                continue;
                            }

                            if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS - width || (queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS - width)
                            {
                                continue;
                            }

                            orientation_a = sqrt(SQR(avgx) + SQR(avgy) + SQR(avgz));
                            orientation_b = sqrt(SQR(Xsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2]) + SQR(Ysn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2]) + SQR(Zsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2]));
                            dotprod = (Xsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] * avgx) + (Ysn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] * avgy) + (Zsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] * avgz);
                            angle = acos(dotprod / (orientation_a * orientation_b));
                            angle = fabs(angle);


                            /* if pixel is greater than threshold, do not include it in region */
                            if (angle > ANGLTHRESH)
                            {
                                continue;
                            }

                            /* If all checks have passed, pixel joins region */
                            count++;

                            totalx += Xsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];
                            totaly += Ysn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];
                            totalz += Zsn[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];

                            /* Compute new average with newly joined pixel */
                            avgx = totalx / count;
                            avgy = totaly / count;
                            avgz = totalz / count;

                            region_img[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] = new_label;

                            queue[qh] = (queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2;
                            qh = (qh + 1) % MAX_QUEUE;
                            if (qh == qt)
                            {
                                printf("Max queue size exceeded\n");
                                exit(0);
                            }
                        }
                    }
                    qt = (qt + 1) % MAX_QUEUE;
                }

                /* If region is not large enough (noise) repaint the region */
                if (count > 1 && count < MINREG)
                {
                    for (r = 0; r < ROWS; r++)
                    {
                        for (c = 0; c < COLS; c++)
                        {
                            if (region_img[r * COLS + c] == new_label)
                            {
                                region_img[r * COLS + c] = 0;
                            }
                        }
                    }
                }
                else if (count > 1)
                {
                    TotalRegions++;
                    printf("Region #%d\tNumber of Pixels: %d\tAvg Surface Normals[x,y,z]: %0.3lf, %0.3lf, %0.3lf\n", TotalRegions, count, avgx, avgy, avgz);
                }
            }
        }
    }

    if ((fpt = fopen("region_img.ppm", "wb")) == 0)
    {
        printf("Unable to open region_img.ppm for writing\n");
        exit(0);
    }
    fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
    fwrite(region_img, ROWS * COLS, sizeof(unsigned char), fpt);
    fclose(fpt);

    free(X);
    free(Y);
    free(Z);

    free(Xsn);
    free(Ysn);
    free(Zsn);

    free(image);
    free(thresh_img);
    free(region_img);

    return (0);
}