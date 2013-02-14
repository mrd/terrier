NAME = uart
C_FILES = uart.c
S_FILES =

include ../../config.mk
LDSCRIPT = ../../ldscripts/program.ld
LIBS = -luser

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
endif

CFLAGS += $(USER_CFLAGS)
SFLAGS +=
LDFLAGS += -r $(USER_LDFLAGS)

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
S_OBJS = $(patsubst %.S,%.o,$(S_FILES))
OBJS = $(C_OBJS) $(S_OBJS)
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(S_FILES))

program: $(NAME).elf
	$(LD) -r -b binary $< -o $@

$(NAME).elf: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) $(OBJS) -o $@ $(LIBS)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

.PHONY: clean distclean
clean:
	rm -f $(OBJS) program $(NAME).elf
distclean: clean
	rm -f $(DFILES)

-include $(DFILES)
