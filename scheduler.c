/* ************************************************* *
 * </ SOURCE /> ===== SCHEDULER.C ===== </ SOURCE /> *
 * ************************************************* */

/**
 * @file scheduler.c
 * @author Alastair Kho Ying Thai (20214878)

 * @task: Operating Systems Assignment
 * @unit: COMP2006 Operating Systems
 * @institution: Curtin University
 * 
 * @brief Code file that contains the scheduler functions.
 * @version 0.1
 * @date 2022-05-02
 * 
 */



/* ********************************************************* *
 * </ HEADER /> ===== HEADER DECLARATIONS ===== </ HEADER /> *
 * ********************************************************* */

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>

#include "scheduler.h"


/* ********************************************************* *
 * </ METHODS /> ===== SCHEDULER METHODS ===== </ METHODS /> *
 * ********************************************************* */


/**
 * @brief FCFS scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int firstComeFirstServe( DiskSectorRequest* pDiskRequests ) {
    int icount = 0, seekTime = 0; /* iteration value and return value */
    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;


    for ( icount = 0 ; icount < numberOfRequests ; icount++ ) {  /* o(n) one pass to add */
        seekTime += abs( diskRequestArray[icount] - head );
        head = diskRequestArray[icount];
    }
    return seekTime;
}


/**
 * @brief SSTF scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int shortestSeekTimeFirst( DiskSectorRequest* pDiskRequests ) {
    int icount = 0, seekTime = 0; /* iteration value and return value */
    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;

    int *completedTaskArray = ( int* ) calloc( numberOfRequests, sizeof( int ) ); /* array to represent if request has been served */
    int completedCount = 0;

    while ( completedCount != numberOfRequests ) {
        int minDiff, minPosition;
        int firstPass = TRUE; /* used to check if iterator has found it's first value that has not been served */

        /* find the min */
        for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
            int diff = abs( head - diskRequestArray[icount] ); /* get difference */

            if ( completedTaskArray[icount] != 1 ) {/* if not already served */
                if ( firstPass == TRUE || diff < minDiff ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
                    minDiff = diff;
                    minPosition = icount;
                }

                firstPass = FALSE; /* set firstPass to false, so that the rest of the numbers only compare diff with minDiff. This is implemented so we don't need to
                initialise minDiff to some large number and compare min with that. Considers the case if the initiliased large min value is in fact the smallest value.*/
            }
        }

        /* after min has been found, execute and declare served */
        seekTime += minDiff;
        head = diskRequestArray[minPosition]; /* jump to new found min disk position */
        completedTaskArray[minPosition] = 1; /* declare SERVED */
        completedCount += 1; /* indicate less elements to compare min with in while loop */
    }

    free( completedTaskArray ); /* free malloc'ed variables */
    completedTaskArray = NULL;

    return seekTime;
}


/**
 * @brief SCAN scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int scan( DiskSectorRequest* pDiskRequests ) 
{
    int icount = 0, seekTime = 0; /* iteration value and return value */

    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;

    int *completedTaskArray = ( int* ) calloc( numberOfRequests, sizeof( int ) ); /* array to represent if request has been served */
    int completedCount = 0;

    int upperBound = pDiskRequests -> totalCylinders - 1;
    int lowerBound = 0; 
    int direction = ( head - pDiskRequests -> previousDiskRequest ) / abs( head - pDiskRequests -> previousDiskRequest ); /* direction: -1 descending or 1 ascending */
    int valueFound = FALSE;

    int boundToBounce;


    if ( direction == -1 ) {
        boundToBounce = lowerBound;
    }
    else if ( direction == 1 ) {
        boundToBounce = upperBound;
    }

    while ( completedCount != numberOfRequests ) {

        int minDiff = INT_MAX, minPosition = 0;
        /* find the min */
        valueFound = FALSE;

        for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
            int diff = abs( head - diskRequestArray[icount] ); /* get difference */
            if ( completedTaskArray[icount] != 1 ) {/* if not already served */
                if ( ( direction == 1 && head <= diskRequestArray[icount] && diff < minDiff ) || ( direction == -1 && head >= diskRequestArray[icount] && diff < minDiff ) ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
                    valueFound = TRUE;
                    minDiff = diff;
                    minPosition = icount;
                }
            }
        }

        if (valueFound == FALSE) {
            int bounceDiff = abs( head - boundToBounce );
            seekTime += bounceDiff;
            head = boundToBounce;
            direction *= -1;
        }
        else {
            /* after min has been found, execute and declare served */
            seekTime += minDiff;
            head = diskRequestArray[minPosition]; /* jump to new found min disk position */
            completedTaskArray[minPosition] = 1; /* declare SERVED */
            completedCount += 1; /* indicate less elements to compare min with in while loop */
        }
    }

    free( completedTaskArray ); /* free malloc'ed variables */
    completedTaskArray = NULL;


    return seekTime;
}


