#ifndef _RINGBUF_H_
#define _RINGBUF_H_


#if 1
//typedef char int8_t;
typedef signed char sint8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef signed short sint16_t;
typedef unsigned short uint16_t; 
//typedef long int32_t; 
typedef signed int sint32_t;
typedef unsigned int uint32_t; 

#define RB_ENTER_CRITICAL_SECTION()	
#define RB_LEAVE_CRITICAL_SECTION()	

#endif


struct RingBuffer {
        uint8_t* buf;
        uint8_t  status;
        uint32_t limit;
        uint32_t sectorsize;
        uint32_t fifowidth;
        uint32_t rp,wp;
};
typedef struct RingBuffer RingBuffer;

void tools_rb_init(RingBuffer *rb, uint8_t* mem, uint32_t numsectors, uint32_t sectorsize);
uint32_t tools_rb_write(RingBuffer *rb, uint8_t* data, uint32_t sectors);
uint32_t tools_rb_read(RingBuffer *rb, uint32_t* data, uint32_t sectors);

uint32_t tools_rb_write_block(RingBuffer *rb, uint8_t* data, uint32_t sectors, uint32_t timeout);
uint32_t tools_rb_read_block(RingBuffer *rb, uint32_t* data, uint32_t sectors, uint32_t timeout);

void tools_dump_memory(uint8_t* data, uint32_t len);

#endif
