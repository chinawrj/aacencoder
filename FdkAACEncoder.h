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

#ifndef __FDKAACENCODER_H
#define __FDKAACENCODER_H

#include "aacenc_lib.h"
#include "IAACEncoder.h"

class FdkAACEncoder : public IAACEncoder
{
public:
    FdkAACEncoder();

    ~FdkAACEncoder();

    int init(IAACEncoderListener *listener,
             int sampleRate, int channels, int bitsPerSample,
             int bitRate = 0, bool adtsUsed = true);

    int encode(unsigned char *inBuffer, int inLength);

    void deinit();

private:
    typedef struct {
        unsigned char     *buffer;     /*!< Buffer pointer */
        unsigned int       length;     /*!< Buffer size in byte */
    } AACENC_BUFFER;

    IAACEncoderListener   *mListener;
    HANDLE_AACENCODER      mCodecHandle;
    AACENC_BUFFER          mOutBuffer;
    AACENC_BUFFER          mInBuffer;
    int                    mBytesRemain;
    int                    mBytesFrame;
    int preferredBitRate(int sampleRate, int channels);
};

#endif // __FDKAACENCODER_H
