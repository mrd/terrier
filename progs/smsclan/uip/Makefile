include ../../../config.mk

CFLAGS =
UIP_SOURCES=uip.c uip_arp.c uiplib.c psock.c timer.c uip-neighbor.c

CCDEP = $(CC)

uip.a: $(UIP_SOURCES:.c=.o)
        $(AR) rcf $@ $^

$(OBJECTDIR)/%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

%.d: %.c
        $(CCDEP) -MM $(CCDEPCFLAGS) $< > $@

.PHONY: clean
clean:
        rm -f $(UIP_SOURCES:.c=.o) uip.a

ifneq ($(MAKECMDGOALS),clean)
-include $(UIP_SOURCES:.c=.d)
endif
