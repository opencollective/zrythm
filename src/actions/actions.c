/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>
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
 * GAction actions.
 */

#include "config.h"

#include "actions/actions.h"
#include "actions/undo_manager.h"
#include "actions/copy_tracks_action.h"
#include "actions/create_timeline_selections_action.h"
#include "actions/create_tracks_action.h"
#include "actions/delete_tracks_action.h"
#include "actions/delete_midi_arranger_selections_action.h"
#include "actions/delete_timeline_selections_action.h"
#include "actions/duplicate_midi_arranger_selections_action.h"
#include "actions/quantize_timeline_selections.h"
#include "audio/instrument_track.h"
#include "audio/midi.h"
#include "audio/track.h"
#include "audio/transport.h"
#include "gui/backend/midi_arranger_selections.h"
#include "gui/backend/timeline_selections.h"
#include "gui/backend/tracklist_selections.h"
#include "gui/widgets/about_dialog.h"
#include "gui/widgets/arranger.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/bot_bar.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/center_dock_bot_box.h"
#include "gui/widgets/clip_editor.h"
#include "gui/widgets/clip_editor_inner.h"
#include "gui/widgets/export_dialog.h"
#include "gui/widgets/file_browser_window.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/midi_arranger.h"
#include "gui/widgets/midi_modifier_arranger.h"
#include "gui/widgets/editor_ruler.h"
#include "gui/widgets/midi_editor_space.h"
#include "gui/widgets/mixer.h"
#include "gui/widgets/preferences.h"
#include "gui/widgets/quantize_dialog.h"
#include "gui/widgets/ruler.h"
#include "gui/widgets/timeline_arranger.h"
#include "gui/widgets/timeline_bg.h"
#include "gui/widgets/timeline_minimap.h"
#include "gui/widgets/timeline_ruler.h"
#include "gui/widgets/toolbox.h"
#include "gui/widgets/tracklist.h"
#include "project.h"
#include "utils/dialogs.h"
#include "utils/flags.h"
#include "utils/gtk.h"
#include "utils/io.h"
#include "utils/localization.h"
#include "utils/resources.h"
#include "utils/string.h"

#include <gtk/gtk.h>

#include <glib/gi18n.h>

static GtkWindow * file_browser_window = NULL;

void
action_enable_window_action (
  const char * action_name)
{
  GAction * action =
    g_action_map_lookup_action (
      G_ACTION_MAP (MAIN_WINDOW),
      action_name);
  g_simple_action_set_enabled (
    G_SIMPLE_ACTION (action),
    1);
}

void
action_disable_window_action (
  const char * action_name)
{
  GAction * action =
    g_action_map_lookup_action (
      G_ACTION_MAP (MAIN_WINDOW),
      action_name);
  g_simple_action_set_enabled (
    G_SIMPLE_ACTION (action),
    0);
}

void
action_enable_app_action (
  const char * action_name)
{
  GAction * action =
    g_action_map_lookup_action (
      G_ACTION_MAP (ZRYTHM),
      action_name);
  g_simple_action_set_enabled (
    G_SIMPLE_ACTION (action),
    1);
}

void
action_disable_app_action (
  const char * action_name)
{
  GAction * action =
    g_action_map_lookup_action (
      G_ACTION_MAP (ZRYTHM),
      action_name);
  g_simple_action_set_enabled (
    G_SIMPLE_ACTION (action),
    0);
}

void
activate_news (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    "https://git.zrythm.org/cgit/zrythm/plain/"
    "CHANGELOG.md",
    0,
    NULL);
}

void
activate_manual (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
#ifdef MANUAL_PATH
  UiLanguage lang =
    (UiLanguage)
    g_settings_get_enum (
      S_PREFERENCES,
      "language");
  char * lang_code =
    localization_get_string_code (lang);
  char * path =
    g_strdup_printf (
      "file://%s/%s/index.html",
      MANUAL_PATH,
      lang_code);
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    path,
    0,
    NULL);
  g_free (lang_code);
  g_free (path);
#else
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    "https://manual.zrythm.org",
    0,
    NULL);
#endif
}

void
activate_chat (GSimpleAction *action,
                GVariant      *variant,
                gpointer       user_data)
{
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    "https://matrix.to/#/#zrythmdaw:matrix.org",
    0,
    NULL);
}

