include ../config.mk

ATSFLAGS += -IATS ../include
ATSCFLAGS += -D_ATS_HEADER_NONE -D_ATS_PRELUDE_NONE -I$$ATSHOME -I../include -Iinclude -Wno-unused-function -Wno-unused-label

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
ATSCFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
ATSCFLAGS += -O$(OPT)
endif

CFLAGS +=

C_FILES = printf.c arm.c
DATS_FILES =

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
DATS_OBJS = $(patsubst %.dats,%_dats.o,$(DATS_FILES))
ATSCTEMPS = $(patsubst %.dats,%_dats.c,$(DATS_FILES))
STARTASM = start.S
STARTOBJ = start.o
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(STARTASM)) $(patsubst %.dats,%_dats.d,$(DATS_FILES))

USERLIB = libuser.a

USERLIBOBJS = $(C_OBJS) $(STARTOBJ) $(DATS_OBJS)

$(USERLIB): $(USERLIBOBJS)
	$(AR) r $@ $(USERLIBOBJS)
	ranlib $@

$(STARTOBJ): $(STARTASM)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

%_dats.o: %.dats
	$(ATSOPT) $(ATSFLAGS) --output $(patsubst %.dats,%_dats.c,$<) --dynamic $<
	$(CC) $(ATSCFLAGS) $(CFLAGS) -o $@ -c $(patsubst %.dats,%_dats.c,$<)

.PHONY: clean distclean
clean:
	rm -f $(STARTOBJ) $(USERLIB) $(C_OBJS) $(DATS_OBJS) $(ATSCTEMPS)
distclean:
	rm -f start.d
