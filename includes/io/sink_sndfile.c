/*
  Copyright (C) 2012-2014 Paul Brossier <piem@aubio.org>

  This file is part of aubio.

  aubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "config.h"

#ifdef HAVE_SNDFILE

#include <sndfile.h>

#include "aubio_priv.h"
#include "fvec.h"
#include "fmat.h"
#include "io/sink_sndfile.h"

#define MAX_CHANNELS 6
#define MAX_SIZE 4096

struct _aubio_sink_sndfile_t {
  uint_t samplerate;
  uint_t channels;
  char_t *path;

  uint_t max_size;

  SNDFILE *handle;
  uint_t scratch_size;
  smpl_t *scratch_data;
};

uint_t aubio_sink_sndfile_open(aubio_sink_sndfile_t *s);

aubio_sink_sndfile_t * new_aubio_sink_sndfile(char_t * path, uint_t samplerate) {
  aubio_sink_sndfile_t * s = AUBIO_NEW(aubio_sink_sndfile_t);
  s->max_size = MAX_SIZE;
  s->path = path;

  if (path == NULL) {
    AUBIO_ERR("sink_sndfile: Aborted opening null path\n");
    return NULL;
  }

  s->samplerate = 0;
  s->channels = 0;

  // negative samplerate given, abort
  if ((sint_t)samplerate < 0) goto beach;
  // zero samplerate given. do not open yet
  if ((sint_t)samplerate == 0) return s;

  s->samplerate = samplerate;
  s->channels = 1;

  if (aubio_sink_sndfile_open(s) != AUBIO_OK) {;
    goto beach;
  }
  return s;

beach:
  del_aubio_sink_sndfile(s);
  return NULL;
}

uint_t aubio_sink_sndfile_preset_samplerate(aubio_sink_sndfile_t *s, uint_t samplerate)
{
  if ((sint_t)(samplerate) <= 0) return AUBIO_FAIL;
  s->samplerate = samplerate;
  // automatically open when both samplerate and channels have been set
  if (s->samplerate != 0 && s->channels != 0) {
    return aubio_sink_sndfile_open(s);
  }
  return AUBIO_OK;
}

uint_t aubio_sink_sndfile_preset_channels(aubio_sink_sndfile_t *s, uint_t channels)
{
  if ((sint_t)(channels) <= 0) return AUBIO_FAIL;
  s->channels = channels;
  // automatically open when both samplerate and channels have been set
  if (s->samplerate != 0 && s->channels != 0) {
    return aubio_sink_sndfile_open(s);
  }
  return AUBIO_OK;
}

uint_t aubio_sink_sndfile_get_samplerate(aubio_sink_sndfile_t *s)
{
  return s->samplerate;
}

uint_t aubio_sink_sndfile_get_channels(aubio_sink_sndfile_t *s)
{
  return s->channels;
}

uint_t aubio_sink_sndfile_open(aubio_sink_sndfile_t *s) {
  /* set output format */
  SF_INFO sfinfo;
  AUBIO_MEMSET(&sfinfo, 0, sizeof (sfinfo));
  sfinfo.samplerate = s->samplerate;
  sfinfo.channels   = s->channels;
  sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  /* try creating the file */
  s->handle = sf_open (s->path, SFM_WRITE, &sfinfo);

  if (s->handle == NULL) {
    /* show libsndfile err msg */
    AUBIO_ERR("sink_sndfile: Failed opening %s. %s\n", s->path, sf_strerror (NULL));
    return AUBIO_FAIL;
  }	

  s->scratch_size = s->max_size*s->channels;
  /* allocate data for de/interleaving reallocated when needed. */
  if (s->scratch_size >= MAX_SIZE * MAX_CHANNELS) {
    AUBIO_ERR("sink_sndfile: %d x %d exceeds maximum aubio_sink_sndfile buffer size %d\n",
        s->max_size, s->channels, MAX_CHANNELS * MAX_CHANNELS);
    return AUBIO_FAIL;
  }
  s->scratch_data = AUBIO_ARRAY(float,s->scratch_size);

  return AUBIO_OK;
}

void aubio_sink_sndfile_do(aubio_sink_sndfile_t *s, fvec_t * write_data, uint_t write){
  uint_t i, j,	channels = s->channels;
  int nsamples = channels*write;
  smpl_t *pwrite;
  sf_count_t written_frames;

  if (write > s->max_size) {
    AUBIO_WRN("trying to write %d frames, but only %d can be written at a time",
      write, s->max_size);
    write = s->max_size;
  }

  /* interleaving data  */
  for ( i = 0; i < channels; i++) {
    pwrite = (smpl_t *)write_data->data;
    for (j = 0; j < write; j++) {
      s->scratch_data[channels*j+i] = pwrite[j];
    }
  }

  written_frames = sf_write_float (s->handle, s->scratch_data, nsamples);
  if (written_frames/channels != write) {
    AUBIO_WRN("sink_sndfile: trying to write %d frames to %s, but only %d could be written",
      write, s->path, (uint_t)written_frames);
  }
  return;
}

void aubio_sink_sndfile_do_multi(aubio_sink_sndfile_t *s, fmat_t * write_data, uint_t write){
  uint_t i, j,	channels = s->channels;
  int nsamples = channels*write;
  smpl_t *pwrite;
  sf_count_t written_frames;

  if (write > s->max_size) {
    AUBIO_WRN("trying to write %d frames, but only %d can be written at a time",
      write, s->max_size);
    write = s->max_size;
  }

  /* interleaving data  */
  for ( i = 0; i < write_data->height; i++) {
    pwrite = (smpl_t *)write_data->data[i];
    for (j = 0; j < write; j++) {
      s->scratch_data[channels*j+i] = pwrite[j];
    }
  }

  written_frames = sf_write_float (s->handle, s->scratch_data, nsamples);
  if (written_frames/channels != write) {
    AUBIO_WRN("sink_sndfile: trying to write %d frames to %s, but only %d could be written",
      write, s->path, (uint_t)written_frames);
  }
  return;
}

uint_t aubio_sink_sndfile_close (aubio_sink_sndfile_t *s) {
  if (!s->handle) {
    return AUBIO_FAIL;
  }
  if (sf_close(s->handle)) {
    AUBIO_ERR("sink_sndfile: Error closing file %s: %s", s->path, sf_strerror (NULL));
    return AUBIO_FAIL;
  }
  s->handle = NULL;
  return AUBIO_OK;
}

void del_aubio_sink_sndfile(aubio_sink_sndfile_t * s){
  if (!s) return;
  aubio_sink_sndfile_close(s);
  AUBIO_FREE(s->scratch_data);
  AUBIO_FREE(s);
}

#endif /* HAVE_SNDFILE */
