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

/** \file
 */

#include <math.h>

#include "audio/automation_point.h"
#include "audio/automation_region.h"
#include "audio/automation_track.h"
#include "audio/channel.h"
#include "audio/control_port.h"
#include "audio/instrument_track.h"
#include "audio/port.h"
#include "audio/position.h"
#include "audio/track.h"
#include "gui/backend/event.h"
#include "gui/backend/event_manager.h"
#include "gui/widgets/automation_arranger.h"
#include "gui/widgets/automation_point.h"
#include "gui/widgets/center_dock.h"
#include "plugins/lv2_plugin.h"
#include "plugins/plugin.h"
#include "project.h"
#include "settings/settings.h"
#include "utils/flags.h"
#include "utils/math.h"
#include "zrythm_app.h"

static AutomationPoint *
_create_new (
  const Position *        pos)
{
  AutomationPoint * self =
    calloc (1, sizeof (AutomationPoint));

  ArrangerObject * obj =
    (ArrangerObject *) self;
  obj->pos = *pos;
  obj->type = ARRANGER_OBJECT_TYPE_AUTOMATION_POINT;
  self->curve_opts.curviness = 0;
  self->curve_opts.algo =
    ZRYTHM_TESTING ?
      CURVE_ALGORITHM_SUPERELLIPSE :
      (CurveAlgorithm)
      g_settings_get_enum (
        S_P_EDITING_AUTOMATION,
        "curve-algorithm");

  self->index = -1;

  arranger_object_init (obj);

  return self;
}

/**
 * Sets the ZRegion and the index in the
 * region that the AutomationPoint
 * belongs to, in all its counterparts.
 */
void
automation_point_set_region_and_index (
  AutomationPoint * ap,
  ZRegion *         region,
  int               index)
{
  g_return_if_fail (ap && region);
  ArrangerObject * obj = (ArrangerObject *) ap;
  region_identifier_copy (
    &obj->region_id, &region->id);
  ap->index = index;

  /* set the info to the transient too */
  if (ZRYTHM_HAVE_UI &&
      obj->transient &&
      arranger_object_should_orig_be_visible (obj))
    {
      ArrangerObject * trans_obj = obj->transient;
      AutomationPoint * trans_ap =
        (AutomationPoint *) trans_obj;
      region_identifier_copy (
        &trans_obj->region_id, &region->id);
      trans_ap->index = index;
    }
}

int
automation_point_is_equal (
  AutomationPoint * a,
  AutomationPoint * b)
{
  ArrangerObject * a_obj =
    (ArrangerObject *) a;
  ArrangerObject * b_obj =
    (ArrangerObject *) b;
  return
    position_is_equal_ticks (
      &a_obj->pos, &b_obj->pos) &&
    math_floats_equal_epsilon (
      a->fvalue, b->fvalue, 0.001f);
}

/**
 * Creates an AutomationPoint in the given
 * AutomationTrack at the given Position.
 */
AutomationPoint *
automation_point_new_float (
  const float         value,
  const float         normalized_val,
  const Position *    pos)
{
  AutomationPoint * self =
    _create_new (pos);

  self->fvalue = value;
  self->normalized_val = normalized_val;

  return self;
}

/**
 * Returns the normalized value (0.0 to 1.0).
 */
/*float*/
/*automation_point_get_normalized_value (*/
  /*AutomationPoint * self)*/
/*{*/
  /*g_warn_if_fail (self->region->at);*/

  /*[> TODO convert to macro <]*/
  /*return automatable_real_val_to_normalized (*/
    /*self->region->at->automatable,*/
    /*self->fvalue);*/
/*}*/

/**
 * Moves the AutomationPoint by the given amount of
 * ticks.
 *
 * @param use_cached_pos Add the ticks to the cached
 *   Position instead of its current Position.
 * @param trans_only Only move transients.
 * @return Whether moved or not.
 * FIXME always call this after move !!!!!!!!!
 */
/*void*/
/*automation_point_on_move (*/
  /*AutomationPoint * automation_point,*/
  /*long     ticks,*/
  /*int      use_cached_pos,*/
  /*int      trans_only)*/
/*{*/
  /*Position tmp;*/
  /*POSITION_MOVE_BY_TICKS (*/
    /*tmp, use_cached_pos, automation_point, pos,*/
    /*ticks, trans_only);*/

  /*AutomationPoint * ap = automation_point;*/

  /*[> get prev and next value APs <]*/
  /*AutomationPoint * prev_ap =*/
    /*automation_region_get_prev_ap (*/
      /*ap->region, ap);*/
  /*AutomationPoint * next_ap =*/
    /*automation_region_get_next_ap (*/
      /*ap->region, ap);*/

  /*[> get adjusted pos for this automation point <]*/
  /*Position ap_pos;*/
  /*Position * prev_pos = &ap->cache_pos;*/
  /*position_set_to_pos (&ap_pos,*/
                       /*prev_pos);*/
  /*position_add_ticks (&ap_pos, ticks);*/

  /*Position mid_pos;*/
  /*AutomationCurve * ac;*/

  /*[> update midway points <]*/
  /*if (prev_ap &&*/
      /*position_is_after_or_equal (*/
        /*&ap_pos, &prev_ap->pos))*/
    /*{*/
      /*[> set prev curve point to new midway pos <]*/
      /*position_get_midway_pos (*/
        /*&prev_ap->pos, &ap_pos, &mid_pos);*/
      /*ac =*/
        /*automation_region_get_next_curve_ac (*/
          /*ap->region, prev_ap);*/
      /*position_set_to_pos (&ac->pos, &mid_pos);*/

      /*[> set pos for ap <]*/
      /*if (!next_ap)*/
        /*{*/
          /*position_set_to_pos (&ap->pos, &ap_pos);*/
        /*}*/
    /*}*/
  /*if (next_ap &&*/
      /*position_is_before_or_equal (*/
        /*&ap_pos, &next_ap->pos))*/
    /*{*/
      /*[> set next curve point to new midway pos <]*/
      /*position_get_midway_pos (*/
        /*&ap_pos, &next_ap->pos, &mid_pos);*/
      /*ac =*/
        /*automation_region_get_next_curve_ac (*/
          /*ap->region, ap);*/
      /*position_set_to_pos (&ac->pos, &mid_pos);*/

      /* set pos for ap - if no prev ap exists
       * or if the position is also after the
       * prev ap */
      /*if ((prev_ap &&*/
           /*position_is_after_or_equal (*/
            /*&ap_pos, &prev_ap->pos)) ||*/
          /*(!prev_ap))*/
        /*{*/
          /*position_set_to_pos (&ap->pos, &ap_pos);*/
        /*}*/
    /*}*/
  /*else if (!prev_ap && !next_ap)*/
    /*{*/
      /*[> set pos for ap <]*/
      /*position_set_to_pos (&ap->pos, &ap_pos);*/
    /*}*/
