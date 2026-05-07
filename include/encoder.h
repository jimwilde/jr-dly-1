#include "miniaudio_utils.h"

typedef struct
{
  char *filename;
  ma_device *device;
  ma_encoder_config *encoderConfig;
  ma_encoder *encoder;
} cfg_encoder_args;

ma_result configure_encoder(cfg_encoder_args *args);