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

#include "audio/audio_region.h"
#include "audio/automation_region.h"
#include "audio/chord_region.h"
#include "audio/chord_track.h"
#include "audio/channel.h"
#include "audio/clip.h"
#include "audio/midi_note.h"
#include "audio/midi_region.h"
#include "audio/instrument_track.h"
#include "audio/pool.h"
#include "audio/recording_manager.h"
#include "audio/region.h"
#include "audio/region_link_group_manager.h"
#include "audio/stretcher.h"
#include "audio/track.h"
#include "gui/widgets/automation_region.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/chord_region.h"
#include "gui/widgets/clip_editor.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/midi_arranger.h"
#include "gui/widgets/midi_region.h"
#include "gui/widgets/region.h"
#include "gui/widgets/timeline_arranger.h"
#include "gui/widgets/timeline_panel.h"
#include "project.h"
#include "settings/settings.h"
#include "utils/arrays.h"
#include "utils/flags.h"
#include "utils/audio.h"
#include "utils/yaml.h"

#include <glib/gi18n.h>

#include <sndfile.h>
#include <samplerate.h>


/**
 * Only to be used by implementing structs.
 */
void
region_init (
  ZRegion *         self,
  const Position * start_pos,
  const Position * end_pos,
  int              track_pos,
  int              lane_pos_or_at_idx,
  int              idx_inside_lane_or_at)
{
  self->id.track_pos = track_pos;
  self->id.lane_pos = lane_pos_or_at_idx;
  self->id.at_idx = lane_pos_or_at_idx;
  self->id.idx = idx_inside_lane_or_at;
  self->id.link_group = -1;

  ArrangerObject * obj =
    (ArrangerObject *) self;
  obj->type = ARRANGER_OBJECT_TYPE_REGION;
  position_set_to_pos (
    &obj->pos, start_pos);
  obj->pos.frames = start_pos->frames;
  position_set_to_pos (
    &obj->end_pos, end_pos);
  obj->end_pos.frames = end_pos->frames;
  position_init (&obj->clip_start_pos);
  long length =
    arranger_object_get_length_in_frames (obj);
  g_warn_if_fail (length > 0);
  position_from_frames (
    &obj->loop_end_pos, length);
  position_init (&obj->loop_start_pos);
  obj->loop_end_pos.frames = length;

  /* set fade positions to start/end */
  position_init (&obj->fade_in_pos);
  position_from_frames (
    &obj->fade_out_pos, length);

  arranger_object_init (obj);
  self->magic = REGION_MAGIC;
}

/**
 * Generates a name for the ZRegion, either using
 * the given AutomationTrack or Track, or appending
 * to the given base name.
 */
void
region_gen_name (
  ZRegion *         self,
  const char *      base_name,
  AutomationTrack * at,
  Track *           track)
{
  g_return_if_fail (IS_REGION (self));

  /* Name to try to assign */
  char * orig_name = NULL;
  if (base_name)
    orig_name =
      g_strdup (base_name);
  else if (at)
    orig_name =
      g_strdup_printf (
        "%s - %s",
        track->name, at->port_id.label);
  else
    orig_name = g_strdup (track->name);

  region_set_name (self, orig_name, 0);
  g_free (orig_name);
}

/**
 * Sets the track lane.
 */
void
region_set_lane (
  ZRegion *    self,
  TrackLane * lane)
{
  g_return_if_fail (IS_REGION (self) && lane);
  self->id.lane_pos = lane->pos;
  self->id.track_pos = lane->track_pos;
}

/**
 * Moves the ZRegion to the given Track, maintaining
 * the selection status of the ZRegion and the
 * TrackLane position.
 *
 * Assumes that the ZRegion is already in a
 * TrackLane.
 */
