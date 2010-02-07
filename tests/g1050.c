/*
 * SpanDSP - a series of DSP components for telephony
 *
 * g1050.c - IP network modelling, as per G.1050/TIA-921.
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2007 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
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
 * $Id: g1050.c,v 1.9 2007/03/29 12:28:37 steveu Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <audiofile.h>
#include <tiffio.h>
#if defined(HAVE_TGMATH_H)
#include <tgmath.h>
#endif
#if defined(HAVE_MATH_H)
#define GEN_CONST
#include <math.h>
#endif

#include "spandsp.h"

#include "g1050.h"

#define PACKET_LOSS_TIME    -1

#define FALSE 0
#define TRUE (!FALSE)

g1050_constants_t g1050_constants[1] =
{
    {
        {
            {   /* Side A LAN */
                {
                    0.004,
                    0.1
                },
                {
                    {
                        0.0,
                        0.0,
                    },
                    {
                        0.5,
                        0.0
                    }
                },
                1.0,
                0.0,
                0.001,
                0.15
            },
            {   /* Side A access link */
                {
                    0.0002,
                    0.2
                },
                {
                    {
                        0.001,
                        0.0,
                    },
                    {
                        0.3,
                        0.4
                    }
                },
                40.0,
                0.75,
                0.0005,
                0.0
            },
            {   /* Side B access link */
                {
                    0.0002,
                    0.2
                },
                {
                    {
                        0.001,
                        0.0,
                    },
                    {
                        0.3,
                        0.4
                    }
                },
                40.0,
                0.75,
                0.0005,
                0.0
            },
            {   /* Side B LAN */
                {
                    0.004,
                    0.1
                },
                {
                    {
                        0.0,
                        0.0,
                    },
                    {
                        0.5,
                        0.0
                    }
                },
                1.0,
                0.0,
                0.001,
                0.15
            }
        }
    }
};

