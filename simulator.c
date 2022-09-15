/* ************************************************* *
 * </ SOURCE /> ===== SIMULATOR.C ===== </ SOURCE /> *
 * ************************************************* */

/**
 * @file simulator.c
 * @author Alastair Kho Ying Thai (20214878)

 * @task: Operating Systems Assignment
 * @unit: COMP2006 Operating Systems
 * @institution: Curtin University
 * 
 * @brief Code file that contains the core code for running the thread simulator
 * @version 0.1
 * @date 2022-05-02
 * 
 */



/* ********************************************************* *
 * </ HEADER /> ===== HEADER DECLARATIONS ===== </ HEADER /> *
 * ********************************************************* */

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <string.h>

#include "scheduler.h"



/* ********************************************************************* *
 * </ GLOBALS /> ===== INITIALISING SHARED VARIABLES ===== </ GLOBALS /> *
 * ********************************************************************* */

/* # ===== GLOBAL SHARED VAR ===== # */

/* -- Flags -- */
int threadControl = PARENT; /* Flag that determines which thread holds control. PARENT 1, CHILD 0 */

/* -- Variables -- */
int turn = 0; /* Integer variable that expresses the turn of schedulers, or program quit. 
QUIT (-1), FCFS (0), SSTF (1), SCAN (2), C-SCAN (3), LOOK (4), C-LOOK (5) */

/* -- Scheduler Reference -- */

/* Array of function pointers to scheduler functions that return integer seek times. 
This array serves as the order in which each function's corresponding thread is executed. */
int ( * schedulingAlgorithms[6] ) ( DiskSectorRequest * pDiskRequests ) = { firstComeFirstServe, 
                                                                            shortestSeekTimeFirst, 
                                                                            scan, 
                                                                            cscan, 
                                                                            look, 
                                                                            clook };

/* Array of function names */
char * schedulingNames[6] = { "FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK" };


/* # ===== BUFFERS ===== # */

/* -- Buffer 1 -- */
int * buffer1 = NULL; /* Integer array variable that holds the read contents from an input file */
int buffer1Size; /* Integer variable that holds the size of buffer1 n+3 */

/* -- Buffer 2 -- */
int buffer2; /* Integer variable that holds the computed seek time */


/* # ===== MUTEX AND COND VAR INIT ===== # */

/* -- Mutex -- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* pthread mutex lock */

/* -- Condition Variables -- */
pthread_cond_t childCondThreadA = PTHREAD_COND_INITIALIZER, /* FCFS pthread condition variable */
               childCondThreadB = PTHREAD_COND_INITIALIZER, /* SSTF pthread condition variable */
               childCondThreadC = PTHREAD_COND_INITIALIZER, /* SCAN pthread condition variable */
               childCondThreadD = PTHREAD_COND_INITIALIZER, /* C-SCAN pthread condition variable */
               childCondThreadE = PTHREAD_COND_INITIALIZER, /* LOOK pthread condition variable */
               childCondThreadF = PTHREAD_COND_INITIALIZER; /* C-LOOK pthread condition variable */
pthread_cond_t parentCond = PTHREAD_COND_INITIALIZER; /* Parent pthread condition variable */

/* pthread_cond_t array that holds addresses of individual conditional variables of child threads */
pthread_cond_t childConditionArray[6];



/* ****************************************************** *
 * </ METHODS /> ===== THREAD METHODS ===== </ METHODS /> *
 * ****************************************************** */

/* # ===== CHILD THREAD ===== # */

/**
 * @name: childThreadFunc
 * @brief: Function for child thread. Child thread relies on parent thread for spawning.
 * @param pScheduledTurn Expects a pointer to a malloc'ed integer that represents the child's scheduled 
 *                       position in thread execution.
 * @assertion: Parent thread exists. Each thread is assigned a unique number from 0 to 5 and is accesed 
 *             via the parameter' pScheduledTurn'
 * @return: void* Returns NULL
 */
