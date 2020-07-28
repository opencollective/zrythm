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

/**
 * \file
 *
 * Meter DSP.
 */

#ifndef __AUDIO_METER_H__
#define __AUDIO_METER_H__

#include <stdbool.h>

#include "utils/types.h"

#include <gtk/gtk.h>

typedef struct TruePeakDsp TruePeakDsp;
typedef struct KMeterDsp KMeterDsp;
typedef struct PeakDsp PeakDsp;
typedef struct Port Port;

/**
 * @addtogroup audio
 *
 * @{
 */

typedef enum MeterAlgorithm
{
  /** Use default algorithm for the port. */
  METER_ALGORITHM_AUTO,

  METER_ALGORITHM_DIGITAL_PEAK,

  /** @note True peak is intensive, only use it
   * where needed (mixer). */
  METER_ALGORITHM_TRUE_PEAK,
  METER_ALGORITHM_RMS,
  METER_ALGORITHM_K,
} MeterAlgorithm;

/**
 * A Meter used by a single GUI element.
 */
typedef struct Meter
{
  /** Port associated with this meter. */
  Port *          port;

  /** True peak processor. */
  TruePeakDsp *   true_peak_processor;
  TruePeakDsp *   true_peak_max_processor;

  /** Current true peak. */
  float           true_peak;
  float           true_peak_max;

  /** K RMS processor, if K meter. */
  KMeterDsp *     kmeter_processor;

  PeakDsp *       peak_processor;

  /**
   * Algorithm to use.
   *
   * Auto by default.
   */
  MeterAlgorithm  algorithm;

  /** Previous max, used when holding the max
   * value. */
  float           prev_max;

  /** Last meter value (in amplitude), used to
   * show a falloff and avoid sudden dips. */
  float           last_amp;

  /** Time the last val was taken at (last draw
   * time). */
  gint64          last_draw_time;

  gint64          last_midi_trigger_time;

} Meter;

Meter *
meter_new_for_port (
  Port * port);

/**
 * Get the current meter value.
 *
 * This should only be called once in a draw
 * cycle.
 */
void
meter_get_value (
  Meter *          self,
  AudioValueFormat format,
  float *          val,
  float *          max);

void
meter_free (
  Meter * self);

#endif
