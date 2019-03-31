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

#include <assert.h>

#include "uk_co_caprica_picam_Camera.h"

#include "Camera.h"
#include "Configuration.h"
#include "Defaults.h"
#include "Encoder.h"
#include "Picam.h"

#include "interface/mmal/util/mmal_util_params.h"

#define REQUIRED_JNI_VERSION JNI_VERSION_1_6

static void jniThreadDestructor(void *env);
static void setupJniContext(JNIEnv *env, jobject handler);
static void cleanupJniContext(JNIEnv *env);
static uint32_t pictureDataCallback(uint8_t *data, uint32_t length);
static void cleanup(JNIEnv *env);

/**
 * Global state pertaining to JNI.
 */
static struct {
    JavaVM        *jvm;
    pthread_key_t threadKey;
    jobject       handler;
    jclass        handlerClass;
    jmethodID     beginMethod;
    jmethodID     pictureDataMethod;
    jmethodID     endMethod;
} JniContext;

/**
 * Global state.
 */
static PicamContext context;

/**
 * JNI library initialisation, invoked once when the native library is loaded.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    // Stash the JVM reference - this is safe to keep and use across any thread
    JniContext.jvm = jvm;

    // Create a thread-key (for all threads in the application) and attach a destructor function
    // that will be executed when a thread is destroyed (if that thread has set a value for this
    // key - the destruction function is used to detach the previously attached Java thread
    if (pthread_key_create(&(JniContext.threadKey), jniThreadDestructor) < 0) {
        printf("failed to create threadkey\n");
        return 0;
    }

    context.pictureDataCallback = &pictureDataCallback;

    return REQUIRED_JNI_VERSION;
}

/**
 * Create all of the native resources necessary for using the camera.
 *
 * @param env JNI environment
 * @param obj camera object reference
 * @param configurationObj camera configuration object reference, may be NULL
 */
JNIEXPORT jboolean JNICALL Java_uk_co_caprica_picam_Camera_create(JNIEnv *env, jobject cameraObj, jobject configurationObj) {
    setConfigurationDefaults(&context.config);

    if (configurationObj) {
        extractConfiguration(env, configurationObj, &context.config);
    }

    if (VCOS_SUCCESS != vcos_semaphore_create(&context.captureFinishedSemaphore, "picam-capture-finished", 0)) {
        goto error;
    }

    if (!createCamera(&context)) {
        goto error;
    }

    if (!createEncoder(&context)) {
        goto error;
    }

    return true;

error:
    cleanup(env);
    return false;
}

/**
 * Capture a picture.
 * 
 * The camera must previously have been "opened" - this is enforced on the Java side.
 * 
 * @param env JNI environment
 * @param obj camera object reference
 * @param handler picture capture handler object reference
 * @param delay
 * @return true if the capture was successfully triggered; false if it was not
 * @throws IllegalArgumentException if handler is null
 */
JNIEXPORT jboolean JNICALL Java_uk_co_caprica_picam_Camera_capture(JNIEnv *env, jobject obj, jobject handler, jint delay) {
    if (!handler) {
        // FIXME capturefailedexception?
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/IllegalArgumentException"), "Handler must not be null");
        return false;
    }

    // Make sure the JNI global state is property initialised to reflect the supplied handler
    // object (existing JNI object references will be used where possible)
    setupJniContext(env, handler);

    if (delay > 0) {
        vcos_sleep(delay);
    }

    // PictureCaptureHandler#begin():void
    (*env)->CallNonvirtualVoidMethod(env, JniContext.handler, JniContext.handlerClass, JniContext.beginMethod);
    if ((*env)->ExceptionCheck(env)) {
        // Caller will see the thrown exception, not this return value
        return false;
    }

    // FIXME probably i should just throw CaptureFailedException rather than have this method return a boolean
    bool captureSuccess = false;

    if (mmal_port_parameter_set_boolean(context.cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT], MMAL_PARAMETER_CAPTURE, 1) == MMAL_SUCCESS) {
        // printf("Waiting for completion semaphore\n");
        // fflush(stdout);

        if (context.config.camera.captureTimeout > 0) {
            // printf("Waiting for semaphore timed %d\n", context.config.camera.captureTimeout); fflush(stdout);
            // FIXME remember that note in Raspistill.c about this sometimes returning bad parameter, should pick that out and at least report/log differently so we can see
            captureSuccess = vcos_semaphore_wait_timeout(&context.captureFinishedSemaphore, context.config.camera.captureTimeout) == VCOS_SUCCESS;
        } else {
            // printf("Waiting for untimed semaphore"); fflush(stdout);
            vcos_semaphore_wait(&context.captureFinishedSemaphore);
            captureSuccess = true;
        }

        // printf("capture success was %d\n", captureSuccess); fflush(stdout);

        // printf("Got completion semaphore\n"); fflush(stdout);
        // fflush(stdout);
    }

    // PictureCaptureHandler#end():void
    (*env)->CallNonvirtualVoidMethod(env, JniContext.handler, JniContext.handlerClass, JniContext.endMethod);
    if ((*env)->ExceptionCheck(env)) {
        // Caller will see the thrown exception, not this return value
        return false;
    }

    // FIXME 1. exception is not being raised on Java side
    //       2. we are not stopping the callback from running, so there's a potential race of the callback going back into pictureData after the timeout
    if (!captureSuccess) {
        printf("Throwing CaptureFailedException\n"); fflush(stdout);
        (*env)->ThrowNew(env, (*env)->FindClass(env, "uk/co/caprica/picam/CaptureFailedException"), "Arse");
        printf("After Throwing CaptureFailedException\n"); fflush(stdout);
    // JavaVM *jvm = JniContext.jvm;
    // (*jvm)->DetachCurrentThread(jvm);
    }

    // FIXME even though this method returns, the exception does not arrive in the Java side until the fake sleep in the callback finishes - i.e. until the callback finishes execution, which in the error case it never will!
    //         WTF is going on?

    return (jboolean) captureSuccess;
}