void * childThreadFunc( void* pScheduleNumber ) {

    /* ----------------------------------------------- *
     * </ BEGIN /> ---- START THREAD  ---- </ BEGIN /> *
     * ----------------------------------------------- */

    /* # ---- INITIALISING VARIABLES ---- # */

    int scheduledTurn = *( ( int * ) pScheduleNumber ); /* Integer variable that holds the thread's order of 
    execution and corresponding function in global function pointer array 'schedulingAlgorithms' */
    
    DiskSectorRequest *pDiskRequests = ( DiskSectorRequest* ) malloc( sizeof( DiskSectorRequest ) ); 
    /* Struct DiskSectorRequest that is passed to a function in schedulingAlgorithms to calculate seek time */

    /* -------------------------------------------------- *
     * </ BEGIN /> ---- CRITICAL SECTION ---- </ BEGIN /> *
     * -------------------------------------------------- */

    pthread_mutex_lock( &mutex );

    /* # ---- THREAD CORE ---- # */

    while ( turn != QUIT ) { /* While turn flag is not set to QUIT (-1) by parent */
        while ( ( turn != scheduledTurn || threadControl == PARENT ) && turn != QUIT ) { /* While not thread's turn 
        or turn belongs to the PARENT, and turn is not QUIT */
            /* Block child thread until signalled from parent thread*/
            pthread_cond_wait( &childConditionArray[scheduledTurn], &mutex );      
            /* Child thread is signalled by parent, assert that threadControl is now of value CHILD (0) and thread 
            exits blocked and runs */
        }
        
        /* # -- Child Thread Running -- # */

        if ( turn == scheduledTurn ) { /* Additional checking, if child thread is signalled and turn is QUIT (-1), 
        this condition prevents below from running and directs child thread to QUIT */

            /* # -- Buffer 1 Read -- # */

            pDiskRequests -> requestCount = buffer1Size - 3;
            pDiskRequests -> totalCylinders = buffer1[0];
            pDiskRequests -> currentPosition = buffer1[1];
            pDiskRequests -> previousDiskRequest = buffer1[2];
            pDiskRequests -> diskRequestArray = &buffer1[3]; /* Index = 3 is when the disk request array begins 
            in buffer1, this assignment points to address of buffer 1 at index 3. */

            /* # -- Buffer 2 Write -- # */

            buffer2 = ( *schedulingAlgorithms[scheduledTurn] )( pDiskRequests ); /* Run function from function
            pointer array based on assigned number from 'scheduledTurn' */
            
            /* # -- Return To Parent Thread -- # */

            pDiskRequests -> diskRequestArray = NULL; /* Assign array to NULL */
            turn += 1;  /* Increment turn to indicate next thread's turn */

            threadControl = PARENT; /* pass control back to PARENT */
            pthread_cond_signal( &parentCond ); /* Signal parent thread*/

        }
        
    }
    
    /* # ---- QUIT THREAD ---- # */

    pthread_mutex_unlock( &mutex );
    
    /* ---------------------------------------------- *
     * </ END /> ---- CRITICAL SECTION ---- </ END /> *
     * ---------------------------------------------- */

    /* # ---- FREE ASSIGNED MEMORY ---- # */

    free(pDiskRequests); 
    pDiskRequests = NULL;

    free(pScheduleNumber);
    pScheduleNumber = NULL;
    printf( "Thread_%lu has terminated.\n", pthread_self() );

    /* ------------------------------------------ *
     * </ END /> ---- EXIT THREAD  ---- </ END /> *
     * ------------------------------------------ */

    return NULL;
}


/* # ===== PARENT THREAD ===== # */

/**
 * @name parentThreadFunc
 * @brief Function for parent thread. Parent thread creates all child threads.
 * 
 * @return void* 
 */
