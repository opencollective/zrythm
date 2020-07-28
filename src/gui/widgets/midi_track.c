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

#include "audio/automatable.h"
#include "plugins/lv2_plugin.h"
#include "audio/automation_track.h"
#include "audio/automation_tracklist.h"
#include "audio/audio_bus_track.h"
#include "audio/engine.h"
#include "audio/midi_track.h"
#include "audio/track.h"
#include "audio/region.h"
#include "plugins/plugin.h"
#include "gui/widgets/arranger.h"
#include "gui/widgets/automation_track.h"
#include "gui/widgets/automation_tracklist.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/color_area.h"
#include "gui/widgets/midi_track.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/track.h"
#include "gui/widgets/track_lanelist.h"
#include "gui/widgets/track_top_grid.h"
#include "gui/widgets/tracklist.h"
#include "project.h"
#include "utils/gtk.h"
#include "utils/resources.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

G_DEFINE_TYPE (MidiTrackWidget,
               midi_track_widget,
               TRACK_WIDGET_TYPE)

void
midi_track_widget_refresh_buttons (
  MidiTrackWidget * self)
{
  TRACK_WIDGET_GET_PRIVATE (self);
  g_signal_handler_block (
    self->record, tw_prv->record_toggle_handler_id);
      gtk_toggle_button_set_active (
        self->record,
        tw_prv->track->recording);
  g_signal_handler_unblock (
    self->record, tw_prv->record_toggle_handler_id);

  g_signal_handler_block (
    self->solo, tw_prv->solo_toggled_handler_id);
      gtk_toggle_button_set_active (
        self->solo,
        tw_prv->track->solo);
  g_signal_handler_unblock (
    self->solo, tw_prv->solo_toggled_handler_id);

  g_signal_handler_block (
    self->mute, tw_prv->mute_toggled_handler_id);
      gtk_toggle_button_set_active (
        self->mute,
        tw_prv->track->mute);
  g_signal_handler_unblock (
    self->mute, tw_prv->mute_toggled_handler_id);

}

void
midi_track_widget_refresh (
  MidiTrackWidget * self)
{
  TRACK_WIDGET_GET_PRIVATE (self);
  Track * track = tw_prv->track;

  midi_track_widget_refresh_buttons (self);

  track_widget_set_name (
    Z_TRACK_WIDGET (self), track->name);

  AutomationTracklist * automation_tracklist =
    track_get_automation_tracklist (tw_prv->track);
  automation_tracklist_widget_refresh (
    automation_tracklist->widget);

  track_lanelist_widget_refresh (tw_prv->lanelist);
}

/**
 * Creates a new track widget using the given track.
 *
 * 1 track has 1 track widget.
 * The track widget must always have at least 1 automation track in the automation
 * paned.
 */
MidiTrackWidget *
midi_track_widget_new (Track * track)
{
  MidiTrackWidget * self =
    g_object_new (
      MIDI_TRACK_WIDGET_TYPE,
      NULL);

  TRACK_WIDGET_GET_PRIVATE (self);

  /* setup color */
  color_area_widget_set_color (tw_prv->color,
                               &track->color);

  /* create lanelist */
  tw_prv->lanelist =
    track_lanelist_widget_new (track);
  gtk_container_add (
    GTK_CONTAINER (tw_prv->paned),
    GTK_WIDGET (tw_prv->lanelist));

  /* setup automation tracklist */
  AutomationTracklist * automation_tracklist =
    track_get_automation_tracklist (track);
  automation_tracklist->widget =
    automation_tracklist_widget_new (
      automation_tracklist);
  gtk_container_add (
    GTK_CONTAINER (tw_prv->paned),
    GTK_WIDGET (automation_tracklist->widget));

  tw_prv->record_toggle_handler_id =
    g_signal_connect (
      self->record, "toggled",
      G_CALLBACK (track_widget_on_record_toggled),
      self);
  tw_prv->mute_toggled_handler_id =
    g_signal_connect (
      self->mute, "toggled",
      G_CALLBACK (track_widget_on_mute_toggled),
      self);
  tw_prv->solo_toggled_handler_id =
    g_signal_connect (
      self->solo, "toggled",
      G_CALLBACK (track_widget_on_solo_toggled),
      self);
  g_signal_connect (
    self->show_lanes,
    "toggled",
    G_CALLBACK (track_widget_on_show_lanes_toggled),
    self);
  g_signal_connect (
    self->show_automation,
    "toggled",
    G_CALLBACK (track_widget_on_show_automation_toggled),
    self);

  /*ChannelTrack * ct = (ChannelTrack *) track;*/
  /*Channel * chan = ct->channel;*/
  /*Plugin * plugin = chan->plugins[0];*/

  return self;
}

static void
midi_track_widget_init (
  MidiTrackWidget * self)
{
  GtkStyleContext * context;
  TRACK_WIDGET_GET_PRIVATE (self);

  /* create buttons */
  self->record =
    z_gtk_toggle_button_new_with_icon (
      "media-record");
  gtk_widget_set_tooltip_text (
    GTK_WIDGET (self->record),
    _("Record"));
  context =
    gtk_widget_get_style_context (
      GTK_WIDGET (self->record));
  gtk_style_context_add_class (
    context, "record-button");
  self->solo =
    z_gtk_toggle_button_new_with_resource (
      ICON_TYPE_ZRYTHM,
      "solo.svg");
  gtk_widget_set_tooltip_text (
    GTK_WIDGET (self->solo), "Solo");
  context =
    gtk_widget_get_style_context (
      GTK_WIDGET (self->solo));
  gtk_style_context_add_class (
    context, "solo-button");
  self->mute =
    z_gtk_toggle_button_new_with_resource (
      ICON_TYPE_ZRYTHM,
      "mute.svg");
  gtk_widget_set_tooltip_text (
    GTK_WIDGET (self->mute), "Mute");
  self->show_automation =
    z_gtk_toggle_button_new_with_icon (
      "node-type-cusp");
  ui_add_widget_tooltip (
    self->show_automation,
    _("Show automation lanes"));
  self->show_lanes =
    z_gtk_toggle_button_new_with_icon (
      "format-justify-fill");
  ui_add_widget_tooltip (
    self->show_lanes,
    _("Show track lanes"));
  self->lock =
    z_gtk_toggle_button_new_with_icon (
      "document-decrypt");
  gtk_widget_set_tooltip_text (
    GTK_WIDGET (self->lock),
    _("Lock track"));

  /* set buttons to upper controls */
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->upper_controls),
    GTK_WIDGET (self->record),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->upper_controls),
    GTK_WIDGET (self->solo),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->upper_controls),
    GTK_WIDGET (self->mute),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);

  /* pack buttons to bot controls */
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->bot_controls),
    GTK_WIDGET (self->lock),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->bot_controls),
    GTK_WIDGET (self->show_lanes),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);
  gtk_box_pack_start (
    GTK_BOX (tw_prv->top_grid->bot_controls),
    GTK_WIDGET (self->show_automation),
    Z_GTK_NO_EXPAND,
    Z_GTK_NO_FILL,
    0);

  /* set icon */
  SET_TRACK_ICON ("audio-midi");

  gtk_widget_show_all (GTK_WIDGET (self));
}

static void
midi_track_widget_class_init (
  MidiTrackWidgetClass * klass)
{
}

