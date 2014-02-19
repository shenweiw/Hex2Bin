#ifndef _FSM_TYPES_
#define _FSM_TYPES_


#ifndef TRUE
#define TRUE    1                    /* from msdn: The bool type participates in integral promotions.*/
#define FALSE   0                    /* ... An r-value of type bool can be converted to an r-value */
#endif                               /* ... of type int, with false becoming zero and true becoming one.*/

#ifndef True
#define True    TRUE 
#define False   FALSE
#endif       

#ifndef BOOL
typedef int     BOOL;
#endif  /* BOOL */

#endif

