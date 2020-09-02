/*
 ** Copyright 2019-2020, LUOYUN <sysu.zqlong@gmail.com>.
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "stdio.h"
#include "stdbool.h"

#ifndef __IAACENCODER_H
#define __IAACENCODER_H

class IAACEncoderListener {
public:
    /**
     * Listener to save encoded data.
     * \param outBuffer [IN] output buffer pointer, output encoded data.
     * \param outLength [IN] output buffer length in byte. 
     * \retval N/A.
     */
    virtual void output(unsigned char *outBuffer, int outLength) = 0;
    virtual ~IAACEncoderListener() {}
};

class IAACEncoder
{
public:
    enum EncoderError {
        ENCODER_NOERROR             =  0,
        ENCODER_ERROR_GENERIC       = -1,
        ENCODER_ERROR_BADSAMPLERATE = -10,
        ENCODER_ERROR_BADCHANNELS   = -11,
        ENCODER_ERROR_BADSAMPLEBITS = -12,
        ENCODER_ERROR_BADBITRATE    = -13,
        ENCODER_ERROR_INVALIDARG    = -14,
        ENCODER_ERROR_NOMEM         = -15,
        ENCODER_ERROR_NULLPOINTER   = -16,
        ENCODER_ERROR_EOF           = -17,
    };

    virtual ~IAACEncoder() {}

    /**
     * Init aac encoder.
     * \param listener [IN] listener to save encoded data.
     * \param sampleRate [IN] pcm sample rate.
     * \param channels [IN] number of channels on input (1,2).
     * \param bits [IN] pcm bits per sample.
     * \param bitRate [IN] encoder bit rate in bits/sec.
     * \param adtsUsed [IN] whether write adts header. 
     * \retval ENCODER_NOERROR Succeeded. Others Failed.
     */
    virtual int init(IAACEncoderListener *listener,
                     int sampleRate, int channels, int bitsPerSample,
                     int bitRate = 0, bool adtsUsed = true) = 0;

    /**
     * Feed pcm data.
     * \param inBuffer [IN] input buffer pointer, input pcm data.
     * \param inLength [IN] input buffer length in byte.
     * \retval ENCODER_NOERROR Succeeded. Others Failed.
     */
    virtual int encode(unsigned char *inBuffer, int inLength) = 0;

    /**
     * Uninit aac encoder.
     * \retval N/A.
     */
    virtual void deinit() = 0;
};

#endif // __IAACENCODER_H
