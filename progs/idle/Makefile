NAME = idle
C_FILES = idle.c
S_FILES =

include ../../config.mk
LDSCRIPT = ../../ldscripts/program.ld

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
S_OBJS = $(patsubst %.S,%.o,$(S_FILES))
OBJS = $(C_OBJS) $(S_OBJS)
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(S_FILES))

program: $(NAME).elf
	$(LD) -r -b binary $< -o $@

$(NAME).elf: $(OBJS)
	$(LD) -T $(LDSCRIPT) $(OBJS) -o $@

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

.PHONY: clean distclean
clean:
	rm -f $(OBJS) program $(NAME).elf
distclean: clean
	rm -f $(DFILES)

-include $(DFILES)
