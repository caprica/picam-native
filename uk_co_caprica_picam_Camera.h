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

#ifndef _Included_uk_co_caprica_picam_Camera
#define _Included_uk_co_caprica_picam_Camera

#include <jni.h>

JNIEXPORT jboolean JNICALL Java_uk_co_caprica_picam_Camera_create(JNIEnv *, jobject, jobject);
JNIEXPORT jboolean JNICALL Java_uk_co_caprica_picam_Camera_capture(JNIEnv *, jobject, jobject, jint);
JNIEXPORT void JNICALL Java_uk_co_caprica_picam_Camera_destroy(JNIEnv *, jobject);

#endif