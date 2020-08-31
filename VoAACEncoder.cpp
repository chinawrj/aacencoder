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
#include "voAAC.h"
#include "cmnMemory.h"
#include "VoAACEncoder.h"

#define TAG "VoAACEncoder"

#define pr_dbg(fmt, ...)  fprintf(stdout, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)

VoAACEncoder::VoAACEncoder()
    : mListener(NULL)
    , mCodecHandle(NULL)
{
    voGetAACEncAPI(&mCodecApi);

    mCodecMem.Alloc = cmnMemAlloc;
    mCodecMem.Copy = cmnMemCopy;
    mCodecMem.Free = cmnMemFree;
    mCodecMem.Set = cmnMemSet;
    mCodecMem.Check = cmnMemCheck;

    mOutBuffer.Length = 2*sizeof(short)*1024;
    mOutBuffer.Buffer = (VO_PBYTE)malloc(mOutBuffer.Length);

    mInBuffer.Length = 2*sizeof(short)*1024;
    mInBuffer.Buffer = (VO_PBYTE)malloc(mInBuffer.Length);
    mBytesRemain = 0;
}

VoAACEncoder::~VoAACEncoder()
{
    if (mOutBuffer.Buffer != NULL)
        free(mOutBuffer.Buffer);
    if (mInBuffer.Buffer != NULL)
        free(mInBuffer.Buffer);
}

int VoAACEncoder::init(IAudioEncoderListener *listener, int sampleRate, int channels, int bitsPerSample, int bitRate)
{
    if (mOutBuffer.Buffer == NULL || mInBuffer.Buffer == NULL) {
        pr_err("Invalid in/out buffer\n");
        return ENCODER_ERROR_NULLPOINTER;
    }

    if (listener == NULL) {
        pr_err("None listner to save encoded data\n");
        return ENCODER_ERROR_INVALIDARG;
    }

    if (channels != 1 && channels != 2) {
        pr_err("Unsupported pcm channels %d\n", channels);
        return ENCODER_ERROR_BADCHANNELS;
    }

    if (bitsPerSample != 16) {
        // todo: 24/32bit convert to 16bit
        pr_err("Unsupported pcm sample depth %d\n", bitsPerSample);
        return ENCODER_ERROR_BADSAMPLEBITS;
    }

    if (bitRate == 0)
        bitRate = getPreferredBitRate(sampleRate, channels);
    // notice that:
    // bitRate/nChannels > 8000
    // bitRate/nChannels < 160000
    // bitRate/nChannels < sampleRate*6
    if (bitRate < 8000*channels || bitRate > 160000*channels || bitRate > sampleRate*channels*6){
        pr_err("Unsupported bitrate %d\n", bitRate);
        return ENCODER_ERROR_BADBITRATE;
    }

    VO_CODEC_INIT_USERDATA userData;
    userData.memflag = VO_IMF_USERMEMOPERATOR;
    userData.memData = &mCodecMem;
    if (mCodecApi.Init(&mCodecHandle, VO_AUDIO_CodingAAC, &userData) != VO_ERR_NONE) {
        pr_err("Unable to init aac encoder\n");
        return ENCODER_ERROR_GENERIC;
    }

    AACENC_PARAM params;
    params.sampleRate = sampleRate;
    params.bitRate = bitRate;
    params.nChannels = channels;
    params.adtsUsed = 1;
    if (mCodecApi.SetParam(mCodecHandle, VO_PID_AAC_ENCPARAM, &params) != VO_ERR_NONE) {
        pr_err("Unable to set aac encoder parameters\n");
        return ENCODER_ERROR_GENERIC;
    }

    mListener = listener;
    mBytesFrame = channels*sizeof(short)*1024;
    return ENCODER_NOERROR;
}

int VoAACEncoder::input(unsigned char *inBuffer, int inLength)
{
    if ((VO_U32)inLength > mOutBuffer.Length) {
        VO_U32 newLength = (inLength/mOutBuffer.Length + 1) * mOutBuffer.Length;
        VO_PBYTE newBuffer = (VO_PBYTE)realloc(mOutBuffer.Buffer, newLength);
        if (newBuffer == NULL) {
            pr_err("Unable to reallocate output buffer\n");
            return ENCODER_ERROR_NOMEM;
        }
        mOutBuffer.Buffer = newBuffer;
        mOutBuffer.Length = newLength;
    }

    VO_CODECBUFFER       inData;
    VO_CODECBUFFER       outData;
    VO_AUDIO_OUTPUTINFO  outInfo;
    int bytesRead = 0;
    int bytesEncoded = 0;

    if (mBytesRemain > 0) {
        if ((mBytesRemain + inLength) >= mBytesFrame) {
            int bytesFilled = mBytesFrame - mBytesRemain;
            memcpy(mInBuffer.Buffer + mBytesRemain, inBuffer, bytesFilled);

            inData.Buffer = mInBuffer.Buffer;
            inData.Length = mBytesFrame;
            mCodecApi.SetInputData(mCodecHandle, &inData);

            outData.Buffer = mOutBuffer.Buffer;
            outData.Length = mOutBuffer.Length;
            if (mCodecApi.GetOutputData(mCodecHandle, &outData, &outInfo) != VO_ERR_NONE) {
                pr_err("Unable to encode frame\n");
                return ENCODER_ERROR_GENERIC;
            }

            bytesRead += bytesFilled;
            bytesEncoded += outData.Length;
        }
        else {
            memcpy(mInBuffer.Buffer + mBytesRemain, inBuffer, inLength);
            mBytesRemain += inLength;
            return ENCODER_NOERROR;
        }
    }

    mBytesRemain = inLength - bytesRead;
    while (mBytesRemain >= mBytesFrame) {
        inData.Buffer = inBuffer + bytesRead;
        inData.Length = mBytesFrame;
        mCodecApi.SetInputData(mCodecHandle, &inData);

        outData.Buffer = mOutBuffer.Buffer + bytesEncoded;
        outData.Length = mOutBuffer.Length - bytesEncoded;
        if (mCodecApi.GetOutputData(mCodecHandle, &outData, &outInfo) != VO_ERR_NONE) {
            pr_err("Unable to encode frame\n");
            return ENCODER_ERROR_GENERIC;
        }

        mBytesRemain -= mBytesFrame;
        bytesRead += mBytesFrame;
        bytesEncoded += outData.Length;
    }

    if (mBytesRemain > 0) {
        memcpy(mInBuffer.Buffer, inBuffer + bytesRead, mBytesRemain);
    }

    if (bytesEncoded > 0)
        mListener->output(mOutBuffer.Buffer, bytesEncoded);
    return ENCODER_NOERROR;
}

void VoAACEncoder::deinit()
{
    if (mCodecHandle != NULL) {
        mCodecApi.Uninit(mCodecHandle);
        mCodecHandle = NULL;
    }
}

int VoAACEncoder::getPreferredBitRate(int sampleRate, int channels)
{
    int scale = 441;
    if (sampleRate%8000 == 0)
        scale = 480;
    return 640*channels*sampleRate/scale;
}
