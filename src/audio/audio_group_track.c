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

/**
 * \file
 *
 * Track logic specific to bus tracks.
 */

#include <stdlib.h>

#include "audio/automation_tracklist.h"
#include "audio/audio_group_track.h"
#include "audio/channel_track.h"
#include "project.h"

void
audio_group_track_init (Track * track)
{
  track->type = TRACK_TYPE_AUDIO_GROUP;
  gdk_rgba_parse (&track->color, "#D9DADD");
}

void
audio_group_track_setup (Track * self)
{
  channel_track_setup (self);
}
