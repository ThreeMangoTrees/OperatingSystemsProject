#include <stdio.h>
#include <unistd.h>
#include<time.h>

#define SIZE 500

void multiply(int mat1[][SIZE], int mat2[][SIZE]) { 
    int i, j, k;
    FILE *stFile = fopen("outputSingleThread.txt", "w");
    int mult;
    for (i = 0; i < SIZE; i++) { 
        for (j = 0; j < SIZE; j++) { 
            mult = 0; 
            for (k = 0; k < SIZE; k++) { 
                mult += mat1[i][k] * mat2[k][j];
            }
            fprintf(stFile, "%d ", mult); 

            usleep(10);
        }
        fprintf(stFile, "\n");
    }
    fclose(stFile);
}

int main() {
    int i, j;
    FILE *f1, *f2;
    int m1[SIZE][SIZE];
    int m2[SIZE][SIZE];
    time_t begin, end;

    time(&begin);
    f1 = fopen("m1.txt", "r");
    f2 = fopen("m2.txt", "r");
    
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            fscanf(f1, "%d", &m1[i][j]);
            fscanf(f2, "%d", &m2[i][j]);
        }
    }
    
    multiply(m1, m2);
    time(&end);
    double difference = difftime(end, begin);
    //printf ("Time taken by prog1.c = %.2lf seconds.\n", difference );
    return 0;
}