void
region_move_to_track (
  ZRegion *  region,
  Track *    track)
{
  g_return_if_fail (IS_REGION (region) && track);

  /*g_message ("moving region %s to track %s",*/
    /*region->name, track->name);*/

  Track * region_track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_if_fail (region_track);

  if (region_track == track)
    return;

  int selected = region_is_selected (region);
  int lane_pos = region->id.lane_pos;

  /* create lanes if they don't exist */
  track_create_missing_lanes (
    track, lane_pos);

  /* remove the region from its old track */
  track_remove_region (
    region_track,
    region, F_NO_PUBLISH_EVENTS, F_NO_FREE);

  /* add the region to its new track */
  track_add_region (
    track, region, NULL, lane_pos, F_NO_GEN_NAME,
    F_NO_PUBLISH_EVENTS);
  g_warn_if_fail (region->id.lane_pos == lane_pos);
  g_warn_if_fail (
    track->lanes[lane_pos]->num_regions > 0 &&
    track->lanes[lane_pos]->regions[
      region->id.idx] == region);
  region_set_lane (
    region, track->lanes[lane_pos]);

  /* reselect if necessary */
  arranger_object_select (
    (ArrangerObject *) region, selected,
    F_APPEND);

  /* remove empty lanes if the region was the
   * last on its track lane */
  track_remove_empty_last_lanes (
    region_track);
}

/**
 * Stretch the region's contents.
 *
 * This should be called right after changing the
 * region's size.
 *
 * @param ratio The ratio to stretch by.
 */
void
region_stretch (
  ZRegion * self,
  double    ratio)
{
  g_return_if_fail (IS_REGION (self));

  self->stretching = true;
  ArrangerObject * obj = (ArrangerObject *) self;

  switch (self->id.type)
    {
    case REGION_TYPE_MIDI:
      for (int i = 0; i < self->num_midi_notes; i++)
        {
          MidiNote * mn =
            self->midi_notes[i];
          ArrangerObject * mn_obj =
            (ArrangerObject *) mn;

          /* set start pos */
          double before_ticks =
            mn_obj->pos.total_ticks;
          double new_ticks = before_ticks * ratio;
          Position tmp;
          position_from_ticks (
            &tmp, new_ticks);
          arranger_object_pos_setter (
            mn_obj, &tmp);

          /* set end pos */
          before_ticks =
            mn_obj->end_pos.
              total_ticks;
          new_ticks = before_ticks * ratio;
          position_from_ticks (
            &tmp, new_ticks);
          arranger_object_end_pos_setter (
            mn_obj, &tmp);
        }
      break;
    case REGION_TYPE_AUDIO:
      {
        AudioClip * clip =
          audio_region_get_clip (self);
        Stretcher * stretcher =
          stretcher_new_rubberband (
            AUDIO_ENGINE->sample_rate,
            clip->channels, ratio, 1.0, false);
        ssize_t returned_frames =
          stretcher_stretch_interleaved (
            stretcher, self->frames,
            (size_t) self->num_frames,
            &self->frames);
        g_warn_if_fail (returned_frames > 0);
        self->num_frames =
          (size_t) returned_frames;
        (void) obj;
        /* readjust end position to match the
         * number of frames exactly */
        Position new_end_pos;
        position_from_frames (
          &new_end_pos, returned_frames);
        arranger_object_set_position (
          obj, &new_end_pos,
          ARRANGER_OBJECT_POSITION_TYPE_LOOP_END,
          F_NO_VALIDATE);
        position_add_frames (
          &new_end_pos, obj->pos.frames);
        arranger_object_set_position (
          obj, &new_end_pos,
          ARRANGER_OBJECT_POSITION_TYPE_END,
          F_NO_VALIDATE);
        stretcher_free (stretcher);
      }
      break;
    default:
      break;
    }

  obj->use_cache = false;
  self->stretching = false;
}

/**
 * Moves the given ZRegion to the given TrackLane.
 *
 * Works with TrackLane's of other Track's as well.
 *
 * Maintains the selection status of the
 * Region.
 *
 * Assumes that the ZRegion is already in a
 * TrackLane.
 */