g1050_channel_speeds_t g1050_speed_patterns[133] =
{
    {  4000000, 0,    128000,   768000, 0,     4000000, 0,    128000,   768000, 0,  2.2500},
    {  4000000, 0,    128000,   768000, 0,    20000000, 0,    128000,   768000, 0,  1.5000},
    { 20000000, 0,    128000,   768000, 0,    20000000, 0,    128000,   768000, 0,  0.2500},
    {  4000000, 0,    128000,  1536000, 0,     4000000, 0,    384000,   768000, 0,  3.4125},
    {  4000000, 0,    128000,  1536000, 0,    20000000, 0,    384000,   768000, 0,  2.6750},
    { 20000000, 0,    128000,  1536000, 0,    20000000, 0,    384000,   768000, 0,  0.5125},
    {  4000000, 0,    128000,  3000000, 0,     4000000, 0,    384000,   768000, 0,  0.7875},
    {  4000000, 0,    128000,  3000000, 0,    20000000, 0,    384000,   768000, 0,  0.6750},
    { 20000000, 0,    128000,  3000000, 0,    20000000, 0,    384000,   768000, 0,  0.1375},
    {  4000000, 0,    384000,   768000, 0,     4000000, 0,    128000,  1536000, 0,  3.4125},
    {  4000000, 0,    384000,   768000, 0,    20000000, 0,    128000,  1536000, 0,  2.6750},
    { 20000000, 0,    384000,   768000, 0,    20000000, 0,    128000,  1536000, 0,  0.5125},
    {  4000000, 0,    384000,  1536000, 0,     4000000, 0,    384000,  1536000, 0,  5.1756},
    {  4000000, 0,    384000,  1536000, 0,    20000000, 0,    384000,  1536000, 0,  4.6638},
    { 20000000, 0,    384000,  1536000, 0,    20000000, 0,    384000,  1536000, 0,  1.0506},
    {  4000000, 0,    384000,  3000000, 0,     4000000, 0,    384000,  1536000, 0,  1.1944},
    {  4000000, 0,    384000,  3000000, 0,    20000000, 0,    384000,  1536000, 0,  1.1638},
    { 20000000, 0,    384000,  3000000, 0,    20000000, 0,    384000,  1536000, 0,  0.2819},
    {  4000000, 0,    384000,   768000, 0,     4000000, 0,    128000,  3000000, 0,  0.7875},
    {  4000000, 0,    384000,   768000, 0,    20000000, 0,    128000,  3000000, 0,  0.6750},
    { 20000000, 0,    384000,   768000, 0,    20000000, 0,    128000,  3000000, 0,  0.1375},
    {  4000000, 0,    384000,  1536000, 0,     4000000, 0,    384000,  3000000, 0,  1.1944},
    {  4000000, 0,    384000,  1536000, 0,    20000000, 0,    384000,  3000000, 0,  1.1638},
    { 20000000, 0,    384000,  1536000, 0,    20000000, 0,    384000,  3000000, 0,  0.2819},
    {  4000000, 0,    384000,  3000000, 0,     4000000, 0,    384000,  3000000, 0,  0.2756},
    {  4000000, 0,    384000,  3000000, 0,    20000000, 0,    384000,  3000000, 0,  0.2888},
    { 20000000, 0,    384000,  3000000, 0,    20000000, 0,    384000,  3000000, 0,  0.0756},
    {  4000000, 0,    128000,  1536000, 0,   100000000, 0,    384000,   768000, 0,  1.8000},
    { 20000000, 0,    128000,  1536000, 0,   100000000, 0,    384000,   768000, 0,  0.6000},
    {  4000000, 0,    128000,  3000000, 0,   100000000, 0,    384000,   768000, 0,  0.6750},
    { 20000000, 0,    128000,  3000000, 0,   100000000, 0,    384000,   768000, 0,  0.2250},
    {  4000000, 0,    128000,  1536000, 0,     4000000, 0,    768000,  1536000, 0,  0.6000},
    {  4000000, 0,    128000,  1536000, 0,    20000000, 0,    768000,  1536000, 0,  0.8000},
    {  4000000, 0,    128000,  1536000, 0,   100000000, 0,    768000,  1536000, 0,  1.8000},
    { 20000000, 0,    128000,  1536000, 0,    20000000, 0,    768000,  1536000, 0,  0.2000},
    { 20000000, 0,    128000,  1536000, 0,   100000000, 0,    768000,  1536000, 0,  0.6000},
    {  4000000, 0,    128000, 43000000, 0,     4000000, 0,    768000, 43000000, 0,  0.0750},
    {  4000000, 0,    128000, 43000000, 0,    20000000, 0,    768000, 43000000, 0,  0.1000},
    {  4000000, 0,    128000, 43000000, 0,   100000000, 0,    768000, 43000000, 0,  0.2250},
    { 20000000, 0,    128000, 43000000, 0,    20000000, 0,    768000, 43000000, 0,  0.0250},
    { 20000000, 0,    128000, 43000000, 0,   100000000, 0,    768000, 43000000, 0,  0.0750},
    {  4000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  5.4600},
    { 20000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  2.4600},
    {  4000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  1.6538},
    { 20000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  0.7913},
    {  4000000, 0,    384000,  1536000, 0,     4000000, 0,   1536000,  1536000, 0,  0.9100},
    {  4000000, 0,    384000,  1536000, 0,    20000000, 0,   1536000,  1536000, 0,  1.3200},
    {  4000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  3.2100},
    { 20000000, 0,    384000,  1536000, 0,    20000000, 0,   1536000,  1536000, 0,  0.4100},
    { 20000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  1.7100},
    {  4000000, 0,    384000, 43000000, 0,     4000000, 0,   1536000, 43000000, 0,  0.1138},
    {  4000000, 0,    384000, 43000000, 0,    20000000, 0,   1536000, 43000000, 0,  0.1650},
    {  4000000, 0,    384000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.4013},
    { 20000000, 0,    384000, 43000000, 0,    20000000, 0,   1536000, 43000000, 0,  0.0513},
    { 20000000, 0,    384000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.2138},
    {  4000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  3000000, 0,  1.6538},
    { 20000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  3000000, 0,  0.7913},
    {  4000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  3000000, 0,  0.4725},
    { 20000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  3000000, 0,  0.2475},
    {  4000000, 0,    384000,  1536000, 0,     4000000, 0,   1536000,  3000000, 0,  0.2100},
    {  4000000, 0,    384000,  1536000, 0,    20000000, 0,   1536000,  3000000, 0,  0.3200},
    {  4000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  3000000, 0,  0.8100},
    { 20000000, 0,    384000,  1536000, 0,    20000000, 0,   1536000,  3000000, 0,  0.1100},
    { 20000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  3000000, 0,  0.5100},
    {  4000000, 0,    384000, 43000000, 0,     4000000, 0,   3000000, 43000000, 0,  0.0263},
    {  4000000, 0,    384000, 43000000, 0,    20000000, 0,   3000000, 43000000, 0,  0.0400},
    {  4000000, 0,    384000, 43000000, 0,   100000000, 0,   3000000, 43000000, 0,  0.1013},
    { 20000000, 0,    384000, 43000000, 0,    20000000, 0,   3000000, 43000000, 0,  0.0138},
    { 20000000, 0,    384000, 43000000, 0,   100000000, 0,   3000000, 43000000, 0,  0.0638},
    {  4000000, 0,    384000,   768000, 0,   100000000, 0,    128000,  1536000, 0,  1.8000},
    { 20000000, 0,    384000,   768000, 0,   100000000, 0,    128000,  1536000, 0,  0.6000},
    {  4000000, 0,    384000,   768000, 0,   100000000, 0,    128000,  3000000, 0,  0.6750},
    { 20000000, 0,    384000,   768000, 0,   100000000, 0,    128000,  3000000, 0,  0.2250},
    {  4000000, 0,    768000,  1536000, 0,     4000000, 0,    128000,  1536000, 0,  0.6000},
    {  4000000, 0,    768000,  1536000, 0,    20000000, 0,    128000,  1536000, 0,  0.8000},
    { 20000000, 0,    768000,  1536000, 0,    20000000, 0,    128000,  1536000, 0,  0.2000},
    {  4000000, 0,    768000,  1536000, 0,   100000000, 0,    128000,  1536000, 0,  1.8000},
    { 20000000, 0,    768000,  1536000, 0,   100000000, 0,    128000,  1536000, 0,  0.6000},
    {  4000000, 0,   1536000,  1536000, 0,     4000000, 0,    384000,  1536000, 0,  0.9100},
    {  4000000, 0,   1536000,  1536000, 0,    20000000, 0,    384000,  1536000, 0,  1.3200},
    { 20000000, 0,   1536000,  1536000, 0,    20000000, 0,    384000,  1536000, 0,  0.4100},
    {  4000000, 0,   1536000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  3.2100},
    { 20000000, 0,   1536000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  1.7100},
    {  4000000, 0,   1536000,  3000000, 0,     4000000, 0,    384000,  1536000, 0,  0.2100},
    {  4000000, 0,   1536000,  3000000, 0,    20000000, 0,    384000,  1536000, 0,  0.3200},
    { 20000000, 0,   1536000,  3000000, 0,    20000000, 0,    384000,  1536000, 0,  0.1100},
    {  4000000, 0,   1536000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  0.8100},
    { 20000000, 0,   1536000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  0.5100},
    {  4000000, 0,    768000, 43000000, 0,     4000000, 0,    128000, 43000000, 0,  0.0750},
    {  4000000, 0,    768000, 43000000, 0,    20000000, 0,    128000, 43000000, 0,  0.1000},
    { 20000000, 0,    768000, 43000000, 0,    20000000, 0,    128000, 43000000, 0,  0.0250},
    {  4000000, 0,    768000, 43000000, 0,   100000000, 0,    128000, 43000000, 0,  0.2250},
    { 20000000, 0,    768000, 43000000, 0,   100000000, 0,    128000, 43000000, 0,  0.0750},
    {  4000000, 0,   1536000, 43000000, 0,     4000000, 0,    384000, 43000000, 0,  0.1138},
    {  4000000, 0,   1536000, 43000000, 0,    20000000, 0,    384000, 43000000, 0,  0.1650},
    { 20000000, 0,   1536000, 43000000, 0,    20000000, 0,    384000, 43000000, 0,  0.0513},
    {  4000000, 0,   1536000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.4013},
    { 20000000, 0,   1536000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.2138},
    {  4000000, 0,   3000000, 43000000, 0,     4000000, 0,    384000, 43000000, 0,  0.0263},
    {  4000000, 0,   3000000, 43000000, 0,    20000000, 0,    384000, 43000000, 0,  0.0400},
    { 20000000, 0,   3000000, 43000000, 0,    20000000, 0,    384000, 43000000, 0,  0.0138},
    {  4000000, 0,   3000000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.1013},
    { 20000000, 0,   3000000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.0638},
    {100000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  1.4400},
    {100000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  0.5400},
    {100000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  1.4400},
    {100000000, 0,    384000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.1800},
    {100000000, 0,    384000,  1536000, 0,   100000000, 0,    384000,  3000000, 0,  0.5400},
    {100000000, 0,    384000,  3000000, 0,   100000000, 0,    384000,  3000000, 0,  0.2025},
    {100000000, 0,    384000,  1536000, 0,   100000000, 0,   1536000,  3000000, 0,  0.5400},
    {100000000, 0,    384000, 43000000, 0,   100000000, 0,   3000000, 43000000, 0,  0.0675},
    {100000000, 0,   1536000,  1536000, 0,   100000000, 0,    384000,  1536000, 0,  1.4400},
    {100000000, 0,   1536000,  3000000, 0,   100000000, 0,    384000,  1536000, 0,  0.5400},
    {  4000000, 0,   1536000,  1536000, 0,     4000000, 0,   1536000,  1536000, 0,  0.1600},
    {  4000000, 0,   1536000,  1536000, 0,    20000000, 0,   1536000,  1536000, 0,  0.3200},
    {  4000000, 0,   1536000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  0.9600},
    { 20000000, 0,   1536000,  1536000, 0,    20000000, 0,   1536000,  1536000, 0,  0.1600},
    { 20000000, 0,   1536000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  0.9600},
    {100000000, 0,   1536000,  1536000, 0,   100000000, 0,   1536000,  1536000, 0,  1.4400},
    {  4000000, 0,   1536000, 43000000, 0,     4000000, 0,   1536000, 43000000, 0,  0.0400},
    {  4000000, 0,   1536000, 43000000, 0,    20000000, 0,   1536000, 43000000, 0,  0.0800},
    {  4000000, 0,   1536000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.2400},
    { 20000000, 0,   1536000, 43000000, 0,    20000000, 0,   1536000, 43000000, 0,  0.0400},
    { 20000000, 0,   1536000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.2400},
    {100000000, 0,   1536000, 43000000, 0,   100000000, 0,   1536000, 43000000, 0,  0.3600},
    {100000000, 0,   1536000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.1800},
    {100000000, 0,   3000000, 43000000, 0,   100000000, 0,    384000, 43000000, 0,  0.0675},
    {  4000000, 0,  43000000, 43000000, 0,     4000000, 0,  43000000, 43000000, 0,  0.0025},
    {  4000000, 0,  43000000, 43000000, 0,    20000000, 0,  43000000, 43000000, 0,  0.0050},
    {  4000000, 0,  43000000, 43000000, 0,   100000000, 0,  43000000, 43000000, 0,  0.0150},
    { 20000000, 0,  43000000, 43000000, 0,    20000000, 0,  43000000, 43000000, 0,  0.0025},
    { 20000000, 0,  43000000, 43000000, 0,   100000000, 0,  43000000, 43000000, 0,  0.0150},
    {100000000, 0,  43000000, 43000000, 0,   100000000, 0,  43000000, 43000000, 0,  0.0225}
};