/*}*/

/**
 * Returns if the curve of the AutomationPoint
 * curves upwards as you move right on the x axis.
 */
bool
automation_point_curves_up (
  AutomationPoint * self)
{
  g_return_val_if_fail (self, -1);

  ZRegion * region =
    arranger_object_get_region (
      (ArrangerObject *) self);
  AutomationPoint * next_ap =
    automation_region_get_next_ap (
      region, self, true, true);

  if (!next_ap)
    return false;

  if (next_ap->fvalue > self->fvalue)
    return true;
  else
    return false;
}

/**
 * Sets the value from given real or normalized
 * value and notifies interested parties.
 *
 * @param is_normalized Whether the given value is
 *   normalized.
 */
void
automation_point_set_fvalue (
  AutomationPoint * self,
  float             real_val,
  bool              is_normalized)
{
  Port * port =
    automation_point_get_port (self);
  if (is_normalized)
    {
      g_message ("received val %f",
        (double) real_val);
      self->normalized_val =
        CLAMP (real_val, 0.f, 1.f);
      real_val =
        control_port_normalized_val_to_real (
          port, self->normalized_val);
    }
  else
    {
      real_val =
        CLAMP (real_val, port->minf, port->maxf);
      self->normalized_val =
        control_port_real_val_to_normalized (
          port, real_val);
    }
  g_message ("setting to %f", (double) real_val);
  self->fvalue = real_val;

  ZRegion * region =
    arranger_object_get_region (
      (ArrangerObject *) self);
  g_return_if_fail (region);
  control_port_set_val_from_normalized (
    port, self->normalized_val, 1);

  EVENTS_PUSH (
    ET_ARRANGER_OBJECT_CHANGED, self);
}

/**
 * The function to return a point on the curve.
 *
 * See https://stackoverflow.com/questions/17623152/how-map-tween-a-number-based-on-a-dynamic-curve
 *
 * @param ap The start point (0, 0).
 * @param x Normalized x.
 */
double
automation_point_get_normalized_value_in_curve (
  AutomationPoint * self,
  double            x)
{
  g_return_val_if_fail (
    self && x >= 0.0 && x <= 1.0, 0.0);

  ZRegion * region =
    arranger_object_get_region (
      (ArrangerObject *) self);
  AutomationPoint * next_ap =
    automation_region_get_next_ap (
      region, self, true, true);
  g_return_val_if_fail (next_ap, self->fvalue);

  double dy;

  int start_higher =
    next_ap->normalized_val < self->normalized_val;
  dy =
    curve_get_normalized_y (
      x, &self->curve_opts, start_higher);
  return dy;
}

/**
 * Sets the curviness of the AutomationPoint.
 */
void
automation_point_set_curviness (
  AutomationPoint * self,
  const curviness_t curviness)
{
  if (math_doubles_equal (
        self->curve_opts.curviness, curviness))
    return;

  self->curve_opts.curviness = curviness;
}

/**
 * Convenience function to return the control port
 * that this AutomationPoint is for.
 */
Port *
automation_point_get_port (
  AutomationPoint * self)
{
  AutomationTrack * at =
    automation_point_get_automation_track (self);
  g_return_val_if_fail (at, NULL);
  Port * port =
    automation_track_get_port (at);
  g_return_val_if_fail (port, NULL);

  return port;
}

/**
 * Returns Y in pixels from the value based on
 * the given height of the parent.
 */
int
automation_point_get_y (
  AutomationPoint * self,
  int               height)
{
  /* ratio of current value in the range */
  float ap_ratio = self->normalized_val;

  int allocated_h =
    gtk_widget_get_allocated_height (
      GTK_WIDGET (self));
  int point =
    allocated_h -
    (int) (ap_ratio * (float) allocated_h);
  return point;
}

/**
 * Convenience function to return the
 * AutomationTrack that this AutomationPoint is in.
 */
AutomationTrack *
automation_point_get_automation_track (
  AutomationPoint * self)
{
  g_return_val_if_fail (self, NULL);
  ZRegion * region =
    arranger_object_get_region (
      (ArrangerObject *) self);
  g_return_val_if_fail (region, NULL);
  return region_get_automation_track (region);
}
