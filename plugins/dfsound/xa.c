/***************************************************************************
                            xa.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

#include "stdafx.h"
#define _IN_XA
#include <stdint.h>

// will be included from spu.c
#ifdef _IN_SPU

////////////////////////////////////////////////////////////////////////
// XA GLOBALS
////////////////////////////////////////////////////////////////////////

xa_decode_t   * xapGlobal=0;

uint32_t * XAFeed  = NULL;
uint32_t * XAPlay  = NULL;
uint32_t * XAStart = NULL;
uint32_t * XAEnd   = NULL;

uint32_t   XARepeat  = 0;
uint32_t   XALastVal = 0;

uint32_t * CDDAFeed  = NULL;
uint32_t * CDDAPlay  = NULL;
uint32_t * CDDAStart = NULL;
uint32_t * CDDAEnd   = NULL;

int             iLeftXAVol  = 32767;
int             iRightXAVol = 32767;

static int gauss_ptr = 0;
static int gauss_window[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define gvall0 gauss_window[gauss_ptr]
#define gvall(x) gauss_window[(gauss_ptr+x)&3]
#define gvalr0 gauss_window[4+gauss_ptr]
#define gvalr(x) gauss_window[4+((gauss_ptr+x)&3)]

////////////////////////////////////////////////////////////////////////
// MIX XA & CDDA
////////////////////////////////////////////////////////////////////////

INLINE void MixXA(void)
{
 int ns;
 uint32_t l;

 for(ns=0;ns<NSSIZE && XAPlay!=XAFeed;ns++)
  {
   XALastVal=*XAPlay++;
   if(XAPlay==XAEnd) XAPlay=XAStart;
#ifdef XA_HACK
   SSumL[ns]+=(((short)(XALastVal&0xffff))       * iLeftXAVol)/32768;
   SSumR[ns]+=(((short)((XALastVal>>16)&0xffff)) * iRightXAVol)/32768;
#else
   SSumL[ns]+=(((short)(XALastVal&0xffff))       * iLeftXAVol)/32767;
   SSumR[ns]+=(((short)((XALastVal>>16)&0xffff)) * iRightXAVol)/32767;
#endif
  }

 if(XAPlay==XAFeed && XARepeat)
  {
   XARepeat--;
   for(;ns<NSSIZE;ns++)
    {
#ifdef XA_HACK
     SSumL[ns]+=(((short)(XALastVal&0xffff))       * iLeftXAVol)/32768;
     SSumR[ns]+=(((short)((XALastVal>>16)&0xffff)) * iRightXAVol)/32768;
#else
     SSumL[ns]+=(((short)(XALastVal&0xffff))       * iLeftXAVol)/32767;
     SSumR[ns]+=(((short)((XALastVal>>16)&0xffff)) * iRightXAVol)/32767;
#endif
    }
  }

 for(ns=0;ns<NSSIZE && CDDAPlay!=CDDAFeed && (CDDAPlay!=CDDAEnd-1||CDDAFeed!=CDDAStart);ns++)
  {
   l=*CDDAPlay++;
   if(CDDAPlay==CDDAEnd) CDDAPlay=CDDAStart;
   SSumL[ns]+=(((short)(l&0xffff))       * iLeftXAVol)/32767;
   SSumR[ns]+=(((short)((l>>16)&0xffff)) * iRightXAVol)/32767;
  }
}

////////////////////////////////////////////////////////////////////////
// small linux time helper... only used for watchdog
////////////////////////////////////////////////////////////////////////

unsigned long timeGetTime_spu()
{
 struct timeval tv;
 gettimeofday(&tv, 0);                                 // well, maybe there are better ways
 return tv.tv_sec * 1000 + tv.tv_usec/1000;            // to do that, but at least it works
}

////////////////////////////////////////////////////////////////////////
// FEED XA 
////////////////////////////////////////////////////////////////////////

INLINE void FeedXA(xa_decode_t *xap)
{
 int sinc,spos,i,iSize,iPlace,vl,vr;

 if(!bSPUIsOpen) return;

 xapGlobal = xap;                                      // store info for save states
 XARepeat  = 100;                                      // set up repeat

#ifdef XA_HACK
 iSize=((45500*xap->nsamples)/xap->freq);              // get size
#else
 iSize=((44100*xap->nsamples)/xap->freq);              // get size
#endif
 if(!iSize) return;                                    // none? bye

 if(XAFeed<XAPlay) iPlace=XAPlay-XAFeed;               // how much space in my buf?
 else              iPlace=(XAEnd-XAFeed) + (XAPlay-XAStart);

 if(iPlace==0) return;                                 // no place at all

 //----------------------------------------------------//
 if(iXAPitch)                                          // pitch change option?
  {
   static DWORD dwLT=0;
   static DWORD dwFPS=0;
   static int   iFPSCnt=0;
   static int   iLastSize=0;
   static DWORD dwL1=0;
   DWORD dw=timeGetTime_spu(),dw1,dw2;

   iPlace=iSize;

   dwFPS+=dw-dwLT;iFPSCnt++;

   dwLT=dw;
                                       
   if(iFPSCnt>=10)
    {
     if(!dwFPS) dwFPS=1;
     dw1=1000000/dwFPS; 
     if(dw1>=(dwL1-100) && dw1<=(dwL1+100)) dw1=dwL1;
     else dwL1=dw1;
     dw2=(xap->freq*100/xap->nsamples);
     if((!dw1)||((dw2+100)>=dw1)) iLastSize=0;
     else
      {
       iLastSize=iSize*dw2/dw1;
       if(iLastSize>iPlace) iLastSize=iPlace;
       iSize=iLastSize;
      }
     iFPSCnt=0;dwFPS=0;
    }
   else
    {
     if(iLastSize) iSize=iLastSize;
    }
  }
 //----------------------------------------------------//

 spos=0x10000L;
 sinc = (xap->nsamples << 16) / iSize;                 // calc freq by num / size

 if(xap->stereo)
{
   uint32_t * pS=(uint32_t *)xap->pcm;
   uint32_t l=0;

   if(iXAPitch)
    {
     int32_t l1,l2;short s;
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           gauss_window[gauss_ptr] = (short)LOWORD(l);
           gauss_window[4+gauss_ptr] = (short)HIWORD(l);
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l= (vr >> 11) & 0xffff;
         vr=(gauss[vl]*gvalr0)&~2047;
         vr+=(gauss[vl+1]*gvalr(1))&~2047;
         vr+=(gauss[vl+2]*gvalr(2))&~2047;
         vr+=(gauss[vl+3]*gvalr(3))&~2047;
         l |= vr << 5;
        }
       else
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           spos -= 0x10000L;
          }
        }

       s=(short)LOWORD(l);
       l1=s;
       l1=(l1*iPlace)/iSize;
       if(l1<-32767) l1=-32767;
       if(l1> 32767) l1=32767;
       s=(short)HIWORD(l);
       l2=s;
       l2=(l2*iPlace)/iSize;
       if(l2<-32767) l2=-32767;
       if(l2> 32767) l2=32767;
       l=(l1&0xffff)|(l2<<16);

       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
   else
    {
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           gauss_window[gauss_ptr] = (short)LOWORD(l);
           gauss_window[4+gauss_ptr] = (short)HIWORD(l);
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l= (vr >> 11) & 0xffff;
         vr=(gauss[vl]*gvalr0)&~2047;
         vr+=(gauss[vl+1]*gvalr(1))&~2047;
         vr+=(gauss[vl+2]*gvalr(2))&~2047;
         vr+=(gauss[vl+3]*gvalr(3))&~2047;
         l |= vr << 5;
        }
       else
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           spos -= 0x10000L;
          }
        }

       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
  }
 else
  {
   unsigned short * pS=(unsigned short *)xap->pcm;
   uint32_t l;short s=0;

   if(iXAPitch)
    {
     int32_t l1;
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           gauss_window[gauss_ptr] = (short)*pS++;
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l1=s= vr >> 11;
         l1 &= 0xffff;
        }
       else
        {
         while(spos>=0x10000L)
          {
           s = *pS++;
           spos -= 0x10000L;
          }
         l1=s;
        }

       l1=(l1*iPlace)/iSize;
       if(l1<-32767) l1=-32767;
       if(l1> 32767) l1=32767;
       l=(l1&0xffff)|(l1<<16);
       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
   else
    {
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           gauss_window[gauss_ptr] = (short)*pS++;
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l=s= vr >> 11;
         l &= 0xffff;
        }
       else
        {
         while(spos>=0x10000L)
          {
           s = *pS++;
           spos -= 0x10000L;
          }
         l=s;
        }

       *XAFeed++=(l|(l<<16));

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// FEED CDDA
////////////////////////////////////////////////////////////////////////

INLINE void FeedCDDA(unsigned char *pcm, int nBytes)
{
 while(nBytes>0)
  {
   if(CDDAFeed==CDDAEnd) CDDAFeed=CDDAStart;
   while(CDDAFeed==CDDAPlay-1||
         (CDDAFeed==CDDAEnd-1&&CDDAPlay==CDDAStart))
   {
    if (!iUseTimer) usleep(1000);
    else return;
   }
   *CDDAFeed++=(*pcm | (*(pcm+1)<<8) | (*(pcm+2)<<16) | (*(pcm+3)<<24));
   nBytes-=4;
   pcm+=4;
  }
}

#endif
