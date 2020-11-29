/*
 * This filter is a volume ramp up filter.
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/common.h"
#include "mp_msg.h"
#include "af.h"

// Data for specific instances of this filter
typedef struct af_rampup_s
{
  float curr_level; // Current volume level
  float step;       // Volume increase step
} af_rampup_t;

// Initialization and runtime control
static int control(struct af_instance_s* af, int cmd, void* arg)
{
  af_rampup_t* s = (af_rampup_t*)af->setup;

  switch(cmd) {
  case AF_CONTROL_REINIT:
    memcpy(af->data,(af_data_t*)arg, sizeof(af_data_t));
    mp_msg(MSGT_AFILTER, MSGL_V, "[rampup] Was reinitialized: %iHz/%ich/%s\n",
	     af->data->rate,af->data->nch, af_fmt2str_short(af->data->format));
    s->curr_level = 0.0;
    s->step = 0.1;
    return AF_OK;
  }
  return AF_UNKNOWN;
}

// Deallocate memory
static void uninit(struct af_instance_s* af)
{
    free(af->data);
    free(af->setup);
}

// Filter data through filter
static af_data_t* play(struct af_instance_s* af, af_data_t* data)
{
  af_rampup_t*  setup = af->setup;    // Setup for this instance

  int16_t* a = (int16_t*)data->audio; // Audio data
  int len = data->len / 2;     // Number of samples
  int i;
  register int vol;

  if (setup->curr_level < 1.0) {
    setup->curr_level += setup->step;
    if (setup->curr_level > 1.0)
      setup->curr_level = 1.0;

    vol = (int)(255.0 * setup->curr_level);

    for (i = 0; i < len; i++) {
      a[i] = (a[i] * vol) >> 8;
    }
  }

  return data;
}

// Allocate memory and set function pointers
static int af_open(af_instance_t* af){
  af->control=control;
  af->uninit=uninit;
  af->play=play;
  af->mul=1;
  af->data=calloc(1, sizeof(af_data_t));
  af->setup=calloc(1,sizeof(af_rampup_t));
  if(af->data == NULL || af->setup == NULL)
    return AF_ERROR;
  return AF_OK;
}

// Description of this filter
const af_info_t af_info_rampup = {
    "Volume ramp up",
    "rampup",
    "Boris Chernov",
    "",
    AF_FLAGS_REENTRANT,
    af_open
};