void
region_move_to_lane (
  ZRegion *    region,
  TrackLane * lane)
{
  g_return_if_fail (IS_REGION (region) && lane);

  Track * region_track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_if_fail (region_track);

  int selected = region_is_selected (region);
  int is_clip_editor_region =
    region == clip_editor_get_region (CLIP_EDITOR);

  Track * lane_track =
    track_lane_get_track (lane);
  track_remove_region (
    region_track, region,
    F_NO_PUBLISH_EVENTS, F_NO_FREE);
  track_add_region (
    lane_track, region, NULL, lane->pos,
    F_NO_GEN_NAME, F_NO_PUBLISH_EVENTS);

  /* reset the clip editor region because
   * track_remove_region clears it */
  if (is_clip_editor_region)
    {
      clip_editor_set_region (
        CLIP_EDITOR, region, true);
    }

  arranger_object_select (
    (ArrangerObject *) region, selected,
    F_APPEND);
  region_set_lane (region, lane);
  g_warn_if_fail (
    lane->pos == region->id.lane_pos);

  track_create_missing_lanes (
    region_track, lane->pos);
  track_remove_empty_last_lanes (region_track);
}

/**
 * Sets the automation track.
 */
void
region_set_automation_track (
  ZRegion *         self,
  AutomationTrack * at)
{
  g_return_if_fail (IS_REGION (self) && at);

  /*int is_clip_editor_region = 0;*/
  if (region_identifier_is_equal (
        &self->id, &CLIP_EDITOR->region_id))
    {
      /*is_clip_editor_region = 1;*/
      clip_editor_set_region (
        CLIP_EDITOR, NULL, true);
    }
  self->id.at_idx = at->index;
  Track * track =
    automation_track_get_track (at);
  self->id.track_pos = track->pos;

  region_update_identifier (self);
}

void
region_get_type_as_string (
  RegionType type,
  char *     buf)
{
  g_return_if_fail (
    type >= 0 && type <= REGION_TYPE_CHORD);
  switch (type)
    {
    case REGION_TYPE_MIDI:
      strcpy (buf, _("MIDI"));
      break;
    case REGION_TYPE_AUDIO:
      strcpy (buf, _("Audio"));
      break;
    case REGION_TYPE_AUTOMATION:
      strcpy (buf, _("Automation"));
      break;
    case REGION_TYPE_CHORD:
      strcpy (buf, _("Chord"));
      break;
    }
}

/**
 * Returns if the given ZRegion type can exist
 * in TrackLane's.
 */
int
region_type_has_lane (
  const RegionType type)
{
  return
    type == REGION_TYPE_MIDI ||
    type == REGION_TYPE_AUDIO;
}

/**
 * Sanity checking.
 *
 * @param is_project Whether this region ispart
 *   of the project (as opposed to a clone in
 *   the undo stack, etc.).
 */
bool
region_sanity_check (
  ZRegion * self,
  bool      is_project)
{
  g_return_val_if_fail (IS_REGION (self), false);

  if (is_project)
    {
      if (!region_find (&self->id))
        {
          return false;
        }
    }

  return true;
}

TrackLane *
region_get_lane (
  const ZRegion * self)
{
  g_return_val_if_fail (IS_REGION (self), NULL);

  Track * track =
    arranger_object_get_track (
      (ArrangerObject *) self);
  g_return_val_if_fail (track, NULL);
  if (self->id.lane_pos < track->num_lanes)
    {
      TrackLane * lane =
        track->lanes[self->id.lane_pos];
      g_return_val_if_fail (lane, NULL);
      return lane;
    }

  g_return_val_if_reached (NULL);
}

RegionLinkGroup *
region_get_link_group (
  ZRegion * self)
{
  g_return_val_if_fail (
    self && self->id.link_group >= 0 &&
    REGION_LINK_GROUP_MANAGER->num_groups >
    self->id.link_group, NULL);
  RegionLinkGroup * group =
    region_link_group_manager_get_group (
      REGION_LINK_GROUP_MANAGER,
      self->id.link_group);
  return group;
}

/**
 * Sets the link group to the region.
 *
 * @param group_idx If -1, the region will be
 *   removed from its current link group, if any.
 */