/**
 * Clean up the camera and all associated resources.
 * 
 * @param env JNI environment
 * @param obj camera object reference
 */
JNIEXPORT void JNICALL Java_uk_co_caprica_picam_Camera_destroy(JNIEnv *env, jobject obj) {
    cleanup(env);
}

// === Private implementation =====================================================================

/**
 * Native thread destructor.
 * 
 * Detaches the thread that was attached earlier for the native callback.
 */
static void jniThreadDestructor(void *env) {
    JavaVM *jvm = JniContext.jvm;
    (*jvm)->DetachCurrentThread(jvm);
}

/**
 * Initialise the global JNI context, used to cache various JNI object references.
 * 
 * A JNI global ref must be created for the handler object and its class as we need to keep valid
 * references for use in the native callback thread.
 * 
 * Method ids can be safely kept across threads.
 * 
 * The context must be re-initialised each time the handler instance changes.
 * 
 * @param env JNI environment
 * @param handler picture capture handler object reference
 */
static void setupJniContext(JNIEnv *env, jobject handler) {
    jclass handlerClass = (*env)->GetObjectClass(env, handler);

    if (JNI_TRUE != (*env)->IsSameObject(env, handler, JniContext.handler)) {

        if (JNI_TRUE != (*env)->IsSameObject(env, handlerClass, JniContext.handlerClass)) {
            JniContext.beginMethod       = (*env)->GetMethodID(env, handlerClass, "begin"      , "()V"  );
            JniContext.pictureDataMethod = (*env)->GetMethodID(env, handlerClass, "pictureData", "([B)I");
            JniContext.endMethod         = (*env)->GetMethodID(env, handlerClass, "end"        , "()V"  );

            assert(JniContext.beginMethod       != NULL);
            assert(JniContext.pictureDataMethod != NULL);
            assert(JniContext.endMethod         != NULL);
        }

        // Now we can delete the old global handler and handler class refs before creating new ones
        cleanupJniContext(env);

        JniContext.handler      = (*env)->NewGlobalRef(env, handler     );
        JniContext.handlerClass = (*env)->NewGlobalRef(env, handlerClass);
    }
}

/**
 * Delete the handler and handler class global references from the global JNI context.
 * 
 * @param env JNI environment
 */
static void cleanupJniContext(JNIEnv *env) {
    if (JniContext.handler) {
        (*env)->DeleteGlobalRef(env, JniContext.handler);
    }

    if (JniContext.handlerClass) {
        (*env)->DeleteGlobalRef(env, JniContext.handlerClass);
    }

    JniContext.handler = JniContext.handlerClass = NULL;
}

static uint32_t pictureDataCallback(uint8_t *data, uint32_t length) {
    JavaVM *jvm = JniContext.jvm;
    JNIEnv *env = NULL;

    // Get a JNI environment for this thread
    if (JNI_EDETACHED == (*jvm)->GetEnv(jvm, (void**) &env, REQUIRED_JNI_VERSION)) {
        // There is no JNI environment available, meaning there is no current thread attached, so
        // attach the thread
        if (JNI_OK == (*jvm)->AttachCurrentThread(jvm, (void**) &env, NULL)) {
            // A non-NULL thread-key value must be set for the thread destructor to run later, so
            // set a value for the key (if one is not already set)
            if (!pthread_getspecific(JniContext.threadKey)) {
                if (pthread_setspecific(JniContext.threadKey, env)) {
                    (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "Failed to set native thread key");
                    return -1;
                }
            }
        } else {
            (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "Failed to attach thread");
            return -1;
        }
    }

    jbyteArray array = (*env)->NewByteArray(env, length);
    (*env)->SetByteArrayRegion(env, array, 0, length, (jbyte *) data);

    // PictureCaptureHandler#pictureData(byte[]):int
    jint written = (*env)->CallNonvirtualIntMethod(env, JniContext.handler, JniContext.handlerClass, JniContext.pictureDataMethod, array);

    (*env)->DeleteLocalRef(env, array);

    if ((*env)->ExceptionCheck(env)) {
        return -1;
    }

    return written;
}

static void cleanup(JNIEnv *env) {
    destroyCamera (&context);
    destroyEncoder(&context);

    vcos_semaphore_delete(&context.captureFinishedSemaphore);

    cleanupJniContext(env);
}