/**
 * @brief C-SCAN scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int cscan( DiskSectorRequest* pDiskRequests ) {
    int icount = 0, seekTime = 0; /* iteration value and return value */

    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;

    int *completedTaskArray = ( int* ) calloc( numberOfRequests, sizeof( int ) ); /* array to represent if request has been served */
    int completedCount = 0;

    int upperBound;
    int lowerBound;
    int direction = ( head - pDiskRequests -> previousDiskRequest ) / abs( head - pDiskRequests -> previousDiskRequest ); /* direction: -1 descending or 1 ascending */
    int valueFound = FALSE;

    if ( direction == -1 ) {
        upperBound = pDiskRequests -> totalCylinders - 1;
        lowerBound = 0; 
    }
    else if ( direction == 1 ) {
        upperBound = 0; 
        lowerBound = pDiskRequests -> totalCylinders - 1;
    }

    while ( completedCount != numberOfRequests ) {
        int minDiff = INT_MAX, minPosition = 0;
        /* find the min */
        valueFound = FALSE;

        for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
            int diff = abs( head - diskRequestArray[icount] ); /* get difference */
            if ( completedTaskArray[icount] != 1 ) {/* if not already served */
                if ( ( direction == 1 && head <= diskRequestArray[icount] && diff < minDiff ) || ( direction == -1 && head >= diskRequestArray[icount] && diff < minDiff ) ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
                    valueFound = TRUE;
                    minDiff = diff;
                    minPosition = icount;
                }
            }
        }

        if (valueFound == FALSE) {
            int bounceDiff = abs( head - lowerBound ) + abs( lowerBound - upperBound );
            seekTime += bounceDiff;
            head = upperBound;
        }
        else {
            /* after min has been found, execute and declare served */
            seekTime += minDiff;
            head = diskRequestArray[minPosition]; /* jump to new found min disk position */
            completedTaskArray[minPosition] = 1; /* declare SERVED */
            completedCount += 1; /* indicate less elements to compare min with in while loop */
        }
    }

    free( completedTaskArray ); /* free malloc'ed variables */
    completedTaskArray = NULL;


    return seekTime;
}


/**
 * @brief LOOK scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int look( DiskSectorRequest* pDiskRequests ) {
    int icount = 0, seekTime = 0; /* iteration value and return value */

    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;

    int *completedTaskArray = ( int* ) calloc( numberOfRequests, sizeof( int ) ); /* array to represent if request has been served */
    int completedCount = 0;
    
    int direction = ( head - pDiskRequests -> previousDiskRequest ) / abs( head - pDiskRequests -> previousDiskRequest ); /* direction: -1 descending or 1 ascending */
    
    int minDiff = INT_MAX;
    int valueFound = FALSE;

    while ( completedCount != numberOfRequests ) {
        int minPosition = 0;
        minDiff = INT_MAX;

        /* find the min */
        valueFound = FALSE;

        for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
            int diff = abs( head - diskRequestArray[icount] ); /* get difference */
            if ( completedTaskArray[icount] != 1 ) {/* if not already served */
                if ( ( direction == 1 && head <= diskRequestArray[icount] && diff < minDiff ) || ( direction == -1 && head >= diskRequestArray[icount] && diff < minDiff ) ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
                    valueFound = TRUE;
                    minDiff = diff;
                    minPosition = icount;
                }
            }
        }

        if (valueFound == FALSE) {
            direction *= -1;

        }
        else {

            /* after min has been found, execute and declare served */
            seekTime += minDiff;
            head = diskRequestArray[minPosition]; /* jump to new found min disk position */
            completedTaskArray[minPosition] = 1; /* declare SERVED */
            completedCount += 1; /* indicate less elements to compare min with in while loop */
        }
    }

    free( completedTaskArray ); /* free malloc'ed variables */
    completedTaskArray = NULL;
    completedCount = 0;

    return seekTime;
}


/**
 * @brief C-LOOK scheduler algorithm
 * 
 * @param pDiskRequests 
 * @return int 
 */
int clook( DiskSectorRequest* pDiskRequests ) {
    int icount = 0, seekTime = 0; /* iteration value and return value */

    int head = pDiskRequests -> currentPosition; /* head disk request value */
    int *diskRequestArray = pDiskRequests -> diskRequestArray; /* disk request array */
    int numberOfRequests = pDiskRequests -> requestCount;

    int *completedTaskArray = ( int* ) calloc( numberOfRequests, sizeof( int ) ); /* array to represent if request has been served */
    int completedCount = 0;

    int maxValue = 0, maxValuePosition;
    int minValue = INT_MAX, minValuePosition;
    
    int direction = ( head - pDiskRequests -> previousDiskRequest ) / abs( head - pDiskRequests -> previousDiskRequest ); /* direction: -1 descending or 1 ascending */
    int valueFound = FALSE;
    
    for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
        if ( diskRequestArray[icount] > maxValue ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
            maxValue = diskRequestArray[icount];
            maxValuePosition = icount;
        }
        
        if ( diskRequestArray[icount] < minValue) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
            minValue = diskRequestArray[icount];
            minValuePosition = icount;
        }
    }


    while ( completedCount != numberOfRequests ) {
        int minPosition = 0;
        int minDiff = INT_MAX;
        /* find the min */
        valueFound = FALSE;

        for ( icount = 0 ; icount < numberOfRequests ; icount ++ ) {
            int diff = abs( head - diskRequestArray[icount] ); /* get difference */
            if ( completedTaskArray[icount] != 1 ) {/* if not already served */
                if ( ( direction == 1 && head <= diskRequestArray[icount] && diff < minDiff ) || ( direction == -1 && head >= diskRequestArray[icount] && diff < minDiff ) ) {/* if first search of value, accept as min, compare the rest with this value to find true min value */
                    valueFound = TRUE;
                    minDiff = diff;
                    minPosition = icount;
                }
            }
        }

        if (valueFound == FALSE) {
            if ( direction == -1 ) {
                minDiff = abs( head - maxValue );
                minPosition = maxValuePosition;
            }
            else if ( direction == 1 ) {
                minDiff = abs( head - minValue );
                minPosition = minValuePosition;
            }
        }

        /* after min has been found, execute and declare served */
        seekTime += minDiff;
        head = diskRequestArray[minPosition]; /* jump to new found min disk position */
        completedTaskArray[minPosition] = 1; /* declare SERVED */
        completedCount += 1; /* indicate less elements to compare min with in while loop */
    }

    free( completedTaskArray ); /* free malloc'ed variables */
    completedTaskArray = NULL;

    return seekTime;
}

