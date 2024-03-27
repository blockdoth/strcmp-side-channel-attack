#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <math.h>




#define WIDTH 1000
#define HEIGHT 600
#define MARGIN 20
#define FRAME_TARGET 30
#define FONT_SIZE 20

#define SOURCE_LENGTH 26
#define ITERATIONS 10000
#define SKIPP_START 2
#define BLOCKS 1
#define BLOCK_SIZE 50
//#define PRINT_CONTENT
#define CHAR_OFFSET 12
#define ITERATIONS_PER_ATTACK 200

#define NAMING_THINGS_IS_HARD 1000
#define SCALE(value, scalar) (value / NAMING_THINGS_IS_HARD) * scalar

char* fillWithChar(char* buffer, int length, int offset, const char* character);
double getCurrentTime();
char* heapString(char constString[]);

char source[] = "abcdefghijklmnopqrstuvwxyz";

typedef struct {
    double results[ITERATIONS][SOURCE_LENGTH];
    int iteration;
    double averages[SOURCE_LENGTH];
    double variance[SOURCE_LENGTH];
    double totalAverage;
    double standardDeviation;
    double zScore[ITERATIONS][SOURCE_LENGTH];
    int highestCharIndex;
    bool skippedIndices[SOURCE_LENGTH];
} Stats;


Stats* iterateAttack(Stats* stats, char* password){
    char* guess = (char*) malloc((BLOCK_SIZE + 1) * sizeof(char*));
    guess[BLOCK_SIZE] = '\0';
    for (int x = 0; x < ITERATIONS_PER_ATTACK; ++x) {
        for (int  i = 0; i < SOURCE_LENGTH; i++) {
            fillWithChar(guess,BLOCK_SIZE,0,&source[i]);
            double startTime = getCurrentTime();
            if(strcmp(password, guess) == 0){
                (void)0;
            }else{
                (void)0;
            }
            double diffTime = getCurrentTime() - startTime;
            stats->results[stats->iteration][i] = diffTime;
//            if(i == 0){
//                printf("%.2f ", diffTime);
//            }

        }
    }

    for (int  i = 0; i < SOURCE_LENGTH; i++) {
        double rowSum = 0;
        for (int j = 0; j < stats->iteration; ++j) {
            rowSum += stats->results[j][i];
        }
        double avg = rowSum / stats->iteration;
        stats->averages[i] = avg;
        if(avg > stats->averages[stats->highestCharIndex]){
            stats->highestCharIndex = i;
        }
    }
    stats->iteration++;
    free(guess);


    return stats;
}

int main() {

//    // Compute variance and standard div
//    for (int charIndex = 0; charIndex < SOURCE_LENGTH; charIndex++) {
//        double sumSquares = 0;
//        for (int iterIndex = 0; iterIndex < ITERATIONS; iterIndex++) {
//            sumSquares += pow(stats.results[iterIndex][charIndex] - stats.averages[charIndex], 2);
//        }
//
//        stats.variance[charIndex] = sqrt(sumSquares / (ITERATIONS - SKIPP_START));
//        for (int iterIndex = 0; iterIndex < ITERATIONS; iterIndex++) {
//            stats.zScore[iterIndex][charIndex] = (stats.results[iterIndex][charIndex] - stats.averages[charIndex]) / stats.variance[charIndex];
//        }
//
//    }
    // Raylib
    SetTraceLogLevel(LOG_FATAL);
    InitWindow(WIDTH, HEIGHT, "Side Channel attack");
    srand(time(NULL));
    SetTargetFPS(FRAME_TARGET);


    int passwordLength = BLOCKS * BLOCK_SIZE;
    char* password = (char*) malloc(passwordLength * sizeof(char*));
    password[passwordLength] = '\0';

    for (int i = 0; i < BLOCKS; ++i) {
        fillWithChar(password, passwordLength , i * BLOCK_SIZE, &source[(i + CHAR_OFFSET) % SOURCE_LENGTH]);
    }


    int spacing = (WIDTH) / SOURCE_LENGTH;
    Stats* stats = (Stats*) malloc(sizeof(Stats));
    stats->iteration = 0;

    int startLine = HEIGHT - 2 * MARGIN;
    int endLine = 3 * MARGIN;
    int graphSize = startLine - endLine;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (IsKeyDown(KEY_BACKSPACE)) {
            stats->iteration = 0;
        }

        if (IsKeyDown(KEY_SPACE)) {
            if(stats->iteration < ITERATIONS){
                stats = iterateAttack(stats, password);
            }
        }
        DrawText(TextFormat("Password: %s", password), MARGIN,  MARGIN, FONT_SIZE, BLACK);
        DrawText(TextFormat("Iteration: %d", stats->iteration), WIDTH - 8 * MARGIN,  MARGIN, FONT_SIZE, BLACK);

        for (int i = 0; i < SOURCE_LENGTH; i++) {
            int xPos = MARGIN + spacing * i;
            if(stats->iteration > 0){
                for (int j = 0; j < stats->iteration; ++j) {
                    double value = stats->results[j][i];
                    int yPos = startLine - SCALE(value, graphSize);
                    if(yPos > startLine ){
                        yPos = startLine;
                    }
                    //int jitter = (i*i*j*j % 10) - 5;
                    DrawCircle(xPos , yPos, 4.0f, BLUE);
                }
                DrawCircle(xPos , startLine - SCALE(stats->averages[i], graphSize), 6.0f, RED);
                if(stats->highestCharIndex == i){
                    DrawRectangle(xPos - (FONT_SIZE / 2.0f), startLine + 5, 22, 22, RED);
                }
            }
            DrawLine(xPos,startLine ,xPos , endLine,GRAY);
            DrawText(TextFormat("%c", source[i]), xPos - FONT_SIZE / 4, startLine + 5, FONT_SIZE, BLACK);

        }

        EndDrawing();
    }
    free(stats);
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
