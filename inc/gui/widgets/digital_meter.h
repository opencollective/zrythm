/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Digital meter used for displaying Position,
 * BPM, etc.
 */

#ifndef __GUI_WIDGETS_DIGITAL_METER_H__
#define __GUI_WIDGETS_DIGITAL_METER_H__

#include <stdbool.h>

#include "utils/types.h"

#include <gtk/gtk.h>

#define DIGITAL_METER_WIDGET_TYPE \
  (digital_meter_widget_get_type ())
G_DECLARE_FINAL_TYPE (
  DigitalMeterWidget,
  digital_meter_widget,
  Z, DIGITAL_METER_WIDGET,
  GtkDrawingArea)

typedef enum NoteLength NoteLength;
typedef enum NoteType NoteType;
typedef struct Position Position;

/**
 * @addtogroup widgets
 *
 * @{
 */

typedef enum DigitalMeterType
{
  DIGITAL_METER_TYPE_BPM,
  DIGITAL_METER_TYPE_POSITION,
  DIGITAL_METER_TYPE_TIMESIG,
  DIGITAL_METER_TYPE_NOTE_TYPE,
  DIGITAL_METER_TYPE_NOTE_LENGTH,
} DigitalMeterType;

typedef struct SnapGrid SnapGrid;

typedef struct _DigitalMeterWidget
{
  GtkDrawingArea           parent_instance;

  DigitalMeterType         type;

  bool                     is_transport;

  GtkGestureDrag           * drag;
  double                   last_y;
  double                   last_x;
  int                      height_start_pos;
  int                      height_end_pos;

  /* ========= BPM ========= */
  /* for BPM */
  int                      num_part_start_pos;
  int                      num_part_end_pos;
  int                      dec_part_start_pos;
  int                      dec_part_end_pos;

  /** Used when changing the BPM. */
  bpm_t                    prev_bpm;

  /** Flag to update BPM. */
  bool                     update_num;
  /** Flag to update BPM decimal. */
  bool                     update_dec;

  /* ========= BPM end ========= */

  /* ========= position ========= */

  int                      bars_start_pos;
  int                      bars_end_pos;
  int                      beats_start_pos;
  int                      beats_end_pos;
  int                      sixteenths_start_pos;
  int                      sixteenths_end_pos;
  int                      ticks_start_pos;
  int                      ticks_end_pos;

  /** Update flags. */
  int                      update_bars;
  int                      update_beats;
  int                      update_sixteenths;
  int                      update_ticks;

  /* ========= position end ========= */

  /* ========= time ========= */

  /** For time. */
  int                      minutes_start_pos;
  int                      minutes_end_pos;
  int                      seconds_start_pos;
  int                      seconds_end_pos;
  int                      ms_start_pos;
  int                      ms_end_pos;

  /** Update flags. */
  int                      update_minutes;
  int                      update_seconds;
  int                      update_ms;

  /* ========= time end ========= */

  /* for note length/type */
  NoteLength *             note_length;
  NoteType *               note_type;
  int                      update_note_length; ///< flag to update note length
  int                      start_note_length; ///< start note length
  int                      update_note_type; ///< flag to update note type
  int                      start_note_type; ///< start note type

  /* for time sig */
  int                      update_timesig_top;
  int                      start_timesig_top;
  int                      update_timesig_bot;
  int                      start_timesig_bot;


  /* ---------- FOR POSITION ---------------- */
  void *     obj;

  /** Getter for Position. */
  void       (*getter)(void*, Position*);
  /** Setter for Position. */
  void       (*setter)(void*, Position*);
  /** Function to call on drag begin. */
  void       (*on_drag_begin)(void*);
  /** Function to call on drag end. */
  void       (*on_drag_end)(void*);

  /* ----------- position end --------------- */

  /** Draw line above the meter or not. */
  int        draw_line;

  /** Caption to show above, NULL to not show. */
  char *     caption;

  /** Cached layouts for drawing text. */
  PangoLayout * caption_layout;
  PangoLayout * seg7_layout;
  PangoLayout * normal_layout;
} DigitalMeterWidget;

/**
 * Creates a digital meter with the given type (
 * bpm or position).
 */
DigitalMeterWidget *
digital_meter_widget_new (
  DigitalMeterType  type,
  NoteLength *      note_length,
  NoteType *        note_type,
  const char *      caption);


#define digital_meter_widget_new_for_position( \
  obj,drag_begin,getter,setter,drag_end,caption) \
  _digital_meter_widget_new_for_position ( \
    (void *) obj, \
    (void (*) (void *)) drag_begin, \
    (void (*) (void *, Position *)) getter, \
    (void (*) (void *, Position *)) setter, \
    (void (*) (void *)) drag_end, \
    caption)

/**
 * Creates a digital meter for an arbitrary position.
 *
 * @param obj The object to call the get/setters with.
 *
 *   E.g. Region.
 * @param get_val The getter func to get the position,
 *   passing the obj and the position to save to.
 * @param set_val The setter function to set the
 *   position.
 * @param drag_begin Function to call when
 *   starting the action.
 * @parram drag_end Function to call when ending
 *   the action.
 */
DigitalMeterWidget *
_digital_meter_widget_new_for_position(
  void * obj,
  void (*drag_begin)(void *),
  void (*get_val)(void *, Position *),
  void (*set_val)(void *, Position *),
  void (*drag_end)(void *),
  const char * caption);

void
digital_meter_set_draw_line (
  DigitalMeterWidget * self,
  int                  draw_line);

/**
 * @}
 */

#endif
