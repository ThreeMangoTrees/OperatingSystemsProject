#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 500

int main() {
    int i = 0, j = 0, rand_number, MAX_NUMBER = 2000, MIN_NUMBER = 1;
    time_t t;
    FILE *f1 = fopen("m1.txt", "w");
    FILE *f2 = fopen("m2.txt", "w");

    if (f1 == NULL || f2 == NULL) {
        printf("Error opening file!\n");
        return 0;
    }
    
    srand((unsigned) time(&t));
    
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            rand_number = rand() % (MAX_NUMBER + 1 - MIN_NUMBER) + MIN_NUMBER;
            fprintf(f1, "%d ", rand_number); 
            rand_number = rand() % (MAX_NUMBER + 1 - MIN_NUMBER) + MIN_NUMBER;
            fprintf(f2, "%d ", rand_number);
        }
        fprintf(f1, "\n");
        fprintf(f2, "\n");
    }

    fclose(f1);
    fclose(f2);
    return 0;
}
