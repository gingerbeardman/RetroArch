/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <string/string_list.h>
#include "menu_driver.h"
#include "menu.h"
#include "menu_entries.h"
#include "../driver.h"
#include "../general.h"

static const menu_ctx_driver_t *menu_ctx_drivers[] = {
#ifdef IOS
   &menu_ctx_ios,
#endif
#if defined(HAVE_RMENU)
   &menu_ctx_rmenu,
#endif
#if defined(HAVE_RMENU_XUI)
   &menu_ctx_rmenu_xui,
#endif
#if defined(HAVE_GLUI)
   &menu_ctx_glui,
#endif
#if defined(HAVE_XMB)
   &menu_ctx_xmb,
#endif
#if defined(HAVE_RGUI)
   &menu_ctx_rgui,
#endif
   &menu_ctx_null,
   NULL
};

/**
 * menu_driver_find_handle:
 * @idx              : index of driver to get handle to.
 *
 * Returns: handle to menu driver at index. Can be NULL
 * if nothing found.
 **/
const void *menu_driver_find_handle(int idx)
{
   const void *drv = menu_ctx_drivers[idx];
   if (!drv)
      return NULL;
   return drv;
}

/**
 * menu_driver_find_ident:
 * @idx              : index of driver to get handle to.
 *
 * Returns: Human-readable identifier of menu driver at index. Can be NULL
 * if nothing found.
 **/
const char *menu_driver_find_ident(int idx)
{
   const menu_ctx_driver_t *drv = menu_ctx_drivers[idx];
   if (!drv)
      return NULL;
   return drv->ident;
}

/**
 * config_get_menu_driver_options:
 *
 * Get an enumerated list of all menu driver names,
 * separated by '|'.
 *
 * Returns: string listing of all menu driver names,
 * separated by '|'.
 **/
const char* config_get_menu_driver_options(void)
{
   union string_list_elem_attr attr;
   unsigned i;
   char *options = NULL;
   int options_len = 0;
   struct string_list *options_l = string_list_new();

   attr.i = 0;

   if (!options_l)
      return NULL;

   for (i = 0; menu_driver_find_handle(i); i++)
   {
      const char *opt = menu_driver_find_ident(i);
      options_len += strlen(opt) + 1;
      string_list_append(options_l, opt, attr);
   }

   options = (char*)calloc(options_len, sizeof(char));

   if (!options)
   {
      string_list_free(options_l);
      options_l = NULL;
      return NULL;
   }

   string_list_join_concat(options, options_len, options_l, "|");

   string_list_free(options_l);
   options_l = NULL;

   return options;
}

void find_menu_driver(void)
{
   driver_t *driver     = driver_get_ptr();
   settings_t *settings = config_get_ptr();

   int i = find_driver_index("menu_driver", settings->menu.driver);
   if (i >= 0)
      driver->menu_ctx = (const menu_ctx_driver_t*)menu_driver_find_handle(i);
   else
   {
      unsigned d;
      RARCH_WARN("Couldn't find any menu driver named \"%s\"\n",
            settings->menu.driver);
      RARCH_LOG_OUTPUT("Available menu drivers are:\n");
      for (d = 0; menu_driver_find_handle(d); d++)
         RARCH_LOG_OUTPUT("\t%s\n", menu_driver_find_ident(d));
      RARCH_WARN("Going to default to first menu driver...\n");

      driver->menu_ctx = (const menu_ctx_driver_t*)menu_driver_find_handle(0);

      if (!driver->menu_ctx)
         rarch_fail(1, "find_menu_driver()");
   }
}

void init_menu(void)
{
   driver_t *driver = driver_get_ptr();
   if (driver->menu)
      return;

   find_menu_driver();
   if (!(driver->menu = (menu_handle_t*)menu_init(driver->menu_ctx)))
   {
      RARCH_ERR("Cannot initialize menu.\n");
      rarch_fail(1, "init_menu()");
   }

   if (!(menu_entries_init(driver->menu)))
   {
      RARCH_ERR("Cannot initialize menu lists.\n");
      rarch_fail(1, "init_menu()");
   }

   menu_driver_context_reset();
}

menu_handle_t *menu_driver_get_ptr(void)
{
   driver_t *driver = driver_get_ptr();
   if (!driver->menu)
      return NULL;
   return driver->menu;
}

void menu_driver_navigation_increment(void)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->navigation_increment)
      driver->menu_ctx->navigation_increment();
}

void menu_driver_navigation_decrement(void)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->navigation_decrement)
      driver->menu_ctx->navigation_decrement();
}

void menu_driver_navigation_clear(bool pending_push)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->navigation_clear)
      driver->menu_ctx->navigation_clear(pending_push);
}

void menu_driver_navigation_set(bool scroll)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->navigation_set)
      driver->menu_ctx->navigation_set(scroll);
}

void menu_driver_navigation_set_last(void)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->navigation_set_last)
      driver->menu_ctx->navigation_set_last();
}

void menu_driver_set_texture(void)
{
   driver_t *driver = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->set_texture)
      driver->menu_ctx->set_texture();
}

void menu_driver_context_reset(void)
{
   driver_t *driver     = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->context_reset)
      driver->menu_ctx->context_reset();
}

void menu_driver_frame(void)
{
   driver_t *driver     = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->frame)
      driver->menu_ctx->frame();
}

void menu_driver_entry_iterate(unsigned action)
{
   driver_t *driver     = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->entry_iterate)
      driver->menu_ctx->entry_iterate(action);
}

void menu_driver_free(menu_handle_t *menu)
{
   driver_t *driver     = driver_get_ptr();
   if (!driver)
      return;

   if (driver->menu_ctx && driver->menu_ctx->free)
      driver->menu_ctx->free(menu);
}

void menu_driver_render(void)
{
   driver_t *driver     = driver_get_ptr();
   if (!driver)
      return;
   if (!driver->video_data)
      return;
   
   if (driver->menu_ctx && driver->menu_ctx->render)
      driver->menu_ctx->render();
}
