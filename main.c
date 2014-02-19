
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fsm.h"

#define  FSM_REFRESH_CYCLES        10000
static unsigned long sts_fsm_coding = 0; 

/* Instantiate the FSM object(s) */
static FSM sts_fsm = {&sts_fsm_coding, 0x0000000F, 0, 0, 0, 0};     


STATE_GEN(test, reset, 1)
STATE_GEN(test, run, 2)
STATE_GEN(test, stop, 3)


STATE_FUNC_ENTRY_GEN(TEST, Reset)
STATE_FUNC_ENTRY_GEN(TEST, Run)
STATE_FUNC_ENTRY_GEN(TEST, Stop)

STATE_FUNC_STA2STA_GEN(TEST, Reset, Run, test, reset, run)
STATE_FUNC_STA2STA_GEN(TEST, Run, Stop, test, run, stop)
STATE_FUNC_STA2STA_GEN(TEST, Stop, Reset, test, stop, reset)


STATE_FUNC_CRITERION_GEN(TEST, Reset, Run)
STATE_FUNC_CRITERION_GEN(TEST, Run, Stop)
STATE_FUNC_CRITERION_GEN(TEST, Stop, Reset)




/* State tables */
static STATE test_reset_state = /* RESET state */
{
	TEST_ResetEnter, /* Entry task */
   	{
      	{TEST_Reset2RunCriterion,  TEST_Reset2Run},      
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	test_reset_STATE_NO /* state code */	
};

static STATE test_run_state = /* RUN state */
{
	TEST_RunEnter, /* Entry task */
   	{
      	{TEST_Run2StopCriterion,  TEST_Run2Stop},      
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	test_run_STATE_NO /* state code */	
};

static STATE test_stop_state = /* STOP state */
{
	TEST_StopEnter, /* Entry task */
   	{
      	{TEST_Stop2ResetCriterion,  TEST_Stop2Reset},      
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	test_stop_STATE_NO /* state code */	
};



void main1()
{
    unsigned char refresh_cycles = 0;

	printf("entered.\r\n");
	srand( (unsigned)time( NULL ) );

	sts_fsm.p_curr = &test_reset_state; 
    do 
    {
        FSM_Poll(&sts_fsm); 
        
        if (FSM_CheckState(&sts_fsm, test_stop_STATE_NO) == True)        
        {
        	  FSM_Poll(&sts_fsm); 
            FSM_Init(&sts_fsm);

            return;
        }
        refresh_cycles++;
    } while (refresh_cycles < FSM_REFRESH_CYCLES);
   
    
    return;
}




#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include "ringbuf.h"

#define FIFO_LEN 7
static RingBuffer s_ringBuffer;
static uint32_t s_fifo_memory[FIFO_LEN];
static uint32_t sendBuffer[32];
static uint32_t recvBuffer[32];

int flag=1;//?????
void *fun(void *p){
    int ret=0;
    int i = 0;
	
	sleep(2);
	
	while(1)
	{
		i=0;
		ret = tools_rb_read_block(&s_ringBuffer, recvBuffer, 4, 0);
		if(ret == 0)
			break;		
			
		printf("[FUN] ");
		while(i<ret)
		{
			printf("%08X\t ", recvBuffer[i]);
			i++;
		}
		printf("\r\n");
					
		if(ret < 4)
		{
    		//printf("fun sleep \n");
    		sleep(1);
    	}
    }

    return NULL;
}

extern void hex2bin_start(char *hexfile);

int main(int argc,char *argv[])
{
    pthread_t th;
    int ret = 0, len = 0;
    
    if (argc != 2)
    {
    	printf(" %s <File>\r\n", argv[0]);
    	return 0;
    }	
    
    hex2bin_start(argv[1]);
 
 /*   
    tools_rb_init(&s_ringBuffer, s_fifo_memory, FIFO_LEN, 4);
    
    if(pthread_create(&th,NULL,fun,NULL)<0){//pthread_create()???????
        perror("pthread_create");
        return -1;
    }
//    sleep(1);//????sleep??,???main????????????????????????
    int i=0;
    while(i<5){
        sprintf(sendBuffer, "%04d%04d%04d",i, i, i);
        i++;
        len = strlen(sendBuffer);
        
        ret = tools_rb_write_block(&s_ringBuffer, (uint8_t*) sendBuffer, 3, 0);
        printf("[MAIN] %s\r\n\r\n", sendBuffer);
        if(ret < 3)
        {
        	printf("################## main sleep (%d)\r\n", ret);
        	sleep(2);
        }
        
    }
    //pthread_join(th,NULL);
 */
 
    printf("main bye bye\n");
    return 0;
}
