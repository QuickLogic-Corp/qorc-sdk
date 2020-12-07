#ifndef SENSOR_GENERIC_H
#define SENSOR_GENERIC_H

typedef struct st_sensor_generic_config {
    int      is_running;
    int      enabled;
    uint32_t rate_hz;
    uint32_t sensor_id;
    int      n_channels;
    int      bit_depth;
} sensor_generic_config_t;

#endif /* SENSOR_GENERIC_H */
