/* ASE - Allegro Sprite Editor
 * Copyright (C) 2001-2008  David A. Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <assert.h>
#include <string.h>

#include "jinete/jbase.h"
#include "jinete/jlist.h"
#include "jinete/jmutex.h"

#include "raster/gfxobj.h"

/* typedef struct GfxObjProperty */
/* { */
/*   char *key; */
/*   void *data; */
/* } Property; */

static JMutex objects_mutex;
static gfxobj_id object_id = 0;		/* last object ID created */
static JList objects;			/* graphics objects list */

bool gfxobj_init(void)
{
  objects_mutex = jmutex_new();
  if (!objects_mutex)
    return FALSE;

  objects = jlist_new();
  if (!objects)
    return FALSE;

  return TRUE;
}

void gfxobj_exit(void)
{
  jlist_free(objects);
  jmutex_free(objects_mutex);
}

GfxObj *gfxobj_new(int type, int size)
{
  GfxObj *gfxobj;

  gfxobj = jmalloc(size);
  if (!gfxobj)
    return NULL;

  jmutex_lock(objects_mutex);
  {
    gfxobj->type = type;
    gfxobj->id = ++object_id;
    /* gfxobj->properties = NULL; */
    jlist_append(objects, gfxobj);
  }
  jmutex_unlock(objects_mutex);

  return gfxobj;
}

void gfxobj_free(GfxObj *gfxobj)
{
  jmutex_lock(objects_mutex);
  {
    jlist_remove(objects, gfxobj);
  }
  jmutex_unlock(objects_mutex);

  jfree(gfxobj);
}

GfxObj *gfxobj_find(gfxobj_id id)
{
  GfxObj *ret = NULL;
  JLink link;

  jmutex_lock(objects_mutex);
  {
    JI_LIST_FOR_EACH(objects, link)
      if (((GfxObj *)link->data)->id == id) {
	ret = (GfxObj *)link->data;
	break;
      }
  }
  jmutex_unlock(objects_mutex);

  return ret;
}

void _gfxobj_set_id(GfxObj *gfxobj, gfxobj_id id)
{
  assert(gfxobj_find(id) == NULL);

  gfxobj->id = id;
}
