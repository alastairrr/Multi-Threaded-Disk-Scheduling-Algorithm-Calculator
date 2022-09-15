/* ************************************************* *
 * </ HEADER /> ===== SCHEDULER.H ===== </ HEADER /> *
 * ************************************************* */

/**
 * @file scheduler.c
 * @author Alastair Kho Ying Thai (20214878)

 * @task: Operating Systems Assignment
 * @unit: COMP2006 Operating Systems
 * @institution: Curtin University
 * 
 * @brief Header file that holds function declarations, macro defines, and DiskSectorRequest Struct
 * @version 0.1
 * @date 2022-05-02
 * 
 */



/* ****************************************************** *
 * </ MACRO /> ===== MACRO DECLARATIONS ===== </ MACRO /> *
 * ****************************************************** */

#ifndef SCHEDULER_H
#define SCHEDUER_H

#define FALSE 0
#define TRUE !FALSE

#define PARENT 1
#define CHILD 0
#define QUIT -1



/* ********************************************************* *
 * </ STRUCT /> ===== STRUCT DECLARATIONS ===== </ STRUCT /> *
 * ********************************************************* */

typedef struct DiskSectorRequest {
    int *diskRequestArray;
    int requestCount;
    int totalCylinders;
    int currentPosition;
    int previousDiskRequest;

} DiskSectorRequest;



/* *************************************************************** *
 * </ FUNCTION /> ===== FUNCTION DECLARATIONS ===== </ FUNCTION /> *
 * *************************************************************** */

int firstComeFirstServe( DiskSectorRequest* pDiskRequests );
int shortestSeekTimeFirst( DiskSectorRequest* pDiskRequests );
int scan( DiskSectorRequest* pDiskRequests );
int cscan( DiskSectorRequest* pDiskRequests );
int look( DiskSectorRequest* pDiskRequests );
int clook( DiskSectorRequest* pDiskRequests );



#endif