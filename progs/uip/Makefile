NAME = uip
C_FILES = uip/clock-arch.c
S_FILES = uip_start.S
DATS_FILES = uip.dats

include ../../config.mk
LDSCRIPT = ../../ldscripts/program.ld
LIBS = -luser

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
endif

CFLAGS += $(USER_CFLAGS) -Wno-attributes -ffreestanding
SFLAGS +=
LDFLAGS += -r -d $(USER_LDFLAGS)

ATSFLAGS += -IATS ../../include -IATS ../../userlib
ATSCFLAGS += -I../../userlib -I../../include $(USER_ATSCFLAGS)

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
S_OBJS = $(patsubst %.S,%.o,$(S_FILES))
DATS_OBJS = $(patsubst %.dats,%_dats.o,$(DATS_FILES))
ATSCTEMPS = $(patsubst %.dats,%_dats.c,$(DATS_FILES))
OBJS = $(C_OBJS) $(S_OBJS) $(DATS_OBJS) uip/uip.a
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(S_FILES)) $(patsubst %.dats,%_dats.d,$(DATS_FILES))

program: $(NAME).elf
	$(LD) -r -b binary $< -o $@

$(NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) $(OBJS) -o $@ $(LIBS) $(USER_LIBS) $(LIBS)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

%_dats.o: %.dats
	$(ATSOPT) $(ATSFLAGS) --output $(patsubst %.dats,%_dats.c,$<) --dynamic $<
	$(CC) $(ATSCFLAGS) $(CFLAGS) -o $@ -c $(patsubst %.dats,%_dats.c,$<)

uip/uip.a:
	$(MAKE) -C uip

.PHONY: clean distclean
clean:
	rm -f $(OBJS) $(DFILES) $(ATSCTEMPS) program $(NAME).elf
#	$(MAKE) -C uip clean

distclean: clean
	rm -f $(DFILES)

-include $(DFILES)
