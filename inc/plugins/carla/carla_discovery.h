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
 * Carla discovery.
 */

#ifndef __PLUGINS_CARLA_DISCOVERY_H__
#define __PLUGINS_CARLA_DISCOVERY_H__

#include "zrythm-config.h"

#ifdef HAVE_CARLA

#include <CarlaUtils.h>

typedef struct PluginDescriptor PluginDescriptor;

/**
 * @addtogroup plugins
 *
 * @{
 */

/**
 * Returns the absolute path to carla-discovery-*
 * as a newly allocated string.
 */
char *
z_carla_discovery_get_discovery_path (
  PluginArchitecture arch);

/**
 * Runs carla discovery for the given arch with the
 * given arguments and returns the output as a
 * newly allocated string.
 */
char *
z_carla_discovery_run (
  PluginArchitecture arch,
  const char *       arg1,
  const char *       arg2);

/**
 * Create a descriptor using carla discovery.
 *
 * @path Path to the plugin bundle.
 * @arch Architecture.
 * @protocol Protocol.
 */
PluginDescriptor *
z_carla_discovery_create_vst_descriptor (
  const char *       path,
  PluginArchitecture arch,
  PluginProtocol     protocol);

#ifdef __APPLE__
/**
 * Create a descriptor for the given AU plugin.
 */
PluginDescriptor *
z_carla_discovery_create_au_descriptor_from_info (
  const CarlaCachedPluginInfo * info);

/**
 * Create a descriptor for the given AU plugin.
 */
PluginDescriptor *
z_carla_discovery_create_au_descriptor_from_string (
  char ** all_plugins,
  int     idx);
#endif

/**
 * Parses plugin info into a new PluginDescriptor.
 *
 * @param plugin_path Identifier to use for debugging.
 */
PluginDescriptor *
z_carla_discovery_parse_plugin_info (
  const char * plugin_path,
  char * results);

CarlaBridgeMode
z_carla_discovery_get_bridge_mode (
  const PluginDescriptor * descr);

/**
 * @}
 */

#endif
#endif
