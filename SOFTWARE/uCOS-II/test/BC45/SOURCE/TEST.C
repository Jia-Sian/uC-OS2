#include "includes.h"

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */
#define JOBNUM 2

Logs logs;

typedef enum{
    Delay,Run,Lock,Unlock
}OpType;

typedef struct{
    OpType optype;
    INT16U t;
    OS_EVENT *mutex;
}Operation;

typedef struct{
    Operation op[10];
    int n;
}Job;

void startSim(void *pdata){
    INT8U i;
    INT8U err;
    Operation *op;
    Job *job=(Job*)pdata;

    for(i=0;i<job->n;++i){
        op=&(job->op[i]);

        if(op->optype==Delay){
            OSTimeDly(op->t);
        }else if(op->optype==Run){
            OS_ENTER_CRITICAL();
            OSTCBCur->counter=0;
            OS_EXIT_CRITICAL();
            while(OSTCBCur->counter<op->t){}
        }else if(op->optype==Lock){
            OSMutexPend(op->mutex,0,&err);
        }else if(op->optype==Unlock){
            OSMutexPost(op->mutex);
        }
    }

    OSTaskDel(OSTCBCur->OSTCBPrio);
}

void pushOp(Job *job,OpType optype,INT16U t,OS_EVENT *mutex){
    INT8U i=job->n;
    job->n+=1;

    job->op[i].optype=optype;
    job->op[i].t=t;
    job->op[i].mutex=mutex;
}

void initSim(INT8U num){
    // declaration should be put at begining for old compiler
    static OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];
    static Job job[5];
    static OS_EVENT *mutex[5];
    INT8U i;

    mutex[0]=OSMutexCreate(1,&i);
    mutex[1]=OSMutexCreate(2,&i);

    if(num==3){
        job[0].n=0;
        pushOp(job+0,Delay,8,0);
        pushOp(job+0,Run,2,0);
        pushOp(job+0,Lock,0,mutex[0]);
        pushOp(job+0,Run,2,0);
        pushOp(job+0,Lock,0,mutex[1]);
        pushOp(job+0,Run,2,0);
        pushOp(job+0,Unlock,0,mutex[1]);
        pushOp(job+0,Unlock,0,mutex[0]);

        job[1].n=0;
        pushOp(job+1,Delay,4,0);
        pushOp(job+1,Run,2,0);
        pushOp(job+1,Lock,0,mutex[1]);
        pushOp(job+1,Run,4,0);
        pushOp(job+1,Unlock,0,mutex[1]);

        job[2].n=0;
        pushOp(job+2,Delay,0,0);
        pushOp(job+2,Run,2,0);
        pushOp(job+2,Lock,0,mutex[0]);
        pushOp(job+2,Run,7,0);
        pushOp(job+2,Unlock,0,mutex[0]);
    }else if(num==2){
        job[0].n=0;
        pushOp(job+0,Delay,5,0);
        pushOp(job+0,Run,2,0);
        pushOp(job+0,Lock,0,mutex[1]);
        pushOp(job+0,Run,3,0);
        pushOp(job+0,Lock,0,mutex[0]);
        pushOp(job+0,Run,3,0);
        pushOp(job+0,Unlock,0,mutex[0]);
        pushOp(job+0,Run,3,0);
        pushOp(job+0,Unlock,0,mutex[1]);

        job[1].n=0;
        pushOp(job+1,Delay,0,0);
        pushOp(job+1,Run,2,0);
        pushOp(job+1,Lock,0,mutex[0]);
        pushOp(job+1,Run,6,0);
        pushOp(job+1,Lock,0,mutex[1]);
        pushOp(job+1,Run,2,0);
        pushOp(job+1,Unlock,0,mutex[1]);
        pushOp(job+1,Run,2,0);
        pushOp(job+1,Unlock,0,mutex[0]);
    }

    for(i=0;i<num;++i){
        OSTaskCreate(startSim,(void *)(job+i),&TaskStk[i][TASK_STK_SIZE-1],i+3);
    }
}

void printLogs(void){ // Don't lock here cuz we have top priority.
    INT8U cur;
    for(cur=0;cur<logs.num;++cur){
        printf("%d\t",logs.clk[cur]);
        if(logs.vol[cur]==0){
            printf("preempt         ");
            printf("%d\t%d\n",logs.src[cur],logs.dst[cur]);
        }else if(logs.vol[cur]==1){
            printf("complete        ");
            printf("%d\t%d\n",logs.src[cur],logs.dst[cur]);
        }else if(logs.vol[cur]==2){
            printf("lock            ");
            if(logs.src[cur]>logs.dst[cur]){
                printf("R%d\t(Prio=%d changes to=%d)\n",logs.dst[cur],logs.src[cur],logs.dst[cur]);
            }else{
                printf("R%d\t(Prio=%d changes to=%d)\n",logs.dst[cur],logs.src[cur],logs.src[cur]);
            }
        }else if(logs.vol[cur]==3){
            printf("unlock          ");
            if(logs.src[cur]>logs.dst[cur]){
                printf("R%d\t(Prio=%d changes to=%d)\n",logs.dst[cur],logs.dst[cur],logs.src[cur]);
            }else{
                printf("R%d\t(Prio=%d changes to=%d)\n",logs.dst[cur],logs.src[cur],logs.src[cur]);
            }
        }
    }

    logs.num=0;
}

void  TaskStart(void *pdata){
    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    initSim(JOBNUM);
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