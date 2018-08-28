TOPDIR	:= $(shell pwd)
INCDIR	:= -I $(TOPDIR)/include/ 
LIBDIR	:= -ljpeg

CROSS_COMPILE	?= arm-linux-
CC	:= $(CROSS_COMPILE)gcc
CFLAGS	+= -Wall $(INCDIR)
LFLAGS	+= $(LIBDIR)
LFLAGS	+= -ljpeg

export	CC CFLAGS

SRCS	:= $(wildcard *.c)
OBJS	:= $(patsubst %.c, %.o, $(SRCS))
DPES	:= $(patsubst %.o, %.d, $(OBJS))

APPS	:= app2



all:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LFLAGS) -o $(APPS)

clean:
	$(RM) $(OBJS) $(APPS) *.d*

.PHONY: clean

sinclude $(SRCS:.c=.d) 

%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,/($*/).o[ :]*,/1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
