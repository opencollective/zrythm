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

#include "zrythm-test-config.h"

#include "audio/master_track.h"
#include "audio/midi_mapping.h"
#include "helpers/project.h"
#include "helpers/zrythm.h"
#include "project.h"
#include "utils/math.h"
#include "zrythm.h"

static void
test_midi_mappping ()
{
  MidiMappings * mappings =
    midi_mappings_new ();
  g_assert_nonnull (mappings);
  free (mappings);

  ExtPort * ext_port = calloc (1, sizeof (ExtPort));
  ext_port->type = EXT_PORT_TYPE_RTAUDIO;
  ext_port->full_name = g_strdup ("ext port1");
  ext_port->short_name = g_strdup ("extport1");

  midi_byte_t buf[3] = { 0xB0, 0x07, 121 };
  midi_mappings_bind (
    MIDI_MAPPINGS, buf, ext_port,
    P_MASTER_TRACK->channel->fader->amp);
  g_assert_cmpint (
    MIDI_MAPPINGS->num_mappings, ==, 1);

  int size = 0;
  midi_mappings_get_for_port (
    MIDI_MAPPINGS,
    P_MASTER_TRACK->channel->fader->amp, &size);
  g_assert_cmpint (size, ==, 1);

  midi_mappings_apply (
    MIDI_MAPPINGS, buf);

  test_project_save_and_reload ();

  g_assert_true (
    P_MASTER_TRACK->channel->fader->amp ==
      MIDI_MAPPINGS->mappings[0].dest);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  test_helper_zrythm_init ();

#define TEST_PREFIX "/audio/midi_mapping/"

  g_test_add_func (
    TEST_PREFIX "test midi mapping",
    (GTestFunc) test_midi_mappping);

  return g_test_run ();
}


