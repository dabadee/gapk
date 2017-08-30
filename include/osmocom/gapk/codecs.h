/* Codecs handling */

/*
 * This file is part of gapk (GSM Audio Pocket Knife).
 *
 * gapk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gapk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gapk.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

#define FR_CANON_LEN	33
#define HR_CANON_LEN	14
#define EFR_CANON_LEN	31
#define PCM_CANON_LEN	(160*sizeof(uint16_t))
#define HR_REF_ENC_LEN  (20 * sizeof(uint16_t))
#define HR_REF_DEC_LEN  (22 * sizeof(uint16_t))

enum codec_type {
	CODEC_INVALID = 0,
	CODEC_PCM,	/* 16 bits PCM samples */
	CODEC_HR,	/* GSM Half Rate codec GSM 06.20 */
	CODEC_FR,	/* GSM Full Rate codec GSM 06.10 */
	CODEC_EFR,	/* GSM Enhanced Full Rate codec GSM 06.60 */
	CODEC_AMR,	/* GSM Adaptive Multi Rate codec GSM 26.071 */
	_CODEC_MAX,
};

#include <osmocom/gapk/formats.h>	/* need to import here because or enum interdep */

/*! call-back for actual codec conversion function
 *  \param[in] state opaque state pointer (returned by codec->init)
 *  \param[out] dst caller-allocated buffer for output data
 *  \param[in] src input data
 *  \param[in] src_len length of input data \a src
 *  \returns number of output bytes written to \a dst; negative on error */
typedef int (*codec_conv_cb_t)(void *state, uint8_t *dst, const uint8_t *src, unsigned int src_len);

struct codec_desc {
	enum codec_type		type;
	const char *		name;
	const char *		description;
	/*! canonical frame size (in bytes); 0 in case of variable length */
	unsigned int		canon_frame_len;

	enum format_type	codec_enc_format_type;	/* what the encoder provides */
	enum format_type	codec_dec_format_type;	/* what to give the decoder */
	/*! codec initialization function pointer, returns opaque state */
	void *			(*codec_init)(void);
	/*! codec exit function pointer, gets passed opaque state */
	void			(*codec_exit)(void *state);
	codec_conv_cb_t		codec_encode;
	codec_conv_cb_t		codec_decode;
};

const struct codec_desc *codec_get_from_type(enum codec_type type);