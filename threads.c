#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>

#define FIRST_FILE     1
#define SECOND_FILE    2
#define READ_FROM_FILE "r"
#define METHOD_1       1
#define METHOD_2       2
#define METHOD_3       3
#define WRITE_IN_FILE  "w"

int rowsA = 0;
int colsA = 0;
int rowsB = 0;
int colsB = 0;
long **matA = NULL;
long **matB = NULL;
long **matC_whole = NULL;
long **matC_perRow = NULL;
long **matC_perEle = NULL;


typedef struct myStruct
{
    int cuurentRow;
    int currentColoumn;
} matData;


void heapAllocator(void){
    matC_perEle = (long **)malloc(sizeof(long *) * rowsA);
    matC_perRow = (long **)malloc(sizeof(long*) * rowsA);
    matC_whole = (long **)malloc(sizeof(long *) * rowsA);
    for(int i = 0 ; i < rowsA ; i++){
        matC_perEle[i] = (long *)malloc(sizeof(long) * colsB);
        matC_perRow[i] = (long *)malloc(sizeof(long) * colsB);
        matC_whole[i] = (long *)malloc(sizeof(long) * colsB);
    }

}

void allocateMatrix(FILE* fileName , int fileNumber){
    if(fileNumber == FIRST_FILE){
        matA = (long **)malloc(rowsA * sizeof(long **));
        for(int i = 0 ; i < rowsA ; i++){
            matA[i] = (long *)malloc(colsA * sizeof(long *));
            for(int j = 0 ; j < colsA ; j++){
                fscanf(fileName , "%ld" , &matA[i][j]);
            }
            fgetc(fileName);
        }
    }
    else{
        matB = (long **)malloc(rowsB * sizeof(long **));
        for(int i = 0 ; i < rowsB ; i++){
            matB[i] = (long *)malloc(colsB * sizeof(long *));
            for(int j = 0 ; j < colsB ; j++){
                fscanf(fileName , "%ld" , &matB[i][j]);
            }
            fgetc(fileName);
        }
    }
}

void readFromFile(char *path, int fileNumber)
{
    FILE *f = fopen(path, READ_FROM_FILE);
    if(fileNumber == FIRST_FILE){
      fscanf(f , "row=%d col=%d",&rowsA , &colsA);
      allocateMatrix(f, fileNumber);
    }
    else{
      fscanf(f , "row=%d col=%d",&rowsB , &colsB);
      allocateMatrix(f, fileNumber);
    }
    fclose(f);
}

void writeInFile(char* fileName , int method){
    FILE * f = fopen(fileName , WRITE_IN_FILE);
    if(method == METHOD_1){
        /*print the matrix in method 1*/
        fprintf(f , "Method: A thread per matrix\nrow=%d col=%d\n",rowsA , colsB);
        for(int i = 0 ; i < rowsA ; i++){
            for(int j = 0 ; j < colsB ; j++){
                fprintf(f , "%ld  " , matC_whole[i][j]);
            }
            fprintf(f , "\n");
        }
    }
    else if(method == METHOD_2){
        /*print the matrix in method 2*/
        fprintf(f , "Method: A thread per row\nrow=%d col=%d\n",rowsA , colsB);
        for(int i = 0 ; i < rowsA ; i++){
            for(int j = 0 ; j < colsB ; j++){
                fprintf(f , "%ld  " , matC_perRow[i][j]);
            }
            fprintf(f , "\n");
        }
    }
    else if(method == METHOD_3){
        /*print the matrix in method 3*/
        fprintf(f , "Method: A thread per element\nrow=%d col=%d\n",rowsA , colsB);
        for(int i = 0 ; i < rowsA ; i++){
            for(int j = 0 ; j < colsB ; j++){
                fprintf(f , "%ld  " , matC_perEle[i][j]);
            }
            fprintf(f , "\n");
        }
    }
    else{
        perror("Error\n");
        exit(EXIT_FAILURE);
    }
    fclose(f);
}

void * mutrixMul(){
    long element = 0;
    for(int i = 0 ; i < rowsA ; i++){
        for(int j = 0 ; j < colsB ; j++){
          for(int k = 0 ; k < colsA ; k++){
            element+= matA[i][k] * matB[k][j];
          }
          matC_whole[i][j] = element;
          element = 0;
        }
    }
}

void * mutrixMulPerRow(void* args){
    long element = 0;
    int dynamicRow = *(int *)args;
    for(int i = 0 ; i < colsB ; i++){
        for(int j = 0 ; j < colsA ; j++){
            element+= matA[dynamicRow][j] * matB[j][i];
        }
        matC_perRow[dynamicRow][i] = element;
        element = 0;
    }
    free(args);
}

void * mutrixMulPerElement(void * args){
    long element = 0;
    matData dynamicmat = *(matData *)args;
    for(int i = 0 ; i < colsA ; i++){
        element+= matA[dynamicmat.cuurentRow][i] * matB[i][dynamicmat.currentColoumn];
    }
    matC_perEle[dynamicmat.cuurentRow][dynamicmat.currentColoumn] = element;
    free(args);
}


