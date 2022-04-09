/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/

#include <stdio.h>
#include "includes.h"

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */

Logs logs;

typedef struct{
    INT8U c,p; // INT32U will fail
}Job;

void startSim(void *pdata){
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    Job* job=(Job*)pdata;
    INT8U c=job->c;
    INT8U p=job->p;
    INT16U cur; // first period start at clk=0
    INT8U idx;

    while(1){
        while(OSTCBCur->counter<c){
            if(OSTimeGet()>OSTCBCur->deadline){
                OS_ENTER_CRITICAL();
                idx=logs.num;
                if(idx<MAXLOGNUM){
                    logs.clk[idx]=OSTimeGet();
                    logs.vol[idx]=2;
                    logs.src[idx]=OSTCBCur->OSTCBPrio;
                    logs.num+=1;
                }
                OS_EXIT_CRITICAL();
                break;
            }
        }

        OS_ENTER_CRITICAL();
        OSTCBCur->counter=0;
        OSTCBCur->deadline+=p;
        OS_EXIT_CRITICAL();

        cur=OSTimeGet();
        if(OSTCBCur->deadline-p>cur){
            OSTimeDly(OSTCBCur->deadline-p-cur);
        }
    }
}

void initSim(INT8U num){
    // declaration should be put at begining for old compiler
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    static OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];
    static Job jobs[5];
    INT8U i;

    if(num==2){
        jobs[0].c=1;jobs[0].p=3;
        jobs[1].c=3;jobs[1].p=5;
    }else{
        jobs[0].c=1;jobs[0].p=4;
        jobs[1].c=2;jobs[1].p=5;
        jobs[2].c=2;jobs[2].p=10;
    }

    for(i=0;i<num;++i){
        OSTaskCreate(startSim,(void *)(jobs+i),&TaskStk[i][TASK_STK_SIZE-1],i+1);
    }

    // avoid preemption of task with deadline_valid=1
    OS_ENTER_CRITICAL();
    for(i=0;i<num;++i){
        OSTCBPrioTbl[i+1]->deadline_valid=1;
        OSTCBPrioTbl[i+1]->deadline=jobs[i].p;
    }
    OSTimeSet(0);
    OS_EXIT_CRITICAL();
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
        }else{
            printf("killed          ");
            printf("%d\n",logs.src[cur]);
        }
    }

    logs.num=0;
}

void  TaskStart(void *pdata){
    initSim(3);
    while(1){
        OSTimeDly(OS_TICKS_PER_SEC);
        printLogs();
    }
}

void  main(void){
    static OS_STK TaskStartStk[TASK_STK_SIZE];
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
