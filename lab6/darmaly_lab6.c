/*
    Deeb Armaly
    ECE 4310
    Lab 6
    
    This lab assumes the data to be read in is stored in a file called data.txt
    and is in the current working derectory as the executable code file
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define Pi 3.142857

float variance(int count, float *data, int i);

int main(int argc, char *argv[]) {

    FILE *fpt;
    char header[7][10];
    int i, j;
    int count = 0;
    float temp1, temp2, temp3, temp4, temp5, temp6, temp7;

    fpt = fopen("data.txt", "rb");
    for(i = 0; i < 7; i++){
        fscanf(fpt,"%s ", header[i]);
    }

        /* Get the number of data points in txt file */
    while(1){
        i = fscanf(fpt,"%f %f %f %f %f %f %f ", &temp1, &temp2, &temp3, &temp4, &temp5, &temp6, &temp7);
        if(i != 7) break;
        count++;
    }

    // printf("Number of data points: %d\n", count);

    float *time = (float *)calloc(count,sizeof(float));
    float *accX = (float *)calloc(count,sizeof(float));
    float *accY = (float *)calloc(count,sizeof(float));
    float *accZ = (float *)calloc(count,sizeof(float));
    float *pitch = (float *)calloc(count,sizeof(float));
    float *roll = (float *)calloc(count,sizeof(float));
    float *yaw = (float *)calloc(count,sizeof(float));

    rewind(fpt);
    for(i = 0; i < 7; i++){
        fscanf(fpt,"%s ", header[i]);
    }

        /* Store Data from txt file */
    i = 0;
    while(fscanf(fpt,"%f %f %f %f %f %f %f ", &temp1, &temp2, &temp3, &temp4, &temp5, &temp6, &temp7) == 7) {
        time[i] = temp1;
        accX[i] = temp2;
        accY[i] = temp3;
        accZ[i] = temp4;
        pitch[i] = temp5;
        roll[i] = temp6;
        yaw[i] = temp7;

        i++;
    }

    //printf("%f %f %f %f %f %f %f\n",time[0],accX[0],accY[0],accZ[0],pitch[0],roll[0],yaw[0]);
    fclose(fpt);

    float sampleTime = 0.05;
    float accThresh = 0.0009;
    float gyroThresh = 0.01;
    float varX, varY, varZ;
    float pitchVar, rollVar, yawVar;
    int   moveDetectAcc, moveDetectGyr;
    int   movStartA, movEndA, movStartG, movEndG;
    float startTimeA, endTimeA, startTimeG, endTimeG;
    float velX, velY, velZ;
    float prevVelX, prevVelY, prevVelZ;
    float distX, distY, distZ;
    float totalX = 0, totalY = 0, totalZ = 0;
    float distPitch, distRoll, distYaw;
    float totalPitch = 0, totalRoll = 0, totalYaw = 0;
    int   x;

    moveDetectAcc = 0;
    moveDetectGyr = 0;
    movStartA = 0;
    movEndA = 0;
    movStartG = 0;
    movEndG = 0;

    FILE *gyr;
    FILE *acc;

    gyr = fopen("gyro_movement.txt", "w");
    fprintf(gyr,"Gyro Data\n\n");
    acc = fopen("accelerometer_movement.txt", "w");
    fprintf(acc, "Accelerometer Data\n\n");
    

    for(i = 0; i < count; i++) {

        distX = 0;
        distY = 0;
        distZ = 0;

        varX = 0;
        varY = 0;
        varZ = 0;

        velX = 0;
        velY = 0;
        velZ = 0;

        prevVelX = 0;
        prevVelY = 0;
        prevVelZ = 0;


        distPitch = 0;
        distRoll = 0;
        distYaw = 0;

        pitchVar = 0;
        rollVar = 0;
        yawVar = 0;

            /* calculate variances */
        varX = variance(count, accX, i);
        varY = variance(count, accY, i);
        varZ = variance(count, accZ, i);

        pitchVar = variance(count, pitch, i);
        rollVar = variance(count, roll, i);
        yawVar = variance(count, yaw, i);



        if((varX > accThresh) || (varY > accThresh) || (varZ > accThresh)) {
            moveDetectAcc = 1;
        }
        if((pitchVar > gyroThresh) || (rollVar > gyroThresh) || (yawVar > gyroThresh)) {
            moveDetectGyr = 1;
        }

            /* Detect when movement has started */
        if(moveDetectAcc == 1 && movStartA == 0) {
            startTimeA = time[i];
            movStartA = i;
        }
        if(moveDetectGyr == 1 && movStartG == 0) {
            startTimeG = time[i];
            movStartG = i;
        }

            /* Detect when movement has ended */
        if(moveDetectAcc == 0 && movStartA != 0 && movEndA == 0) {
            endTimeA = time[i];
            movEndA = i;
        }
        if(moveDetectGyr == 0 && movStartG != 0 && movEndG == 0) {
            endTimeG = time[i];
            movEndG = i;
        }


            /* Calculate movement for accelerometer */
        if(movStartA != 0 && movEndA != 0) {
            for(x = movStartA; x < movEndA; x++) {
                prevVelX = velX;
                prevVelY = velY;
                prevVelZ = velZ; 

                velX += (accX[x] * 9.81 * sampleTime);
                velY += (accY[x] * 9.81 * sampleTime);
                velZ += (accZ[x] * 9.81 * sampleTime);

                distX += ((velX * prevVelX) / 2) * sampleTime;
                distY += ((velY * prevVelY) / 2) * sampleTime;
                distZ += ((velZ * prevVelZ) / 2) * sampleTime; 
            }

            totalX += distX;
            totalY += distY;
            totalZ += distZ;

            fprintf(acc, "Start time: %.2f\tEnd time: %.2f\tIndices: %d through %d\tLinear Distance: X - %f Y - %f Z - %f\n",startTimeA,endTimeA,movStartA,movEndA,distX,distY,distZ);
            movStartA = 0;
            movEndA = 0;
            startTimeA = 0;
            endTimeA = 0; 
        }

            /* Calculate movement for Gyroscope */
        if(movStartG != 0 && movEndG != 0) {
            for(x = movStartG; x < movEndG; x++) {
                distPitch += (pitch[x] * sampleTime * 180) / Pi;
                distRoll += (roll[x] * sampleTime * 180) / Pi;
                distYaw += (yaw[x] * sampleTime * 180) / Pi;
            }

            totalPitch += distPitch;
            totalRoll += distRoll;
            totalYaw += distYaw;

            fprintf(gyr, "Start time: %.2f\tEnd time: %.2f\tIndices: %d through %d\tAngular Rotations: Pitch - %f Roll - %f Yaw - %f\n",startTimeG,endTimeG,movStartG,movEndG,distPitch,distRoll,distYaw);
            movStartG = 0;
            movEndG = 0;    
            startTimeG = 0;
            endTimeG = 0;
        }

        moveDetectAcc = 0;
        moveDetectGyr = 0;
    }

    fprintf(acc,"\nTotal Linear distance moved: X - %f Y - %f Z - %f\n", totalX,totalY,totalZ);
    fprintf(gyr,"\nTotal Angular rotations: Pitch - %f Roll - %f Yaw - %f\n", totalPitch, totalRoll, totalYaw);



    fclose(gyr);
    fclose(acc);
    free(time);
    free(accX);
    free(accY);
    free(accZ);
    free(pitch);
    free(roll);
    free(yaw);

}


float variance(int count, float *data, int i) {
    int j, n = 0;
    float avg = 0, var = 0, sum = 0, sum1 = 0;

    if((i + 10) > count) {
        if((i - 10) > 0) {
            n = i - 10;
        }
        else {
            n = 10 - i;
        }
    }
    else {
        n = 10;
    }

    for(j = i; j < (i + n); j++) {
        sum += data[j];
    }

    avg = sum / (float)n;

    for(j = i; j < (i + n); j++) {
        sum1 += pow((data[j] - avg), 2);
    }

    var = sum1 / (float)n;

    return var;
}