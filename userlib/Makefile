include ../config.mk

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
endif

USERLIB = libuser.a

STARTASM = start.S
STARTOBJ = start.o

$(USERLIB): $(STARTOBJ)
	$(AR) r $@ $<
	ranlib $@

$(STARTOBJ): $(STARTASM)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

.PHONY: clean distclean
clean:
	rm -f $(STARTOBJ) $(USERLIB)
distclean:
	rm -f start.d