void runCase1(void){
    struct timeval stop , start;
    gettimeofday(&start , NULL);
    pthread_t threadPerWholeMatrix ;
    /*one thread for the whole matrix no arguments*/
    if(pthread_create(&threadPerWholeMatrix , NULL , &mutrixMul , NULL)!= 0){
        perror("Error creating thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(threadPerWholeMatrix , NULL);
    gettimeofday(&stop , NULL);
    printf("Thread per matrix taken in Micro Second %lu\n",stop.tv_usec - start.tv_usec);
    printf("Threads Created = 1\n");    
}

void runCase2(void){
    struct timeval stop , start ;
    gettimeofday(&start , NULL);
    pthread_t threadPerRow[rowsA];
    for(int i = 0 ; i < rowsA ; i++){
        int* arg = malloc(sizeof(int)*20);
        *arg = i;
        if(pthread_create(&threadPerRow[i] , NULL , &mutrixMulPerRow , arg) != 0){
            perror("Error creating thread\n");
            exit(EXIT_FAILURE);
        }
    }
    /*wait for all threads being created*/
    for(int i = 0 ; i < rowsA ; i++){
        pthread_join(threadPerRow[i] , NULL);
    }
    gettimeofday(&stop , NULL);
    printf("Thread per Row taken in Micro Second %lu\n",stop.tv_usec - start.tv_usec);
    printf("Threads Created = %d\n",rowsA);  
}

void runCase3(void){
    struct timeval stop , start;
    gettimeofday(&start , NULL);
    pthread_t threadPerElement[rowsA * colsB];
    int threadIndex = 0 ;
    for(int i = 0 ; i < rowsA ; i++){
        for(int j = 0 ; j < colsB ; j++){
            matData *args = malloc(sizeof(matData));
            args->cuurentRow = i;
            args->currentColoumn = j;
            if(pthread_create(&threadPerElement[threadIndex++] , NULL ,
             &mutrixMulPerElement , args)){
                perror("Error creating thread\n");
                exit(EXIT_FAILURE);
             }  
        }
    }
    for(int i = 0 ; i < rowsA *colsB; i++){
        pthread_join(threadPerElement[i], NULL);
    }
    gettimeofday(&stop , NULL);
    printf("Thread per Element taken in Micro Second %lu\n",stop.tv_usec - start.tv_usec);
    printf("Threads Created = %d\n",(rowsA * colsB));  
}

void inputHandler(int argc , char*argv[]){
    if(argc == 1){
        readFromFile("a.txt" , FIRST_FILE);
        readFromFile("b.txt" , SECOND_FILE);
    }
    else{
        char* arr1 = (char*)malloc(sizeof(char) * 20);
        strcpy(arr1 , argv[1]);
        strcat(arr1 , ".txt");
        readFromFile(arr1 , FIRST_FILE);
        char* arr2 = (char*)malloc(sizeof(char) * 20);
        strcpy(arr2 , argv[2]);
        strcat(arr2 , ".txt");
        readFromFile(arr2 , SECOND_FILE);
        free(arr1);
        free(arr2);
    }
    if(colsA != rowsB){
        printf("Error in dimensions\n");
        exit(EXIT_FAILURE);
    }
}


void outputHandler(int argc , char*argv[]){
    if(argc == 1){
        writeInFile("c_per_matrix.txt",METHOD_1);
        writeInFile("c_per_row.txt",METHOD_2);
        writeInFile("c_per_element.txt",METHOD_3);
    }
    else{
        char*matrix_1 = malloc(sizeof(char) * 20);
        char*matrix_2 = malloc(sizeof(char) * 20);
        char*matrix_3 = malloc(sizeof(char) * 20);
        strcpy(matrix_1 , argv[3]);
        strcat(matrix_1 , "_per_matrix.txt");
        writeInFile(matrix_1 , METHOD_1);
        free(matrix_1);
        strcpy(matrix_2 , argv[3]);
        strcat(matrix_2 , "_per_row.txt");
        writeInFile(matrix_2 , METHOD_2);
        free(matrix_2);
        strcpy(matrix_3 , argv[3]);
        strcat(matrix_3 , "_per_element.txt");
        writeInFile(matrix_3 , METHOD_3);
        free(matrix_3);
    }
}

void liteGarbageCollector(void){
    /*free pointers that take place in heap*/
    for(int i = 0 ; i < rowsA ; i++){
        free(matA[i]);
        free(matC_whole[i]);
        free(matC_perRow[i]);
        free(matC_perEle[i]);
    }
    for(int i = 0 ; i < rowsB ; i++) free(matB[i]);
    free(matA);
    free(matB);
    free(matC_whole);
    free(matC_perRow);
    free(matC_perEle);
}


int main(int argc , char*argv[]){
    inputHandler(argc , argv);
    heapAllocator();
    runCase1();
    runCase2();
    runCase3(); 
    outputHandler(argc , argv);
    liteGarbageCollector();
    return 0;
}


   
    
 