void * parentThreadFunc() {

    /* ----------------------------------------------- *
     * </ BEGIN /> ---- START THREAD  ---- </ BEGIN /> *
     * ----------------------------------------------- */

    /* # ---- INITIALISING VARIABLES ---- # */

    int icount = 0, /* Integer variable for iterator */
        scannedNum = 0; /* Integer variable to hold scanf value */ 

    char inputRequest[10]; /* Variable to hold input request, assert that input request is always less than
    10 characters in length and will not have buffer overflows. */

    FILE *inputFile; /* Struct FILE to reference to input file */

    pthread_t threadA, threadB, threadC, threadD, threadE, threadF; /* Thread reference for each scheduler */
    pthread_t *threadArray[6]; /* Thread address array to hold the addresses of each thread */

    /* Assigning each thread address to corresponding position in threadArray. C89 pedantic requires this as 
    initializing address to the array is not computable at load time. */
    threadArray[0] = &threadA; threadArray[1] = &threadB; threadArray[2] = &threadC;
    threadArray[3] = &threadD; threadArray[4] = &threadE; threadArray[5] = &threadF;

    /* -------------------------------------------------- *
     * </ BEGIN /> ---- CRITICAL SECTION ---- </ BEGIN /> *
     * -------------------------------------------------- */
    
    pthread_mutex_lock( &mutex );

    /* # ---- THREAD CREATION ---- # */

    for ( icount = 0 ; icount < 6 ; icount ++ ) {
        int *arg = malloc( sizeof( *arg ) );
        *arg = icount; /* Create a pointer to a malloc'ed value */
        
        /* Create thread and assign value to function argument */
        if ( pthread_create( threadArray[icount], NULL, &childThreadFunc, arg ) != 0 ) {
            perror( "Failed to create thread" );
        }
    }

    /* # ---- THREAD CORE ---- # */

    printf( "Disk Scheduler Simulation: " );
    scanf( "%s", inputRequest ); /* Scan user input, expects a file name or "QUIT" */

    while( strcmp( inputRequest, "QUIT" ) != 0 ) { /* While not "QUIT" */

        /* -- Read File -- */

        printf( "\nFor %s: \n", inputRequest );

        inputFile = fopen( inputRequest, "r" ); /* Attempt to open the file */

        if ( inputFile == NULL ) {
            perror( "Error, could not open file" ); /* Catch error in opening file */
        }

        else {

            int totalCylinders = 0; /* Integer variable to hold total number of read cylinders from file input */
            int readCount = 0; /* Integer variable to hold amount of numbers in input file */ 

            /* -- Bad File Content Scan -- */

            /* Checks if total cylinders value is detected and valid */
            if ( fscanf( inputFile, "%d", &totalCylinders ) == 1 && totalCylinders > 0 ) { 
                readCount += 1; /* This will not increment if no number is scanned and will be caught later, i.e. empty file and fscanf() == EOF */
                
                /* Increment readCount if total cylinders was valid, and next few numbers are also valid */
                while ( fscanf( inputFile, "%d", &scannedNum ) != EOF && scannedNum >= 0 && scannedNum < totalCylinders ) {
                    /* Disk requests must not be negative or greater than totalCylinders */
                    readCount += 1; /* This will not increment if a bad number is detected and or it is the end of file */
                }
            }
            
            /* -- Bad File Content Catch -- */

            /* Catch and output to stderr if a scanned value was less than 0 or file is empty  */
            if ( ( scannedNum < 0 ) || ( readCount == 0 )) { 
                fprintf(stderr, "Invalid File Read: File %s is empty or contains invalid values.\n", inputRequest ); 
            }
            
            /* Catch and output to stderr if a scanned value was greater than total cylinders  */
            else if ( scannedNum >= totalCylinders ) {
                fprintf(stderr, "Invalid File Read: File %s contains invalid disk request, < %d > out of bounds. Max set to < %d >\n", inputRequest, scannedNum, totalCylinders - 1 ); 
            }

            /* -- Core -- */

            else {

                /* Initialising buffer1 and result array */
                int * seekTimeResults = ( int * ) malloc( sizeof( int ) * ( 6 ) );
                buffer1 = ( int * ) malloc( sizeof( int ) * ( readCount ) );
                buffer1Size = readCount;
                /* Set file position to the begining of the file */
                rewind( inputFile );    

                /* Read input file and store in buffer */
                for ( icount = 0 ; icount < readCount ; icount++ ) {
                    fscanf( inputFile, "%d", &scannedNum );
                    buffer1[icount] = scannedNum;
                }

                /* Execute threads in order - FCFS (0), SSTF (1), SCAN (2), C-SCAN (3), LOOK (4), C-LOOK (5) */
                for ( icount = 0 ; icount < 6 ; icount ++ ) {  
                    
                    int nextTurn = turn + 1; /* Integer variable used to see if active child thread has incremented this value */
                    threadControl = CHILD; /* Hand control over to child thread */
                    pthread_cond_signal( &childConditionArray[icount] ); /* Signal corresponding thread in order */

                    /* Parent thread is blocked until turn is incremented or threadControl is passed back to parent */
                    while ( turn != nextTurn || threadControl != PARENT ) {
                        /* Block parent thread until signalled from child thread*/
                        pthread_cond_wait( &parentCond, &mutex );
                    }
                    /* Assert that threadControl == PARENT and turn == nextTurn */
                    seekTimeResults[icount] = buffer2; /* write buffer2 result to seekTimeResults array */
                }
                                
                /* -- Output Results -- */

                for ( icount = 0 ; icount < 6 ; icount++ ) {
                    printf( "%s: %d.\n", schedulingNames[icount], seekTimeResults[icount] );
                }
            
                /* -- Free Assigned Memory and Clean Up -- */

                turn = 0;
                
                /* Free seek time array */
                free( seekTimeResults );
                seekTimeResults = NULL;

                /* Free and reset buffer1 */
                free ( buffer1 );
                buffer1 = NULL;
            }
            
            fclose( inputFile ); /* Close input file */
        }

        printf( "\n\nDisk Scheduler Simulation: " );
        scanf( "%s", inputRequest ); /* Scan user input again, expects a file name or "QUIT" */
    }

    /* -- Call Quit -- */

    /* Assert that inputRequest == "QUIT" and CORE while loop was exited */

    turn = QUIT; /* Set turn variable to QUIT, used to signal to child threads to QUIT */

    for ( icount = 0 ; icount < 6 ; icount ++ ) {   
        /* Signal all child threads which will observe turn variable as QUIT, and end itself accordingly */     
        pthread_cond_signal( &childConditionArray[icount] );
    }

    pthread_mutex_unlock( &mutex );

    /* ---------------------------------------------- *
     * </ END /> ---- CRITICAL SECTION ---- </ END /> *
     * ---------------------------------------------- */

    /* # ---- THREAD JOIN ---- # */

    for ( icount = 0 ; icount < 6 ; icount ++ ) {        
         if ( pthread_join( *threadArray[icount], NULL ) != 0 ) {
            perror( "Failed to join thread" );
        }
    }

    /* ------------------------------------------ *
     * </ END /> ---- EXIT THREAD  ---- </ END /> *
     * ------------------------------------------ */

    pthread_exit(NULL);
}



