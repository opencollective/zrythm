/*
 * Copyright (C) 2018-2019 Alexandros Theodotou <alex at zrythm dot org>
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

#include <stdlib.h>

#include "zrythm-config.h"

#include "audio/automation_track.h"
#include "audio/automation_tracklist.h"
#include "audio/instrument_track.h"
#include "audio/midi.h"
#include "audio/midi_note.h"
#include "audio/position.h"
#include "audio/region.h"
#include "audio/track.h"
#include "audio/velocity.h"
#include "gui/backend/event.h"
#include "gui/backend/event_manager.h"
#include "plugins/lv2/lv2_control.h"
#include "plugins/lv2_plugin.h"
#include "project.h"
#include "gui/widgets/track.h"
#include "utils/arrays.h"
#include "utils/stoat.h"
#include "zrythm_app.h"

#include <gtk/gtk.h>

/**
 * Initializes an instrument track.
 */
void
instrument_track_init (Track * track)
{
  track->type = TRACK_TYPE_INSTRUMENT;
  gdk_rgba_parse (&track->color, "#F79616");
}

void
instrument_track_setup (InstrumentTrack * self)
{
  channel_track_setup (self);
}

Plugin *
instrument_track_get_instrument (
  Track * self)
{
  g_return_val_if_fail (
    self &&
    self->type == TRACK_TYPE_INSTRUMENT &&
    self->channel, 0);

  Plugin * plugin = self->channel->instrument;
  g_return_val_if_fail (plugin, NULL);

  return plugin;
}

/**
 * Returns if the first plugin's UI in the
 * instrument track is visible.
 */
int
instrument_track_is_plugin_visible (
  Track * self)
{
  Plugin * plugin =
    instrument_track_get_instrument (self);
  g_return_val_if_fail (plugin, 0);

  return plugin->visible;
}

/**
 * Toggles whether the first plugin's UI in the
 * instrument Track is visible.
 */
void
instrument_track_toggle_plugin_visible (
  Track * self)
{
  Plugin * plugin =
    instrument_track_get_instrument (self);
  g_return_if_fail (plugin);

  plugin->visible = !plugin->visible;

  EVENTS_PUSH (
    ET_PLUGIN_VISIBILITY_CHANGED, plugin);
}
