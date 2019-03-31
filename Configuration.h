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

#ifndef _PICAM_CONFIGURATION_H
#define _PICAM_CONFIGURATION_H

#include <jni.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Configuration pertaining to the camera itself.
 */
typedef struct CameraConfig {
    int32_t  cameraNumber;
    uint32_t customSensorConfig;
    uint32_t width;
    uint32_t height;
    uint32_t captureTimeout;
} CameraConfig;

/**
 * Configuration pertaining to the camera control port.
 */
typedef struct ControlConfig {
    int32_t  brightness;
    int32_t  contrast;
    int32_t  saturation;
    int32_t  sharpness;
    bool     videoStabilisation;
    uint32_t shutterSpeed;
    uint32_t iso;
    int32_t  exposureMode;
    int32_t  exposureMeteringMode;
    int32_t  exposureCompensation;
    int32_t  dynamicRangeCompressionStrength;
    int32_t  automaticWhiteBalanceMode;
    float    automaticWhiteBalanceRedGain;
    float    automaticWhiteBalanceBlueGain;
    int32_t  imageEffect;
    bool     colourEffect;
    int32_t  u;
    int32_t  v;
    double   cropX;
    double   cropY;
    double   cropW;
    double   cropH;
} ControlConfig;

/**
 * Configuration pertaining to the camera capture port.
 */
typedef struct CaptureConfig {
    int32_t stereoscopicMode;
    bool    decimate;
    bool    swapEyes;
    int32_t mirror;
    int32_t rotation;
} CaptureConfig;

/**
 * Configuration pertaining to the image encoder.
 */
typedef struct EncoderConfig {
    int32_t  encoding;
    uint32_t quality;
} EncoderConfig;

/**
 * Configuration;
 */
typedef struct PicamConfig {
    CameraConfig  camera;
    ControlConfig control;
    CaptureConfig capture;
    EncoderConfig encoder;
} PicamConfig;

void extractConfiguration(JNIEnv *env, jobject obj, PicamConfig *config);

#endif // _PICAM_CONFIGURATION_H