void
region_set_link_group (
  ZRegion * region,
  int       group_idx,
  bool      update_identifier)
{
  ArrangerObject * obj =
    (ArrangerObject *) region;
  if (obj->flags & ARRANGER_OBJECT_FLAG_NON_PROJECT)
    {
      region->id.link_group = group_idx;
      return;
    }

  if (region->id.link_group >= 0 &&
      region->id.link_group != group_idx)
    {
      region_link_group_remove_region (
        region_get_link_group (region),
        region, true, update_identifier);
    }
  if (group_idx >= 0)
    {
      RegionLinkGroup * group =
        region_link_group_manager_get_group (
          REGION_LINK_GROUP_MANAGER, group_idx);
      region_link_group_add_region (
        group, region);
    }

  if (update_identifier)
    region_update_identifier (region);
}

void
region_create_link_group_if_none (
  ZRegion * region)
{
  ArrangerObject * obj =
    (ArrangerObject *) region;
  if (obj->flags & ARRANGER_OBJECT_FLAG_NON_PROJECT)
    return;

  if (region->id.link_group < 0)
    {
      int new_group =
        region_link_group_manager_add_group (
          REGION_LINK_GROUP_MANAGER);
      region_set_link_group (
        region, new_group, true);
    }
}

bool
region_has_link_group (
  ZRegion * region)
{
  return region->id.link_group >= 0;
}

/**
 * Removes the link group from the region, if any.
 */
void
region_unlink (
  ZRegion * region)
{
  ArrangerObject * obj =
    (ArrangerObject *) region;
  if (obj->flags & ARRANGER_OBJECT_FLAG_NON_PROJECT)
    {
      region->id.link_group = -1;
    }
  else if (region->id.link_group >= 0)
    {
      RegionLinkGroup * group =
        region_link_group_manager_get_group (
          REGION_LINK_GROUP_MANAGER,
          region->id.link_group);
      region_link_group_remove_region (
        group, region, true, true);
    }
  else
    {
      g_warn_if_reached ();
    }

  g_warn_if_fail (region->id.link_group == -1);

  region_update_identifier (region);
}

/**
 * Looks for the ZRegion matching the identifier.
 */
ZRegion *
region_find (
  RegionIdentifier * id)
{
  Track * track = NULL;
  TrackLane * lane = NULL;
  AutomationTrack * at = NULL;
  if (id->type == REGION_TYPE_MIDI ||
      id->type == REGION_TYPE_AUDIO)
    {
      g_return_val_if_fail (
        id->track_pos < TRACKLIST->num_tracks,
        NULL);

      track =  TRACKLIST->tracks[id->track_pos];
      g_return_val_if_fail (track, NULL);

      g_return_val_if_fail (
        id->lane_pos < track->num_lanes, NULL);
      lane = track->lanes[id->lane_pos];
      g_return_val_if_fail (lane, NULL);

      g_return_val_if_fail (
        id->idx < lane->num_regions, NULL);

      ZRegion * region = lane->regions[id->idx];
      g_return_val_if_fail (region, NULL);

      return region;
    }
  else if (id->type == REGION_TYPE_AUTOMATION)
    {
      if (id->track_pos >= TRACKLIST->num_tracks)
        g_return_val_if_reached (NULL);
      track = TRACKLIST->tracks[id->track_pos];
      g_return_val_if_fail (track, NULL);

      AutomationTracklist * atl =
        &track->automation_tracklist;
      if (id->at_idx >= atl->num_ats)
        g_return_val_if_reached (NULL);
      at = atl->ats[id->at_idx];
      g_return_val_if_fail (at, NULL);

      if (id->idx >= at->num_regions)
        g_return_val_if_reached (NULL);
      ZRegion * region = at->regions[id->idx];
      g_return_val_if_fail (region, NULL);

      return region;
    }
  else if (id->type == REGION_TYPE_CHORD)
    {
      track = P_CHORD_TRACK;
      g_return_val_if_fail (track, NULL);

      if (id->idx >= track->num_chord_regions)
        g_return_val_if_reached (NULL);
      ZRegion * region =
        track->chord_regions[id->idx];
      g_return_val_if_fail (region, NULL);

      return region;
    }

  g_return_val_if_reached (NULL);
}

