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

#include "Camera.h"
#include "Port.h"

#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"

#define ALIGN_WIDTH  32
#define ALIGN_HEIGHT 16

#define STILLS_FRAME_RATE_NUM 0
#define STILLS_FRAME_RATE_DEN 1

static void cameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
static int applyCameraPreConfiguration(PicamContext *context);
static int applyCameraConfiguration(PicamContext *context);
static int applyCameraCapturePortFormat(PicamContext *context);

/**
 * Create a camera component.
 *
 * @param context global state
 * @return non-zero on success; zero on error
 */
int createCamera(PicamContext *context) {
    if (MMAL_SUCCESS != mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &context->cameraComponent)) {
        return 0;
    }

    if (!applyCameraPreConfiguration(context)) {
        return 0;
    }

    if (MMAL_SUCCESS != mmal_port_enable(context->cameraComponent->control, cameraControlCallback)) {
        return 0;
    }

    if (!applyCameraConfiguration(context)) {
        return 0;
    }

    if (!applyCameraCapturePortFormat(context)) {
        return 0;
    }

    if (MMAL_SUCCESS != mmal_component_enable(context->cameraComponent)) {
        return 0;
    }

    return 1;
}

/**
 * Destroy the camera component and all associated resources.
 *
 * It is safe to call this method no matter what the state is.
 *
 * @param context global state
 */
void destroyCamera(PicamContext *context) {
    if (context->cameraComponent) {
        mmal_component_disable(context->cameraComponent);
        mmal_component_destroy(context->cameraComponent);
        context->cameraComponent = NULL;
    }
}

// === Private implementation =====================================================================

static void cameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    mmal_buffer_header_release(buffer);
}

/**
 * Apply the necessary pre-configuration to select the camera (before the camera control port is
 * enabled).
 * 
 * @param context global state
 * @return non-zero if successful; zero on error
 */
static int applyCameraPreConfiguration(PicamContext *context) {
    MMAL_PORT_T   *controlPort = context->cameraComponent->control;
    MMAL_PORT_T   *capturePort = context->cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT];
    CameraConfig  *camera      = &context->config.camera;
    CaptureConfig *capture     = &context->config.capture;

    return
        setStereoscopicMode(capturePort, capture->stereoscopicMode, capture->decimate, capture->swapEyes) &&
        setInt32           (controlPort, MMAL_PARAMETER_CAMERA_NUM                 , camera->cameraNumber) &&
        setUInt32          (controlPort, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, camera->customSensorConfig);
}

/**
 * Apply the necessary camera configuration.
 * 
 * @param context global state
 * @return non-zero if successful; zero on error
 */
static int applyCameraConfiguration(PicamContext *context) {
    MMAL_PORT_T   *controlPort = context->cameraComponent->control;
    MMAL_PORT_T   *capturePort = context->cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT];
    CameraConfig  *camera      = &context->config.camera;
    ControlConfig *control     = &context->config.control;
    CaptureConfig *capture     = &context->config.capture;

    return
        setCameraConfig              (controlPort, camera->width, camera->height) &&

        setRational                  (controlPort, MMAL_PARAMETER_BRIGHTNESS         , control->brightness, 100) &&
        setRational                  (controlPort, MMAL_PARAMETER_CONTRAST           , control->contrast, 100) &&
        setRational                  (controlPort, MMAL_PARAMETER_SATURATION         , control->saturation, 100) &&
        setRational                  (controlPort, MMAL_PARAMETER_CONTRAST           , control->sharpness, 100) &&
        setBoolean                   (controlPort, MMAL_PARAMETER_VIDEO_STABILISATION, control->videoStabilisation) &&
        setUInt32                    (controlPort, MMAL_PARAMETER_SHUTTER_SPEED      , control->shutterSpeed) &&
        setUInt32                    (controlPort, MMAL_PARAMETER_ISO                , control->iso) &&
        setExposureMode              (controlPort                                    , control->exposureMode) &&
        setExposureMeteringMode      (controlPort                                    , control->exposureMeteringMode) &&
        setInt32                     (controlPort, MMAL_PARAMETER_EXPOSURE_COMP      , control->exposureCompensation) &&
        setDynamicRangeCompression   (controlPort                                    , control->dynamicRangeCompressionStrength) &&
        setAutomaticWhiteBalanceMode (controlPort                                    , control->automaticWhiteBalanceMode) &&
        setAutomaticWhiteBalanceGains(controlPort                                    , control->automaticWhiteBalanceRedGain, control->automaticWhiteBalanceBlueGain) &&
        setImageEffect               (controlPort                                    , control->imageEffect) &&
        setColourEffect              (controlPort                                    , control->colourEffect, control->u, control->v) &&
        setCrop                      (controlPort                                    , control->cropX, control->cropY, control->cropW, control->cropH) &&

        setMirror                    (capturePort                                    , capture->mirror) &&
        setInt32                     (capturePort, MMAL_PARAMETER_ROTATION           , capture->rotation) &&
        setFpsRange                  (capturePort, control->shutterSpeed);
}

static int applyCameraCapturePortFormat(PicamContext *context) {
    uint32_t width  = context->config.camera.width;
    uint32_t height = context->config.camera.height;

    MMAL_PORT_T *cameraCapturePort = context->cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT];

    cameraCapturePort->format->encoding                 = MMAL_ENCODING_OPAQUE;
    cameraCapturePort->format->es->video.width          = VCOS_ALIGN_UP(width, ALIGN_WIDTH);
    cameraCapturePort->format->es->video.height         = VCOS_ALIGN_UP(height, ALIGN_HEIGHT);
    cameraCapturePort->format->es->video.crop.x         = 0;
    cameraCapturePort->format->es->video.crop.y         = 0;
    cameraCapturePort->format->es->video.crop.width     = width;
    cameraCapturePort->format->es->video.crop.height    = height;
    cameraCapturePort->format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
    cameraCapturePort->format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;

    return mmal_port_format_commit(cameraCapturePort) == MMAL_SUCCESS ? 1: 0;
}
