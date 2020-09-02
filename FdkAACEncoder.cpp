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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aacenc_lib.h"
#include "FdkAACEncoder.h"

#define TAG "FdkAACEncoder"

#define pr_dbg(fmt, ...)  fprintf(stdout, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)

FdkAACEncoder::FdkAACEncoder()
    : mListener(NULL)
    , mCodecHandle(NULL)
    , mBytesRemain(0)
{
    mOutBuffer.length = 0;
    mOutBuffer.buffer = NULL;
    mInBuffer.length = 0;
    mInBuffer.buffer = NULL;
}

FdkAACEncoder::~FdkAACEncoder()
{
    if (mCodecHandle != NULL)
        aacEncClose(&mCodecHandle);
    if (mOutBuffer.buffer != NULL)
        free(mOutBuffer.buffer);
    if (mInBuffer.buffer != NULL)
        free(mInBuffer.buffer);
}

int FdkAACEncoder::init(IAACEncoderListener *listener,
                        int sampleRate, int channels, int bitsPerSample,
                        int bitRate, bool adtsUsed)
{
    if (listener == NULL) {
        pr_err("None listner to save encoded data\n");
        return ENCODER_ERROR_INVALIDARG;
    }

    CHANNEL_MODE mode;
    AACENC_InfoStruct info = { 0 };

    switch (channels) {
    case 1: mode = MODE_1;       break;
    case 2: mode = MODE_2;       break;
    default:
        pr_err("Unsupported channel count %d\n", channels);
        return ENCODER_ERROR_BADCHANNELS;
    }
    if (bitsPerSample != 16) {
        // todo: 24/32bit convert to 16bit
        pr_err("Unsupported pcm sample depth %d\n", bitsPerSample);
        return ENCODER_ERROR_BADSAMPLEBITS;
    }
    if (bitRate == 0)
        bitRate = preferredBitRate(sampleRate, channels);
    // notice that:
    // bitRate/nChannels > 8000
    // bitRate/nChannels < 160000
    // bitRate/nChannels < sampleRate*6
    if (bitRate < 8000*channels || bitRate > 160000*channels || bitRate > sampleRate*channels*6){
        pr_err("Unsupported bitrate %d\n", bitRate);
        return ENCODER_ERROR_BADBITRATE;
    }

    if (aacEncOpen(&mCodecHandle, 0, channels) != AACENC_OK) {
        pr_err("Unable to open aac encoder\n");
        goto bail;
    }
    // Note: fixed AOT_AAC_LC
    if (aacEncoder_SetParam(mCodecHandle, AACENC_AOT, AOT_AAC_LC) != AACENC_OK) {
        pr_err("Unable to set the AOT\n");
        goto bail;
    }
    // Note: disable sbr
    if (aacEncoder_SetParam(mCodecHandle, AACENC_SBR_MODE, 0) != AACENC_OK) {
        pr_err("Unable to set SBR mode\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_SAMPLERATE, sampleRate) != AACENC_OK) {
        pr_err("Unable to set the samplerate\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        pr_err("Unable to set the channel mode\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        pr_err("Unable to set the wav channel order\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_BITRATE, bitRate) != AACENC_OK) {
        pr_err("Unable to set the bitrate\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_TRANSMUX, adtsUsed ? TT_MP4_ADTS : TT_MP4_RAW) != AACENC_OK) {
        pr_err("Unable to set the transmux\n");
        goto bail;
    }
    if (aacEncoder_SetParam(mCodecHandle, AACENC_AFTERBURNER, 1) != AACENC_OK) {
        pr_err("Unable to set the afterburner mode\n");
        goto bail;
    }
    if (aacEncEncode(mCodecHandle, NULL, NULL, NULL, NULL) != AACENC_OK) {
        pr_err("Unable to initialize the encoder\n");
        goto bail;
    }
    if (aacEncInfo(mCodecHandle, &info) != AACENC_OK) {
        pr_err("Unable to get the encoder info\n");
        goto bail;
    }

    mListener = listener;
    mBytesFrame = channels*sizeof(short)*info.frameLength;

    mInBuffer.length  = mBytesFrame;
    mOutBuffer.length = mBytesFrame;
    mInBuffer.buffer  = (unsigned char *)malloc(mInBuffer.length);
    mOutBuffer.buffer = (unsigned char *)malloc(mOutBuffer.length);
    if (mInBuffer.buffer == NULL || mOutBuffer.buffer == NULL) {
        pr_err("Unable to allocate in/out buffer\n");
        goto bail;
    }

    return ENCODER_NOERROR;

bail:
    if (mOutBuffer.buffer != NULL) {
        free(mOutBuffer.buffer);
        mOutBuffer.buffer = NULL;
    }
    if (mInBuffer.buffer != NULL) {
        free(mInBuffer.buffer);
        mInBuffer.buffer = NULL;
    }
    if (mCodecHandle != NULL) {
        aacEncClose(&mCodecHandle);
        mCodecHandle = NULL;
    }
    return ENCODER_ERROR_GENERIC;
}

int FdkAACEncoder::encode(unsigned char *inBuffer, int inLength)
{
    if ((unsigned int)inLength > mOutBuffer.length) {
        unsigned int newLength = (inLength/mOutBuffer.length + 1) * mOutBuffer.length;
        unsigned char *newBuffer = (unsigned char *)realloc(mOutBuffer.buffer, newLength);
        if (newBuffer == NULL) {
            pr_err("Unable to reallocate output buffer\n");
            return ENCODER_ERROR_NOMEM;
        }
        mOutBuffer.buffer = newBuffer;
        mOutBuffer.length = newLength;
    }

    AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
    AACENC_InArgs in_args = { 0 };
    AACENC_OutArgs out_args = { 0 };
    int in_identifier = IN_AUDIO_DATA;
    int in_size, in_elem_size;
    int out_identifier = OUT_BITSTREAM_DATA;
    int out_size, out_elem_size;
    void *in_ptr, *out_ptr;
    AACENC_ERROR ret;
    int bytesRead = 0;
    int bytesEncoded = 0;

    if (mBytesRemain > 0) {
        if ((mBytesRemain + inLength) >= mBytesFrame) {
            int bytesFilled = mBytesFrame - mBytesRemain;
            memcpy(mInBuffer.buffer + mBytesRemain, inBuffer, bytesFilled);

            in_ptr = mInBuffer.buffer;
            in_size = mBytesFrame;
            in_elem_size = 2;
            in_args.numInSamples = mBytesFrame/2;
            in_buf.numBufs = 1;
            in_buf.bufs = &in_ptr;
            in_buf.bufferIdentifiers = &in_identifier;
            in_buf.bufSizes = &in_size;
            in_buf.bufElSizes = &in_elem_size;

            out_ptr = mOutBuffer.buffer;
            out_size = mOutBuffer.length;
            out_elem_size = 1;
            out_buf.numBufs = 1;
            out_buf.bufs = &out_ptr;
            out_buf.bufferIdentifiers = &out_identifier;
            out_buf.bufSizes = &out_size;
            out_buf.bufElSizes = &out_elem_size;

            ret = aacEncEncode(mCodecHandle, &in_buf, &out_buf, &in_args, &out_args);
            if (ret != AACENC_OK) {
                if (ret == AACENC_ENCODE_EOF)
                    return ENCODER_ERROR_EOF;
                pr_err("Unable to encode frame\n");
                return ENCODER_ERROR_GENERIC;
            }
            bytesRead += bytesFilled;
            bytesEncoded += out_args.numOutBytes;
        }
        else {
            memcpy(mInBuffer.buffer + mBytesRemain, inBuffer, inLength);
            mBytesRemain += inLength;
            return ENCODER_NOERROR;
        }
    }

    mBytesRemain = inLength - bytesRead;
    while (mBytesRemain >= mBytesFrame) {
        in_ptr = inBuffer + bytesRead;
        in_size = mBytesFrame;
        in_elem_size = 2;
        in_args.numInSamples = mBytesFrame/2;
        in_buf.numBufs = 1;
        in_buf.bufs = &in_ptr;
        in_buf.bufferIdentifiers = &in_identifier;
        in_buf.bufSizes = &in_size;
        in_buf.bufElSizes = &in_elem_size;

        out_ptr = mOutBuffer.buffer + bytesEncoded;
        out_size = mOutBuffer.length - bytesEncoded;
        out_elem_size = 1;
        out_buf.numBufs = 1;
        out_buf.bufs = &out_ptr;
        out_buf.bufferIdentifiers = &out_identifier;
        out_buf.bufSizes = &out_size;
        out_buf.bufElSizes = &out_elem_size;

        ret = aacEncEncode(mCodecHandle, &in_buf, &out_buf, &in_args, &out_args);
        if (ret != AACENC_OK) {
            if (ret == AACENC_ENCODE_EOF)
                return ENCODER_NOERROR;
            pr_err("Unable to encode frame\n");
            return ENCODER_ERROR_GENERIC;
        }

        mBytesRemain -= mBytesFrame;
        bytesRead += mBytesFrame;
        bytesEncoded += out_args.numOutBytes;
    }

    if (mBytesRemain > 0)
        memcpy(mInBuffer.buffer, inBuffer + bytesRead, mBytesRemain);

    if (bytesEncoded > 0)
        mListener->output(mOutBuffer.buffer, bytesEncoded);
    return ENCODER_NOERROR;
}

void FdkAACEncoder::deinit()
{
    if (mCodecHandle != NULL) {
        aacEncClose(&mCodecHandle);
        mCodecHandle = NULL;
    }
    if (mOutBuffer.buffer != NULL) {
        free(mOutBuffer.buffer);
        mOutBuffer.buffer = NULL;
    }
    if (mInBuffer.buffer != NULL) {
        free(mInBuffer.buffer);
        mInBuffer.buffer = NULL;
    }
}

int FdkAACEncoder::preferredBitRate(int sampleRate, int channels)
{
    int bitRate = -1;
    switch (sampleRate) {
    case 8000:
    case 11025:
    case 12000:
        bitRate = 20000; // 20kbps
        break;
    case 16000:
    case 22050:
    case 24000:
        bitRate = 32000; // 32kbps
        break;
    case 32000:
        bitRate = 48000; // 48kbps
        break;
    case 44100:
    case 48000:
        bitRate = 96000; // 96kbps
        break;
    case 64000:
    case 88200:
    case 96000:
        bitRate = 128000; // 128kbps
        break;
    default:
        return -1; // return -1 for unsupported sampling rates
    }
    return bitRate*channels;
}
