#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <math.h>




#define WIDTH 200
#define HEIGHT 200
#define FRAME_TARGET 20

#define SOURCE_LENGTH 26
#define ITERATIONS 10000
#define SKIPP_START 2
#define CHUNK_LENGTH 20
//#define PRINT_CONTENT



char* fillWithChar(char* buffer, int length, int offset, const char* character);
double getCurrentTime();
char* heapString(char constString[]);

char source[] = "abcdefghijklmnopqrstuvwxyz";

typedef struct {
    double results[ITERATIONS][SOURCE_LENGTH];
    double averages[SOURCE_LENGTH];
    double variance[SOURCE_LENGTH];
    double totalAverage;
    double standardDeviation;
    double zScore[ITERATIONS][SOURCE_LENGTH];
    int highestCharIndex;
    bool skippedIndices[SOURCE_LENGTH];
} Stats;


int main() {
    char* password = (char*) malloc((CHUNK_LENGTH + 1) * sizeof(char*));
    password[CHUNK_LENGTH] = '\0';

    fillWithChar(password, CHUNK_LENGTH , 0, "u");
    fillWithChar(password, CHUNK_LENGTH , CHUNK_LENGTH / 2, "k");
    //password[passwordLength - 1] = 'a';
    //password = heapString("");

    printf("Password\n%s\n",  password);
    // Attacking them side channels
    Stats stats;
    printf("Attacking them side channels\n");
    for (int i = 0; i < ITERATIONS; i++) {
        for (int  j = 0; j < SOURCE_LENGTH; j++) {
            char* guess = (char*) malloc((CHUNK_LENGTH + 1) * sizeof(char*));
            guess[CHUNK_LENGTH] = '\0';
            fillWithChar(guess,CHUNK_LENGTH,0,&source[j]);
            double startTime = getCurrentTime();
            if(strcmp(password, guess) == 0){
                (void)0;
            }else{
                (void)0;
            }
            stats.results[i][j] = getCurrentTime() - startTime;
        }
    }

    // Compute averages
    double totalSum = 0;
    stats.highestCharIndex = 0;
    for (int i = 0; i < SOURCE_LENGTH; i++) {
        double sum = 0;
        for (int  j = SKIPP_START; j < ITERATIONS; j++) {
            double num = stats.results[j][i];
            sum += num;
        }
        float avg = (float) sum / (float) (ITERATIONS - SKIPP_START);
        stats.averages[i] = avg;
        if(avg > stats.averages[stats.highestCharIndex]){
            stats.highestCharIndex = i;
        }
        totalSum += sum;
    }
    stats.totalAverage =  (float) totalSum / (float) (SOURCE_LENGTH * (ITERATIONS - SKIPP_START));

    // Compute variance and standard div
    for (int charIndex = 0; charIndex < SOURCE_LENGTH; charIndex++) {
        double sumSquares = 0;
        for (int iterIndex = 0; iterIndex < ITERATIONS; iterIndex++) {
            sumSquares += pow(stats.results[iterIndex][charIndex] - stats.averages[charIndex], 2);
        }

        stats.variance[charIndex] = sqrt(sumSquares / (ITERATIONS - SKIPP_START));
        for (int iterIndex = 0; iterIndex < ITERATIONS; iterIndex++) {
            stats.zScore[iterIndex][charIndex] = (stats.results[iterIndex][charIndex] - stats.averages[charIndex]) / stats.variance[charIndex];
        }

    }

    //Print results
    for (int i = 0; i < SOURCE_LENGTH; i++) {
        if(stats.highestCharIndex == i){
            printf("\33[31m");
        }

        printf("%c - Avg: %.2f  Variance: %.2f ", source[i],stats.averages[i], stats.variance[i]);
        #ifdef PRINT_CONTENT
        printf("\t[");
        for (int  j = SKIPP_START; j < ITERATIONS; j++) {
            //printf("%.4f ", stats.results[j][i]);
//            continue;
            double value = stats.zScore[j][i];
            if(fabs(value) < 1 || stats.highestCharIndex == i){
                printf("%.2f ", value);
            }else{
                printf("\33[31m%.2f\33[0m ", value);
            }
        }
        printf("]");
        #endif
        printf("\33[0m\n");
    }
    printf("Total Average: %.2f\nBest guess: %c", stats.totalAverage, source[stats.highestCharIndex]);




    free(password);
    return 0;
    // Raylib
    SetTraceLogLevel(LOG_FATAL);
    InitWindow(WIDTH, HEIGHT, "Side Channel attack");
    srand(time(NULL));
    SetTargetFPS(FRAME_TARGET);


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

char* heapString(char constString[]){
    char* string = (char*) malloc(strlen(constString) * sizeof(char**));
    strcpy(string, constString);
    return string;
}

char* fillWithChar(char* buffer, int length, int offset, const char* character){
    while(offset  < length ){
        *(buffer + offset++) = *character;
    }
    return buffer;
}


double getCurrentTime(){
    struct timespec time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
    return (double) time.tv_sec * (long)1e9 + time.tv_nsec;
}
