# nwcli Makefile
CC     = gcc
CFLAGS = -Wall
TARGET = nw
OBJS   = nw_cli.o utils.o nw_rw_ioctl.o nw_peer_cmd.o nw_conf.o

.SUFFIXES: .c .o

.PHONY: all
all: depend $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET) depend.inc *~

.PHONY: depend
depend: $(OBJS:.o=.c)
	-@ $(RM) depend.inc
	-@ for i in $^; do cpp -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> depend.inc; done
-include depend.inc
