/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      The floodfill routine.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */

/* Adapted to ASE by David A. Capello */

#include "config.h"

#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <limits.h>
#include <math.h>

#include "raster/algo.h"
#include "raster/image.h"



typedef struct FLOODED_LINE	/* store segments which have been flooded */
{
  short flags;			/* status of the segment */
  short lpos, rpos;		/* left and right ends of segment */
  short y;			/* y coordinate of the segment */
  int next;			/* linked list if several per line */
} FLOODED_LINE;

/* Note: a 'short' is not sufficient for 'next' above in some corner cases. */


static int flood_count;          /* number of flooded segments */

#define FLOOD_IN_USE             1
#define FLOOD_TODO_ABOVE         2
#define FLOOD_TODO_BELOW         4

#define FLOOD_LINE(c)            (((FLOODED_LINE *)_scratch_mem) + c)



/* flooder:
 *  Fills a horizontal line around the specified position, and adds it
 *  to the list of drawn segments. Returns the first x coordinate after
 *  the part of the line which it has dealt with.
 */
static int flooder (Image *image, int x, int y,
		    int src_color, void *data, AlgoHLine proc)
{
  FLOODED_LINE *p;
  int left = 0, right = 0;
  int c;

  switch (image->imgtype) {

    case IMAGE_RGB:
      {
        ase_uint32 *address = ((ase_uint32 **)image->line)[y];

        /* check start pixel */
        if ((int)*(address+x) != src_color)
          return x+1;

        /* work left from starting point */
        for (left=x-1; left>=0; left--) {
          if ((int)*(address+left) != src_color)
            break;
        }

        /* work right from starting point */
        for (right=x+1; right<image->w; right++) {
          if ((int)*(address+right) != src_color)
            break;
        }
      }
      break;

    case IMAGE_GRAYSCALE:
      {
        ase_uint16 *address = ((ase_uint16 **)image->line)[y];

        /* check start pixel */
        if ((int)*(address+x) != src_color)
          return x+1;

        /* work left from starting point */
        for (left=x-1; left>=0; left--) {
          if ((int)*(address+left) != src_color)
            break;
        }

        /* work right from starting point */
        for (right=x+1; right<image->w; right++) {
          if ((int)*(address+right) != src_color)
            break;
        }
      }
      break;

    case IMAGE_INDEXED:
      {
        ase_uint8 *address = ((ase_uint8 **)image->line)[y];

        /* check start pixel */
        if ((int)*(address+x) != src_color)
          return x+1;

        /* work left from starting point */
        for (left=x-1; left>=0; left--) {
          if ((int)*(address+left) != src_color)
            break;
        }

        /* work right from starting point */
        for (right=x+1; right<image->w; right++) {
          if ((int)*(address+right) != src_color)
            break;
        }
      }
      break;

    default:
      /* check start pixel */
      if (image_getpixel(image, x, y) != src_color)
	return x+1;

      /* work left from starting point */
      for (left=x-1; left>=0; left--) {
	if (image_getpixel(image, left, y) != src_color)
	  break;
      }

      /* work right from starting point */
      for (right=x+1; right<image->w; right++) {
	if (image_getpixel(image, right, y) != src_color)
	  break;
      }
      break;
  }

  left++;
  right--;

  /* draw the line */
  (*proc)(left, y, right, data);

  /* store it in the list of flooded segments */
  c = y;
  p = FLOOD_LINE(c);

  if (p->flags) {
    while (p->next) {
      c = p->next;
      p = FLOOD_LINE(c);
    }

    p->next = c = flood_count++;
    _grow_scratch_mem(sizeof(FLOODED_LINE) * flood_count);
    p = FLOOD_LINE(c);
  }

  p->flags = FLOOD_IN_USE;
  p->lpos = left;
  p->rpos = right;
  p->y = y;
  p->next = 0;

  if (y > 0)
    p->flags |= FLOOD_TODO_ABOVE;

  if (y+1 < image->h)
    p->flags |= FLOOD_TODO_BELOW;

  return right+2;
}



/* check_flood_line:
 *  Checks a line segment, using the scratch buffer is to store a list of
 *  segments which have already been drawn in order to minimise the required
 *  number of tests.
 */
static int check_flood_line(Image *image, int y, int left, int right,
			    int src_color, void *data, AlgoHLine proc)
{
  int c;
  FLOODED_LINE *p;
  int ret = FALSE;

  while (left <= right) {
    c = y;

    for (;;) {
      p = FLOOD_LINE(c);

      if ((left >= p->lpos) && (left <= p->rpos)) {
        left = p->rpos+2;
        break;
      }

      c = p->next;

      if (!c) {
        left = flooder (image, left, y, src_color, data, proc);
        ret = TRUE;
        break;
      }
    }
  }

  return ret;
}



/* floodfill:
 *  Fills an enclosed area (starting at point x, y) with the specified color.
 */
void algo_floodfill(Image *image, int x, int y, void *data, AlgoHLine proc)
{
  int src_color;
  int c, done;
  FLOODED_LINE *p;

  /* make sure we have a valid starting point */
  if ((x < 0) || (x >= image->w) ||
      (y < 0) || (y >= image->h))
    return;

  /* what color to replace? */
  src_color = image_getpixel (image, x, y);

  /* set up the list of flooded segments */
  _grow_scratch_mem(sizeof(FLOODED_LINE) * image->h);
  flood_count = image->h;
  p = _scratch_mem;
  for (c=0; c<flood_count; c++) {
    p[c].flags = 0;
    p[c].lpos = SHRT_MAX;
    p[c].rpos = SHRT_MIN;
    p[c].y = y;
    p[c].next = 0;
  }

  /* start up the flood algorithm */
  flooder(image, x, y, src_color, data, proc);

  /* continue as long as there are some segments still to test */
  do {
    done = TRUE;

    /* for each line on the screen */
    for (c=0; c<flood_count; c++) {

      p = FLOOD_LINE(c);

      /* check below the segment? */
      if (p->flags & FLOOD_TODO_BELOW) {
        p->flags &= ~FLOOD_TODO_BELOW;
        if (check_flood_line(image, p->y+1, p->lpos, p->rpos,
			     src_color, data, proc)) {
          done = FALSE;
          p = FLOOD_LINE(c);
        }
      }

      /* check above the segment? */
      if (p->flags & FLOOD_TODO_ABOVE) {
        p->flags &= ~FLOOD_TODO_ABOVE;
        if (check_flood_line(image, p->y-1, p->lpos, p->rpos,
			     src_color, data, proc)) {
          done = FALSE;
          /* special case shortcut for going backwards */
          if ((c < image->h) && (c > 0))
            c -= 2;
        }
      }
    }
  } while (!done);
}