/* ********************************************** *
 * </ MAIN /> ===== MAIN PROGRAM ===== </ MAIN /> *
 * ********************************************** */

/**
 * @brief Beginning of program
 * 
 * @param argc 
 * @param argv 
 * @return exit status 
 */
int main( int argc, char *argv[] ) {
    
    /* ------------------------------------------------ *
     * </ BEGIN /> ---- PROGRAM START  ---- </ BEGIN /> *
     * ------------------------------------------------ */

    /* # ---- INITIALISING VARIABLES ---- # */
    int icount; /* Integer variable for iterator */
    pthread_t parentThread; /* Thread reference for parent thread creation */
    
    /* Assigning each child thread conditional variable to corresponding position in childConditionArray. 
    C89 pedantic requires this as initializing variable to the array is not computable at load time. */
    childConditionArray[0] = childCondThreadA; childConditionArray[1] = childCondThreadB; 
    childConditionArray[2] = childCondThreadC; childConditionArray[3] = childCondThreadD; 
    childConditionArray[4] = childCondThreadE; childConditionArray[5] = childCondThreadF;

    /* # ---- THREAD CORE ---- # */

    /* -- Create Parent Thread -- */

    if ( pthread_create( &parentThread, NULL, &parentThreadFunc, NULL ) != 0 ) {
        perror( "Failed to create thread" );
    }

    /* -- Join Parent Thread -- */

    /* Program waits until parentThread ends */
    if ( pthread_join( parentThread, NULL ) != 0 ) {
        perror( "Failed to join thread" );
    }


    /* # ---- DESTROY LOCK AND COND VAR ---- # */

    pthread_mutex_destroy( &mutex );

    for ( icount = 0; icount < 6 ; icount++ ) {
        pthread_cond_destroy( &childConditionArray[icount] );
    }

    pthread_cond_destroy( &parentCond );

    /* --------------------------------------------- *
     * </ END /> ---- END OF PROGRAM  ---- </ END /> *
     * --------------------------------------------- */

    return 0;
}

