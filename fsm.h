/* fsm.h */
/*+++ *******************************************************************\
*
*  Abstract:
*
*       FSM framework header file.
*
*  Created: 11/15/2013
*
*
*     
*
\******************************************************************* ---*/
#ifndef _FSM_H_
#define _FSM_H_

#include "fsm_types.h"

#define 	FSM_MAX_NUM_CRITERIA		10

/* FSM STATE CODING:
 * -----------------
 * bit-field assignment:
 *----------------------------------------------------------------------------------
 * Sub-FSM#3[15:12]   | Sub-FSM#2[11:8]   | Sub-FSM#1[7:4]     | Main FSM[3:0]     |
 *----------------------------------------------------------------------------------
 
 *----------------------------------------------------------------------------------
 * Actual FSM mapping:
 *----------------------------------------------------------------------------------
 * RISE RANGE SUB-FSM | RISE EDGE SUB-FSM | BIN SEARCH SUB-FSM | STS MAIN FSM      |
 *----------------------------------------------------------------------------------

 *----------------------------------------------------------------------------------
 */


#define STATE_GEN(name, state, num) 							\
	enum {name ## _ ## state ## _STATE_NO  = num};	\
	static STATE name ## _ ## state ## _state;

#define STATE_FUNC_ENTRY_GEN(name, state)		\
	static void  name ## _ ## state ##Enter()	\
	{																					\
		printf("<<%s>> \r\n", __FUNCTION__);		\
																						\
		return;																	\
	}																					

#define STATE_FUNC_STA2STA_GEN(name, state1, state2, sname, sstate1, sstate2)	\
	static STATE* name ## _ ## state1 ## 2 ##state2 ()								\
	{																																	\
		printf("\t-->%s (%lu -> %lu)\r\n", __FUNCTION__, 									\
					sname ## _ ## sstate1 ## _state.coding,										\
					sname ## _ ## sstate2 ## _state.coding);									\
																																		\
		return &sname ## _ ## sstate2 ## _state ;												\
	}																					

#define STATE_FUNC_CRITERION_GEN(name, state1, state2)	\
	static BOOL name ## _ ## state1 ## 2 ## state2 ## Criterion()			\
	{																																	\
		printf("%s\r\n", __FUNCTION__);																	\
		if (rand()%10 == 8)																							\
    return (True);																									\
    else																														\
    	return (False);																								\
}							



#define 	FSM_IDLE_STATE				0         /* Idle state. Common for all FSMs */

/* Main FSM */
#define 	STS_RESET_STATE				1         /* Main scan-the-sky FSM */


/* Sub-FSMs */

/* Maximum number of states in the log */
#define 	STS_MAX_STATE_SEQUENCES		40

/* FSM Data structures */
struct _State;

typedef struct _Criteria
{
	BOOL  (*condition)(); /* Input parameter can be void/ignored if the state does not use timers */
	struct _State *(*exit_task)();
} CRITERIA;

typedef struct _State
{
	void (*enter_task)();
	CRITERIA criteria[FSM_MAX_NUM_CRITERIA];
	void (*regular_task)();
	unsigned char counter_type;  /* The type of counter to pass to the function condition in criterion.  This member is optional */
                          		 /*   0 - none             */
                          		 /*   1 - ticks in msec    */
	unsigned long coding;   
} STATE;


typedef struct _Fsm
{
	volatile unsigned long 	*p_state_code;
	unsigned long 			state_code_mask;
	STATE 					*p_prev;
	STATE 					*p_curr;
	unsigned long			time_counter;     /* This member is optional. See counter_type in STATE struct. */
	unsigned long   		last_time_value;  /* This member is optional. See counter_type in STATE struct. */   
} FSM;

/* Main FSM functions */
void FSM_Init(FSM *fsm);
void FSM_Poll(FSM *fsm);

/* Helper functions */
BOOL FSM_CheckState(FSM *fsm, unsigned long state);

/*******************************************************************************************************/
/* Debug Macros and functions */
/*******************************************************************************************************/

/* Per state information available for debugging */
typedef struct _fsm_state_sequence
{
  	unsigned long  	state;	     /* state code */
  	unsigned short 	time_spent;  /* time spent in each state */
} FSM_STATE_SEQUENCE_DATA;


#define	DBG_UPDATE_STATE_SEQUENCE(s) 

#endif /* _FSM_H_ */
