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

#include "audio/midi.h"
#include "helpers/project.h"
#include "helpers/zrythm.h"
#include "project.h"
#include "utils/math.h"
#include "zrythm.h"

static void
test_msb_lsb_conversions ()
{
  /* first byte */
  midi_byte_t lsb;
  /* second byte */
  midi_byte_t msb;
  midi_get_bytes_from_int (
    12280, &lsb, &msb);
  g_assert_cmpint (lsb, ==, 120);
  g_assert_cmpint (msb, ==, 95);

  g_assert_cmpint (
    midi_combine_bytes_to_int (lsb, msb), ==, 12280);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  test_helper_zrythm_init ();

#define TEST_PREFIX "/audio/midi/"

  g_test_add_func (
    TEST_PREFIX "test msb lsb conversions",
    (GTestFunc) test_msb_lsb_conversions);

  return g_test_run ();
}



