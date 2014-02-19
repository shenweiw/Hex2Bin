
#include <stdio.h>
#include "ringbuf.h"

static void internal_ringbuffer_fill_08bit(RingBuffer *rb, uint8_t*data, uint32_t sectors);
static void internal_ringbuffer_fill_16bit(RingBuffer *rb, uint8_t*data, uint32_t sectors);
static void internal_ringbuffer_fill_24bit(RingBuffer *rb, uint8_t*data, uint32_t sectors);
static void internal_ringbuffer_fill_32bit(RingBuffer *rb, uint8_t*data, uint32_t sectors);

static void (*s_internal_convert_fill[])(RingBuffer *rb, uint8_t*data, uint32_t sectors) =
{
	internal_ringbuffer_fill_08bit,
	internal_ringbuffer_fill_08bit,
	internal_ringbuffer_fill_16bit,
	internal_ringbuffer_fill_24bit,
	internal_ringbuffer_fill_32bit
};


void tools_rb_init(RingBuffer *rb, uint8_t* mem, uint32_t numsectors, uint32_t sectorsize)
{
	if((rb == 0) || (mem == 0))
	{
		printf("%s error!\r\n", __FUNCTION__);
		return;
	}	
	
	rb->buf = mem;
	rb->limit = numsectors;
	rb->sectorsize = sectorsize;
	rb->fifowidth = 4; //32bit width fifo
	rb->status = 0;
	rb->rp = 0;
	rb->wp = 0;
}

static void internal_ringbuffer_fill_08bit(RingBuffer *rb, uint8_t*data, uint32_t sectors)
{
	uint32_t i = 0;
	uint32_t* buf2fill = 0;
	
	buf2fill = (uint32_t *)(rb->buf + rb->wp);
	
	do
	{
		*(buf2fill + i) = *(data + i);
		//printf("<%d> buf2fill(%p) = %08X data(%p)=%X\r\n", i, buf2fill+i, *(buf2fill+i), data+i, *(data+i));
		i++;
	}while(i<sectors);
	

	return;
}

static void internal_ringbuffer_fill_16bit(RingBuffer *rb, uint8_t*data, uint32_t sectors)
{
	uint32_t i = 0;
	uint32_t* buf2fill = 0;
	uint16_t* align_data = 0;
	
	buf2fill = (uint32_t*)(rb->buf + rb->wp);
	align_data = (uint16_t*) data;
	
	do
	{
		*(buf2fill + i) = *(align_data + i);
		i++;
	}while(i<sectors);
	
	
	return;
}

static void internal_ringbuffer_fill_24bit(RingBuffer *rb, uint8_t*data, uint32_t sectors)
{
	uint32_t i = 0;
	uint32_t* buf2fill = 0;
	uint8_t* align_data = 0;
	
	buf2fill = (uint32_t*)(rb->buf + rb->wp);
	align_data = data;
	
	do
	{
		align_data = data + 3*i;
		*(buf2fill + i) = *(align_data) | (*(align_data + 1) << 8) | ((*(align_data + 2)<<16));
		i++;
	}while(i<sectors);
	
	
	return;
}


static void internal_ringbuffer_fill_32bit(RingBuffer *rb, uint8_t*data, uint32_t sectors)
{
	uint32_t i = 0;
	uint32_t* buf2fill = 0;
	uint32_t* align_data = 0;
	
	buf2fill = (uint32_t*)(rb->buf + rb->wp);
	align_data = (uint32_t *)data;
	
	do
	{
		*(buf2fill + i) = *(align_data + i);
		i++;
	}while(i<sectors);
	
	
	return;
}


/*
	return value:
	0 -- means full.
	return value less than sectors mean the buffer is full now.
*/

uint32_t tools_rb_write(RingBuffer *rb, uint8_t* data, uint32_t sectors)
{
	uint32_t step1 = 0, step2 = 0, tofill = 0, space = 0;
	
	if((rb == 0) || (data == 0))
		return 0;
			

	printf("<%s> 0 rp=%d, wp=%d sectors=%d <%d>\r\n", __FUNCTION__, rb->rp, rb->wp, sectors, rb->limit);	
	tools_dump_memory(data, sectors*rb->sectorsize);	
	
	if(rb->wp < rb->rp)
	{
		space = (rb->rp - rb->wp)/rb->fifowidth - 1;
		step1 = space>sectors?sectors:space;
		step2 = 0;
	}
	else
	{
		space = rb->limit - rb->wp/rb->fifowidth;
		step1 = space;
		step2 = rb->rp/rb->fifowidth;
		space += step2;
		
		if(sectors < step1)
		{
			step1 = sectors;
			step2 = 0;
		}
		else if (sectors < space)
		{
			step2 = sectors - step1;
		}		
	}
	
	s_internal_convert_fill[rb->sectorsize](rb, data, step1);
	rb->wp += (step1 * rb->fifowidth);
	
	if (step2 != 0)
	{
		rb->wp = 0;
		s_internal_convert_fill[rb->sectorsize](rb, data, step2);
		rb->wp = step2 * rb->fifowidth;
	}
	
	//Should wakeup the blocked read task here.
	//To do...
	
	printf("<%s> 1 rp=%d, wp=%d ret=%d\r\n", __FUNCTION__, rb->rp, rb->wp, (step1+step2));
	return (step1+step2);
}


