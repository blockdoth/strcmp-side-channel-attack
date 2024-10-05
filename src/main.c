#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 600
#define MARGIN 20
#define FRAME_TARGET 30
#define FONT_SIZE 20
#define SOURCE_LENGTH 26
#define MAX_ITERATIONS 10000
#define SKIPP_START 5
#define BLOCKS 1
#define BLOCK_SIZE 50
#define ITERATIONS_PER_PRESS 50
#define ZSCORE_THRESHOLD 3.0f

#define SCALAR 1200
#define SCALE(value, scalar) (value / SCALAR) * scalar


char* fillWithChar(char* buffer, int length, int offset, const char* character);
long getCurrentTime();
char* heapString(char constString[]);
void generatePassword(char *password, int passwordLength, int charIndex);
char source[] = "abcdefghijklmnopqrstuvwxyz";

typedef struct {
    double results[MAX_ITERATIONS][SOURCE_LENGTH];
    int iteration;
    double averages[SOURCE_LENGTH];
    double variance[SOURCE_LENGTH];
    double totalAverage;
    double standardDeviation;
    double zScore[MAX_ITERATIONS][SOURCE_LENGTH];
    double bestGuess[MAX_ITERATIONS];
    int highestCharIndex;
    int skippedCount;
    double pValue;
    bool skippedIndices[MAX_ITERATIONS];
} Stats;

void resetStats(Stats* stats);


Stats* iterateAttack(Stats* stats, char* password, int passwordLength){

    char** strings = (char**) malloc(SOURCE_LENGTH * sizeof(char*));
    for (int i = 0; i < SOURCE_LENGTH; ++i) {
        char* character = (char*)malloc(passwordLength + 1);
        fillWithChar(character,passwordLength,0,&source[i]);
        character[passwordLength] = '\0';
        strings[i] = character;
    }

    for (int x = 0; x < ITERATIONS_PER_PRESS + SKIPP_START; x++) {
        for (int  i = 0; i < SOURCE_LENGTH; i++) {
            long startTime = getCurrentTime();
            if(strcmp(password, strings[i]) == 0){
                (void)0;
            }else{
                (void)0;
            }
            long endTime = getCurrentTime();
            long diffTime = endTime - startTime;
            stats->results[stats->iteration][i] = diffTime;
        }
        if(x > SKIPP_START){
            stats->iteration++;
        }
    }

    int iterationsCount = stats->iteration;
    double totalSumAvg = 0;
    double totalSumVariance = 0;
    for (int  i = 0; i < SOURCE_LENGTH; i++) {
        double rowSum = 0;
        double sumSquares = 0;

        // Calculate avg for each char
        for (int j = 0; j < iterationsCount; ++j) {
            if(!stats->skippedIndices[j]) {
                rowSum += stats->results[j][i];
            }
        }
        double avg = rowSum / (iterationsCount - stats->skippedCount);
        // Calculate sum of squares

        for (int j = 0; j < iterationsCount; j++) {
            if(!stats->skippedIndices[j]) {
                sumSquares += pow(stats->averages[j] - avg, 2);
            }
        }
        //Calculate variance
        double sumSquared = sumSquares / (iterationsCount - stats->skippedCount - 1);

        //Calculate zScore
        for (int j = 0; j < iterationsCount; j++) {
            if(!stats->skippedIndices[j]){
                stats->skippedIndices[j] = false;
                double zScore = (stats->results[j][i] - avg) / sqrt(sumSquared);
                if(zScore > ZSCORE_THRESHOLD){
                    stats->skippedIndices[j] = true;
                    stats->skippedCount++;
                }
                stats->zScore[j][i] = zScore;
            }
        }

        if(avg > stats->averages[stats->highestCharIndex]){
            stats->highestCharIndex = i;
        }
        stats->averages[i] = avg;
        stats->variance[i] = sqrt(sumSquared);
        totalSumVariance += sqrt(sumSquared);
        totalSumAvg += avg;
        free(strings[i]);
    }
    double meanMean = (double) totalSumAvg / (stats->iteration - stats->skippedCount);
    double bigboySumSquared = 0;
    for (int i = 0; i < SOURCE_LENGTH; i++) {
        bigboySumSquared += pow(meanMean - stats->averages[i],2);
    }

    double bigboyZscore = (stats->averages[stats->highestCharIndex] - stats->totalAverage) / sqrt(bigboySumSquared);


    double pValue = 2 * (1 - 0.5 * (1 + erf(bigboyZscore / sqrt(2))));
    stats->pValue = pValue;
    return stats;
}

