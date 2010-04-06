/***************************************************************************
 *   Copyright (C) 2007 PCSX-df Team                                       *
 *   Copyright (C) 2009 Wei Mingzhi                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02111-1307 USA.            *
 ***************************************************************************/

#ifndef CDRISO_H
#define CDRISO_H

void imageReaderInit(void);

#ifdef GEKKO

long CALLBACK ISOinit(void);
long CALLBACK ISOshutdown(void);
long CALLBACK ISOopen(void);
long CALLBACK ISOclose(void);
long CALLBACK ISOgetTN(unsigned char *);
long CALLBACK ISOgetTD(unsigned char , unsigned char *);
long CALLBACK ISOreadTrack(unsigned char *);
unsigned char *CALLBACK ISOgetBuffer(void);
unsigned char *CALLBACK ISOgetBufferSub(void);
long CALLBACK ISOplay(unsigned char *time);
long CALLBACK ISOstop(void);

#endif

#endif
