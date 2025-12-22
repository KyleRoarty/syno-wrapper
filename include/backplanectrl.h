#ifndef _SYNO_WRAPPER_BACKPLANECTRL_H_
#define _SYNO_WRAPPER_BACKPLANECTRL_H_

#include <linux/device.h>

struct bp_ctrl;

#ifdef CONFIG_SYNO_WRAPPER_BACKPLANECTRL

struct bp_ctrl *backplanectrl_create(struct device *dev, const char *sense_str,
				     const char *power_str);

void backplanectrl_start(struct bp_ctrl *bp);

void backplanectrl_cleanup(struct bp_ctrl *bp);

#else
#include <linux/stddef.h>

static inline struct bp_ctrl *backplanectrl_create(struct device *dev,
						   const char *sense_str,
						   const char *power_str)
{
	return NULL;
}

static inline void backplanectrl_start(struct bp_ctrl *bp) {};

static inline void backplanectrl_cleanup(struct bp_ctrl *bp) {};

#endif // CONFIG_SYNO_WRAPPER_BACKPLANECTRL
#endif // _SYNO_WRAPPER_BACKPLANECTRL_H_
