#ifndef foopstreamutilhfoo
#define foopstreamutilhfoo

/* $Id$ */

/***
  This file is part of polypaudio.
 
  polypaudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.
 
  polypaudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public
  License along with polypaudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <inttypes.h>
#include "pstream.h"
#include "tagstruct.h"

/* The tagstruct is freed!*/
void pa_pstream_send_tagstruct(pa_pstream *p, pa_tagstruct *t);

void pa_pstream_send_error(pa_pstream *p, uint32_t tag, uint32_t error);
void pa_pstream_send_simple_ack(pa_pstream *p, uint32_t tag);

#endif
