#pragma once

struct fan_ctrl;

struct fan_ctrl_ops {
    int (*write_fan_speed)(void *, const u8);
};

struct fan_point {
    int temp;
    u8 speed;
};

struct fan_curve {
    const struct fan_point *points;
    size_t num_points;
};

#ifdef CONFIG_SYNO_WRAPPER_FANCTL
void fanctrl_start(struct fan_ctrl *fc);

// Unimplemented because lazy
static inline void fanctrl_stop(struct fan_ctrl *fc) {};

struct fan_ctrl *fanctrl_create(void *priv,
    const struct fan_ctrl_ops *ops,
    const struct fan_curve *curve,
    const char *thermal_zone);

void fanctrl_cleanup(struct fan_ctrl *fc);

#else
static inline struct fan_ctrl *
fanctrl_create(void *priv,
    const struct fan_ctrl_ops *ops,
    const struct fan_curve *curve,
    const char *thermal_zone)
{
    return nullptr;
}

static inline void fanctrl_start(struct fan_ctrl *fc) {};

static inline void fanctrl_stop(struct fan_ctrl *fc) {};

static inline void fanctrl_cleanup(struct fan_ctrl *fc) {};
#endif // CONFIG_SYNO_WRAPPER_FANCTL