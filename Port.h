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

#ifndef _PICAM_PORT_H
#define _PICAM_PORT_H

#include "interface/mmal/mmal_port.h"

int setCameraConfig(MMAL_PORT_T *port, uint32_t width, uint32_t height);
int setStereoscopicMode(MMAL_PORT_T *port, int value, bool decimate, bool swapEyes);
int setRational(MMAL_PORT_T *port, int id, int32_t num, int32_t den);
int setBoolean(MMAL_PORT_T *port, int id, bool value);
int setUInt32(MMAL_PORT_T *port, int id, uint32_t value);
int setInt32(MMAL_PORT_T *port, int id, int32_t value);
int setExposureMode(MMAL_PORT_T *port, int value);
int setExposureMeteringMode(MMAL_PORT_T *port, int value);
int setDynamicRangeCompression(MMAL_PORT_T *port, int value);
int setAutomaticWhiteBalanceMode(MMAL_PORT_T *port, int value);
int setAutomaticWhiteBalanceGains(MMAL_PORT_T *port, float red, float blue);
int setImageEffect(MMAL_PORT_T *port, int value);
int setColourEffect(MMAL_PORT_T *port, int value, int u, int v);
int setMirror(MMAL_PORT_T *port, int value);
int setCrop(MMAL_PORT_T *port, double x, double y, double w, double h);
int setFpsRange(MMAL_PORT_T *port, uint32_t shutterSpeed);

#endif // _PICAM_PORT_H