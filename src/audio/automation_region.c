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
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "audio/automation_point.h"
#include "audio/automation_region.h"
#include "audio/position.h"
#include "audio/region.h"
#include "gui/backend/automation_selections.h"
#include "gui/backend/event.h"
#include "gui/backend/event_manager.h"
#include "gui/widgets/automation_arranger.h"
#include "gui/widgets/automation_editor_space.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/clip_editor_inner.h"
#include "gui/widgets/clip_editor.h"
#include "project.h"
#include "utils/arrays.h"
#include "utils/flags.h"
#include "utils/object_utils.h"
#include "utils/objects.h"
#include "zrythm_app.h"

int
automation_region_sort_func (
  const void * _a, const void * _b)
{
  AutomationPoint * a =
    *(AutomationPoint * const *) _a;
  AutomationPoint * b =
    *(AutomationPoint * const *)_b;
  ArrangerObject * a_obj =
    (ArrangerObject *) a;
  ArrangerObject * b_obj =
    (ArrangerObject *) b;
  long ret =
    position_compare (
      &a_obj->pos, &b_obj->pos);
  if (ret == 0 &&
      a->index <
      b->index)
    {
      return -1;
    }

  return (int) CLAMP (ret, -1, 1);
}

ZRegion *
automation_region_new (
  const Position * start_pos,
  const Position * end_pos,
  int              track_pos,
  int              at_idx,
  int              idx_inside_at)
{
  ZRegion * self =
    calloc (1, sizeof (ZRegion));

  self->id.type = REGION_TYPE_AUTOMATION;

  self->aps_size = 2;
  self->aps =
    malloc (self->aps_size *
            sizeof (AutomationPoint *));

  region_init (
    self, start_pos, end_pos, track_pos, at_idx,
    idx_inside_at);

  return self;
}

/**
 * Prints the automation in this Region.
 */
void
automation_region_print_automation (
  ZRegion * self)
{
  AutomationPoint * ap;
  ArrangerObject * ap_obj;
  for (int i = 0; i < self->num_aps; i++)
    {
      ap = self->aps[i];
      ap_obj = (ArrangerObject *) ap;
      g_message ("%d", i);
      position_print_yaml (&ap_obj->pos);
    }
}

/**
 * Forces sort of the automation points.
 */
void
automation_region_force_sort (
  ZRegion * self)
{
  /* sort by position */
  qsort (self->aps,
         (size_t) self->num_aps,
         sizeof (AutomationPoint *),
         automation_region_sort_func);

  /* refresh indices */
  for (int i = 0; i < self->num_aps; i++)
    {
      automation_point_set_region_and_index (
        self->aps[i], self, i);
    }
}

/**
 * Adds an AutomationPoint to the Region.
 */
void
automation_region_add_ap (
  ZRegion *         self,
  AutomationPoint * ap,
  int               pub_events)
{
  /* add point */
  array_double_size_if_full (
    self->aps, self->num_aps, self->aps_size,
    AutomationPoint *);
  array_append (
    self->aps, self->num_aps, ap);

  /* re-sort */
  automation_region_force_sort (self);

  if (pub_events)
    {
      EVENTS_PUSH (
        ET_ARRANGER_OBJECT_CREATED, ap);
    }
}


/**
 * Returns the AutomationPoint before the given
 * one.
 */
AutomationPoint *
automation_region_get_prev_ap (
  ZRegion *          self,
  AutomationPoint * ap)
{
  if (ap->index > 0)
    return self->aps[ap->index - 1];

  return NULL;
}

/**
 * Returns the AutomationPoint after the given
 * one.
 *
 * @param check_positions Compare positions instead
 *   of just getting the next index.
 * @param check_transients Also check the transient
 *   of each object. This only matters if \ref
 *   check_positions is true. FIXME not used at
 *   the moment. Keep it around for abit then
 *   delete it if not needed.
 */
