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

#include "Encoder.h"

#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"

static int createPicturePool(PicamContext *context);
static int connectCameraToEncoder(PicamContext *context);
static int sendBuffersToEncoder(PicamContext *context);

static void encoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

/**
 * Create an Encoder component.
 *
 * @param context
 * @return non-zero on success; zero on error
 */
int createEncoder(PicamContext *context) {
    if (MMAL_SUCCESS != mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &context->encoderComponent)) {
        return 0;
    }

    MMAL_PORT_T *encoderInputPort  = context->encoderComponent->input [0];
    MMAL_PORT_T *encoderOutputPort = context->encoderComponent->output[0];

    mmal_format_copy(encoderOutputPort->format, encoderInputPort->format);

    encoderOutputPort->format->encoding = context->config.encoder.encoding;

    encoderOutputPort->buffer_size = encoderOutputPort->buffer_size_recommended;
    if (encoderOutputPort->buffer_size < encoderOutputPort->buffer_size_min) {
        encoderOutputPort->buffer_size = encoderOutputPort->buffer_size_min;
    }

    encoderOutputPort->buffer_num = encoderOutputPort->buffer_num_recommended;
    if (encoderOutputPort->buffer_num < encoderOutputPort->buffer_num_min) {
        encoderOutputPort->buffer_num = encoderOutputPort->buffer_num_min;
    }

    if (MMAL_SUCCESS != mmal_port_format_commit(encoderOutputPort)) {
        return 0;
    }


    if (MMAL_SUCCESS != mmal_port_parameter_set_uint32(encoderOutputPort, MMAL_PARAMETER_JPEG_Q_FACTOR, context->config.encoder.quality)) {
        return 0;
    }

    if (MMAL_SUCCESS != mmal_component_enable(context->encoderComponent)) {
        return 0;
    }

    if (!createPicturePool(context)) {
        return 0;
    }

    if (!connectCameraToEncoder(context)) {
        return 0;
    }

    encoderOutputPort->userdata = (struct MMAL_PORT_USERDATA_T *) context;

    if (MMAL_SUCCESS != mmal_port_enable(encoderOutputPort, encoderBufferCallback)) {
        return 0;
    }

    if (!sendBuffersToEncoder(context)) {
        return 0;
    }

    return 1;
}

void destroyEncoder(PicamContext *context) {
    if (context->encoderComponent) {
        mmal_component_disable(context->encoderComponent);

        MMAL_PORT_T *encoderOutputPort = context->encoderComponent->output[0];
        if (encoderOutputPort->is_enabled) {
            mmal_port_disable(encoderOutputPort);
        }

        if (context->picturePool) {
            mmal_port_pool_destroy(context->encoderComponent->output[0], context->picturePool);
            context->picturePool = NULL;
        }

        mmal_component_destroy(context->encoderComponent);
        context->encoderComponent = NULL;
    }
}

static int createPicturePool(PicamContext *context) {
    MMAL_PORT_T *encoderOutputPort = context->encoderComponent->output[0];

    MMAL_POOL_T *picturePool = mmal_port_pool_create(encoderOutputPort, encoderOutputPort->buffer_num, encoderOutputPort->buffer_size);

    if (!picturePool) {
        return 0;
    }

    context->picturePool = picturePool;

    return 1;
}

static int connectCameraToEncoder(PicamContext *context) {
    MMAL_PORT_T *cameraCapturePort = context->cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT];
    MMAL_PORT_T *encoderInputPort  = context->encoderComponent->input[0];

    if (MMAL_SUCCESS != mmal_connection_create(&context->cameraEncoderConnection, cameraCapturePort, encoderInputPort, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT)) {
        return 0;
    }

    if (MMAL_SUCCESS != mmal_connection_enable(context->cameraEncoderConnection)) {
        return 0;
    }

    return 1;
}

static int sendBuffersToEncoder(PicamContext *context) {
    MMAL_PORT_T *encoderOutputPort = context->encoderComponent->output[0];

    unsigned int bufferCount = mmal_queue_length(context->picturePool->queue);

    for (unsigned int i = 0; i < bufferCount; i++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(context->picturePool->queue);

        if (buffer == NULL) {
            return 0;
        }

        if (MMAL_SUCCESS != mmal_port_send_buffer(encoderOutputPort, buffer)) {
            return 0;
        }
    }

    return 1;
}

/**
 * Encoder buffer callback.
 *
 * Process the picture data supplied by the image encoder.
 *
 * Note that when cleaning up, this callback will be invoked with a buffer length of zero, and
 * buffer flags of zero. The implemntation handles this scenario safely.
 *
 * @param port
 * @param buffer
 */
static void encoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    bool finished = false;
    int written = 0;

    PicamContext *context = (PicamContext *) port->userdata;

    if (buffer->length) {
        mmal_buffer_header_mem_lock(buffer);
        // looks like we don't need to worry about buffer->offset
        written = context->pictureDataCallback(buffer->data, buffer->length);
        mmal_buffer_header_mem_unlock(buffer);
    }

    if (written != buffer->length) {
        finished = true;
    }

    if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED)) {
        finished = true;
    }

    mmal_buffer_header_release(buffer);

    if (port->is_enabled) {
        MMAL_BUFFER_HEADER_T *nextBuffer = mmal_queue_get(context->picturePool->queue);
        if (nextBuffer) {
            mmal_port_send_buffer(port, nextBuffer);
        }
    }

    if (finished) {
        vcos_semaphore_post(&context->captureFinishedSemaphore);
    }
}
