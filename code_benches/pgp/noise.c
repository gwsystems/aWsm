/*
 * Get environmental noise.
 *
 * (c) Copyright 1990-1996 by Philip Zimmermann.  All rights reserved.
 * The author assumes no liability for damages resulting from the use
 * of this software, even if the damage results from defects in this
 * software.  No warranty is expressed or implied.
 *
 * Note that while most PGP source modules bear Philip Zimmermann's
 * copyright notice, many of them have been revised or entirely written
 * by contributors who frequently failed to put their names in their
 * code.  Code that has been incorporated into PGP from other authors
 * was either originally published in the public domain or is used with
 * permission from the various authors.
 *
 * PGP is available for free to the public under certain restrictions.
 * See the PGP User's Guide (included in the release package) for
 * important information about licensing, patent restrictions on
 * certain algorithms, trademarks, copyrights, and export controls.
 *
 * Written by Colin Plumb.
 */


#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>		/* For gettimeofday() */
#include <sys/times.h>		/* for times() */
#include <stdlib.h>		/* For qsort() */
#endif /* UNIX */

#include <time.h>
#include "usuals.h"
#include "randpool.h"
#include "noise.h"
#ifdef MACTC5
#include "TimeManager.h"
#endif

#ifdef AMIGA  /* RKNOP 940613 */
#include <devices/timer.h>
#include <hardware/custom.h>
#include <exec/execbase.h>
extern __far struct Custom custom;   /* Custom chips */
#include <proto/exec.h>
#include <proto/timer.h>

/* Stuff used in noise.c, defined in random.c -- RKNOP 940613*/
extern struct timerequest *TimerIO;
extern union { struct timeval t;
               struct EClockVal e;
             } time0,time1;
extern unsigned short use_eclock;
#endif  /* AMIGA */

/* Some machines just don't have clock_t */
#if defined(sun) && defined(i386)
typedef long clock_t;
#endif

#if defined(MSDOS) || defined(__MSDOS__)

/* Use IBM PC hardware timer (1.19 MHz) */

#ifdef __GO32__
#include <pc.h>
#else
#include <conio.h>		/* for inp() and outp() */
#include <dos.h>                /* same for Turbo C EWS */
#endif

/* timer0 on 8253-5 on IBM PC or AT tics every .84 usec. */
#define timer0		0x40	/* 8253 timer 0 port */
#define timercntl	0x43	/* 8253 control register */

/*
 * On an IBM PC, timer 0 ticks every .84 usec.  It counts down
 * from 65536 by twos, toggling its output line after each
 * step.  On an original IBM PC, we can thus only get 15 bits
 * of the timer.  On a PC-AT or later, with an 8284 timer chip,
 * we can get all 16 bits by reading the status, which has the
 * state of the output bit in bit 7, and is effectively the
 * high bit of the counter.
 *
 * But latching the status is a command which the 8283 does not
 * recognize; the subsequent load is interpreted as one of
 * a pair to read the counter instead of the status.  (We get a
 * garbage bit instead of the one we expected, but that's no worse
 * than constant 0.)  But the 8283 doesn't like single reads.
 * (The 8284 is more forgiving.)
 *
 * So, to resolve all this, the following sequence is used:
 *
 * - Dummy read from counter 0 (low byte)
 * - Latch status and count (ignored by 8283)
 * - Read status (high byte on 8283)
 * - Latch count (ignored by 8284, as count is already latched)
 * - Read count (low)
 * - Read count (high)
 *
 * It would be better (a project for the future) to capture the counter
 * in a keyboard ISR, put it in a global variable, and have noise() read
 * the global.  This gets the most accurate possible time, and avoids
 * possible harmonic relationships with a keyboard polling loop.
 * (Which MS-DOS, silly thing that it is, almost certainly uses
 * internally.)
 */
