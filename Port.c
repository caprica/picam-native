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

#include <stdbool.h>

#include "Port.h"

#include "interface/mmal/util/mmal_util_params.h"

int setCameraConfig(MMAL_PORT_T *port, uint32_t width, uint32_t height) {
    // Preview configuration must be set to something reasonable, even though preview is not used
    MMAL_PARAMETER_CAMERA_CONFIG_T param = {
        {MMAL_PARAMETER_CAMERA_CONFIG, sizeof(param)},
        .max_stills_w                          = width,
        .max_stills_h                          = height,
        .stills_yuv422                         = 0,
        .one_shot_stills                       = 1,
        .max_preview_video_w                   = 320,
        .max_preview_video_h                   = 240,
        .num_preview_video_frames              = 3,
        .stills_capture_circular_buffer_height = 0,
        .fast_preview_resume                   = 0,
        .use_stc_timestamp                     = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setStereoscopicMode(MMAL_PORT_T *port, int value, bool decimate, bool swapEyes) {
    MMAL_PARAMETER_STEREOSCOPIC_MODE_T param = {{MMAL_PARAMETER_STEREOSCOPIC_MODE, sizeof(param)}};
    if (value != MMAL_STEREOSCOPIC_MODE_NONE) {
        param.mode = value;
        param.decimate = decimate;
        param.swap_eyes = swapEyes;
    }
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setRational(MMAL_PORT_T *port, int id, int32_t num, int32_t den) {
    MMAL_RATIONAL_T param = {num, den};
    return mmal_port_parameter_set_rational(port, id, param) == MMAL_SUCCESS ? 1 : 0;
}

int setBoolean(MMAL_PORT_T *port, int id, bool value) {
    return mmal_port_parameter_set_boolean(port, id, value ? 1 : 0) == MMAL_SUCCESS ? 1 : 0;
}

int setUInt32(MMAL_PORT_T *port, int id, uint32_t value) {
    return mmal_port_parameter_set_uint32(port, id, value) == MMAL_SUCCESS ? 1 : 0;
}

int setInt32(MMAL_PORT_T *port, int id, int32_t value) {
    return mmal_port_parameter_set_int32(port, id, value) == MMAL_SUCCESS ? 1 : 0;
}

int setExposureMode(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_EXPOSUREMODE_T param = {{MMAL_PARAMETER_EXPOSURE_MODE, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setExposureMeteringMode(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_EXPOSUREMETERINGMODE_T param = {{MMAL_PARAMETER_EXP_METERING_MODE, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setDynamicRangeCompression(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_DRC_T param = {{MMAL_PARAMETER_DYNAMIC_RANGE_COMPRESSION, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1: 0;
}

int setAutomaticWhiteBalanceMode(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setAutomaticWhiteBalanceGains(MMAL_PORT_T *port, float red, float blue) {
    MMAL_PARAMETER_AWB_GAINS_T param = {{MMAL_PARAMETER_CUSTOM_AWB_GAINS,sizeof(param)}, {red * 65536, 65536}, {blue * 65536, 65536}};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setImageEffect(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_IMAGEFX_T param = {{MMAL_PARAMETER_IMAGE_EFFECT, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setColourEffect(MMAL_PORT_T *port, int value, int u, int v) {
    MMAL_PARAMETER_COLOURFX_T param = {{MMAL_PARAMETER_COLOUR_EFFECT, sizeof(param)}, value, u, v};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setMirror(MMAL_PORT_T *port, int value) {
    MMAL_PARAMETER_MIRROR_T param = {{MMAL_PARAMETER_MIRROR, sizeof(param)}, value};
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setCrop(MMAL_PORT_T *port, double x, double y, double w, double h) {
   MMAL_PARAMETER_INPUT_CROP_T param = {{MMAL_PARAMETER_INPUT_CROP, sizeof(param)}, {x *65536, y * 65536, w * 65536, h * 65536}};
   return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}

int setFpsRange(MMAL_PORT_T *port, uint32_t shutterSpeed) {
    MMAL_PARAMETER_FPS_RANGE_T param = {{MMAL_PARAMETER_FPS_RANGE, sizeof(param)}};
    if (shutterSpeed > 6000000) {
        param.fps_low.num = 5;
        param.fps_low.den = 1000;
        param.fps_high.num = 166;
        param.fps_high.den = 1000;
    } else if (shutterSpeed > 1000000) {
        param.fps_low.num = 167;
        param.fps_low.den = 1000;
        param.fps_high.num = 999;
        param.fps_high.den = 1000;
    }
    return mmal_port_parameter_set(port, &param.hdr) == MMAL_SUCCESS ? 1 : 0;
}
