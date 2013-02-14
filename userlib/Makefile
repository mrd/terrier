include ../config.mk

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
endif

CFLAGS +=

C_OBJS = printf.o

USERLIB = libuser.a

STARTASM = start.S
STARTOBJ = start.o

USERLIBOBJS = $(STARTOBJ) $(C_OBJS)

$(USERLIB): $(USERLIBOBJS)
	$(AR) r $@ $(USERLIBOBJS)
	ranlib $@

$(STARTOBJ): $(STARTASM)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

.PHONY: clean distclean
clean:
	rm -f $(STARTOBJ) $(USERLIB) $(C_OBJS)
distclean:
	rm -f start.d