/**
 * To be called every time the identifier changes
 * to update the region's children.
 */
void
region_update_identifier (
  ZRegion * self)
{
  /* reset link group */
  region_set_link_group (
    self, self->id.link_group, false);

  switch (self->id.type)
    {
    case REGION_TYPE_AUDIO:
      break;
    case REGION_TYPE_MIDI:
      for (int i = 0; i < self->num_midi_notes; i++)
        {
          MidiNote * mn = self->midi_notes[i];
          midi_note_set_region_and_index (
            mn, self, i);
        }
      break;
    case REGION_TYPE_AUTOMATION:
      for (int i = 0; i < self->num_aps; i++)
        {
          AutomationPoint * ap = self->aps[i];
          automation_point_set_region_and_index (
            ap, self, i);
        }
      break;
    case REGION_TYPE_CHORD:
      for (int i = 0; i < self->num_chord_objects;
           i++)
        {
          ChordObject * co = self->chord_objects[i];
          chord_object_set_region_and_index (
            co, self, i);
        }
      break;
    default:
      break;
    }
}

/**
 * Updates all other regions in the region link
 * group, if any.
 */
void
region_update_link_group (
  ZRegion * self)
{
  g_message ("updating link group %d",
    self->id.link_group);
  if (self->id.link_group >= 0)
    {
      RegionLinkGroup * group =
        region_link_group_manager_get_group (
          REGION_LINK_GROUP_MANAGER,
          self->id.link_group);
      region_link_group_update (
        group, self);
    }
}

/**
 * Removes all children objects from the region.
 */
void
region_remove_all_children (
  ZRegion * region)
{
  g_message ("removing all children from %d %s",
    region->id.idx, region->name);
  switch (region->id.type)
    {
    case REGION_TYPE_MIDI:
      {
        g_message (
          "%d midi notes", region->num_midi_notes);
        for (int i = region->num_midi_notes - 1;
             i >= 0; i--)
          {
            MidiNote * mn =
              region->midi_notes[i];
            midi_region_remove_midi_note (
              region, mn, F_FREE,
              F_NO_PUBLISH_EVENTS);
          }
        g_warn_if_fail (
          region->num_midi_notes == 0);
      }
      break;
    case REGION_TYPE_AUDIO:
      break;
    case REGION_TYPE_AUTOMATION:
      {
        /* add automation points */
        for (int i = region->num_aps - 1;
             i >= 0; i--)
          {
            AutomationPoint * ap = region->aps[i];
            automation_region_remove_ap (
              region, ap, F_FREE);
          }
      }
      break;
    case REGION_TYPE_CHORD:
      {
        for (int i = region->num_chord_objects - 1;
             i >= 0; i--)
          {
            ChordObject * co =
              region->chord_objects[i];
            chord_region_remove_chord_object (
              region, co, F_FREE,
              F_NO_PUBLISH_EVENTS);
          }
      }
      break;
    }
}

/**
 * Clones and copies all children from \ref src to
 * \ref dest.
 */
