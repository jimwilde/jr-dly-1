#include "encoder.h"

ma_result configure_encoder(cfg_encoder_args *args)
{
  *args->encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, ma_format_f32, args->device->capture.channels, args->device->sampleRate);
  return ma_encoder_init_file(args->filename, args->encoderConfig, args->encoder);
}
