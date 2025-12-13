#pragma once

#ifdef CONFIG_SYNO_WRAPPER_FANCTL
static inline void enable_fanctrl(void) {}

#else
static inline void enable_fanctrl(void) {}
#endif // CONFIG_SYNO_WRAPPER_FANCTL