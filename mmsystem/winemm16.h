/*
 * Copyright 1998, Luiz Otavio L. Zorzella
 *           1999, Eric Pouech
 *
 * Purpose:   multimedia declarations (internal to WINMM & MMSYSTEM DLLs)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wine/mmsystem16.h"
#include "wownt32.h"

/* mmsystem (16 bit files) only functions */
void            MMDRV_Init16(void);
void 		MMSYSTEM_MMTIME32to16(LPMMTIME16 mmt16, const MMTIME* mmt32) DECLSPEC_HIDDEN;

typedef LONG			(*MCIPROC16)(DWORD, HDRVR16, WORD, DWORD, DWORD);

#define WOW_TYPE_HANDLE     (WOW_TYPE_FULLHWND + 1)

/* HANDLE16 -> HANDLE conversions */
#define HDRVR_32(h16)		((HDRVR)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIDI_32(h16)		((HMIDI)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIDIIN_32(h16)		((HMIDIIN)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIDIOUT_32(h16)	((HMIDIOUT)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIDISTRM_32(h16)	((HMIDISTRM)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIXER_32(h16)		((HMIXER)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMIXEROBJ_32(h16)	((HMIXEROBJ)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HMMIO_32(h16)		((HMMIO)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HWAVE_32(h16)		((HWAVE)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HWAVEIN_32(h16)		((HWAVEIN)WOWHandle32(h16, WOW_TYPE_HANDLE))
#define HWAVEOUT_32(h16)	((HWAVEOUT)WOWHandle32(h16, WOW_TYPE_HANDLE))

/* HANDLE -> HANDLE16 conversions */
#define HDRVR_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIDI_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIDIIN_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIDIOUT_16(h32)	WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIDISTRM_16(h32)	WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIXER_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMIXEROBJ_16(h32)	WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HMMIO_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HWAVE_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HWAVEIN_16(h32)		WOWHandle16(h32, WOW_TYPE_HANDLE)
#define HWAVEOUT_16(h32)	WOWHandle16(h32, WOW_TYPE_HANDLE)

typedef enum {
    MMSYSTEM_MAP_NOMEM, 	/* ko, memory problem */
    MMSYSTEM_MAP_MSGERROR,      /* ko, unknown message */
    MMSYSTEM_MAP_OK, 	        /* ok, no memory allocated. to be sent to the proc. */
    MMSYSTEM_MAP_OKMEM, 	/* ok, some memory allocated, need to call UnMapMsg. to be sent to the proc. */
} MMSYSTEM_MapType;

extern  CRITICAL_SECTION        mmdrv_cs DECLSPEC_HIDDEN;

enum MMSYSTEM_DriverType
{
    MMSYSTDRV_MIXER,
    MMSYSTDRV_MIDIIN,
    MMSYSTDRV_MIDIOUT,
    MMSYSTDRV_WAVEIN,
    MMSYSTDRV_WAVEOUT,
    MMSYSTDRV_MAX
};

extern  struct mmsystdrv_thunk* MMSYSTDRV_AddThunk(DWORD callback, DWORD flags, enum MMSYSTEM_DriverType kind) DECLSPEC_HIDDEN;
extern  void                    MMSYSTDRV_DeleteThunk(struct mmsystdrv_thunk* thunk) DECLSPEC_HIDDEN;
extern  void                    MMSYSTDRV_SetHandle(struct mmsystdrv_thunk* thunk, void* h) DECLSPEC_HIDDEN;
extern  void                    MMSYSTDRV_CloseHandle(void* h) DECLSPEC_HIDDEN;
extern  DWORD                   MMSYSTDRV_Message(void* h, UINT msg, DWORD_PTR param1, DWORD_PTR param2) DECLSPEC_HIDDEN;

#define WINE_MMTHREAD_CREATED	0x4153494C	/* "BSIL" */
#define WINE_MMTHREAD_DELETED	0xDEADDEAD

typedef struct {
       DWORD			dwSignature;		/* 00 "BSIL" when ok, 0xDEADDEAD when being deleted */
       DWORD			dwCounter;		/* 04 > 1 when in mmThread functions */
       HANDLE			hThread;		/* 08 hThread */
       DWORD                    dwThreadID;     	/* 0C */
       DWORD    		fpThread;		/* 10 address of thread proc (segptr or lin depending on dwFlags) */
       DWORD			dwThreadPmt;    	/* 14 parameter to be passed upon thread creation to fpThread */
       LONG                     dwSignalCount;	     	/* 18 counter used for signaling */
       HANDLE                   hEvent;     		/* 1C event */
       HANDLE                   hVxD;		     	/* 20 return from OpenVxDHandle */
       DWORD                    dwStatus;       	/* 24 0x00, 0x10, 0x20, 0x30 */
       DWORD			dwFlags;		/* 28 dwFlags upon creation */
       UINT16			hTask;          	/* 2C handle to created task */
} WINE_MMTHREAD;