AutomationPoint *
automation_region_get_next_ap (
  ZRegion *         self,
  AutomationPoint * ap,
  bool              check_positions,
  bool              check_transients)
{
  g_return_val_if_fail (
    self && ap, NULL);

  if (check_positions)
    {
      check_transients =
        ZRYTHM_HAVE_UI &&
        MW_AUTOMATION_ARRANGER &&
        MW_AUTOMATION_ARRANGER->action ==
          UI_OVERLAY_ACTION_MOVING_COPY;
      ArrangerObject * obj = (ArrangerObject *) ap;
      AutomationPoint * next_ap = NULL;
      ArrangerObject * next_obj = NULL;
      for (int i = 0; i < self->num_aps; i++)
        {
          for (int j = 0;
               j < (check_transients ? 2 : 1); j++)
            {
              AutomationPoint * cur_ap =
                self->aps[i];
              ArrangerObject * cur_obj =
                (ArrangerObject *) cur_ap;
              if (j == 1)
                {
                  if (cur_obj->transient)
                    {
                      cur_obj = cur_obj->transient;
                      cur_ap =
                        (AutomationPoint *)
                        cur_obj;
                    }
                  else
                    continue;
                }

              if (cur_ap == ap)
                continue;

              if (position_is_after_or_equal (
                    &cur_obj->pos, &obj->pos) &&
                  (!next_obj ||
                   position_is_before (
                     &cur_obj->pos,
                     &next_obj->pos)))
                {
                  next_obj = cur_obj;
                  next_ap = cur_ap;
                }
            }
        }
      return next_ap;
    }
  else if (ap->index < self->num_aps - 1)
    return self->aps[ap->index + 1];

  return NULL;
}

/**
 * Removes the AutomationPoint from the ZRegion,
 * optionally freeing it.
 *
 * @param free Free the AutomationPoint after
 *   removing it.
 */
void
automation_region_remove_ap (
  ZRegion *         self,
  AutomationPoint * ap,
  int               free)
{
  /* deselect */
  arranger_object_select (
    (ArrangerObject *) ap, F_NO_SELECT,
    F_APPEND);

  array_delete (
    self->aps, self->num_aps, ap);

  if (free)
    {
      arranger_object_free (
        (ArrangerObject *) ap);
    }

  EVENTS_PUSH (
    ET_ARRANGER_OBJECT_REMOVED,
    ARRANGER_OBJECT_TYPE_AUTOMATION_POINT);
}

/**
 * Returns the automation points since the last
 * recorded automation point (if the last recorded
 * automation point was before the current pos).
 */
void
automation_region_get_aps_since_last_recorded (
  ZRegion *          self,
  Position *         pos,
  AutomationPoint ** aps,
  int *              num_aps)
{
  *num_aps = 0;

  ArrangerObject * last_recorded_obj =
    (ArrangerObject *) self->last_recorded_ap;
  if (!last_recorded_obj ||
      position_is_before_or_equal (
        pos, &last_recorded_obj->pos))
    return;

  for (int i = 0; i < self->num_aps; i++)
    {
      AutomationPoint * ap = self->aps[i];
      ArrangerObject * ap_obj =
        (ArrangerObject *) ap;

      if (position_is_after (
            &ap_obj->pos, &last_recorded_obj->pos) &&
          position_is_before_or_equal (
            &ap_obj->pos, pos))
        {
          aps[*num_aps] = ap;
          (*num_aps)++;
        }
    }
}

/**
 * Returns an automation point found within +/-
 * delta_ticks from the position, or NULL.
 *
 * @param before_only Only check previous automation
 *   points.
 */
AutomationPoint *
automation_region_get_ap_around (
  ZRegion *  self,
  Position * _pos,
  double     delta_ticks,
  bool       before_only)
{
  Position pos;
  position_set_to_pos (&pos, _pos);
  AutomationTrack * at =
    region_get_automation_track (self);
  /* FIXME only check aps in this region */
  AutomationPoint * ap =
    automation_track_get_ap_before_pos (at, &pos);
  ArrangerObject * ap_obj =
    (ArrangerObject *) ap;
  if (ap &&
      pos.total_ticks -
        ap_obj->pos.total_ticks <=
          (double) delta_ticks)
    {
      return ap;
    }
  else if (!before_only)
    {
      position_add_ticks (&pos, delta_ticks);
      ap =
        automation_track_get_ap_before_pos (
          at, &pos);
      ap_obj = (ArrangerObject *) ap;
      if (ap)
        {
          double diff =
            ap_obj->pos.total_ticks -
              _pos->total_ticks;
          if (diff >= 0.0)
            return ap;
        }
    }

  return NULL;
}

/**
 * Frees members only but not the ZRegion itself.
 *
 * Regions should be free'd using region_free.
 */
void
automation_region_free_members (
  ZRegion * self)
{
  int i;
  for (i = 0; i < self->num_aps; i++)
    automation_region_remove_ap (
      self, self->aps[i], F_FREE);

  free (self->aps);
}
