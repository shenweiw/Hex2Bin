
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "fsm.h"

#define  FSM_REFRESH_CYCLES        10000
static unsigned long sts_fsm_coding = 0; 

/* Instantiate the FSM object(s) */
static FSM sts_fsm = {&sts_fsm_coding, 0x000000F0, 0, 0, 0, 0};     


 
STATE_GEN(hex2bin, reset, 1)
STATE_GEN(hex2bin, linein, 2)
STATE_GEN(hex2bin, convert, 3)
STATE_GEN(hex2bin, errors, 4)
STATE_GEN(hex2bin, idle, 5)

/*
Use the following as an example to name the functions.

STATE_FUNC_ENTRY_GEN(TEST, Stop)
STATE_FUNC_STA2STA_GEN(TEST, Reset, Run, test, reset, run)
*/

#define FILE_ROW_MAX_LENGTH	(256)
static char s_internal_hex_file[FILE_ROW_MAX_LENGTH];
static char s_internal_bin_file[FILE_ROW_MAX_LENGTH];
static char s_internal_line_buf[FILE_ROW_MAX_LENGTH];
static char *s_internal_line_ret = 0;
static int s_internal_line_index = 0;
static FILE *s_internal_fp_hex = 0;
static FILE *s_internal_fp_bin = 0;

static unsigned char s_internal_converted_lines[20];
static int s_internal_converted_index = 0;

static void  HEX2BIN_ResetEnter()
{
	printf("%s \r\n", __FUNCTION__);
	
	s_internal_fp_hex = fopen(s_internal_hex_file,"r");
	s_internal_fp_bin = fopen(s_internal_bin_file,"w");
	
	s_internal_converted_index = 0;

	if(s_internal_fp_hex == NULL)
	{
		printf("%s hex file open error\r\n", s_internal_hex_file);
	}
	
	if(s_internal_fp_bin == NULL)
	{
		printf("%s bin file open error\r\n", s_internal_bin_file);
	}
	
	s_internal_line_index = 0;
	return;
}

static void  HEX2BIN_IdleEnter()
{

	printf("%s \r\n", __FUNCTION__);
	
	if (s_internal_fp_hex)
		fclose(s_internal_fp_hex);
	if (s_internal_fp_bin)
		fclose(s_internal_fp_bin);
	
	s_internal_fp_hex = 0;
	s_internal_fp_bin = 0;
	
	
}

static BOOL HEX2BIN_Reset2LineinCriterion()
{
	if ((s_internal_fp_hex != NULL) && (s_internal_fp_hex != NULL))
		return True;
	else
		return False;
}

static BOOL HEX2BIN_Reset2IdleCriterion()
{
	if ((s_internal_fp_hex == NULL) || (s_internal_fp_hex == NULL))
		return True;
	else
		return False;
}

static STATE* HEX2BIN_Reset2Linein()
{
	return &hex2bin_linein_state;
}

static STATE* HEX2BIN_Reset2Idle()
{
	return &hex2bin_idle_state;
}

static void  HEX2BIN_LineinEnter()
{
	//printf("%s \r\n", __FUNCTION__);
	s_internal_line_ret = 0;
	
	if(s_internal_converted_index)
	{
		fwrite(s_internal_converted_lines, 1, s_internal_converted_index, s_internal_fp_bin);
		s_internal_converted_index = 0;
		//exit(0);
	}
}

static void HEX2BIN_Linein() //Regular task
{
	//printf("%s \r\n", __FUNCTION__);
	s_internal_line_ret = fgets(s_internal_line_buf, sizeof(s_internal_line_buf), s_internal_fp_hex);
	if(s_internal_line_ret)
	{
		printf("<%06d>  %s", s_internal_line_index, s_internal_line_ret);
		s_internal_line_index++;
	}
}


static BOOL HEX2BIN_Linein2ConvertCriterion()
{
	if (s_internal_line_ret)
		return True;
	else
		return False;
}

static STATE* HEX2BIN_Linein2Convert()
{
	return &hex2bin_convert_state;
}

static BOOL HEX2BIN_Linein2IdleCriterion()
{
	if (s_internal_line_ret == NULL)
		return True;
	else
		return False;
}

static STATE* HEX2BIN_Linein2Idle()
{
	return &hex2bin_idle_state;
}

/*
s_internal_line_ret-->
0000000 ffff ffff ffff ffff ffff ffff ffff ffff
*/
static int s_internal_last_line = -0x10;
static char * s_internal_p_line_tok = 0;
static BOOL s_internal_b_error = 0;


static void  HEX2BIN_ConvertEnter()
{
	int current_line = 0;
	static int i = 0;
	
	s_internal_b_error = 0;
	s_internal_converted_index = 0;
	
	s_internal_p_line_tok = strtok(s_internal_line_ret, " ");
	//p = strtok(NULL, ","); 
	//printf(" -%06d-  %s ", s_internal_line_index, s_internal_line_ret);
	if(strlen(s_internal_p_line_tok) == 7)
	{
		current_line = strtol(s_internal_p_line_tok, 0, 16);
		
		if((s_internal_last_line + 0x10) ==  current_line)
		{
			s_internal_last_line = current_line;
			printf("-%06d-  %07X ", s_internal_line_index, current_line);
			
		}
		else
		{
			s_internal_b_error = 1;
			s_internal_last_line = current_line;
			printf("%x -> %x ", s_internal_last_line, current_line);
		}
			
	}
	else
	{
		s_internal_p_line_tok = 0;
		printf("test\r\n");
	}
	
//	printf("%d, %s\r\n", s_internal_last_line, s_internal_p_line_tok);
//	if(i++ >5)
//		exit(0);
	//printf("%s \r\n", __FUNCTION__);
}

