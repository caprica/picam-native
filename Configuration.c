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

#include "Configuration.h"

/**
 * Java type-names for the enumerations used in the Java camera configuration object.
 */
static const char ENUM_AUTOMATIC_WHITE_BALANCE_MODE[]       = "()Luk/co/caprica/picam/enums/AutomaticWhiteBalanceMode;";
static const char ENUM_DYNAMIC_RANGE_COMPRESSION_STRENGTH[] = "()Luk/co/caprica/picam/enums/DynamicRangeCompressionStrength;";
static const char ENUM_ENCODING[]                           = "()Luk/co/caprica/picam/enums/Encoding;";
static const char ENUM_EXPOSURE_METERING_MODE[]             = "()Luk/co/caprica/picam/enums/ExposureMeteringMode;";
static const char ENUM_EXPOSURE_MODE[]                      = "()Luk/co/caprica/picam/enums/ExposureMode;";
static const char ENUM_IMAGE_EFFECT[]                       = "()Luk/co/caprica/picam/enums/ImageEffect;";
static const char ENUM_MIRROR[]                             = "()Luk/co/caprica/picam/enums/Mirror;";
static const char ENUM_STEREOSCOPIC_MODE[]                  = "()Luk/co/caprica/picam/enums/StereoscopicMode;";

/**
 * Configuration context, used internally here to reduce parameter passing.
 */
typedef struct {
    JNIEnv *env;
    jclass  cls;
    jobject obj;
} ConfigContext;

static jobject getObjectValue(ConfigContext *context, const char *name, const char *type) {
    JNIEnv *env = context->env;
    jmethodID getter = (*env)->GetMethodID(env, context->cls, name, type);
    assert (getter != NULL);
    return (*env)->CallObjectMethod(env, context->obj, getter);
}

static jmethodID getValueMethod(ConfigContext *context, jobject value, const char *name, const char *type) {
    JNIEnv *env = context->env;
    jmethodID getter = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, value), name, type);
    assert (getter != NULL);
    return getter;
}

static int setInt(ConfigContext *context, const char *name, int32_t *result) {
    JNIEnv *env = context->env;
    jobject value = getObjectValue(context, name, "()Ljava/lang/Integer;");
    if (value == NULL) {
        return 0;
    }
    *result = (*env)->CallIntMethod(env, value, getValueMethod(context, value, "intValue", "()I"));
    return 1;
}

static int setBool(ConfigContext *context, const char *name, bool *result) {
    JNIEnv *env = context->env;
    jobject value = getObjectValue(context, name, "()Ljava/lang/Boolean;");
    if (value == NULL) {
        return 0;
    }
    *result = (*env)->CallBooleanMethod(env, value, getValueMethod(context, value, "booleanValue", "()Z"));
    return 1;
}

static int setFloat(ConfigContext *context, const char *name, float *result) {
    JNIEnv *env = context->env;
    jobject value = getObjectValue(context, name, "()Ljava/lang/Float;");
    if (value == NULL) {
        return 0;
    }
    *result = (*env)->CallFloatMethod(env, value, getValueMethod(context, value, "floatValue", "()F"));
    return 1;
}

static int setDouble(ConfigContext *context, const char *name, double *result) {
    JNIEnv *env = context->env;
    jobject value = getObjectValue(context, name, "()Ljava/lang/Double;");
    if (value == NULL) {
        return 0;
    }
    *result = (*env)->CallDoubleMethod(env, value, getValueMethod(context, value, "doubleValue", "()D"));
    return 1;
}

/**
 * Set a configuration value from a Java enumeration.
 * 
 * If the value retrieved from the Java object is NULL then the configuration will be skipped.
 * 
 * The Java enumeration must have a method named "value" that returns an integer.
 * 
 * @param context
 * @param name
 * @param result
 * @param type
 */
static int setEnum(ConfigContext *context, const char *name, int *result, const char *type) {
    JNIEnv *env = context->env;
    jobject value = getObjectValue(context, name, type);
    if (value == NULL) {
        return 0;
    }
    *result = (*env)->CallIntMethod(env, value, getValueMethod(context, value, "value", "()I"));
    return 1;
}

