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

#include "audio/curve.h"
#include "project.h"
#include "utils/flags.h"
#include "zrythm.h"

#include "tests/helpers/zrythm.h"

#include <glib.h>

static void
test_curve_algorithms ()
{
  CurveOptions opts;

  double epsilon = 0.0001;
  double val;

  /* ---- EXPONENT ---- */

  opts.algo = CURVE_ALGORITHM_EXPONENT;

  /* -- curviness -1 -- */
  opts.curviness = - 0.95;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.93465, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.93465, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness -0.5 -- */
  opts.curviness = - 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.69496, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.69496, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0 -- */
  opts.curviness = 0;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0.5 -- */
  opts.curviness = 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.69496, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.69496, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 1 -- */
  opts.curviness = 0.95;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.93465, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.93465, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* ---- SUPERELLIPSE ---- */

  opts.algo = CURVE_ALGORITHM_SUPERELLIPSE;

  /* -- curviness - 0.7 -- */
  opts.curviness = - 0.7;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9593, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9593, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0 -- */
  opts.curviness = 0;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0.7 -- */
  opts.curviness = 0.7;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9593, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9593, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* ---- VITAL ---- */

  opts.algo = CURVE_ALGORITHM_VITAL;

  /* -- curviness -1 -- */
  opts.curviness = - 1;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9933, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9933, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness -0.5 -- */
  opts.curviness = - 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9241, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1 - 0.9241, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0 -- */
  opts.curviness = 0;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.5, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0.5 -- */
  opts.curviness = 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9241, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9241, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 1 -- */
  opts.curviness = 1;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9933, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.9933, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* ---- PULSE ---- */

  opts.algo = CURVE_ALGORITHM_PULSE;

  /* -- curviness -1 -- */
  opts.curviness = - 1;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness -0.5 -- */
  opts.curviness = - 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0 -- */
  opts.curviness = 0;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 0.5 -- */
  opts.curviness = 0.5;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);

  /* -- curviness 1 -- */
  opts.curviness = 1;
  val = curve_get_normalized_y (0.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 0);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (0.5, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 1.0, epsilon);
  val = curve_get_normalized_y (1.0, &opts, 1);
  g_assert_cmpfloat_with_epsilon (
    val, 0.0, epsilon);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  test_helper_zrythm_init ();

#define TEST_PREFIX "/audio/curve/"

  g_test_add_func (
    TEST_PREFIX "test_curve_algorithms",
    (GTestFunc) test_curve_algorithms);

  return g_test_run ();
}