static void internal_ringbuffer_read(RingBuffer *rb, uint32_t* data, uint32_t step1, uint32_t step2)
{
	int i = 0;
	uint32_t* pread = 0;
	
	pread = (uint32_t *)(rb->buf + rb->rp);
	
	if(step2 == 0)
	{
		while(i < step1)
		{
			*(data+i) = *(pread+i);
			i++;
		}
		rb->rp += step1*rb->fifowidth;
	}
	else
	{
		while(i < step1)
		{
			*(data+i) = *(pread+i);
			i++;
		}
		i = 0;
		pread = (uint32_t *)rb->buf;
		
		while(i < step2)
		{
			*(data+step1+i) = *(pread+i);
			i++;
		}
		
		rb->rp = (step2-1)*rb->fifowidth;
	}
	
	return;
}

/*
	return value:
	0 -- means empty.
	return value less than sectors means the buffer is empty now.
*/
uint32_t tools_rb_read(RingBuffer *rb, uint32_t* data, uint32_t sectors)
{
	uint32_t step1 = 0, step2 = 0, tofill = 0;
	uint32_t* pread = 0;
	
	if((rb == 0) || (data == 0))
		return 0;
	
	pread = (uint32_t *)(rb->buf + rb->rp);
	
	printf("<%s> 0 rp=%d, wp=%d sectors=%d\r\n", __FUNCTION__, rb->rp, rb->wp, sectors);
	if(rb->rp <= rb->wp)
	{
		tofill = (rb->wp - rb->rp)/rb->fifowidth;
		step1 = (tofill > sectors)?sectors:tofill;
		step2 = 0;
	}
	else
	{
		step1 = rb->limit - rb->rp/rb->fifowidth;
		step2 = rb->wp/rb->fifowidth + 1;
		tofill = step1 + step2;
		
		if(sectors < step1)
		{
			step1 = sectors;
			step2 = 0;
		}
		else if(sectors < tofill)
		{
			step2 = sectors - step1;
		}
	}
	
	internal_ringbuffer_read(rb, data, step1, step2);
	//Should wakeup the blocked write task here.
	//To do ...
	
	tofill = step1 + step2;
	
	if(tofill != 0)
		tools_dump_memory(data, tofill*rb->fifowidth);
	printf("<%s> 1 rp=%d, wp=%d ret=%d\r\n", __FUNCTION__, rb->rp, rb->wp, tofill);
	return (tofill);
}

uint32_t tools_rb_read_block(RingBuffer *rb, uint32_t* data, uint32_t sectors, uint32_t timeout)
{
	int ret = 0, readed = 0;
	
	if((rb == 0) || (data == 0))
		return 0;
		
	while(1)
	{
		readed = tools_rb_read(rb, data, sectors - ret);
		ret += readed;
		if(ret == sectors)
			break;
		
		/*task_sleep(); 
		  If the return value is Timeout, break. Otherwise continue to read.
		*/
	}	
	
	return ret;
}

uint32_t tools_rb_write_block(RingBuffer *rb, uint8_t* data, uint32_t sectors, uint32_t timeout)
{
	int ret = 0, sent = 0;

	if((rb == 0) || (data == 0))
		return 0;
		
	while(1)
	{
		sent = tools_rb_write(rb, data, sectors - ret);
		ret += sent;	
		if(ret == sectors)
			break;
		
		/*task_sleep(); 
		  If the return value is Timeout, break. Otherwise continue to write.
		*/
	}
	
	return ret;
}


#if (0)
#define tools_dump_memory

#else
void tools_dump_memory(uint8_t* data, uint32_t len)
{
    int i=0;
    static char tmpdata[100];
    char * cur=tmpdata;
    
    if (data==NULL)
        return;
    printf("data=0x%p, len=%d\r\n", data, len);
    /*                     0x64 0x35 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x11 0x22 0x33 0x44 0x55 0x66 0x08 0x06*/
    printf("----------------------------------------------------------------------------------\r\n");
    printf(" 0     1    2    3    4    5    6    7    8    9   10   11   12   13   14   15\r\n");
    for (i=0; i<len; i++)
    {
        //*cur = *(data+i);
        sprintf(cur, "0x%02X ", *(data+i));
        cur += 5;
        
        if(cur>(tmpdata+75))
        {
            sprintf(cur, "(%d)", (i/16+1));
            cur = tmpdata;
            printf("%s\r\n", tmpdata);
        }
    }
    if (cur != tmpdata)
    {
        *cur = 0;
        printf("%s\r\n", tmpdata);
    }

    printf("--------------------------------------\r\n");
    return;
}

#endif

