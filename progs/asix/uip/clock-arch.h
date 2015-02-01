#ifndef __CLOCK_ARCH_H__
#define __CLOCK_ARCH_H__

#define OMAP4460 // FIXME: assume OMAP4460 for now

/* 32-kHz sync timer counter */
#ifdef OMAP3530
#define TIMER_ADDR 0x48320010
#endif
#ifdef OMAP4460
#define TIMER_ADDR 0x4A304010
#endif

typedef int clock_time_t;
#define CLOCK_CONF_SECOND (1<<15)

extern clock_time_t clock_time(void);

#endif /* __CLOCK_ARCH_H__ */