void
region_copy_children (
  ZRegion * dest,
  ZRegion * src)
{
  g_return_if_fail (dest->id.type == src->id.type);

  g_message (
    "copying children from %d %s to %d %s",
    src->id.idx, src->name,
    dest->id.idx, dest->name);

  switch (src->id.type)
    {
    case REGION_TYPE_MIDI:
      {
        g_warn_if_fail (dest->num_midi_notes == 0);
        g_message (
          "%d midi notes", src->num_midi_notes);
        for (int i = 0;
             i < src->num_midi_notes; i++)
          {
            MidiNote * orig_mn =
              src->midi_notes[i];
            ArrangerObject * orig_mn_obj =
              (ArrangerObject *) orig_mn;

            MidiNote * mn =
              (MidiNote *)
              arranger_object_clone (
                orig_mn_obj,
                ARRANGER_OBJECT_CLONE_COPY_MAIN);

            midi_region_add_midi_note (
              dest, mn, F_NO_PUBLISH_EVENTS);
          }
      }
      break;
    case REGION_TYPE_AUDIO:
      break;
    case REGION_TYPE_AUTOMATION:
      {
        /* add automation points */
        AutomationPoint * src_ap, * dest_ap;
        for (int j = 0; j < src->num_aps; j++)
          {
            src_ap = src->aps[j];
            ArrangerObject * src_ap_obj =
              (ArrangerObject *) src_ap;

            dest_ap =
              automation_point_new_float (
                src_ap->fvalue,
                src_ap->normalized_val,
                &src_ap_obj->pos);
            automation_region_add_ap (
              dest, dest_ap, F_NO_PUBLISH_EVENTS);
          }
      }
      break;
    case REGION_TYPE_CHORD:
      {
        ChordObject * src_co, * dest_co;
        for (int i = 0;
             i < src->num_chord_objects; i++)
          {
            src_co = src->chord_objects[i];

            dest_co =
              (ChordObject *)
              arranger_object_clone (
                (ArrangerObject *) src_co,
                ARRANGER_OBJECT_CLONE_COPY_MAIN);

            chord_region_add_chord_object (
              dest, dest_co, F_NO_PUBLISH_EVENTS);
          }
      }
      break;
    }
}

/**
 * Returns the MidiNote matching the properties of
 * the given MidiNote.
 *
 * Used to find the actual MidiNote in the region
 * from a cloned MidiNote (e.g. when doing/undoing).
 */
MidiNote *
region_find_midi_note (
  ZRegion * r,
  MidiNote * clone)
{
  MidiNote * mn;
  for (int i = 0; i < r->num_midi_notes; i++)
    {
      mn = r->midi_notes[i];

      if (midi_note_is_equal (
            clone, mn))
        return mn;
    }

  return NULL;
}

/**
 * Gets the AutomationTrack using the saved index.
 */
AutomationTrack *
region_get_automation_track (
  ZRegion * region)
{
  Track * track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_val_if_fail (
    IS_TRACK (track) &&
    track->automation_tracklist.num_ats >
     region->id.at_idx, NULL);

  return
    track->automation_tracklist.ats[
      region->id.at_idx];
}

/**
 * Print region info for debugging.
 */
void
region_print (
  const ZRegion * self)
{
  char * str =
    g_strdup_printf (
      "%s [%s] - track pos %d - lane pos %d",
      self->name,
      region_type_bitvals[self->id.type].name,
      self->id.track_pos,
      self->id.lane_pos);
  g_message ("%s", str);
  g_free (str);
}

/**
 * Sets ZRegion name (without appending anything to
 * it) to all associated regions.
 */
void
region_set_name (
  ZRegion * self,
  char *   name,
  int      fire_events)
{
  arranger_object_set_name (
    (ArrangerObject *) self, name, fire_events);

  region_update_identifier (self);
}

/**
 * Returns the region at the given position in the
 * given Track.
 *
 * @param at The automation track to look in.
 * @param track The track to look in, if at is
 *   NULL.
 * @param pos The position.
 */
ZRegion *
region_at_position (
  Track    *        track,
  AutomationTrack * at,
  Position *        pos)
{
  ZRegion * region;
  if (track)
    {
      TrackLane * lane;
      for (int i = 0; i < track->num_lanes; i++)
        {
          lane = track->lanes[i];
          for (int j = 0; j < lane->num_regions; j++)
            {
              region = lane->regions[j];
              ArrangerObject * r_obj =
                (ArrangerObject *) region;
              if (position_is_after_or_equal (
                    pos, &r_obj->pos) &&
                  position_is_before_or_equal (
                    pos, &r_obj->end_pos))
                {
                  return region;
                }
            }
        }
    }
  else if (at)
    {
      for (int i = 0; i < at->num_regions; i++)
        {
          region = at->regions[i];
          ArrangerObject * r_obj =
            (ArrangerObject *) region;
          if (position_is_after_or_equal (
                pos, &r_obj->pos) &&
              position_is_before_or_equal (
                pos, &r_obj->end_pos))
            {
              return region;
            }
        }
    }
  return NULL;
}

