/*
 * SpanDSP - a series of DSP components for telephony
 *
 * v8_tests.c
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2004 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: v8_tests.c,v 1.8 2005/12/25 17:33:37 steveu Exp $
 */

/*! \page v8_tests_page V.8 tests
\section v8_tests_page_sec_1 What does it do?
*/

#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <audiofile.h>
#include <tiffio.h>

#include "spandsp.h"

#define FALSE 0
#define TRUE (!FALSE)

#define SAMPLES_PER_CHUNK   160

#define OUTPUT_FILE_NAME    "v8.wav"

void handler(void *user_data, int result)
{
    v8_state_t *s;
    
    s = (v8_state_t *) user_data;
    
    v8_log_selected_modulation(s, result);
}

int main(int argc, char *argv[])
{
    int i;
    int j;
    int pitch;
    int16_t amp[160];
    int16_t out_amp[2*160];
    v8_state_t v8_caller;
    v8_state_t v8_answerer;
    int len;
    int hits;
    int frames;
    int outframes;
    int samples;
    int remnant;
    float x;
    AFfilehandle outhandle;
    AFfilesetup filesetup;
    
    filesetup = afNewFileSetup();
    if (filesetup == AF_NULL_FILESETUP)
    {
        fprintf(stderr, "    Failed to create file setup\n");
        exit(2);
    }
    afInitSampleFormat(filesetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
    afInitRate(filesetup, AF_DEFAULT_TRACK, (float) SAMPLE_RATE);
    afInitFileFormat(filesetup, AF_FILE_WAVE);
    afInitChannels(filesetup, AF_DEFAULT_TRACK, 2);
    outhandle = afOpenFile(OUTPUT_FILE_NAME, "w", filesetup);
    if (outhandle == AF_NULL_FILEHANDLE)
    {
        fprintf(stderr, "    Cannot create wave file '%s'\n", OUTPUT_FILE_NAME);
        exit(2);
    }

    v8_init(&v8_caller, TRUE, 0xFFFFFFFF, handler, &v8_caller);
    v8_init(&v8_answerer, FALSE, 0xFFFFFFFF, handler, &v8_answerer);
    v8_caller.logging.level = SPAN_LOG_FLOW | SPAN_LOG_SHOW_TAG;
    v8_caller.logging.tag = "caller";
    v8_answerer.logging.level = SPAN_LOG_FLOW | SPAN_LOG_SHOW_TAG;
    v8_answerer.logging.tag = "answerer";
    for (i = 0;  i < 1000;  i++)
    {
        samples = v8_tx(&v8_caller, amp, SAMPLES_PER_CHUNK);
        if (samples < SAMPLES_PER_CHUNK)
        {
            memset(amp + samples, 0, sizeof(int16_t)*(SAMPLES_PER_CHUNK - samples));
            samples = SAMPLES_PER_CHUNK;
        }
        remnant = v8_rx(&v8_answerer, amp, samples);
        for (i = 0;  i < samples;  i++)
            out_amp[2*i] = amp[i];
        
        samples = v8_tx(&v8_answerer, amp, SAMPLES_PER_CHUNK);
        if (samples < SAMPLES_PER_CHUNK)
        {
            memset(amp + samples, 0, sizeof(int16_t)*(SAMPLES_PER_CHUNK - samples));
            samples = SAMPLES_PER_CHUNK;
        }
        if (v8_rx(&v8_caller, amp, samples)  &&  remnant)
            break;
        for (i = 0;  i < samples;  i++)
            out_amp[2*i + 1] = amp[i];

        outframes = afWriteFrames(outhandle,
                                  AF_DEFAULT_TRACK,
                                  out_amp,
                                  samples);
        if (outframes != samples)
        {
            fprintf(stderr, "    Error writing wave file\n");
            exit(2);
        }
    }
    if (afCloseFile(outhandle))
    {
        fprintf(stderr, "    Cannot close wave file '%s'\n", OUTPUT_FILE_NAME);
        exit(2);
    }
    afFreeFileSetup(filesetup);
    
    v8_release(&v8_caller);
    v8_release(&v8_answerer);
    return  0;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