static unsigned pctimer0(void)
{
    unsigned count;

#ifdef __GO32__
    inportb(timer0);
    outportb(timercntl, 0xC2);        /* Latch status and count for timer 0 */
    count = (inportb(timer0) & 0x80) << 8;
    outportb(timercntl, 0x00);        /* Latch count of timer 0 */
    count |= (inportb(timer0) & 0xFF) >> 1;
    count |= (inportb(timer0) & 0xFF) << 7;
#else
    inp(timer0);
    outp(timercntl, 0xC2);	/* Latch status and count for timer 0 */
    count = (inp(timer0) & 0x80) << 8;
    outp(timercntl, 0x00);	/* Latch count of timer 0 */
    count |= (inp(timer0) & 0xFF) >> 1;
    count |= (inp(timer0) & 0xFF) << 7;
#endif

    return count;
}

#endif				/* MSDOS || __MSDOS__ */


#ifdef UNIX

#define NOISEDEBUG
#ifdef NOISEDEBUG
#include "pgp.h"		/* for verbose and pgpout */
#include <stdio.h>
#endif

/* Function needed for qsort() */
static int noiseCompare(void const *p1, void const *p2)
{
    return *(int const *) p1 - *(int const *) p2;
}


#define DELTAS 15		/* Number of deltas to try */

/*
 * Find the resolution of the gettimeofday() clock by sampling
 * successive values until a tick boundary, at which point
 * the delta is entered into a table.  The median of the table is
 * returned as the system tick size.
 *
 * Some trickery is needed to defeat the habit systems have of
 * always incrementing the microseconds field so that no two calls
 * return the same value.  Thus, a "tick boundary" is assumed
 * when successive calls return a difference of >2 us.
 * (This catches cases where we make successive calls and one
 * other task sneaks in between.  More tasks in between are
 * sufficiently unlikely that they'll get cut off by the median
 * filter.
 *
 * When a tick boundary is found, the *first* time read during
 * the previous tick (tv_base) is subtracted from the new time
 * to get the microseconds per tick.
 *
 * The median of the ticks is taken to eliminate outliers due to
 * descheduling (extra large) or tv_base not being the "zero" time
 * in a given tick (slightly small).
 *
 * Note that Suns have a 1 us timer, and in SunOS 4.1, they return
 * that timer, but there is ~50 us of system-call overhead to get
 * it, so this overestimates the tick size consdierably.  On
 * SunOS 5.x/Solaris, the overhead has been cut to about 2.5 us,
 * so the inter-call time alternates between 2 and 3 us.  Some
 * better algorithms are required to cope with potentially faster
 * machines that really do return 1 us granularity.
 *
 * Current best idea (unimplemented): Sample a large number, and
 * track small (< 100 us) deltas in an array of counters, and
 * large ones in an array of deltas.  There should be three
 * bumps: 1 us auto-increment, the tick size (which may blend into
 * the previous bump), and time-slicing.  We want to throw out
 * the latter, then compute the average delta as the average cost
 * of making a call, then throw out the small values if they
 * are suspisciously smaller than this value.  Then some average
 * of the remainder should provide a good value for the cost of
 * making a call.
 *
 * The alternative to all this is to actually model the keystroke
 * latencies and compute the entropy directly.  A model considering
 * the previous interval only should be adequate.
 */
static unsigned noiseTickSize(void)
{
    int i;
    int j;
    unsigned deltas[DELTAS];
    unsigned t;
    struct timeval tv_base, tv_old, tv_new;

    i = j = 0;
    gettimeofday(&tv_base, 0);
    tv_old = tv_base;
    do {
	gettimeofday(&tv_new, 0);
	if (tv_new.tv_usec > tv_old.tv_usec + 2) {
	    deltas[i++] = tv_new.tv_usec - tv_base.tv_usec +
		1000000 * (tv_new.tv_sec - tv_base.tv_sec);
	    tv_base = tv_new;
	    j = 0;
	}
	tv_old = tv_new;

	/*
	 * If we are forever getting <= 2 us, then just assume
	 * it's 2 us.
	 */
	if (j++ > 10000)
	    return 2;
    } while (i < DELTAS);

    qsort(deltas, DELTAS, sizeof(deltas[0]), noiseCompare);

    t = deltas[DELTAS / 2];	/* Median */

#ifdef NOISEDEBUG
    if (verbose)
	fprintf(pgpout, "t = %u, clock frequency is %u Hz\n",
		t, (2000000 + t) / (2 * t));
#endif

    return t;
}

