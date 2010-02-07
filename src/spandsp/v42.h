/*
 * SpanDSP - a series of DSP components for telephony
 *
 * lapm.h
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
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
 * $Id: v42.h,v 1.6 2005/01/18 14:05:49 steveu Exp $
 */

#if !defined(_V42_H_)
#define _V42_H_

/*! \page V42_page The V.42 modem error correction
\section V42_page_sec_1 What does it do?
The V.42 specification defines an error correcting protocol for PSTN modems, based on
HDLC and LAP. This makes it similar to an X.25 link. A special variant of LAP, known
as LAP-M, is defined in the V.42 specification. A means for modems to determine if the
far modem supports V.42 is also defined.

\section V42_page_sec_2 How does it work?
*/

enum
{
    LAPM_DETECT = 0,
    LAPM_ESTABLISH = 1,
    LAPM_DATA = 2,
    LAPM_RELEASE = 3,
    LAPM_SIGNAL = 4,
    LAPM_SETPARM = 5,
    LAPM_TEST = 6,
    LAPM_UNSUPPORTED = 7
};

typedef void (*v42_status_func_t)(void *user_data, int status);
typedef void (*v42_frame_handler_t)(void *user_data, const uint8_t *pkt, int len);

typedef struct lapm_frame_queue_s
{
    struct lapm_frame_queue_s *next;
    int len;
    uint8_t frame[0];
} lapm_frame_queue_t;

typedef struct
{
    int handle;
    hdlc_rx_state_t hdlc_rx;
    hdlc_tx_state_t hdlc_tx;
    
    v42_frame_handler_t iframe_receive;
    void *iframe_receive_user_data;

    v42_status_func_t status_callback;
    void *status_callback_user_data;

    int state;
    int tx_waiting;
    int debug;
    /*! TRUE if originator. FALSE if answerer */
    int we_are_originator;
    /*! Remote network type (unknown, answerer. originator) */
    int peer_is_originator;
    /*! Next N(S) for transmission */
    int next_tx_frame;
    /*! The last of our frames which the peer acknowledged */
    int last_frame_peer_acknowledged;
    /*! Next N(R) for reception */
    int next_expected_frame;
    /*! The last of the peer's frames which we acknowledged */
    int last_frame_we_acknowledged;
    /*! TRUE if we sent an I or S frame with the F-bit set */
    int solicit_f_bit;
    /*! Retransmission count */
    int retransmissions;
    /*! TRUE if peer is busy */
    int busy;

    /*! Acknowledgement timer */
    int t401_timer;
    /*! Reply delay timer - optional */
    int t402_timer;
    /*! Inactivity timer - optional */
    int t403_timer;
    /*! Maximum number of octets in an information field */
    int n401;
    /*! Window size */
    int window_size_k;
	
    lapm_frame_queue_t *txqueue;
    lapm_frame_queue_t *tx_next;
    lapm_frame_queue_t *tx_last;
    queue_t tx_queue;
    
    sp_sched_state_t sched;
} lapm_state_t;

typedef struct
{
    /*! TRUE if we are the calling party, otherwise FALSE */
    int caller;
    /*! TRUE if we should detect whether the far end is V.42 capable. FALSE if we go
        directly to protocol establishment */
    int detect;

    /*! Stage in negotiating V.42 support */
    int rx_negotiation_step;
    int rxbits;
    int rxstream;
    int rxoks;
    int odp_seen;
    int txbits;
    int txstream;
    int txadps;
    /*! The LAP.M context */
    lapm_state_t lapm;

    /*! V.42 support detection timer */
    int t400_timer;
} v42_state_t;

/*! Log the raw HDLC frames */
#define LAPM_DEBUG_LAPM_RAW         (1 << 0)
/*! Log the interpreted frames */
#define LAPM_DEBUG_LAPM_DUMP        (1 << 1)
/*! Log state machine changes */
#define LAPM_DEBUG_LAPM_STATE 	    (1 << 2)

#ifdef __cplusplus
extern "C" {
#endif

/*! Dump LAP.M frames in a raw and/or decoded forms
    \param frame The frame itself
    \param len The length of the frame, in octets
    \param showraw TRUE if the raw octets should be dumped
    \param txrx TRUE if tx, FALSE if rx. Used to highlight the packet's direction.
*/
void lapm_dump(const uint8_t *frame, int len, int showraw, int txrx);

/*! Accept an HDLC packet
*/
void lapm_receive(void *user_data, int ok, const uint8_t *buf, int len);

/*! Transmit a LAP.M information frame
*/
int lapm_tx_iframe(lapm_state_t *s, const void *buf, int len, int cr);

/*! Assign or remove a callback routine used to deal with V.42 status changes.
*/
void v42_set_status_callback(v42_state_t *s, v42_status_func_t callback, void *user_data);

/*! Process a newly received bit for a V.42 context.
*/
void v42_rx_bit(void *user_data, int bit);

/*! Get the next transmit bit for a V.42 context.
*/
int v42_tx_bit(void *user_data);

/*! Initialise a V.42 context.
*/
void v42_init(v42_state_t *s, int caller, int detect, v42_frame_handler_t frame_handler, void *user_data);

/*! Restart a V.42 context.
*/
void v42_restart(v42_state_t *s);

#ifdef __cplusplus
}
#endif

#endif
/*- End of file ------------------------------------------------------------*/
