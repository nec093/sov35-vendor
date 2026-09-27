/*
 * Compress offload ABI compatibility.
 *
 * Kernels with the msm-5.4 audio techpack (audio-kernel) dropped the CAF
 * fields from the uapi struct snd_codec:
 *  - compr_passthr and flags live in snd_codec.reserved[0] and [1]
 *  - FLAC/ALAC/APE/VORBIS/WMA/WMA PRO/DSD/APTX are sent as
 *    SND_AUDIOCODEC_BESPOKE with the AUDIO_COMP_FORMAT_* type in
 *    options.generic.reserved[0] and the decoder parameters
 *    (struct snd_generic_dec_*) starting at options.generic.reserved[1]
 *
 * The accessors below pick whichever layout the kernel headers provide.
 */

#ifndef AUDIO_HAL_COMPRESS_COMPAT_H
#define AUDIO_HAL_COMPRESS_COMPAT_H

#include <sound/compress_params.h>

#if __has_include(<sound/audio_compressed_formats.h>)
#include <sound/audio_compressed_formats.h>

#define AUDIO_COMPR_GENERIC_DEC 1

#define COMPR_PASSTHR(codec)            ((codec)->reserved[0])
#define COMPR_FLAGS(codec)              ((codec)->reserved[1])
#define COMPR_DEC(codec, gtype, ltype) \
    ((struct snd_generic_dec_##gtype *)&(codec)->options.generic.reserved[1])

#else

#define COMPR_PASSTHR(codec)            ((codec)->compr_passthr)
#define COMPR_FLAGS(codec)              ((codec)->flags)
#define COMPR_DEC(codec, gtype, ltype)  (&(codec)->options.ltype)

#endif

#endif /* AUDIO_HAL_COMPRESS_COMPAT_H */