/**
 * Returns if the position is inside the region
 * or not.
 *
 * @param gframes Global position in frames.
 * @param inclusive Whether the last frame should
 *   be counted as part of the region.
 */
int
region_is_hit (
  const ZRegion * region,
  const long     gframes,
  const int      inclusive)
{
  const ArrangerObject * r_obj =
    (const ArrangerObject *) region;
  return
    r_obj->pos.frames <=
      gframes &&
    ((inclusive &&
      r_obj->end_pos.frames >=
        gframes) ||
     (!inclusive &&
      r_obj->end_pos.frames >
        gframes));
}

/**
 * Returns the ArrangerSelections based on the
 * given region type.
 */
ArrangerSelections *
region_get_arranger_selections (
  ZRegion * self)
{
  ArrangerSelections * sel = NULL;
  switch (self->id.type)
    {
    case REGION_TYPE_MIDI:
      sel =
        (ArrangerSelections *) MA_SELECTIONS;
      break;
    case REGION_TYPE_AUTOMATION:
      sel =
        (ArrangerSelections *)
        AUTOMATION_SELECTIONS;
      break;
    case REGION_TYPE_CHORD:
      sel =
        (ArrangerSelections *)
        CHORD_SELECTIONS;
      break;
    default:
      break;
    }

  return sel;
}

/**
 * Returns whether the region is effectively in
 * musical mode.
 *
 * @note Only applicable to audio regions.
 */
bool
region_get_musical_mode (
  ZRegion * self)
{
  if (ZRYTHM_TESTING)
    {
      return true;
    }

  switch (self->musical_mode)
    {
    case REGION_MUSICAL_MODE_INHERIT:
      return
        g_settings_get_boolean (
          S_UI, "musical-mode");
    case REGION_MUSICAL_MODE_OFF:
      return false;
    case REGION_MUSICAL_MODE_ON:
      return true;
    }
  g_return_val_if_reached (false);
}

/**
 * Returns if any part of the ZRegion is inside the
 * given range, inclusive.
 */
int
region_is_hit_by_range (
  const ZRegion * region,
  const long     gframes_start,
  const long     gframes_end,
  const int      end_inclusive)
{
  const ArrangerObject * obj =
    (const ArrangerObject *) region;
  /* 4 cases:
   * - region start is inside range
   * - region end is inside range
   * - start is inside region
   * - end is inside region
   */
  if (end_inclusive)
    {
      return
        (gframes_start <=
           obj->pos.frames &&
         gframes_end >=
           obj->pos.frames) ||
        (gframes_start <=
           obj->end_pos.frames &&
         gframes_end >=
           obj->end_pos.frames) ||
        region_is_hit (region, gframes_start, 1) ||
        region_is_hit (region, gframes_end, 1);
    }
  else
    {
      return
        (gframes_start <=
           obj->pos.frames &&
         gframes_end >
           obj->pos.frames) ||
        (gframes_start <
           obj->end_pos.frames &&
         gframes_end >
           obj->end_pos.frames) ||
        region_is_hit (region, gframes_start, 0) ||
        region_is_hit (region, gframes_end, 0);
    }
}

/**
 * Copies the data from src to dest.
 *
 * Used when doing/undoing changes in name,
 * clip start point, loop start point, etc.
 */
void
region_copy (
  ZRegion * src,
  ZRegion * dest)
{
  g_free (dest->name);
  dest->name = g_strdup (src->name);

  ArrangerObject * src_obj =
    (ArrangerObject *) src;
  ArrangerObject * dest_obj =
    (ArrangerObject *) dest;

  dest_obj->clip_start_pos = src_obj->clip_start_pos;
  dest_obj->loop_start_pos = src_obj->loop_start_pos;
  dest_obj->loop_end_pos = src_obj->loop_end_pos;
  dest_obj->fade_in_pos = src_obj->fade_in_pos;
  dest_obj->fade_out_pos = src_obj->fade_out_pos;
}