static void HEX2BIN_Convert() //Regular task
{
	unsigned short code = 0;
	
	//printf("%s \r\n", __FUNCTION__);
	
	if(s_internal_p_line_tok)
	{
		s_internal_p_line_tok = strtok(NULL, " ");
		if(s_internal_p_line_tok==NULL)
			return;
		
		code = strtol( s_internal_p_line_tok, 0, 16);	
		
		code = (code&0xFF)<<8 | (code>>8);
		s_internal_converted_lines[s_internal_converted_index] = (code>>8) & 0xFF;
		s_internal_converted_lines[s_internal_converted_index+1] = code& 0xFF;
		
		s_internal_converted_index += 2;
		printf("%04X ", code);
	}
}

static BOOL HEX2BIN_Convert2ErrorsCriterion()
{
	//printf("%s \r\n", __FUNCTION__);
	return s_internal_b_error;
}

static STATE* HEX2BIN_Convert2Errors()
{
	return &hex2bin_errors_state;
}

static BOOL HEX2BIN_Convert2LineinCriterion()
{
	//printf("%s s_internal_p_line_tok=%p\r\n", __FUNCTION__, s_internal_p_line_tok);
	return (s_internal_p_line_tok==NULL);
}

static STATE* HEX2BIN_Convert2Linein()
{
	printf("\r\n");
	return &hex2bin_linein_state;
}

static void  HEX2BIN_ErrorsEnter()
{
	printf("%s \r\n", __FUNCTION__);
}

static BOOL HEX2BIN_Errors2LineinCriterion()
{
	return True;
}

static STATE* HEX2BIN_Errors2Linein()
{
	return &hex2bin_linein_state;
}

/* State tables */
static STATE hex2bin_reset_state = /* RESET state */
{
	HEX2BIN_ResetEnter, /* Entry task */
   	{
      	{HEX2BIN_Reset2LineinCriterion,  HEX2BIN_Reset2Linein}, 
      	{HEX2BIN_Reset2IdleCriterion,  HEX2BIN_Reset2Idle},      
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	hex2bin_reset_STATE_NO /* state code */	
};

static STATE hex2bin_linein_state = /* LINEIN state */
{
	HEX2BIN_LineinEnter, /* Entry task */
   	{
      	{HEX2BIN_Linein2ConvertCriterion,  HEX2BIN_Linein2Convert},
      	{HEX2BIN_Linein2IdleCriterion,  HEX2BIN_Linein2Idle},        
      	{0,0} /* End of Criterion/Exit task */
   	},
   	HEX2BIN_Linein, /* Regular/Routine task */
   	0, /* No counter */
   	hex2bin_linein_STATE_NO /* state code */	
};

static STATE hex2bin_convert_state = /* CONVERT state */
{
	HEX2BIN_ConvertEnter, /* Entry task */
   	{
      	{HEX2BIN_Convert2ErrorsCriterion,  HEX2BIN_Convert2Errors},      
      	{HEX2BIN_Convert2LineinCriterion,  HEX2BIN_Convert2Linein},
      	{0,0} /* End of Criterion/Exit task */
   	},
   	HEX2BIN_Convert, /* Regular/Routine task */
   	0, /* No counter */
   	hex2bin_convert_STATE_NO /* state code */	
};

static STATE hex2bin_errors_state = /* ERRORS state */
{
	HEX2BIN_ErrorsEnter, /* Entry task */
   	{
      	{HEX2BIN_Errors2LineinCriterion,  HEX2BIN_Errors2Linein},      
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	hex2bin_errors_STATE_NO /* state code */	
};

static STATE hex2bin_idle_state = /* IDLE state */
{
	HEX2BIN_IdleEnter, /* Entry task */
   	{
      	{0,0} /* End of Criterion/Exit task */
   	},
   	0, /* Regular/Routine task */
   	0, /* No counter */
   	hex2bin_idle_STATE_NO /* state code */	
};


void hex2bin_start(char *hexfile)
{
    unsigned char refresh_cycles = 0;

	printf("%s entered. Converting %s\r\n", __FUNCTION__, hexfile);

  	if(getcwd(s_internal_hex_file, sizeof(s_internal_hex_file)) == 0)
    {
    	printf("getcwd failed\r\n");
    	return;
    }
    strcat(s_internal_hex_file, "/");
    strcat(s_internal_hex_file, hexfile);
    strcpy(s_internal_bin_file, s_internal_hex_file);
    strcat(s_internal_bin_file, ".bin");
    
    printf("Full Hex File Path = %s\r\n", s_internal_hex_file);
         	
	sts_fsm.p_curr = &hex2bin_reset_state; 
    do 
    {
        FSM_Poll(&sts_fsm); 
        
        if (FSM_CheckState(&sts_fsm, hex2bin_idle_STATE_NO) == True)        
        {
        	  FSM_Poll(&sts_fsm); 
            FSM_Init(&sts_fsm);

            return;
        }
        refresh_cycles++;
    } while (1);
   
    
    return;
}




