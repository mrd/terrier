NAME = ehci
C_FILES =
S_FILES =
DATS_FILES = ehci.dats

include ../../config.mk
LDSCRIPT = ../../ldscripts/program.ld
LIBS = -luser

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
endif

CFLAGS += $(USER_CFLAGS) -Wno-attributes
SFLAGS +=
LDFLAGS += -r $(USER_LDFLAGS)

ATSFLAGS += -IATS ../../include
ATSCFLAGS += -D_ATS_HEADER_NONE -D_ATS_PRELUDE_NONE -I$$ATSHOME -Wno-unused-function -Wno-unused-label

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
S_OBJS = $(patsubst %.S,%.o,$(S_FILES))
DATS_OBJS = $(patsubst %.dats,%_dats.o,$(DATS_FILES))
ATSCTEMPS = $(patsubst %.dats,%_dats.c,$(DATS_FILES))
OBJS = $(C_OBJS) $(S_OBJS) $(DATS_OBJS)
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(S_FILES)) $(patsubst %.dats,%_dats.d,$(DATS_FILES))

program: $(NAME).elf
	$(LD) -r -b binary $< -o $@

$(NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) $(OBJS) -o $@ $(LIBS)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

%_dats.o: %.dats
	$(ATSOPT) $(ATSFLAGS) --output $(patsubst %.dats,%_dats.c,$<) --dynamic $<
	$(CC) $(ATSCFLAGS) $(CFLAGS) -o $@ -c $(patsubst %.dats,%_dats.c,$<)

.PHONY: clean distclean
clean:
	rm -f $(OBJS) $(ATSCTEMPS) program $(NAME).elf
distclean: clean
	rm -f $(DFILES)

-include $(DFILES)
