/*
 * Copyright (C) 2018-2020 Alexandros Theodotou <alex at zrythm dot org>
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

#include "zrythm-config.h"

#ifdef HAVE_RTMIDI

#include "audio/channel.h"
#include "audio/engine.h"
#include "audio/engine_rtmidi.h"
#include "audio/ext_port.h"
#include "audio/midi.h"
#include "audio/router.h"
#include "audio/port.h"
#include "audio/router.h"
#include "audio/rtmidi_device.h"
#include "audio/transport.h"
#include "gui/widgets/main_window.h"
#include "plugins/plugin.h"
#include "plugins/lv2_plugin.h"
#include "project.h"
#include "settings/settings.h"
#include "utils/ui.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <rtmidi/rtmidi_c.h>

/**
 * Sets up the MIDI engine to use Rtmidi.
 *
 * @param loading Loading a Project or not.
 */
int
engine_rtmidi_setup (
  AudioEngine * self)
{
  self->midi_buf_size = 4096;

  g_message ("Rtmidi set up");

  return 0;
}

/**
 * Gets the number of input ports (devices).
 */
unsigned int
engine_rtmidi_get_num_in_ports (
  AudioEngine * self)
{
  RtMidiDevice * dev =
    rtmidi_device_new (1, 0, NULL);
  if (!dev)
    return 0;
  unsigned int num_ports =
    rtmidi_get_port_count (dev->in_handle);
  rtmidi_device_free (dev);

  return num_ports;
}

/**
 * Tests if the backend is working properly.
 *
 * Returns 0 if ok, non-null if has errors.
 *
 * If win is not null, it displays error messages
 * to it.
 */
int
engine_rtmidi_test (
  GtkWindow * win)
{
  return 0;
}

void
engine_rtmidi_tear_down (
  AudioEngine * self)
{
  /* init semaphore */
  zix_sem_init (
    &self->port_operation_lock, 1);
}

int
engine_rtmidi_activate (
  AudioEngine * self,
  bool          activate)
{
  return 0;
}

#endif /* HAVE_RTMIDI */
