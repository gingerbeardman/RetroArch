/*  RetroArch - A frontend for libretro.
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

#include <compat/strl.h>

#include "../menu.h"
#include "../menu_cbs.h"
#include "../menu_hash.h"

#ifndef BIND_ACTION_INFO
#define BIND_ACTION_INFO(cbs, name) \
   cbs->action_info = name; \
   cbs->action_info_ident = #name;
#endif

static int action_info_default(unsigned type, const char *label)
{
   size_t selection             = 0;
   menu_displaylist_info_t info = {0};
   menu_list_t *menu_list       = menu_list_get_ptr();

   if (!menu_list)
      return 0;
   if (!menu_navigation_ctl(MENU_NAVIGATION_CTL_GET_SELECTION, &selection))
      return 0;

   info.list          = menu_list->menu_stack;
   info.directory_ptr = selection;
   strlcpy(info.label,
         menu_hash_to_str(MENU_LABEL_INFO_SCREEN),
        sizeof(info.label));

   return menu_displaylist_push_list(&info, DISPLAYLIST_HELP);
}

int menu_cbs_init_bind_info(menu_file_list_cbs_t *cbs,
      const char *path, const char *label, unsigned type, size_t idx,
      const char *elem0, const char *elem1,
      uint32_t label_hash, uint32_t menu_label_hash)
{
   if (!cbs)
      return -1;

   BIND_ACTION_INFO(cbs, action_info_default);

   return -1;
}
