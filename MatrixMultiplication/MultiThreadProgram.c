#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 500
#define MAX_THREAD 25

struct threadStruct {
    int start;
    int end;
    int threadId;
    int *mat1;
    int *ma2;
    int *mat3;
};

void multiply(void* arg) { 
    int i, j, k;
    struct threadStruct* val = (struct threadStruct*) arg;
    int *m1 = val->mat1;
    int *m2 = val->ma2;
    int *m3 = val->mat3;
    int start = val->start;
    int end = val->end;

    for (i = start; i < end; i++) { 
        for (j = 0; j < SIZE; j++) { 
            *(m3 + i*SIZE + j) = 0; 
            for (k = 0; k < SIZE; k++) { 
                 *(m3 + i*SIZE + j) += *(m1 + i*SIZE + k) * *(m2 + k*SIZE + j);
            }
            usleep(10);
        }
    }
}

int main() {
    int i, j, stepSize;
    pthread_t tid[MAX_THREAD];
    int m1[SIZE][SIZE];
    int m2[SIZE][SIZE];
    int m3[SIZE][SIZE];
    struct threadStruct *val[25];
    FILE *f1, *f2;
    time_t begin, end;
    time(&begin);
    stepSize = SIZE / MAX_THREAD;

    f1 = fopen("m1.txt", "r");
    f2 = fopen("m2.txt", "r");

         
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            fscanf(f1, "%d", &m1[i][j]);
            fscanf(f2, "%d", &m2[i][j]);
        }
    }
    
    for( i = 0; i < MAX_THREAD; i++) {
        val[i] = (struct threadStruct*) malloc(sizeof(struct threadStruct));
        val[i]->start = i * stepSize;
        val[i]->end = val[i]->start + stepSize;
        val[i]->threadId = i;
        val[i]->mat1 = &m1[0][0];
        val[i]->ma2 = &m2[0][0];
        val[i]->mat3 = &m3[0][0];
        pthread_create(&tid[i], NULL, multiply, (void *) val[i]);
    }

    for(i = 0; i < MAX_THREAD; i++) {
        pthread_join(tid[i], NULL);
    }

    FILE *file = fopen("outputMultiThread.txt", "w");

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            fprintf(file, "%d ", m3[i][j]); 
        }
        fprintf(file, "\n");
    }

    fclose(file);

    time(&end);
    double difference = difftime(end, begin);
    return 0;
}