/**
 * Converts frames on the timeline (global)
 * to local frames (in the clip).
 *
 * If normalize is 1 it will only return a position
 * from 0 to loop_end (it will traverse the
 * loops to find the appropriate position),
 * otherwise it may exceed loop_end.
 *
 * @param timeline_frames Timeline position in
 *   frames.
 *
 * @return The local frames.
 */
long
region_timeline_frames_to_local (
  ZRegion * region,
  const long     timeline_frames,
  const int      normalize)
{
  long diff_frames;

  ArrangerObject * r_obj =
    (ArrangerObject *) region;

  if (normalize)
    {
      if (region)
        {
          diff_frames =
            timeline_frames -
            r_obj->pos.frames;
          long loop_end_frames =
            r_obj->loop_end_pos.frames;
          long clip_start_frames =
            r_obj->clip_start_pos.frames;
          long loop_size =
            arranger_object_get_loop_length_in_frames (
              r_obj);
          g_return_val_if_fail (
            loop_size > 0, 0);

          diff_frames += clip_start_frames;

          while (diff_frames >= loop_end_frames)
            {
              diff_frames -= loop_size;
            }
        }
      else
        {
          diff_frames = 0;
        }

      return diff_frames;
    }
  else
    {
      if (region)
        {
          diff_frames =
            timeline_frames -
            r_obj->pos.frames;
        }
      else
        {
          diff_frames = 0;
        }

      return diff_frames;
    }
}

/**
 * Generates the filename for this region.
 *
 * MUST be free'd.
 *
 * FIXME logic needs changing
 */
char *
region_generate_filename (ZRegion * region)
{
  Track * track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  return
    g_strdup_printf (
      REGION_PRINTF_FILENAME, track->name,
      region->name);
}

/**
 * Returns if this region is currently being
 * recorded onto.
 */
bool
region_is_recording (
  ZRegion * self)
{
  if (!RECORDING_MANAGER->is_recording)
    {
      return false;
    }

  for (int i = 0;
       i < RECORDING_MANAGER->num_recorded_ids;
       i++)
    {
      if (region_identifier_is_equal (
            &self->id,
            &RECORDING_MANAGER->recorded_ids[i]))
        {
          return true;
        }
    }

  return false;
}

/**
 * Disconnects the region and anything using it.
 *
 * Does not free the ZRegion or its children's
 * resources.
 */
void
region_disconnect (
  ZRegion * self)
{
  ZRegion * clip_editor_region =
    clip_editor_get_region (CLIP_EDITOR);
  if (clip_editor_region == self)
    {
      clip_editor_set_region (
        CLIP_EDITOR, NULL, true);
    }
  if (TL_SELECTIONS)
    {
      arranger_selections_remove_object (
        (ArrangerSelections *) TL_SELECTIONS,
        (ArrangerObject *) self);
    }
  for (int i = 0; i < self->num_midi_notes; i++)
    {
      MidiNote * mn = self->midi_notes[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) MA_SELECTIONS,
        (ArrangerObject *) mn);
    }
  for (int i = 0; i < self->num_chord_objects; i++)
    {
      ChordObject * c = self->chord_objects[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) CHORD_SELECTIONS,
        (ArrangerObject *) c);
    }
  for (int i = 0; i < self->num_aps; i++)
    {
      AutomationPoint * ap = self->aps[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) AUTOMATION_SELECTIONS,
        (ArrangerObject *) ap);
    }
  if (ZRYTHM_HAVE_UI)
    {
      /*ARRANGER_WIDGET_GET_PRIVATE (MW_TIMELINE);*/
      /*if (ar_prv->start_object ==*/
            /*(ArrangerObject *) self)*/
        /*ar_prv->start_object = NULL;*/
    }
}

SERIALIZE_SRC (ZRegion, region)
DESERIALIZE_SRC (ZRegion, region)
PRINT_YAML_SRC (ZRegion, region)
