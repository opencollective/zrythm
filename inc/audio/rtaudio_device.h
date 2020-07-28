/*
 * Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __AUDIO_RTAUDIO_DEVICE_H__
#define __AUDIO_RTAUDIO_DEVICE_H__

#include "zrythm-config.h"

#ifdef HAVE_RTAUDIO

#define RTAUDIO_DEVICE_BUFFER_SIZE 32000

#include <stdint.h>

#include "zix/ring.h"

#include <rtaudio/rtaudio_c.h>

typedef struct Port Port;

/**
 * @addtogroup audio
 *
 * @{
 */

/**
 * For readability when passing 0/1 for input
 * and output.
 */
enum RtAudioDeviceFlow
{
  RTAUDIO_DEVICE_FLOW_OUTPUT,
  RTAUDIO_DEVICE_FLOW_INPUT,
};

/**
 * This is actually a single channel on a device.
 */
typedef struct RtAudioDevice
{
  int           is_input;

  /** Channel index. */
  unsigned int  channel_idx;

  /** Index (device index from RtAudio). */
  unsigned int  id;

  char *        name;

  /** Whether opened or not. */
  int           opened;

  /** Whether started (running) or not. */
  int           started;

  rtaudio_t     handle;

  /** Associated port. */
  Port *        port;

  /**
   * Audio data buffer (see
   * port_prepare_rtaudio_data()).
   */
  float         buf[16000];

  /** Ring buffer for audio data. */
  ZixRing *     audio_ring;

  /** Semaphore for blocking writing events while
   * events are being read. */
  ZixSem        audio_ring_sem;

} RtAudioDevice;

RtAudioDevice *
rtaudio_device_new (
  int          is_input,
  unsigned int device_id,
  unsigned int channel_idx,
  Port *       port);

/**
 * Opens a device allocated with
 * rtaudio_device_new().
 *
 * @param start Also start the device.
 *
 * @return Non-zero if error.
 */
int
rtaudio_device_open (
  RtAudioDevice * self,
  int             start);

/**
 * Close the RtAudioDevice.
 *
 * @param free Also free the memory.
 */
int
rtaudio_device_close (
  RtAudioDevice * self,
  int            free);

int
rtaudio_device_start (
  RtAudioDevice * self);

int
rtaudio_device_stop (
  RtAudioDevice * self);

void
rtaudio_device_free (
  RtAudioDevice *  self);

/**
 * @}
 */

#endif // HAVE_RTAUDIO
#endif
