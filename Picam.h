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

#ifndef _PICAM_H
#define _PICAM_H

#include <stdint.h>

#include "Configuration.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_connection.h"

/**
 * Camera output port index for the capture (still) port, apparently not defined in mmal headers.
 */
#define MMAL_CAMERA_CAPTURE_PORT 2

/**
 * Global state.
 */
typedef struct PicamContext {

    PicamConfig        config; 

    MMAL_COMPONENT_T*  encoderComponent;
    MMAL_POOL_T*       picturePool;
    MMAL_COMPONENT_T*  cameraComponent;
    MMAL_CONNECTION_T* cameraEncoderConnection;

    VCOS_SEMAPHORE_T   captureFinishedSemaphore;

    uint32_t (*pictureDataCallback)(uint8_t*, uint32_t);

} PicamContext;

#endif // _PICAM_H