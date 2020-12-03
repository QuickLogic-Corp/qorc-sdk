#ifndef __SENSIML_RECOGNITION_RUN_H__
#define __SENSIML_RECOGNITION_RUN_H__

#ifdef __cplusplus
extern "C" {
#endif

int sml_recognition_run_batch(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id);
int sml_recognition_run_single(signed short *data, uint32_t sensor_id);

#ifdef __cplusplus
}
#endif

#endif //__SENSIML_RECOGNITION_RUN_H__
