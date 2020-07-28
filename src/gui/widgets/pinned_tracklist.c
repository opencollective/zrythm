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

#include "audio/engine.h"
#include "audio/track.h"
#include "audio/tracklist.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/pinned_tracklist.h"
#include "gui/widgets/track.h"
#include "gui/widgets/timeline_panel.h"
#include "gui/backend/event_manager.h"
#include "project.h"
#include "utils/gtk.h"
#include "zrythm.h"

G_DEFINE_TYPE (PinnedTracklistWidget,
               pinned_tracklist_widget,
               GTK_TYPE_BOX)

/**
 * Gets TrackWidget hit at the given coordinates.
 */
TrackWidget *
pinned_tracklist_widget_get_hit_track (
  PinnedTracklistWidget *  self,
  double            x,
  double            y)
{
  /* go through each child */
  Track * track;
  TrackWidget * tw;
  GtkAllocation allocation;
  gint wx, wy;
  for(int i = 0;
      i < self->tracklist->num_tracks; i++)
    {
      track = self->tracklist->tracks[i];
      if (!track->visible || !track->pinned)
        continue;

      tw = track->widget;

      gtk_widget_get_allocation (
        GTK_WIDGET (tw), &allocation);

      gtk_widget_translate_coordinates (
        GTK_WIDGET (self),
        GTK_WIDGET (tw),
        (int) x, (int) y, &wx, &wy);

      /* if hit */
      if (wx >= 0 &&
          wx <= allocation.width &&
          wy >= 0 &&
          wy <= allocation.height)
        {
          return tw;
        }
    }
  return NULL;
}

/**
 * Removes and readds the tracks.
 */
void
pinned_tracklist_widget_hard_refresh (
  PinnedTracklistWidget * self)
{
  /* remove all tracks */
  z_gtk_container_remove_all_children (
    GTK_CONTAINER (self));

  /* add tracks */
  Track * track;
  for(int i = 0;
      i < self->tracklist->num_tracks; i++)
    {
      track = self->tracklist->tracks[i];
      if (!track->visible || !track->pinned)
        continue;

      /* create widget */
      if (!GTK_IS_WIDGET (track->widget))
        track->widget = track_widget_new (track);

      track_widget_refresh (track->widget);

      /* add to tracklist widget */
      gtk_container_add (
        GTK_CONTAINER (self),
        GTK_WIDGET (track->widget));
    }
  /*GtkWidget * sep =*/
    /*gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);*/
  /*gtk_widget_set_visible (sep, 1);*/
  /*gtk_container_add (*/
    /*GTK_CONTAINER (self),*/
    /*sep);*/

  /* set handle position.
   * this is done because the position resets to
   * -1 every time a child is added or deleted */
  GList *children, *iter;
  children =
    gtk_container_get_children (
      GTK_CONTAINER (self));
  for (iter = children;
       iter != NULL;
       iter = g_list_next (iter))
    {
      if (Z_IS_TRACK_WIDGET (iter->data))
        {
          TrackWidget * tw =
            Z_TRACK_WIDGET (iter->data);
          TRACK_WIDGET_GET_PRIVATE (tw);
          track = tw_prv->track;
          GValue a = G_VALUE_INIT;
          g_value_init (&a, G_TYPE_INT);
          g_value_set_int (&a, track->handle_pos);
          gtk_container_child_set_property (
            GTK_CONTAINER (self),
            GTK_WIDGET (tw),
            "position",
            &a);
        }
    }
  g_list_free(children);
}

static void
on_size_allocate (
  GtkWidget    *widget,
  GdkRectangle *allocation,
  PinnedTracklistWidget * self)
{
  /*gtk_widget_set_size_request (*/
    /*GTK_WIDGET (*/
      /*MW_CENTER_DOCK->pinned_timeline_scroll),*/
    /*-1, allocation->height);*/
  if (gtk_paned_get_position (
        MW_TIMELINE_PANEL->timeline_divider_pane) !=
      allocation->height)
    {
      gtk_paned_set_position (
        MW_TIMELINE_PANEL->timeline_divider_pane,
        allocation->height);
    }
}

/**
 * Sets up the PinnedTracklistWidget.
 */
void
pinned_tracklist_widget_setup (
  PinnedTracklistWidget * self,
  Tracklist * tracklist)
{
  g_warn_if_fail (tracklist);
  self->tracklist = tracklist;
  tracklist->pinned_widget = self;

  pinned_tracklist_widget_hard_refresh (self);

  g_signal_connect (
    G_OBJECT (self), "size-allocate",
    G_CALLBACK (on_size_allocate), self);
}

static void
pinned_tracklist_widget_class_init (
  PinnedTracklistWidgetClass * _klass)
{
  GtkWidgetClass * klass =
    GTK_WIDGET_CLASS (_klass);

  gtk_widget_class_set_css_name (
    klass, "ruler-tracklist");
}

static void
pinned_tracklist_widget_init (
  PinnedTracklistWidget * self)
{
  gtk_orientable_set_orientation (
    GTK_ORIENTABLE (self),
    GTK_ORIENTATION_VERTICAL);
  gtk_box_set_spacing (
    GTK_BOX (self), 1);
}