#define setUInt setInt

/**
 * Extract the configuration values from the Java CameraConfiguration object instance into the context structure.
 *
 * Default values in the context will be overwritten with any non-null values retrieved from the Java object. If
 * the corresponding value in the Java object is null, it will be skipped.
 *
 * No range-checking is performed here, as it is all done by the Java side.
 *
 * @param env
 * @param obj
 * @param config structure to fill with configuration values
 */
void extractConfiguration(JNIEnv *env, jobject obj, PicamConfig *config) {

    ConfigContext context = {
        env,
        (*env)->GetObjectClass(env, obj),
        obj
    };

    setInt   (&context, "cameraNumber"                   , &config->camera.cameraNumber                                                             );
    setUInt  (&context, "customSensorConfig"             , &config->camera.customSensorConfig                                                       );
    setUInt  (&context, "width"                          , &config->camera.width                                                                    );
    setUInt  (&context, "height"                         , &config->camera.height                                                                   );
    setUInt  (&context, "captureTimeout"                 , &config->camera.captureTimeout                                                           );

    setInt   (&context, "brightness"                     , &config->control.brightness                                                              );
    setInt   (&context, "contrast"                       , &config->control.contrast                                                                );
    setInt   (&context, "saturation"                     , &config->control.saturation                                                              );
    setInt   (&context, "sharpness"                      , &config->control.sharpness                                                               );
    setBool  (&context, "videoStabilisation"             , &config->control.videoStabilisation                                                      );
    setUInt  (&context, "shutterSpeed"                   , &config->control.shutterSpeed                                                            );
    setUInt  (&context, "iso"                            , &config->control.iso                                                                     );
    setEnum  (&context, "exposureMode"                   , &config->control.exposureMode                   , ENUM_EXPOSURE_MODE                     );
    setEnum  (&context, "exposureMeteringMode"           , &config->control.exposureMeteringMode           , ENUM_EXPOSURE_METERING_MODE            );
    setInt   (&context, "exposureCompensation"           , &config->control.exposureCompensation                                                    );
    setEnum  (&context, "dynamicRangeCompressionStrength", &config->control.dynamicRangeCompressionStrength, ENUM_DYNAMIC_RANGE_COMPRESSION_STRENGTH);
    setEnum  (&context, "automaticWhiteBalanceMode"      , &config->control.automaticWhiteBalanceMode      , ENUM_AUTOMATIC_WHITE_BALANCE_MODE      );
    setFloat (&context, "automaticWhiteBalanceRedGain"   , &config->control.automaticWhiteBalanceRedGain                                            );
    setFloat (&context, "automaticWhiteBalanceBlueGain"  , &config->control.automaticWhiteBalanceBlueGain                                           );
    setEnum  (&context, "imageEffect"                    , &config->control.imageEffect                    , ENUM_IMAGE_EFFECT                      );
    setBool  (&context, "colourEffect"                   , &config->control.colourEffect                                                            );
    setInt   (&context, "u"                              , &config->control.u                                                                       );
    setInt   (&context, "v"                              , &config->control.v                                                                       );
    setDouble(&context, "cropX"                          , &config->control.cropX                                                                   );
    setDouble(&context, "cropY"                          , &config->control.cropY                                                                   );
    setDouble(&context, "cropW"                          , &config->control.cropW                                                                   );
    setDouble(&context, "cropH"                          , &config->control.cropH                                                                   );

    setEnum  (&context, "stereoscopicMode"               , &config->capture.stereoscopicMode               , ENUM_STEREOSCOPIC_MODE                 );
    setBool  (&context, "decimate"                       , &config->capture.decimate                                                                );
    setBool  (&context, "swapEyes"                       , &config->capture.swapEyes                                                                );
    setEnum  (&context, "mirror"                         , &config->capture.mirror                         , ENUM_MIRROR                            );
    setInt   (&context, "rotation"                       , &config->capture.rotation                                                                );
    setEnum  (&context, "encoding"                       , &config->encoder.encoding                       , ENUM_ENCODING                          );
    setUInt  (&context, "quality"                        , &config->encoder.quality                                                                 );
}
