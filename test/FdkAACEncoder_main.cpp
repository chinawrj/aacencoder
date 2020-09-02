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
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "IAACEncoder.h"
#include "FdkAACEncoder.h"
#include "wavreader.h"

class AACEncoderOutput : public IAACEncoderListener
{
public:
    AACEncoderOutput(FILE *file) : mFile(file) {}
    virtual void output(unsigned char *outBuffer, int outLength) {
        fprintf(stdout, "Output encoded data, outLength=%d\n", outLength);
        fwrite(outBuffer, 1, outLength, mFile);
    }
private:
    FILE *mFile;
};

void usage(const char *name)
{
    fprintf(stderr, "%s in.wav out.aac\n", name);
}

int main(int argc, char *argv[])
{
    const char *infile, *outfile;
    FILE *out;
    void *wav;
    int format, sampleRate, channels, bitsPerSample;
    int inputSize;
    uint8_t *inputBuf;

    if (argc - optind < 2) {
        usage(argv[0]);
        return 1;
    }
    infile = argv[optind];
    outfile = argv[optind + 1];

    wav = wav_read_open(infile);
    if (!wav) {
        fprintf(stderr, "Unable to open wav file %s\n", infile);
        return 1;
    }
    if (!wav_get_header(wav, &format, &channels, &sampleRate, &bitsPerSample, NULL)) {
        fprintf(stderr, "Bad wav file %s\n", infile);
        return 1;
    }
    if (format != 1) {
        fprintf(stderr, "Unsupported WAV format %d\n", format);
        return 1;
    }
    if (bitsPerSample != 16) {
        fprintf(stderr, "Unsupported WAV sample depth %d\n", bitsPerSample);
        return 1;
    }
    inputSize = channels*2*1024;
    inputBuf = (uint8_t *)malloc(inputSize);

    out = fopen(outfile, "wb");
    if (!out) {
        perror(outfile);
        return 1;
    }

    int ret = 0;
    AACEncoderOutput *output = new AACEncoderOutput(out);
    IAACEncoder *encoder = new FdkAACEncoder();
    ret = encoder->init(output, sampleRate, channels, bitsPerSample);
    if (ret != IAACEncoder::ENCODER_NOERROR) {
        fprintf(stderr, "Unable to init encoder, error=%d\n", ret);
        return 1;
    }

    while (1) {
        int inLength = wav_read_data(wav, inputBuf, inputSize);
        if (inLength <= 0)
            break;
        ret = encoder->encode(inputBuf, inLength);
        if (ret != IAACEncoder::ENCODER_NOERROR) {
            fprintf(stderr, "Unable to encode data, error=%d\n", ret);
            return 1;
        }
    }

    encoder->deinit();
    delete (FdkAACEncoder *)encoder;

    free(inputBuf);
    fclose(out);
    wav_read_close(wav);

    return 0;
}

