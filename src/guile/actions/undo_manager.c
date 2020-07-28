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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "guile/modules.h"

#ifndef SNARF_MODE
#include "actions/undo_manager.h"
#include "project.h"
#endif

SCM_DEFINE (
  s_undo_manager_perform,
  "undo-manager-perform", 1, 0, 0,
  (SCM action),
  "Performs the given action and adds it to the undo stack.")
#define FUNC_NAME s_
{
  undo_manager_perform (
    UNDO_MANAGER,
    (UndoableAction *) scm_to_pointer (action));

  return SCM_BOOL_T;
}
#undef FUNC_NAME

SCM_DEFINE (
  s_undo_manager_undo,
  "undo-manager-undo", 0, 1, 0,
  (SCM action),
  "Undoes the last action.")
#define FUNC_NAME s_
{
  undo_manager_undo (UNDO_MANAGER);

  return SCM_BOOL_T;
}
#undef FUNC_NAME

SCM_DEFINE (
  s_undo_manager_redo,
  "undo-manager-redo", 0, 1, 0,
  (SCM action),
  "Redoes the last undone action.")
#define FUNC_NAME s_
{
  undo_manager_redo (UNDO_MANAGER);

  return SCM_BOOL_T;
}
#undef FUNC_NAME

static void
init_module (void * data)
{
#ifndef SNARF_MODE
#include "actions_undo_manager.x"
#endif

  scm_c_export (
    "undo-manager-perform",
    "undo-manager-undo",
    "undo-manager-redo",
    NULL);
}

void
guile_actions_undo_manager_define_module (void)
{
  scm_c_define_module (
    "actions undo-manager", init_module, NULL);
}
