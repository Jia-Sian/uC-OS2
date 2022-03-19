#include "includes.h"

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */

typedef struct{
    INT8U c,p; // INT32U will fail
}Job;

void startSim(void *pdata){
    Job* job=(Job*)pdata;
    INT8U c=job->c;
    INT8U p=job->p;
    INT16U nextp=p,cur; // first period start at clk=0
    while(1){
        while(OSTCBCur->counter<c){}

        OS_ENTER_CRITICAL();
        OSTCBCur->counter=0;
        OS_EXIT_CRITICAL();

        cur=OSTimeGet();
        if(nextp>cur)OSTimeDly(nextp-cur);
        nextp+=p;
    }
}

void initSim(INT8U num){
    // declaration should be put at begining for old compiler
    static OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];
    static Job jobs[5];
    INT8U i;

    jobs[0].c=1;jobs[0].p=3;
    jobs[1].c=3;jobs[1].p=6;
    jobs[2].c=4;jobs[2].p=9;
    for(i=0;i<num;++i){
        OSTaskCreate(startSim,(void *)(jobs+i),&TaskStk[i][TASK_STK_SIZE-1],i+1); // RM policy
    }
}

Logs logs;

void printLogs(void){
    INT8U cur;
    for(cur=0;cur<logs.num;++cur){
        printf("%d\t",logs.clk[cur]);
        if(logs.vol[cur]==0){
            printf("preempt         ");
        }else{
            printf("complete        ");
        }
        printf("%d\t%d\n",logs.src[cur],logs.dst[cur]);
    }

    OS_ENTER_CRITICAL();
    logs.num=0;
    OS_EXIT_CRITICAL();
}

void  TaskStart(void *pdata){
    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    initSim(2);
    OSTimeSet(0);
    while(1){
        OSTimeDly(OS_TICKS_PER_SEC);
        printLogs();
    }
}

void  main(void){
    static OS_STK TaskStartStk[TASK_STK_SIZE];

    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}