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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2017, 2019 Robin Gareus <robin@gareus.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Routing graph thread.
 */

#ifndef __AUDIO_GRAPH_THREAD_H__
#define __AUDIO_GRAPH_THREAD_H__

#include "zrythm-config.h"

#include <stdbool.h>
#include <pthread.h>

#include "utils/types.h"

#include <gtk/gtk.h>

#ifdef HAVE_JACK
#include "weak_libjack.h"
#endif

typedef struct Graph Graph;

/**
 * @addtogroup audio
 *
 * @{
 */

typedef struct GraphThread
{
#ifdef HAVE_JACK
  jack_native_thread_t jthread;
#endif
  pthread_t            pthread;

  /**
   * Thread index in zrythm.
   *
   * The main thread will be -1 and the rest in
   * sequence starting from 0.
   */
  int                  id;

  /** Pointer back to the graph. */
  Graph *              graph;
} GraphThread;

/**
 * Creates a thread.
 *
 * @param id The index of the thread.
 * @param graph The graph to set to the thread.
 * @param is_main 1 if main thread.
 */
GraphThread *
graph_thread_new (
  const int  id,
  const bool is_main,
  Graph *    graph);

/**
 * @}
 */

#endif
