/*
  Copyright (C) 2012 Paul Brossier <piem@aubio.org>

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
#include "aubio_priv.h"
#include "fvec.h"
#include "fmat.h"
#include "io/source.h"
#ifdef HAVE_LIBAV
#include "io/source_avcodec.h"
#endif /* HAVE_LIBAV */
#ifdef __APPLE__
#include "io/source_apple_audio.h"
#endif /* __APPLE__ */
#ifdef HAVE_SNDFILE
#include "io/source_sndfile.h"
#endif /* HAVE_SNDFILE */
#ifdef HAVE_WAVREAD
#include "io/source_wavread.h"
#endif /* HAVE_WAVREAD */

typedef void (*aubio_source_do_t)(aubio_source_t * s, fvec_t * data, uint_t * read);
typedef void (*aubio_source_do_multi_t)(aubio_source_t * s, fmat_t * data, uint_t * read);
typedef uint_t (*aubio_source_get_samplerate_t)(aubio_source_t * s);
typedef uint_t (*aubio_source_get_channels_t)(aubio_source_t * s);
typedef uint_t (*aubio_source_seek_t)(aubio_source_t * s, uint_t seek);
typedef uint_t (*aubio_source_close_t)(aubio_source_t * s);
typedef void (*del_aubio_source_t)(aubio_source_t * s);

struct _aubio_source_t { 
  void *source;
  aubio_source_do_t s_do;
  aubio_source_do_multi_t s_do_multi;
  aubio_source_get_samplerate_t s_get_samplerate;
  aubio_source_get_channels_t s_get_channels;
  aubio_source_seek_t s_seek;
  aubio_source_close_t s_close;
  del_aubio_source_t s_del;
};

aubio_source_t * new_aubio_source(char_t * uri, uint_t samplerate, uint_t hop_size) {
  aubio_source_t * s = AUBIO_NEW(aubio_source_t);
#if HAVE_LIBAV
  s->source = (void *)new_aubio_source_avcodec(uri, samplerate, hop_size);
  if (s->source) {
    s->s_do = (aubio_source_do_t)(aubio_source_avcodec_do);
    s->s_do_multi = (aubio_source_do_multi_t)(aubio_source_avcodec_do_multi);
    s->s_get_channels = (aubio_source_get_channels_t)(aubio_source_avcodec_get_channels);
    s->s_get_samplerate = (aubio_source_get_samplerate_t)(aubio_source_avcodec_get_samplerate);
    s->s_seek = (aubio_source_seek_t)(aubio_source_avcodec_seek);
    s->s_close = (aubio_source_close_t)(aubio_source_avcodec_close);
    s->s_del = (del_aubio_source_t)(del_aubio_source_avcodec);
    return s;
  }
#endif /* HAVE_LIBAV */
#ifdef __APPLE__
  s->source = (void *)new_aubio_source_apple_audio(uri, samplerate, hop_size);
  if (s->source) {
    s->s_do = (aubio_source_do_t)(aubio_source_apple_audio_do);
    s->s_do_multi = (aubio_source_do_multi_t)(aubio_source_apple_audio_do_multi);
    s->s_get_channels = (aubio_source_get_channels_t)(aubio_source_apple_audio_get_channels);
    s->s_get_samplerate = (aubio_source_get_samplerate_t)(aubio_source_apple_audio_get_samplerate);
    s->s_seek = (aubio_source_seek_t)(aubio_source_apple_audio_seek);
    s->s_close = (aubio_source_close_t)(aubio_source_apple_audio_close);
    s->s_del = (del_aubio_source_t)(del_aubio_source_apple_audio);
    return s;
  }
#endif /* __APPLE__ */
#if HAVE_SNDFILE
  s->source = (void *)new_aubio_source_sndfile(uri, samplerate, hop_size);
  if (s->source) {
    s->s_do = (aubio_source_do_t)(aubio_source_sndfile_do);
    s->s_do_multi = (aubio_source_do_multi_t)(aubio_source_sndfile_do_multi);
    s->s_get_channels = (aubio_source_get_channels_t)(aubio_source_sndfile_get_channels);
    s->s_get_samplerate = (aubio_source_get_samplerate_t)(aubio_source_sndfile_get_samplerate);
    s->s_seek = (aubio_source_seek_t)(aubio_source_sndfile_seek);
    s->s_close = (aubio_source_close_t)(aubio_source_sndfile_close);
    s->s_del = (del_aubio_source_t)(del_aubio_source_sndfile);
    return s;
  }
#endif /* HAVE_SNDFILE */
#if HAVE_WAVREAD
  s->source = (void *)new_aubio_source_wavread(uri, samplerate, hop_size);
  if (s->source) {
    s->s_do = (aubio_source_do_t)(aubio_source_wavread_do);
    s->s_do_multi = (aubio_source_do_multi_t)(aubio_source_wavread_do_multi);
    s->s_get_channels = (aubio_source_get_channels_t)(aubio_source_wavread_get_channels);
    s->s_get_samplerate = (aubio_source_get_samplerate_t)(aubio_source_wavread_get_samplerate);
    s->s_seek = (aubio_source_seek_t)(aubio_source_wavread_seek);
    s->s_close = (aubio_source_close_t)(aubio_source_wavread_close);
    s->s_del = (del_aubio_source_t)(del_aubio_source_wavread);
    return s;
  }
#endif /* HAVE_WAVREAD */
  AUBIO_ERROR("source: failed creating aubio source with %s"
     " at samplerate %d with hop_size %d\n", uri, samplerate, hop_size);
  AUBIO_FREE(s);
  return NULL;
}

void aubio_source_do(aubio_source_t * s, fvec_t * data, uint_t * read) {
  s->s_do((void *)s->source, data, read);
}

void aubio_source_do_multi(aubio_source_t * s, fmat_t * data, uint_t * read) {
  s->s_do_multi((void *)s->source, data, read);
}

uint_t aubio_source_close(aubio_source_t * s) {
  return s->s_close((void *)s->source);
}

void del_aubio_source(aubio_source_t * s) {
  if (!s) return;
  s->s_del((void *)s->source);
  AUBIO_FREE(s);
}

uint_t aubio_source_get_samplerate(aubio_source_t * s) {
  return s->s_get_samplerate((void *)s->source);
}

uint_t aubio_source_get_channels(aubio_source_t * s) {
  return s->s_get_channels((void *)s->source);
}

uint_t aubio_source_seek (aubio_source_t * s, uint_t seek ) {
  return s->s_seek((void *)s->source, seek);
}