#endif				/* UNIX */


/* (Written by Guy Geens 95/12/07)
This routine gets the 200Hz counter from the system area.
This part of memory is only accessible in supervisor mode.
To add to randomness, also store 50/60/70Hz VBL (Vertical BLank)
counter. (There are two flavours of this one: One counter is
stopped while floppy disk access takes place, the other one keeps
running. Which one to use? Both? No: the elapsed time would be
connected (does this harm randomness? I don't know) and, when
using a hard disk drive, the same!
(I've just picked one.)
*/

#ifdef ATARI
#ifdef __PUREC__
#include <tos.h>
#else
#include <osbind.h>
#endif
static word32 counter,vblcount;

long getcount(void) {
	counter= *((long*)0x4baL); /* _hz_200 */
	vblcount= *((long*)0x466L); /* _frlock */
	return counter;
}

#endif


/*
 * Add as much environmentally-derived random noise as possible
 * to the randPool.  Typically, this involves reading the most
 * accurate system clocks available.
 *
 * Returns the number of ticks that has passed since the last call,
 * for entropy estimation purposes.
 */
word32
noise(void)
{
    static word32 lastcounter;
    word32 delta;
    time_t tnow;
    clock_t cnow;

    cnow = clock();
    randPoolAddBytes((byte *) & cnow, sizeof(cnow));

    tnow = time((time_t *) 0);
    randPoolAddBytes((byte *) & tnow, sizeof(tnow));

#if defined(MSDOS) || defined(__MSDOS__)
    {
	unsigned t;

	t = pctimer0();
	randPoolAddBytes((byte *) & t, sizeof(t));
	delta = t - (unsigned) lastcounter;
	lastcounter = t;
    }
#endif

#ifdef MACTC5
	{
		unsigned long t;
		
		delta = TMTicks();
		t=lastcounter+=delta;
		randPoolAddBytes((byte *)&t, sizeof(t));
	}
#endif

#ifdef WIN32
/* Win32 provides QueryPerformanceCounter(), which does precisely what we need here */
    {
        /* What am I doing here ? : We can't #include <windows.h> to get the prototype
           for QueryPerformanceCounter() because there are many namespace clashes
           between PGP and windows.h. So, we hack in the prototype inline. When we get
           a compiler which does namespaces, or someone removes all the clashes in PGP,
           this will go.
        */
#if defined(_MSC_VER)  /* only valid if we're using the Microsoft compiler */
	__declspec(dllimport) long __stdcall
            QueryPerformanceCounter(__int64 *lpPerformanceCount);
	unsigned t;
	__int64 perf_count;

	QueryPerformanceCounter(&perf_count);
	/* it doesn't matter if the return value is zero */
	t = (unsigned) perf_count;
	randPoolAddBytes((byte *) & t, sizeof(t));
	delta = t - (unsigned) lastcounter;
	lastcounter = t;
#else  /* Not Microsoft compiler */
#include "This compiler is not supported, modify the code above accordingly"
#endif /* _MSC_VER */
    }
#endif /* WIN32 */

#ifdef VMS
    {
	word32 t;
	/* VMS Hardware Clock */
	extern unsigned long vms_clock_bits[2];
	/* Clock update int. */
	extern const long vms_ticks_per_update;

	/* Capture fast system timer: */
	SYS$GETTIM(vms_clock_bits);
	randPoolAddBytes((byte *) & vms_clock_bits, sizeof(vms_clock_bits));
	t = vms_clock_bits[0] / vms_ticks_per_update;
	delta = t - lastcounter;
	lastcounter = t;
    }
#endif				/* VMS */

#ifdef UNIX
    /* Get noise from gettimeofday() */
    {
	struct timeval tv;
	word32 t;
	static unsigned ticksize = 0;

	if (!ticksize)
	    ticksize = noiseTickSize();

	gettimeofday(&tv, NULL);
	randPoolAddBytes((byte *) & tv, sizeof(tv));

	/* This may wrap, but it's unsigned, so that's okay */
	t = tv.tv_sec * 1000000 + tv.tv_usec;
	delta = t - lastcounter;
	lastcounter = t;

	delta /= ticksize;
    }
    /* Get noise from times() */
    {
	clock_t t;
	struct tms tms;

	t = times(&tms);
	randPoolAddBytes((byte *) & tms, sizeof(tms));
	randPoolAddBytes((byte *) & t, sizeof(t));
    }
#endif				/* UNIX */

#ifdef AMIGA     /* Whole next section added RKNOP 940613 */
#define AMIGA_DELTAS 15

   {
      static unsigned long ticksize = 0;
      int i=0,j;
      unsigned long deltas[AMIGA_DELTAS],swap;


      /* NOTE -- this next section (reading the Eclock or the
         microHz clock) will only happen within the do loop in
         trueRandAccum()!! */

      if (TimerIO && TimerBase)
      {  if (!ticksize)   /* Get tick size, similar to Unix */
         {  Forbid();    /* Turn off multitasking to get ticksize */
            if (use_eclock)
              ReadEClock(&time0.e);
            else
              am_GetSysTime(&time0.t);
            do
            {   if (use_eclock)
          {  ReadEClock(&time1.e);
             if (time1.e.ev_lo>time0.e.ev_lo)
               deltas[i++]=time1.e.ev_lo-time0.e.ev_lo;
             time0.e.ev_lo=time1.e.ev_lo;
             time0.e.ev_hi=time1.e.ev_hi;
          }
          else
          {  am_GetSysTime(&time1.t);
             if (CmpTime(&time0.t,&time1.t))
                deltas[i++]=1000000*(time1.t.tv_secs
                      -time0.t.tv_secs)
                  +(time1.t.tv_micro-time0.t.tv_micro);
             time0.t.tv_secs=time1.t.tv_secs;
             time0.t.tv_micro=time1.t.tv_micro;
          }
            } while (i<AMIGA_DELTAS);
            for (i=0;i<AMIGA_DELTAS-1;i++)
         for (j=i+1;j<AMIGA_DELTAS;j++)
            if (deltas[j]<deltas[i])
            {  swap=deltas[j];
               deltas[j]=deltas[i];
               deltas[i]=swap;
            }
            if ((ticksize=deltas[AMIGA_DELTAS/2])==0) ticksize=1;
            Permit();
         }

         if (use_eclock)
         {  ReadEClock(&time1.e);
            randPoolAddBytes((byte *)&time1.e.ev_lo,4);
            delta=time1.e.ev_lo-time0.e.ev_lo;  /* wrap ok?, unsigned */
            time0.e.ev_hi=time1.e.ev_hi;
            time0.e.ev_lo=time1.e.ev_lo;
         }
         else
         {  am_GetSysTime(&time1.t);
            randPoolAddBytes((byte *)&time1.t,sizeof(time1.t));
            delta=1000000*(time1.t.tv_secs - time0.t.tv_secs) +
             time1.t.tv_micro - time0.t.tv_micro;
            time0.t.tv_secs=time1.t.tv_secs;
         time0.t.tv_micro=time1.t.tv_micro;
         }
         delta/=ticksize;
      }

      /* Get some additional noise from the video beam poisition */
      randPoolAddBytes((byte *)&custom.vhposr,2);

      /* Pull the ExecBase dispatch count */
      randPoolAddBytes((byte *)
             &((*(struct ExecBase **)4)->DispCount),4);
   }
#endif /* AMIGA (RKNOP 940613) */

#ifdef ATARI	/* Section written by Guy Geens <guy.geens@elis.rug.ac.be> 951207 */
	Supexec(getcount);	/* Xbios 38 */
	delta=counter-lastcounter;
	lastcounter=counter;

#ifndef __PUREC__
	randPoolAddBytes((byte*)&counter,4);
#endif
	/* Under Pure C, counter is the same as cnow (returned by clock()),
	so it doesn't add to randomness.
	I don't have any other C compiler, so I can't check whether they
	also use the same counter. Please mail me further details */

 	randPoolAddBytes((byte*)&vblcount,4);
	/* Other compilers might require to comment this out and activate
	the previous line. */
#endif		/* End of section written by Guy Geens */

    return delta;
}
