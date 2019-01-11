# OperatingSystemsProject

Two Projects that have been developed as per the curriculum of University of Texas at Dallas Operating Systems subject. 
The 2 projects are  -

1. Matrix Multiplication
2. Redesigning the V6 file system.

######################### Matrix Multiplication Description ###############################

In this project, you will write two programs that solve the same problem:
SingleThread.c is single threaded and MultiThread.c is multi-threaded. We would like to know the elapsed time (on the computer) for each of the two programs to run.
We have two very large matrices A and B. Both A and B are 10000 by 10000 matrices and we would like to find the product of the two matrices C. 
Thus C = A x B. Note that C[i, j] = sum(A[i, k]*B[k,j]) for all k from 1 to 10000.

In a real life scenario, these two matrices are built by an elaborate method that takes a lot of time. 
In your programs, you will simulate this by introducing a delay of 10 microseconds between computing the value of one element of array C 
and computing the value of the next element of array C. You will introduce this delay in both SingleThread.c and MultiThread.c

SingleThread.c will solve the matrix multiplication program using a single thread and MultiThread.c will solve the same problem using 25 threads 
with each thread computing 1/25th of the elements of matrix C.
In both cases, after computing one element of matrix C, have a delay of 10 microseconds.
Matrices A and B are generated using independent random numbers in the range 1 to 2000. Make sure that the result produced by both the programs are identical.
Measure the time taken by each of the two programs and report the result. It can be as simple as capturing the dialogue using script as shown below:

$ script output
$ date
$ SingleThread
$ date
$ MultiThread
$ date
$ exit


############################ Redesigning the V6 file system ##################################

