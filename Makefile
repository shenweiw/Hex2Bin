

	
SOURCE = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCE))
	
#CROSS_COMPILE = arm-linux-
CROSS_COMPILE =
CXX = gcc
TARGET = hex2bin

CFLAGS += -g  --static
CLFAGS += -Wall 
LDFLAGS += -lm -lpthread
	
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CROSS_COMPILE)$(CXX) -Wall -pg $(LDFLAGS) -o $@ $^

.PHONY:clean
clean:
	rm -f *.o *.d $(TARGET)

include $(SOURCE:.c=.d)	
	
%.o:%.c
	$(CROSS_COMPILE)$(CXX) -Wall -pg $(CFLAGS) -c $< -o $@

%.d: %.c
	@set -e; rm -f $@; \
	$(CXX) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

