#include "miniaudio_utils.h"

ma_result configure_encoder(
    char *filename,
    ma_device *device,
    ma_encoder_config *encoderConfig,
    ma_encoder *encoder)
{
  *encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, ma_format_f32, device->capture.channels, device->sampleRate);
  return ma_encoder_init_file(filename, encoderConfig, encoder);
}
