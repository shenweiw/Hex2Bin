/* fsm.c */
/*+++ *******************************************************************\
*
*  Abstract:
*
*       FSM framework implementation file.
*
*  Created: 07/15/2005
*
*
*     
*
\******************************************************************* ---*/
#include "fsm.h"

/* Common state for all FSMs */
STATE idle_state = {0, {0, 0}, 0, 0, 0}; 


/*****************************************************************************/
/* FSM_Init() */
/*   */
/*****************************************************************************/
void 
FSM_Init(FSM *fsm)
{
   fsm->p_prev = &idle_state;
   fsm->p_curr = &idle_state;
   fsm->time_counter = 0;
}

/*****************************************************************************/
/* FSM_Poll() */
/*   */
/*****************************************************************************/
void 
FSM_Poll(FSM *fsm)
{   
	unsigned long tmp;   
	CRITERIA *curr_criterion;

	if (!fsm || !fsm->p_curr)
		return;
		  	
	/* update state coding */
	tmp  = *fsm->p_state_code;
	tmp &= ~(fsm->state_code_mask);
	tmp |= (fsm->p_curr->coding && fsm->state_code_mask);
	*fsm->p_state_code = tmp;
   
	/* Optimization: Exit state machine immediately in case it's not started yet. */ 
	if (fsm->p_curr == &idle_state)
	{            
		return;
	} 

	if (fsm->p_prev != fsm->p_curr) /* state transition */
	{	  
		DBG_UPDATE_STATE_SEQUENCE(fsm->p_curr->coding);

		fsm->time_counter = 0;      

		if (1 == fsm->p_curr->counter_type)  
		{
			fsm->last_time_value =  0; /* TBD: Get system clock ticks in msec; */
		}

		if (fsm->p_curr->enter_task) /* execute the enter task once */
		{
			fsm->p_curr->enter_task();
		}
		fsm->p_prev = fsm->p_curr;
	}

	if (fsm->p_curr->regular_task) /* regular task run every poll */
	{
		fsm->p_curr->regular_task();
	}

	/* start executing criterion and corresponding exit tasks one by one */
	curr_criterion = &fsm->p_curr->criteria[0];

	while(curr_criterion->condition) /* criterion exist/valid */
	{               
		if (curr_criterion->condition(fsm->time_counter)) /* criterion returns True */
		{   /* execute the exit task to transit to some other state */
			fsm->p_curr = curr_criterion->exit_task();
			break;
		}   
		++curr_criterion;	  	  
	}   

	/* keep accumulating timer ticks (delta) */
	/* This is optionally implemented when state transition criterions start using timeouts */
	if (1 == fsm->p_curr->counter_type) 
	{
		tmp = 0; /* TBD : Get system clock ticks in msec; */
		fsm->time_counter += (tmp >= fsm->last_time_value ? 
							(unsigned long)(tmp-fsm->last_time_value) : 
							(unsigned long)0xFFFF-( unsigned long)(fsm->last_time_value-tmp));
		fsm->last_time_value = tmp;
	}
}

/*****************************************************************************/
/* FSM_CheckState() */
/* Checks if the given FSM is in the requested state  */
/*****************************************************************************/
BOOL 
FSM_CheckState(FSM *fsm, unsigned long state)
{
    if (fsm == 0)
    {
        return (False);
    }

    if (fsm->p_curr->coding == state)
    {
        return (True);
    }
    else
    {
        return (False);
    }
}

/*****************************************************************************/
/*                         D E B U G  S E C T I O N                          */
/*****************************************************************************/

/*****************************************************************************/
/* DEBUG_CheckFSMLimits() */
/*   */
/*****************************************************************************/