void
activate_donate (GSimpleAction *action,
                GVariant      *variant,
                gpointer       user_data)
{
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    "https://liberapay.com/Zrythm",
    0,
    NULL);
}

void
activate_bugreport (GSimpleAction *action,
                GVariant      *variant,
                gpointer       user_data)
{
#ifdef _WIN32
  ShellExecute (
    0, (LPCSTR)"open",
    (LPCSTR) NEW_ISSUE_URL,
    0, 0, SW_SHOWNORMAL);
#else
  gtk_show_uri_on_window (
    GTK_WINDOW (MAIN_WINDOW),
    NEW_ISSUE_URL,
    0,
    NULL);
#endif
}

void
activate_about (GSimpleAction *action,
                GVariant      *variant,
                gpointer       user_data)
{
  GtkDialog * dialog =
    GTK_DIALOG (
      about_dialog_widget_new (
        GTK_WINDOW (MAIN_WINDOW)));
  gtk_dialog_run (dialog);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
activate_quit (GSimpleAction *action,
               GVariant      *variant,
               gpointer       user_data)
{
  g_application_quit (G_APPLICATION (user_data));
}

void
activate_shortcuts (GSimpleAction *action,
                    GVariant      *variant,
                    gpointer       user_data)
{
  GtkBuilder *builder;
  GtkWidget *overlay;

  builder =
    gtk_builder_new_from_resource (
      RESOURCE_PATH TEMPLATE_PATH "shortcuts.ui");
  overlay =
    GTK_WIDGET (
      gtk_builder_get_object (builder, "shortcuts-builder"));
  gtk_window_set_transient_for (
    GTK_WINDOW (overlay),
    GTK_WINDOW (MAIN_WINDOW));
  gtk_widget_show (overlay);
  g_object_unref (builder);
}

/**
 * Show preferences window.
 */
void
activate_preferences (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  PreferencesWidget * widget =
    preferences_widget_new ();
  gtk_window_set_transient_for (
    GTK_WINDOW (widget),
    GTK_WINDOW (MAIN_WINDOW));
  gtk_widget_set_visible (GTK_WIDGET (widget),
                          1);
}

/**
 * Activate audition mode.
 */
void
activate_audition_mode (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  P_TOOL = TOOL_AUDITION;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/**
 * Activate select mode.
 */
void
activate_select_mode (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  if (P_TOOL == TOOL_SELECT_NORMAL)
    P_TOOL = TOOL_SELECT_STRETCH;
  else
    P_TOOL = TOOL_SELECT_NORMAL;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/**
 * Activate edit mode.
 */
void
activate_edit_mode (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  P_TOOL = TOOL_EDIT;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/**
 * Activate cut mode.
 */
void
activate_cut_mode (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  P_TOOL = TOOL_CUT;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/**
 * Activate eraser mode.
 */
void
activate_eraser_mode (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  P_TOOL = TOOL_ERASER;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/**
 * Activate ramp mode.
 */
void
activate_ramp_mode (GSimpleAction *action,
                      GVariant      *variant,
                      gpointer       user_data)
{
  P_TOOL = TOOL_RAMP;
  EVENTS_PUSH (ET_TOOL_CHANGED, NULL);
}

/* FIXME rename to timeline zoom in */
void
activate_zoom_in (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  RULER_WIDGET_GET_PRIVATE (
    Z_RULER_WIDGET (MW_RULER));
  ruler_widget_set_zoom_level (
    Z_RULER_WIDGET (MW_RULER),
    rw_prv->zoom_level * 1.3);

  EVENTS_PUSH (ET_TIMELINE_VIEWPORT_CHANGED,
               NULL);
}

void
activate_zoom_out (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  RULER_WIDGET_GET_PRIVATE (
    Z_RULER_WIDGET (MW_RULER));
  double zoom_level = rw_prv->zoom_level / 1.3;
  ruler_widget_set_zoom_level (
    Z_RULER_WIDGET (MW_RULER),
    zoom_level);

  EVENTS_PUSH (ET_TIMELINE_VIEWPORT_CHANGED,
               NULL);
}

void
activate_best_fit (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  /* TODO */
  g_message ("ZOOMING IN");

  EVENTS_PUSH (ET_TIMELINE_VIEWPORT_CHANGED,
               NULL);
}

void
activate_original_size (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  RULER_WIDGET_GET_PRIVATE (
    Z_RULER_WIDGET (MW_RULER));
  rw_prv->zoom_level = (double) DEFAULT_ZOOM_LEVEL;

  EVENTS_PUSH (ET_TIMELINE_VIEWPORT_CHANGED,
               NULL);
}

void
activate_loop_selection (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (MAIN_WINDOW->last_focused ==
      GTK_WIDGET (MW_TIMELINE))
    {
      if (!timeline_selections_has_any (
            TL_SELECTIONS))
        return;

      Position start, end;
      timeline_selections_get_start_pos (
        TL_SELECTIONS, &start, F_NO_TRANSIENTS);
      timeline_selections_get_end_pos (
        TL_SELECTIONS, &end, F_NO_TRANSIENTS);

      position_set_to_pos (
        &TRANSPORT->loop_start_pos,
        &start);
      position_set_to_pos (
        &TRANSPORT->loop_end_pos,
        &end);

      transport_update_position_frames (
        TRANSPORT);
      EVENTS_PUSH (
        ET_TIMELINE_LOOP_MARKER_POS_CHANGED,
        NULL);
    }
}

void
activate_new (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  g_message ("ZOOMING IN");
}

static int
run_open_dialog (GtkDialog * dialog)
{
  int res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser =
        GTK_FILE_CHOOSER (dialog);
      char * filename =
        gtk_file_chooser_get_filename (chooser);
      g_message ("filename %s", filename);
      res = project_load (filename, 0);
      g_free (filename);
    }

  return res;
}

void
activate_open (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  GtkDialog * dialog =
    dialogs_get_open_project_dialog (
      GTK_WINDOW (MAIN_WINDOW));

  int res = run_open_dialog (dialog);
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename =
        gtk_file_chooser_get_filename (
          GTK_FILE_CHOOSER (dialog));
      project_load (filename, 0);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
activate_save (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
   if (!PROJECT->dir ||
       !PROJECT->title)
     {
       activate_save_as (action,
                         variant,
                         user_data);
       return;
     }
   g_message ("%s project dir", PROJECT->dir);

   project_save (PROJECT, PROJECT->dir, 0, 1);
}

void
activate_save_as (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction _action =
    GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  dialog =
    gtk_file_chooser_dialog_new (
      _("Save Project"),
      GTK_WINDOW (MAIN_WINDOW),
      _action,
      _("_Cancel"),
      GTK_RESPONSE_CANCEL,
      _("_Save"),
      GTK_RESPONSE_ACCEPT,
      NULL);
  chooser = GTK_FILE_CHOOSER (dialog);

  gtk_file_chooser_set_do_overwrite_confirmation (
    chooser, TRUE);

  char * project_file_path =
    project_get_project_file_path (
      PROJECT, 0);
  char * str =
    io_path_get_parent_dir (
      project_file_path);
  g_free (project_file_path);
  gtk_file_chooser_select_filename (
    chooser,
    str);
  g_free (str);
  gtk_file_chooser_set_current_name (
    chooser, PROJECT->title);

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;

      filename =
        gtk_file_chooser_get_filename (chooser);
      project_save (PROJECT, filename, 0, 1);
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

void
activate_iconify (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  gtk_window_iconify (GTK_WINDOW (MAIN_WINDOW));
}

void
activate_export_as (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  ExportDialogWidget * export =
    export_dialog_widget_new ();
  gtk_window_set_transient_for (
    GTK_WINDOW (export),
    GTK_WINDOW (MAIN_WINDOW));
  gtk_dialog_run (GTK_DIALOG (export));
  gtk_widget_destroy (GTK_WIDGET (export));
}

void
activate_properties (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  g_message ("ZOOMING IN");
}

void
activate_undo (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (stack_is_empty (UNDO_MANAGER->undo_stack))
    return;
  undo_manager_undo (UNDO_MANAGER);

  EVENTS_PUSH (ET_UNDO_REDO_ACTION_DONE,
               NULL);
}

void
activate_redo (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (stack_is_empty (UNDO_MANAGER->redo_stack))
    return;
  undo_manager_redo (UNDO_MANAGER);

  EVENTS_PUSH (ET_UNDO_REDO_ACTION_DONE,
               NULL);
}

void
activate_cut (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  g_message ("ZOOMING IN");
}


void
activate_copy (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (MAIN_WINDOW->last_focused == GTK_WIDGET (MW_TIMELINE))
    {
      gtk_clipboard_set_text (
        DEFAULT_CLIPBOARD,
        timeline_selections_serialize (
          TL_SELECTIONS),
        -1);
    }
}

static void
on_timeline_clipboard_received (
  GtkClipboard *     clipboard,
  const char *       text,
  gpointer           data)
{
  g_message ("clipboard data received: %s",
             text);

  if (!text)
    return;
  TimelineSelections * ts =
    timeline_selections_deserialize (text);
  timeline_selections_post_deserialize (ts);
  g_message ("printing deserialized");
  timeline_selections_print_yaml (ts);

  if (MAIN_WINDOW->last_focused ==
      GTK_WIDGET (MW_TIMELINE))
    {
      timeline_selections_paste_to_pos (
        ts, PLAYHEAD);

      /* create action to make it undoable */
      /* (by now the TL_SELECTIONS should have
       * only the pasted items selected) */
      UndoableAction * ua =
        (UndoableAction *)
        create_timeline_selections_action_new (
          TL_SELECTIONS);
      undo_manager_perform (
        UNDO_MANAGER, ua);
    }

  /* free ts */
  g_message ("paste done");
}

void
activate_paste (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  g_message ("paste");
  gtk_clipboard_request_text (
    DEFAULT_CLIPBOARD,
    on_timeline_clipboard_received,
    NULL);
}

void
activate_delete (
  GSimpleAction * simple_action,
  GVariant      * variant,
  gpointer        user_data)
{
  if (MAIN_WINDOW->last_focused ==
        GTK_WIDGET (MW_TIMELINE))
    {
      UndoableAction * action =
        delete_timeline_selections_action_new (
          TL_SELECTIONS);
      undo_manager_perform (UNDO_MANAGER,
                            action);
    }
  else if (MAIN_WINDOW->last_focused ==
             GTK_WIDGET (MW_MIDI_ARRANGER))
    {
      UndoableAction * action =
        delete_midi_arranger_selections_action_new (
          MA_SELECTIONS);
      undo_manager_perform (
        UNDO_MANAGER, action);
    }
}

void
activate_duplicate (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (MAIN_WINDOW->last_focused ==
        GTK_WIDGET (MW_TIMELINE))
    {
    }
  else if (MAIN_WINDOW->last_focused ==
             GTK_WIDGET (MW_MIDI_ARRANGER))
    {
    }
}

void
activate_clear_selection (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  g_message ("ZOOMING IN");
}

void
activate_select_all (
	GSimpleAction *action,
	GVariant *variant,
	gpointer user_data)
{
	if (MAIN_WINDOW->last_focused
		== GTK_WIDGET (MW_MIDI_ARRANGER))
	{
		midi_arranger_widget_select_all (
			Z_MIDI_ARRANGER_WIDGET (
				MAIN_WINDOW->last_focused),
			1);
	}
}

void
activate_toggle_left_panel (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  CenterDockWidget * self = MW_CENTER_DOCK;
  GValue a = G_VALUE_INIT;
  g_value_init (&a, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (self),
                         "left-visible",
                         &a);
  int val = g_value_get_boolean (&a);
  g_value_set_boolean (&a,
                       val == 1 ? 0 : 1);
  g_object_set_property (G_OBJECT (self),
                         "left-visible",
                         &a);

  /* TODO update header bar buttons */
}

void
activate_toggle_right_panel (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  CenterDockWidget * self = MW_CENTER_DOCK;
  GValue a = G_VALUE_INIT;
  g_value_init (&a, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (self),
                         "right-visible",
                         &a);
  int val = g_value_get_boolean (&a);
  g_value_set_boolean (&a,
                       val == 1 ? 0 : 1);
  g_object_set_property (G_OBJECT (self),
                         "right-visible",
                         &a);
}

void
activate_toggle_bot_panel (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  CenterDockWidget * self = MW_CENTER_DOCK;
  GValue a = G_VALUE_INIT;
  g_value_init (&a, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (self),
                         "bottom-visible",
                         &a);
  int val = g_value_get_boolean (&a);
  g_value_set_boolean (&a,
                       val == 1 ? 0 : 1);
  g_object_set_property (G_OBJECT (self),
                         "bottom-visible",
                         &a);
  gtk_widget_set_visible (
    GTK_WIDGET (MW_BOT_DOCK_EDGE),
    !gtk_widget_get_visible (
      GTK_WIDGET (MW_BOT_DOCK_EDGE)));
}

/**
 * Toggle timeline visibility.
 */
void
activate_toggle_top_panel (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  gtk_widget_set_visible (
    GTK_WIDGET (MW_CENTER_DOCK->editor_top),
    !gtk_widget_get_visible (
      GTK_WIDGET (MW_CENTER_DOCK->editor_top)));
}

void
activate_toggle_status_bar (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  gtk_widget_set_visible (
    GTK_WIDGET (MW_BOT_BAR),
    !gtk_widget_get_visible (
      GTK_WIDGET (MW_BOT_BAR)));
}

/**
 * Toggle drum mode in the piano roll.
 */
void
activate_toggle_drum_mode (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  PIANO_ROLL->drum_mode = !PIANO_ROLL->drum_mode;

  EVENTS_PUSH (ET_DRUM_MODE_CHANGED, NULL);
}

void
activate_fullscreen (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  if (MAIN_WINDOW->is_maximized)
    {
      gtk_window_unmaximize (
        GTK_WINDOW (MAIN_WINDOW));
    }
  else
    {
      gtk_window_maximize (
        GTK_WINDOW (MAIN_WINDOW));
    }
}

void
activate_snap_to_grid (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  SNAP_GRID_TIMELINE->snap_to_grid =
    !SNAP_GRID_TIMELINE->snap_to_grid;
}

void
activate_snap_keep_offset (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  SNAP_GRID_TIMELINE->snap_to_grid_keep_offset =
    !SNAP_GRID_TIMELINE->snap_to_grid_keep_offset;
}

void
activate_snap_events (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  SNAP_GRID_TIMELINE->snap_to_events =
    !SNAP_GRID_TIMELINE->snap_to_events;
}

void
activate_create_audio_track (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_AUDIO,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_ins_track (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_INSTRUMENT,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_midi_track (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_MIDI,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_audio_bus_track (GSimpleAction *action,
                  GVariant      *variant,
                  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_AUDIO_BUS,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_midi_bus_track (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_MIDI_BUS,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_audio_group_track (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_AUDIO_GROUP,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_create_midi_group_track (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  UndoableAction * ua =
    create_tracks_action_new (
      TRACK_TYPE_MIDI_GROUP,
      NULL,
      NULL,
      TRACKLIST->num_tracks,
      1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_duplicate_selected_tracks (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  UndoableAction * ua =
    copy_tracks_action_new (
      TRACKLIST_SELECTIONS,
      TRACKLIST_SELECTIONS->tracks[0]->pos + 1);

  undo_manager_perform (UNDO_MANAGER, ua);
}

void
activate_goto_prev_marker (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  transport_goto_prev_marker (
    TRANSPORT);
}

void
activate_goto_next_marker (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  transport_goto_next_marker (
    TRANSPORT);
}

void
activate_play_pause (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  if (TRANSPORT_IS_ROLLING)
    {
      transport_request_pause (TRANSPORT);
      midi_panic_all (1);
    }
  else if (TRANSPORT_IS_PAUSED)
    {
      transport_request_roll (TRANSPORT);
    }
}

void
activate_delete_selected_tracks (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  g_message ("deleting selected tracks");

  UndoableAction * ua =
    delete_tracks_action_new (
      TRACKLIST_SELECTIONS);
  undo_manager_perform (
    UNDO_MANAGER, ua);
}

void
activate_hide_selected_tracks (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  g_message ("hiding selected tracks");

  tracklist_selections_toggle_visibility (
    TRACKLIST_SELECTIONS);
}

void
change_state_dim_output (
  GSimpleAction * action,
  GVariant *      value,
  gpointer        user_data)
{
  int dim = g_variant_get_boolean (value);

  g_message ("dim is %d", dim);

  g_simple_action_set_state (action, value);
}

void
change_state_loop (
  GSimpleAction * action,
  GVariant *      value,
  gpointer        user_data)
{
  int loop = g_variant_get_boolean (value);

  g_simple_action_set_state (action, value);

  transport_set_loop (TRANSPORT, loop);
}

void
change_state_metronome (
  GSimpleAction * action,
  GVariant *      value,
  gpointer        user_data)
{
  int enabled = g_variant_get_boolean (value);

  g_simple_action_set_state (action, value);

  g_settings_set_int (
    S_UI,
    "metronome-enabled",
    enabled);
  transport_set_metronome_enabled (
    TRANSPORT, enabled);
}

void
activate_quick_quantize (
  GSimpleAction *action,
  GVariant      * _variant,
  gpointer       user_data)
{
  g_return_if_fail (_variant);

  gsize size;
  const char * variant =
    g_variant_get_string (_variant, &size);
  if (string_is_equal (variant, "timeline", 1))
    {
      UndoableAction * ua =
        (UndoableAction *)
        quantize_timeline_selections_action_new (
          TL_SELECTIONS,
          QUANTIZE_OPTIONS_TIMELINE);
      undo_manager_perform (
        UNDO_MANAGER, ua);
    }
  else if (string_is_equal (variant, "editor", 1))
    {
    }
  else if (string_is_equal (variant, "global", 1))
    {
      if (MAIN_WINDOW->last_focused ==
          GTK_WIDGET (MW_TIMELINE))
        {
          UndoableAction * ua =
            (UndoableAction *)
            quantize_timeline_selections_action_new (
              TL_SELECTIONS,
              QUANTIZE_OPTIONS_TIMELINE);
          undo_manager_perform (
            UNDO_MANAGER, ua);
        }
    }
  else
    {
      g_return_if_reached ();
    }
  g_message ("quantize");
}

void
activate_quantize_options (
  GSimpleAction *action,
  GVariant      *_variant,
  gpointer       user_data)
{
  g_return_if_fail (_variant);

  gsize size;
  const char * variant =
    g_variant_get_string (_variant, &size);
  if (string_is_equal (variant, "timeline", 1))
    {
      QuantizeDialogWidget * quant =
        quantize_dialog_widget_new (
          QUANTIZE_OPTIONS_TIMELINE);
      gtk_window_set_transient_for (
        GTK_WINDOW (quant),
        GTK_WINDOW (MAIN_WINDOW));
      gtk_dialog_run (GTK_DIALOG (quant));
      gtk_widget_destroy (GTK_WIDGET (quant));
    }
  else if (string_is_equal (variant, "editor", 1))
    {
      QuantizeDialogWidget * quant =
        quantize_dialog_widget_new (
          QUANTIZE_OPTIONS_EDITOR);
      gtk_window_set_transient_for (
        GTK_WINDOW (quant),
        GTK_WINDOW (MAIN_WINDOW));
      gtk_dialog_run (GTK_DIALOG (quant));
      gtk_widget_destroy (GTK_WIDGET (quant));
    }
  else if (string_is_equal (variant, "global", 1))
    {
      if (MAIN_WINDOW->last_focused ==
          GTK_WIDGET (MW_TIMELINE))
        {
          QuantizeDialogWidget * quant =
            quantize_dialog_widget_new (
              QUANTIZE_OPTIONS_TIMELINE);
          gtk_window_set_transient_for (
            GTK_WINDOW (quant),
            GTK_WINDOW (MAIN_WINDOW));
          gtk_dialog_run (GTK_DIALOG (quant));
          gtk_widget_destroy (GTK_WIDGET (quant));
        }
    }
  else
    {
      g_return_if_reached ();
    }
  g_message ("quantize opts");
}

void
activate_set_timebase_master (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  g_message ("set time base master");
}

void
activate_set_transport_client (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  g_message ("set transport client");
}

void
activate_unlink_jack_transport (
  GSimpleAction *action,
  GVariant      *variant,
  gpointer       user_data)
{
  g_message ("unlink jack transport");
}

void
activate_show_file_browser (
  GSimpleAction * action,
  GVariant *      variant,
  gpointer        user_data)
{
  if (file_browser_window)
    {
      gtk_window_close (
        file_browser_window);
      file_browser_window = FALSE;
    }
  else
    {
      file_browser_window =
        GTK_WINDOW (
          file_browser_window_widget_new ());
      g_return_if_fail (file_browser_window);
      gtk_window_set_transient_for (
        file_browser_window,
        (GtkWindow *) MAIN_WINDOW);
      gtk_widget_show_all (
        (GtkWidget *) file_browser_window);
    }
}