g1050_model_t g1050_standard_models[9] =
{
    {   /* Severity 0 - no impairment */
        {
            0,
            0,
            0,
        },
        {
            0.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            0.0,        /*! Basic delay of the backbone, in seconds */
            0.0,        /*! Percentage packet loss of the backbone */
            0.0,        /*! Maximum jitter of the backbone, in seconds */
            0.0,        /*! Interval between the backbone route flapping between two paths, in seconds */
            0.0,        /*! The difference in backbone delay between the two routes we flap between, in seconds */
            0.0,        /*! The interval between link failures, in seconds */
            0.0,        /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            0.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            0.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity A */
        {
            50,
            5,
            5,
        },
        {
            1.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            0.004,      /*! Basic delay of the backbone, in seconds */
            0.0,        /*! Percentage packet loss of the backbone */
            0.005,      /*! Maximum jitter of the backbone, in seconds */
            0.0,        /*! Interval between the backbone route flapping between two paths, in seconds */
            0.0,        /*! The difference in backbone delay between the two routes we flap between, in seconds */
            0.0,        /*! The interval between link failures, in seconds */
            0.0,        /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            0.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            1.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity B */
        {
            30,
            25,
            5,
        },
        {
            2.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            1.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            0.008,      /*! Basic delay of the backbone, in seconds */
            0.01,       /*! Percentage packet loss of the backbone */
            0.01,       /*! Maximum jitter of the backbone, in seconds */
            3600.0,     /*! Interval between the backbone route flapping between two paths, in seconds */
            0.002,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            3600.0,     /*! The interval between link failures, in seconds */
            0.064,      /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            1.0,        /*! Percentage occupancy */
            512         /*! MTU */
        },
        {
            2.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity C */
        {
            15,
            30,
            10,
        },
        {
            3.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            2.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.016,      /*! Basic delay of the backbone, in seconds */
            0.02,       /*! Percentage packet loss of the backbone */
            0.016,      /*! Maximum jitter of the backbone, in seconds */
            1800.0,     /*! Interval between the backbone route flapping between two paths, in seconds */
            0.004,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            1800.0,     /*! The interval between link failures, in seconds */
            0.128,      /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            2.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            3.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity D */
        {
            5,
            25,
            15,
        },
        {
            5.0,         /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            4.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.032,      /*! Basic delay of the backbone, in seconds */
            0.04,       /*! Percentage packet loss of the backbone */
            0.04,       /*! Maximum jitter of the backbone, in seconds */
            900.0,      /*! Interval between the backbone route flapping between two paths, in seconds */
            0.008,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            900.0,      /*! The interval between link failures, in seconds */
            0.256,      /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            4.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            5.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity E */
        {
            0,
            10,
            20,
        },
        {
            8.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            8.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.064,      /*! Basic delay of the backbone, in seconds */
            0.1,        /*! Percentage packet loss of the backbone */
            0.07,       /*! Maximum jitter of the backbone, in seconds */
            480.0,      /*! Interval between the backbone route flapping between two paths, in seconds */
            0.016,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            480.0,      /*! The interval between link failures, in seconds */
            0.4,        /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            8.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            8.0,        /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity F */
        {
            0,
            0,
            25,
        },
        {
            12.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            15.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.128,      /*! Basic delay of the backbone, in seconds */
            0.2,        /*! Percentage packet loss of the backbone */
            0.1,        /*! Maximum jitter of the backbone, in seconds */
            240.0,      /*! Interval between the backbone route flapping between two paths, in seconds */
            0.032,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            240.0,      /*! The interval between link failures, in seconds */
            0.8,        /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            15.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            12.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity G */
        {
            0,
            0,
            15,
        },
        {
            16.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            30.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.256,      /*! Basic delay of the backbone, in seconds */
            0.5,        /*! Percentage packet loss of the backbone */
            0.15,       /*! Maximum jitter of the backbone, in seconds */
            120.0,      /*! Interval between the backbone route flapping between two paths, in seconds */
            0.064,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            120.0,      /*! The interval between link failures, in seconds */
            1.6,        /*! The duration of link failures, in seconds */
            0.0,        /*! Probability of packet loss in the backbone, in percent */
            0.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            30.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            16.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        }
    },
    {   /* Severity H */
        {
            0,
            0,
            5,
        },
        {
            20.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            50.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            0.512,      /*! Basic delay of the backbone, in seconds */
            1.0,        /*! Percentage packet loss of the backbone */
            0.5,        /*! Maximum jitter of the backbone, in seconds */
            60.0,       /*! Interval between the backbone route flapping between two paths, in seconds */
            0.128,      /*! The difference in backbone delay between the two routes we flap between, in seconds */
            60.0,       /*! The interval between link failures, in seconds */
            3.0,        /*! The duration of link failures, in seconds */
            1.0,        /*! Probability of packet loss in the backbone, in percent */
            1.0         /*! Probability of a packet going out of sequence in the backbone. */
        },
        {
            50.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        },
        {
            20.0,       /*! Percentage occupancy */
            1508        /*! MTU */
        }
    }
};

static __inline__ double q1050_rand(void)
{
    return drand48();
}
/*- End of function --------------------------------------------------------*/

static __inline__ double scale_probability(double prob, double scale)
{
    /* Re-calculate probability based on a different time interval */
    return 1.0 - pow(1.0 - prob, scale);
}
/*- End of function --------------------------------------------------------*/

static void g1050_segment_init(g1050_segment_state_t *s,
                               int link_type,
                               g1050_segment_constants_t *constants,
                               g1050_segment_model_t *parms,
                               int bit_rate,
                               int multiple_access,
                               int qos_enabled,
                               int packet_size,
                               int packet_rate)
{
    double x;
    double packet_interval;

    memset(s, 0, sizeof(*s));

    packet_interval = 1000.0/packet_rate;
    /* Some calculatons are common to both LAN and access links, and those that are not. */
    s->link_type = link_type;
    s->prob_loss_rate_change[0] = scale_probability(constants->prob_loss_rate_change[0]*parms->percentage_occupancy, 1.0/packet_interval);

    s->serial_delay = packet_size*8.0/(bit_rate*1000.0);
    if (link_type == G1050_LAN_LINK)
    {
        s->prob_loss_rate_change[1] = scale_probability(constants->prob_loss_rate_change[1], 1.0/packet_interval);
        s->prob_impulse[0] = constants->prob_impulse[0][0];
        s->prob_impulse[1] = constants->prob_impulse[1][0];
        s->impulse_coeff = constants->impulse_coeff;
        s->impulse_height = parms->mtu*(8.0/(bit_rate*1000.0))*(1.0 + parms->percentage_occupancy/constants->impulse_height);
    }
    else if (link_type == G1050_ACCESS_LINK)
    {
        s->prob_loss_rate_change[1] = scale_probability(constants->prob_loss_rate_change[1]/(1.0 + parms->percentage_occupancy), 1.0/packet_interval);
        s->prob_impulse[0] = scale_probability(constants->prob_impulse[0][0] + (parms->percentage_occupancy/2000.0), 1.0/packet_interval);
        s->prob_impulse[1] = scale_probability(constants->prob_impulse[1][0] + (constants->prob_impulse[1][1]*parms->percentage_occupancy/100.0), 1.0/packet_interval);
        s->impulse_coeff = 1.0 - scale_probability(1.0 - constants->impulse_coeff, 1.0/packet_interval);
        x = (1.0 - constants->impulse_coeff)/(1.0 - s->impulse_coeff);
        s->impulse_height = x*parms->mtu*(8.0/(bit_rate*1000.0))*(1.0 + parms->percentage_occupancy/constants->impulse_height);
    }

    /* The following are calculated the same way for LAN and access links */
    s->prob_packet_loss = constants->prob_packet_loss*parms->percentage_occupancy;
    s->qos_enabled = qos_enabled;
    s->multiple_access = multiple_access;
    s->prob_packet_collision_loss = constants->prob_packet_collision_loss;

    /* The following is common state information to all links. */
    s->high_loss = FALSE;
    s->congestion_delay = 0.0;
    s->last_arrival_time = 0.0;

    /* Count of packets lost in this segment. */
    s->lost_packets = 0;
    s->lost_packets_2 = 0;
}
/*- End of function --------------------------------------------------------*/

static void g1050_core_init(g1050_core_state_t *s, g1050_core_model_t *parms, int packet_rate)
{
    memset(s, 0, sizeof(*s));

    /* Set up route flapping. */
    /* This is the length of the period of both the delayed duration and the non-delayed. */
    s->route_flap_interval = parms->route_flap_interval*G1050_TICKS_PER_SEC;

    /* How much additional delay is added or subtracted during route flaps. */
    s->route_flap_delta = parms->route_flap_delay;

    /* Current tick count. This is initialized so that we are part way into the first
       CLEAN interval before the first change occurs. This is a random portion of the
       period. When we reach the first flap, the flapping in both directions becomes
       periodic. */
    s->route_flap_counter = s->route_flap_interval - 99 - floor(s->route_flap_interval*q1050_rand());
    s->link_failure_interval_ticks = parms->link_failure_interval*G1050_TICKS_PER_SEC;

    /* Link failures occur when the count reaches this number of ticks. */
    /* Duration of a failure. */
    s->link_failure_duration_ticks = floor((G1050_TICKS_PER_SEC*parms->link_failure_duration));
    /* How far into the first CLEAN interval we are. This is like the route flap initialzation. */
    s->link_failure_counter = s->link_failure_interval_ticks - 99 - floor(s->link_failure_interval_ticks*q1050_rand());
    s->link_recovery_counter = s->link_failure_duration_ticks;
    
    s->base_delay = parms->base_delay;
    s->jitter = parms->max_jitter;
    s->prob_packet_loss = parms->prob_packet_loss/100.0;
    s->prob_oos = parms->prob_oos/100.0;
    s->last_arrival_time = 0.0;
    s->delay_delta = 0;

    /* Count of packets lost in this segment. */
    s->lost_packets = 0;
    s->lost_packets_2 = 0;
}
/*- End of function --------------------------------------------------------*/

static void g1050_segment_model(g1050_segment_state_t *s, double delays[], int len)
{
    int i;
    int lose;
    int was_high_loss;
    double impulse;
    double slice_delay;

    /* Compute delay and loss value for each time slice. */
    for (i = 0;  i < len;  i++)
    {
        lose = FALSE;
        /* Initialize delay to serial delay. */
        slice_delay = s->serial_delay;
        /* Add a fixed jitter to all LAN links. */
        if (s->link_type == G1050_LAN_LINK)
            slice_delay += 0.0000015*q1050_rand();
        /* If no QoS, do congestion delay and packet loss analysis. */
        if (!s->qos_enabled)
        {
            /* To match the logic in G.1050 we need to record the current loss state, before
               checking if we should change. */
            was_high_loss = s->high_loss;
            /* Toggle between the low-loss and high-loss states, based on the transition probability. */
            if (q1050_rand() < s->prob_loss_rate_change[was_high_loss])
                s->high_loss = !s->high_loss;
            impulse = 0.0;
            if (q1050_rand() < s->prob_impulse[was_high_loss])
            {
                impulse = s->impulse_height;
                if (!was_high_loss  ||  s->link_type == G1050_LAN_LINK)
                    impulse *= q1050_rand();
            }

            if (was_high_loss  &&  q1050_rand() < s->prob_packet_loss)
                lose = TRUE;
            /* Single pole LPF for the congestion delay impulses. */
            s->congestion_delay = s->congestion_delay*s->impulse_coeff + impulse*(1.0 - s->impulse_coeff);
            slice_delay += s->congestion_delay;
        }
        /* If duplex mismatch on LAN, packet loss based on loss probability. */
        if (s->multiple_access  &&  (q1050_rand() < s->prob_packet_collision_loss))
            lose = TRUE;
        /* Put computed delay into time slice array. */
        /* time_slice_delays are in microseconds!    */
        if (lose)
        {
            delays[i] = PACKET_LOSS_TIME;
            s->lost_packets++;
        }
        else
        {
            delays[i] = slice_delay*1000.0;
        }
    }
}
/*- End of function --------------------------------------------------------*/

static void g1050_core_model(g1050_core_state_t *s, double delays[], int len)
{
    int32_t i;
    int lose;
    double jitter_delay;

    for (i = 0;  i < len;  i++)
    {
        lose = FALSE;
        jitter_delay = s->base_delay + s->jitter*q1050_rand();
        /* Route flapping */
        if (--s->route_flap_counter <= 0)
        {
            /* Route changed */
            s->delay_delta = s->route_flap_delta - s->delay_delta;
            s->route_flap_counter = s->route_flap_interval;
        }
        if (q1050_rand() < s->prob_packet_loss)
            lose = TRUE;
        /* Link failures */
        if (--s->link_failure_counter <= 0)
        {
            /* We are in a link failure */
            lose = TRUE;
            if (--s->link_recovery_counter <= 0)
            {
                /* Leave failure state. */
                s->link_failure_counter = s->link_failure_interval_ticks;
                s->link_recovery_counter = s->link_failure_duration_ticks;
                lose = FALSE;
            }
        }
        if (lose)
        {
            delays[i] = PACKET_LOSS_TIME;
            s->lost_packets++;
        }
        else
        {
            delays[i] = jitter_delay + s->delay_delta;
        }
    }
}
/*- End of function --------------------------------------------------------*/

static int g1050_segment_delay(g1050_segment_state_t *s, double base_time, double arrival_times[], double delays[], int num_packets)
{
    int i;
    int32_t departure_time;
    int lost_packets;

    /* Add appropriate delays to the packets for the segments before the core. */
    lost_packets = 0;
    for (i = 0;  i < num_packets;  i++)
    {
        departure_time = (arrival_times[i] + 0.0005 - base_time)*1000;
        if (arrival_times[i] == PACKET_LOSS_TIME)
        {
            /* Lost already */
        }
        else if (delays[departure_time] == PACKET_LOSS_TIME)
        {
            arrival_times[i] = PACKET_LOSS_TIME;
            lost_packets++;
        }
        else
        {
            arrival_times[i] += delays[departure_time];
            if (arrival_times[i] < s->last_arrival_time)
                arrival_times[i] = s->last_arrival_time;
            else
                s->last_arrival_time = arrival_times[i];
        }
    }
    return lost_packets;
}
/*- End of function --------------------------------------------------------*/

static int g1050_segment_delay_preserve_order(g1050_segment_state_t *s, double base_time, double arrival_times_a[], double arrival_times_b[], double delays[], int num_packets)
{
    int i;
    int j;
    int departure_time;
    double last_arrival_time;
    double last_arrival_time_temp;
    int lost_packets;

    /* Add appropriate delays to the packets for the segments after the core. */
    last_arrival_time = 0.0;
    last_arrival_time_temp = 0.0;
    lost_packets = 0;
    for (i = 0;  i < num_packets;  i++)
    {
        /* We need to preserve the order that came out of the core, so we
           use an alternate array for the results.  */
        departure_time = (arrival_times_a[i] + 0.0005 - base_time)*1000;
        if (arrival_times_a[i] == PACKET_LOSS_TIME)
        {
            /* Lost already */
            arrival_times_b[i] = PACKET_LOSS_TIME;
        }
        else if (delays[departure_time] == PACKET_LOSS_TIME)
        {
            arrival_times_b[i] = PACKET_LOSS_TIME;
            lost_packets++;
        }
        else
        {
            arrival_times_b[i] = arrival_times_a[i] + delays[departure_time];
            if (arrival_times_a[i] < last_arrival_time)
            {
                /* If a legitimate out of sequence packet is detected, search
                   back a fixed amount of time to preserve order. */
                for (j = i - 1;  j >= 0;  j--)
                {
                    if ((arrival_times_a[j] != PACKET_LOSS_TIME)
                        &&
                        (arrival_times_b[j] != PACKET_LOSS_TIME))
                    {
                        if ((arrival_times_a[i] - arrival_times_a[j]) > SEARCHBACK_PERIOD)
                            break;
                        if ((arrival_times_a[j] > arrival_times_a[i])
                            &&
                            (arrival_times_b[j] < arrival_times_b[i]))
                        {
                            arrival_times_b[j] = arrival_times_b[i];
                        }
                    }
                }
            }
            else
            {
                last_arrival_time = arrival_times_a[i];
                if (arrival_times_b[i] < last_arrival_time_temp)
                    arrival_times_b[i] = last_arrival_time_temp;
                else
                    last_arrival_time_temp = arrival_times_b[i];
            }
        }
    }
    return lost_packets;
}
/*- End of function --------------------------------------------------------*/

static int g1050_core_delay(g1050_core_state_t *s, double base_time, double arrival_times[], double delays[], int num_packets)
{
    int i;
    int departure_time;
    int lost_packets;

    /* This element does NOT preserve packet order. */
    lost_packets = 0;
    for (i = 0;  i < num_packets;  i++)
    {
        departure_time = (arrival_times[i] + 0.0005 - base_time)*1000.0;
        if (arrival_times[i] == PACKET_LOSS_TIME)
        {
            /* Lost already */
        }
        else if (delays[departure_time] == PACKET_LOSS_TIME)
        {
            arrival_times[i] = PACKET_LOSS_TIME;
            lost_packets++;
        }
        else
        {
            /* Not lost. Compute arrival time. */
            arrival_times[i] += delays[departure_time];
            if (arrival_times[i] < s->last_arrival_time)
            {
                /* This packet is EARLIER than the last one. It is out of order! */
                /* Do we allow it to stay out of order? */
                if (q1050_rand() >= s->prob_oos)
                    arrival_times[i] = s->last_arrival_time;
            }
            else
            {
                /* Packet is in the correct order, relative to the last one. */
                s->last_arrival_time = arrival_times[i];
            }
        }
    }
    return lost_packets;
}
/*- End of function --------------------------------------------------------*/

static void g1050_simulate_chunk(g1050_state_t *s)
{
    int i;
    
    s->base_time += 1.0;

    memcpy(&s->segment[0].delays[0], &s->segment[0].delays[G1050_TICKS_PER_SEC], 2*G1050_TICKS_PER_SEC*sizeof(s->segment[0].delays[0]));
    g1050_segment_model(&s->segment[0], &s->segment[0].delays[2*G1050_TICKS_PER_SEC], G1050_TICKS_PER_SEC);

    memcpy(&s->segment[1].delays[0], &s->segment[1].delays[G1050_TICKS_PER_SEC], 2*G1050_TICKS_PER_SEC*sizeof(s->segment[1].delays[0]));
    g1050_segment_model(&s->segment[1], &s->segment[1].delays[2*G1050_TICKS_PER_SEC], G1050_TICKS_PER_SEC);

    memcpy(&s->core.delays[0], &s->core.delays[G1050_TICKS_PER_SEC], 2*G1050_TICKS_PER_SEC*sizeof(s->core.delays[0]));
    g1050_core_model(&s->core, &s->core.delays[2*G1050_TICKS_PER_SEC], G1050_TICKS_PER_SEC);

    memcpy(&s->segment[2].delays[0], &s->segment[2].delays[G1050_TICKS_PER_SEC], 2*G1050_TICKS_PER_SEC*sizeof(s->segment[2].delays[0]));
    g1050_segment_model(&s->segment[2], &s->segment[2].delays[2*G1050_TICKS_PER_SEC], G1050_TICKS_PER_SEC);

    memcpy(&s->segment[3].delays[0], &s->segment[3].delays[G1050_TICKS_PER_SEC], 2*G1050_TICKS_PER_SEC*sizeof(s->segment[3].delays[0]));
    g1050_segment_model(&s->segment[3], &s->segment[3].delays[2*G1050_TICKS_PER_SEC], G1050_TICKS_PER_SEC);

    memcpy(&s->arrival_times_1[0], &s->arrival_times_1[s->packet_rate], 2*s->packet_rate*sizeof(s->arrival_times_1[0]));
    memcpy(&s->arrival_times_2[0], &s->arrival_times_2[s->packet_rate], 2*s->packet_rate*sizeof(s->arrival_times_2[0]));
    for (i = 0;  i < s->packet_rate;  i++)
    {
        s->arrival_times_1[2*s->packet_rate + i] = s->base_time + 2.0 + (double) i/(double) s->packet_rate;
        s->arrival_times_2[2*s->packet_rate + i] = 0.0;
    }

    s->segment[0].lost_packets_2 += g1050_segment_delay(&s->segment[0], s->base_time, s->arrival_times_1, s->segment[0].delays, s->packet_rate);
    s->segment[1].lost_packets_2 += g1050_segment_delay(&s->segment[1], s->base_time, s->arrival_times_1, s->segment[1].delays, s->packet_rate);
    s->core.lost_packets_2 += g1050_core_delay(&s->core, s->base_time, s->arrival_times_1, s->core.delays, s->packet_rate);
    s->segment[2].lost_packets_2 += g1050_segment_delay_preserve_order(&s->segment[2], s->base_time, s->arrival_times_1, s->arrival_times_2, s->segment[2].delays, s->packet_rate);
    s->segment[3].lost_packets_2 += g1050_segment_delay_preserve_order(&s->segment[3], s->base_time, s->arrival_times_2, s->arrival_times_1, s->segment[3].delays, s->packet_rate);
}
/*- End of function --------------------------------------------------------*/

g1050_state_t *g1050_init(int model,
                          int speed_pattern,
                          int packet_size,
                          int packet_rate)
{
    g1050_state_t *s;
    g1050_constants_t *constants;
    g1050_channel_speeds_t *sp;
    g1050_model_t *mo;
    int i;

    /* If the random generator has not been seeded it might give endless
       zeroes - it depends on the platform. */
    for (i = 0;  i < 10;  i++)
    {
        if (q1050_rand() != 0.0)
            break;
    }
    if (i >= 10)
        srand48(time(NULL));
    if ((s = (g1050_state_t *) malloc(sizeof(*s))) == NULL)
        return NULL;
    memset(s, 0, sizeof(*s));

    constants = &g1050_constants[0];
    sp = &g1050_speed_patterns[speed_pattern - 1];
    mo = &g1050_standard_models[model];

    memset(s, 0, sizeof(*s));
    
    s->packet_rate = packet_rate;
    s->packet_size = packet_size;

    g1050_segment_init(&s->segment[0],
                       G1050_LAN_LINK,
                       &constants->segment[0],
                       &mo->sidea_lan,
                       sp->sidea_lan_bit_rate,
                       sp->sidea_lan_multiple_access,
                       FALSE,
                       packet_size,
                       packet_rate);
    g1050_segment_init(&s->segment[1],
                       G1050_ACCESS_LINK,
                       &constants->segment[1],
                       &mo->sidea_access_link,
                       sp->sidea_access_link_bit_rate_ab,
                       FALSE,
                       sp->sidea_access_link_qos_enabled,
                       packet_size,
                       packet_rate);
    g1050_core_init(&s->core, &mo->core, packet_rate);
    g1050_segment_init(&s->segment[2],
                       G1050_ACCESS_LINK,
                       &constants->segment[2],
                       &mo->sideb_access_link,
                       sp->sideb_access_link_bit_rate_ba,
                       FALSE,
                       sp->sideb_access_link_qos_enabled,
                       packet_size,
                       packet_rate);
    g1050_segment_init(&s->segment[3],
                       G1050_LAN_LINK,
                       &constants->segment[3],
                       &mo->sideb_lan,
                       sp->sideb_lan_bit_rate,
                       sp->sideb_lan_multiple_access,
                       FALSE,
                       packet_size,
                       packet_rate);

    s->base_time = 0.0;
    /* Start with enough of the future modelled to allow for the worst jitter.
       After this we will always keep at least 2 seconds of the future modelled. */
    g1050_segment_model(&s->segment[0], s->segment[0].delays, 3*G1050_TICKS_PER_SEC);
    g1050_segment_model(&s->segment[1], s->segment[1].delays, 3*G1050_TICKS_PER_SEC);
    g1050_core_model(&s->core, s->core.delays, 3*G1050_TICKS_PER_SEC);
    g1050_segment_model(&s->segment[2], s->segment[2].delays, 3*G1050_TICKS_PER_SEC);
    g1050_segment_model(&s->segment[3], s->segment[3].delays, 3*G1050_TICKS_PER_SEC);

    /* Initialise the arrival times to the departure times */
    for (i = 0;  i < 3*s->packet_rate;  i++)
    {
        s->arrival_times_1[i] = s->base_time + (double) i/(double)s->packet_rate;
        s->arrival_times_2[i] = 0.0;
    }

    s->segment[0].lost_packets_2 += g1050_segment_delay(&s->segment[0], s->base_time, s->arrival_times_1, s->segment[0].delays, s->packet_rate);
    s->segment[1].lost_packets_2 += g1050_segment_delay(&s->segment[1], s->base_time, s->arrival_times_1, s->segment[1].delays, s->packet_rate);
    s->core.lost_packets_2 += g1050_core_delay(&s->core, s->base_time, s->arrival_times_1, s->core.delays, s->packet_rate);
    s->segment[2].lost_packets_2 += g1050_segment_delay_preserve_order(&s->segment[2], s->base_time, s->arrival_times_1, s->arrival_times_2, s->segment[2].delays, s->packet_rate);
    s->segment[3].lost_packets_2 += g1050_segment_delay_preserve_order(&s->segment[3], s->base_time, s->arrival_times_2, s->arrival_times_1, s->segment[3].delays, s->packet_rate);

    s->first = NULL;
    s->last = NULL;
    return s;
}
/*- End of function --------------------------------------------------------*/

void g1050_dump_parms(int model, int speed_pattern)
{
    g1050_channel_speeds_t *sp;
    g1050_model_t *mo;
    
    sp = &g1050_speed_patterns[speed_pattern - 1];
    mo = &g1050_standard_models[model];

    printf("Model %d%c\n", speed_pattern, 'A' + model - 1);
    printf("LOO %.6f%% %.6f%% %.6f%%\n", mo->loo[0]*sp->loo/100.0, mo->loo[1]*sp->loo/100.0, mo->loo[2]*sp->loo/100.0);
    printf("Side A LAN %dbps, %.3f%% occupancy, MTU %d, %s MA\n", sp->sidea_lan_bit_rate, mo->sidea_lan.percentage_occupancy, mo->sidea_lan.mtu, (sp->sidea_lan_multiple_access)  ?  ""  :  "no");
    printf("Side A access %dbps, %.3f%% occupancy, MTU %d, %s QoS\n", sp->sidea_access_link_bit_rate_ab, mo->sidea_access_link.percentage_occupancy, mo->sidea_access_link.mtu, (sp->sidea_access_link_qos_enabled)  ?  ""  :  "no");
    printf("Core delay %.4fs, peak jitter %.4fs, prob loss %.4f%%, prob OOS %.4f%%\n", mo->core.base_delay, mo->core.max_jitter, mo->core.prob_packet_loss, mo->core.prob_oos);
    printf("     Route flap interval %.4fs, delay change %.4fs\n", mo->core.route_flap_interval, mo->core.route_flap_delay);
    printf("     Link failure interval %.4fs, duration %.4fs\n", mo->core.link_failure_interval, mo->core.link_failure_duration);
    printf("Side B access %dbps, %.3f%% occupancy, MTU %d, %s QoS\n", sp->sideb_access_link_bit_rate_ba, mo->sideb_access_link.percentage_occupancy, mo->sideb_access_link.mtu, (sp->sideb_access_link_qos_enabled)  ?  ""  :  "no");
    printf("Side B LAN %dbps, %.3f%% occupancy, MTU %d, %s MA\n", sp->sideb_lan_bit_rate, mo->sideb_lan.percentage_occupancy, mo->sideb_lan.mtu, (sp->sideb_lan_multiple_access)  ?  ""  :  "no");
}
/*- End of function --------------------------------------------------------*/

int g1050_put(g1050_state_t *s, const uint8_t buf[], int len, int seq_no, double departure_time)
{
    g1050_queue_element_t *element;
    g1050_queue_element_t *e;
    double arrival_time;

    while (departure_time >= s->base_time + 1.0)
        g1050_simulate_chunk(s);
    arrival_time = s->arrival_times_1[(int) ((departure_time - s->base_time)*(double) s->packet_rate + 0.5)];
    if (arrival_time < 0)
    {
        /* This packet is lost */
        return 0;
    }
    if ((element = (g1050_queue_element_t *) malloc(sizeof(*element) + len)) == NULL)
        return -1;
    element->next = NULL;
    element->prev = NULL;
    element->seq_no = seq_no;
    element->departure_time = departure_time;
    element->arrival_time = arrival_time;
    element->len = len;
    memcpy(element->pkt, buf, len);
    /* Add it to the queue, in order */
    if (s->last == NULL)
    {
        /* The queue is empty */
        s->first =
        s->last = element;
    }
    else
    {
        for (e = s->last;  e;  e = e->prev)
        {
            if (e->arrival_time <= arrival_time)
                break;
        }
        if (e)
        {
            element->next = e->next; 
            element->prev = e; 
            e->next = element;
        }
        else
        {
            element->next = s->first;
            s->first = element;
        }
        if (element->next)
            element->next->prev = element;
        else
            s->last = element;
    }
    //printf(">> Seq %d, departs %f, arrives %f\n", seq_no, departure_time, arrival_time);
    return len;
}
/*- End of function --------------------------------------------------------*/

int g1050_get(g1050_state_t *s, uint8_t buf[], int max_len, double current_time, int *seq_no, double *departure_time, double *arrival_time)
{
    int len;
    g1050_queue_element_t *element;

    element = s->first;
    if (element == NULL)
    {
        if (seq_no)
            *seq_no = -1;
        if (departure_time)
            *departure_time = -1;
        if (arrival_time)
            *arrival_time = -1;
        return -1;
    }
    if (element->arrival_time > current_time)
    {
        if (seq_no)
            *seq_no = element->seq_no;
        if (departure_time)
            *departure_time = element->departure_time;
        if (arrival_time)
            *arrival_time = element->arrival_time;
        return -1;
    }
    /* Return the first packet in the queue */
    len = element->len;
    memcpy(buf, element->pkt, len);
    if (seq_no)
        *seq_no = element->seq_no;
    if (departure_time)
        *departure_time = element->departure_time;
    if (arrival_time)
        *arrival_time = element->arrival_time;
    //printf("<< Seq %d, arrives %f (%f)\n", element->seq_no, element->arrival_time, current_time);

    /* Remove it from the queue */
    if (s->first == s->last)
        s->last = NULL;
    s->first = element->next;
    if (element->next)
        element->next->prev = NULL;
    free(element);
    return len;
}
/*- End of function --------------------------------------------------------*/

void g1050_queue_dump(g1050_state_t *s)
{
    g1050_queue_element_t *e;

    printf("Queue scanned forewards\n");
    for (e = s->first;  e;  e = e->next)
        printf("Seq %5d, arrival %10.4f, len %3d\n", e->seq_no, e->arrival_time, e->len);
    printf("Queue scanned backwards\n");
    for (e = s->last;  e;  e = e->prev)
        printf("Seq %5d, arrival %10.4f, len %3d\n", e->seq_no, e->arrival_time, e->len);
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
