/*
 * This file is part of picam.
 *
 * picam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * picam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with picam.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2016-2019 Caprica Software Limited.
 */

#include "Defaults.h"

#include "interface/mmal/mmal.h"

void setConfigurationDefaults(PicamConfig* config) {
    config->camera.cameraNumber                     = 0;
    config->camera.customSensorConfig               = 0;
    config->camera.width                            = 2592;
    config->camera.height                           = 1944;
    config->camera.captureTimeout                   = 0;

    config->control.brightness                      = 50;
    config->control.contrast                        = 0;
    config->control.saturation                      = 0;
    config->control.sharpness                       = 0;
    config->control.videoStabilisation              = 0;
    config->control.shutterSpeed                    = 0;
    config->control.iso                             = 0;
    config->control.exposureMode                    = MMAL_PARAM_EXPOSUREMODE_AUTO;
    config->control.exposureMeteringMode            = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
    config->control.exposureCompensation            = 0;
    config->control.dynamicRangeCompressionStrength = MMAL_PARAMETER_DRC_STRENGTH_OFF;
    config->control.automaticWhiteBalanceMode       = MMAL_PARAM_AWBMODE_AUTO;
    config->control.automaticWhiteBalanceRedGain    = 0;
    config->control.automaticWhiteBalanceBlueGain   = 0;
    config->control.imageEffect                     = MMAL_PARAM_IMAGEFX_NONE;
    config->control.colourEffect                    = 0;
    config->control.u                               = 128;
    config->control.v                               = 128;
    config->control.cropX                           = 0.0;
    config->control.cropY                           = 0.0;
    config->control.cropW                           = 1.0;
    config->control.cropH                           = 1.0;

    config->capture.stereoscopicMode                = MMAL_STEREOSCOPIC_MODE_NONE;
    config->capture.decimate                        = MMAL_FALSE;
    config->capture.swapEyes                        = MMAL_FALSE;
    config->capture.rotation                        = 0;
    config->capture.mirror                          = MMAL_PARAM_MIRROR_NONE;

    config->encoder.encoding                        = MMAL_ENCODING_JPEG;
    config->encoder.quality                         = 85;
}