int main(){

    // Raylib
    SetTraceLogLevel(LOG_FATAL);
//    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WIDTH , HEIGHT - 35, "Side Channel attack");
    srand(time(NULL));
    SetTargetFPS(FRAME_TARGET);

    int charIndex = rand() % SOURCE_LENGTH;
    int passwordLength = BLOCKS * BLOCK_SIZE;
    char* password = (char*) malloc(passwordLength);
    generatePassword(password, passwordLength, 0);

    // Infra
    int spacing = (WIDTH) / SOURCE_LENGTH;
    Stats* stats = (Stats*) malloc(sizeof(Stats));
    stats->iteration = 0;

    int startLine = HEIGHT - 4 * MARGIN;
    int endLine = 3 * MARGIN;
    int bottomRow = startLine + 2 * MARGIN;
    int graphSize = startLine - endLine;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if(IsKeyPressed(KEY_UP)){
            resetStats(stats);
            generatePassword(password, passwordLength, ++charIndex);
        }
        if(IsKeyPressed(KEY_DOWN)){
            resetStats(stats);
            generatePassword(password, passwordLength, --charIndex);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            resetStats(stats);
            generatePassword(password, ++passwordLength, charIndex);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            generatePassword(password, --passwordLength, charIndex);
            resetStats(stats);
        }
        if (IsKeyDown(KEY_BACKSPACE)) {
            resetStats(stats);
        }
        if (IsKeyDown(KEY_SPACE)) {
            if(stats->iteration < MAX_ITERATIONS){
                stats = iterateAttack(stats, password, passwordLength);
            }
        }
        DrawText(TextFormat("Password: %s", password), MARGIN,  MARGIN, FONT_SIZE, BLACK);
        DrawText(TextFormat("Char count: %u", passwordLength), WIDTH - 8 * MARGIN,  MARGIN, FONT_SIZE, BLACK);
        DrawText(TextFormat("Iteration: %d", stats->iteration), MARGIN - FONT_SIZE / 4,  bottomRow, FONT_SIZE, BLACK);
        DrawText(TextFormat("Skipped: %d", stats->skippedCount), MARGIN - FONT_SIZE / 4 + 8 * MARGIN, bottomRow , FONT_SIZE, BLACK);
        DrawText(TextFormat("Average: %.2fns", stats->totalAverage), MARGIN - FONT_SIZE / 4 + 16 * MARGIN, bottomRow , FONT_SIZE, BLACK);
        DrawText(TextFormat("Highest: %.2fns", stats->averages[stats->highestCharIndex]), MARGIN - FONT_SIZE / 4 + 26 * MARGIN, bottomRow , FONT_SIZE, BLACK);
        DrawText(TextFormat("P-value: %.2f", stats->pValue), MARGIN - FONT_SIZE / 4 + 36 * MARGIN, bottomRow , FONT_SIZE, BLACK);
        DrawCircle(WIDTH , HEIGHT , 6.0f, RED);

        for (int i = 0; i < SOURCE_LENGTH; i++) {
            int xPos = MARGIN + spacing * i + FONT_SIZE / 4;
            if(stats->iteration > 0){

                for (int j = 0; j < stats->iteration; ++j) {
                    if(stats->skippedIndices[j]) continue;
                    double value = stats->results[j][i];
                    int yPos = startLine - SCALE(value, graphSize);
                    if(yPos > startLine ){
                        yPos = startLine;
                    }
                    //int jitter = (i*i*j*j % 10) - 5;
                    DrawCircle(xPos , yPos, 4.0f, BLUE);
                }
                DrawCircle(xPos , startLine - SCALE(stats->averages[i] , graphSize) , 6.0f, RED);
                if(stats->highestCharIndex == i){
                    DrawRectangle(xPos - (FONT_SIZE / 2.0f), startLine + 5, 22, 22, RED);
                }
            }
            DrawLine(xPos,startLine ,xPos, endLine,GRAY);
            DrawText(TextFormat("%c", source[i]), xPos - FONT_SIZE / 4, startLine + 5, FONT_SIZE, BLACK);
        }

        EndDrawing();
    }
    free(stats);
    CloseWindow();
    return 0;
}

void generatePassword(char *password, int passwordLength, int charIndex) {
    for (int i = 0; i < BLOCKS; ++i) {
        fillWithChar(password, passwordLength , i * BLOCK_SIZE, &source[charIndex % SOURCE_LENGTH]);
    }
    password[passwordLength] = '\0';
}

void resetStats(Stats* stats){
    memset(stats,0, sizeof(Stats));
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


long getCurrentTime(){
    struct timespec time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
    return time.tv_sec * (long)1e9 + time.tv_nsec;
}
