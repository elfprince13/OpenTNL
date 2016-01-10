//-----------------------------------------------------------------------------------
//
//   Torque Network Library
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#ifndef _TNL_NONCE_H_
#define _TNL_NONCE_H_

#ifndef _TNL_BITSTREAM_H_
#include "tnlBitStream.h"
#endif

#ifndef _TNL_RANDOM_H_
#include "tnlRandom.h"
#endif

namespace TNL
{

struct Nonce
{
   enum {
      NonceSize = 8,
   };
   U8 data[NonceSize];

   Nonce() {}
   Nonce(const U8 *ptr) { memcpy(data, ptr, NonceSize); }

   bool operator==(const Nonce &theOtherNonce) const { return !memcmp(data, theOtherNonce.data, NonceSize); }
   bool operator!=(const Nonce &theOtherNonce) const { return memcmp(data, theOtherNonce.data, NonceSize) != 0; }

   void operator=(const Nonce &theNonce) { memcpy(data, theNonce.data, NonceSize); }
   
   void read(BitStream *stream) { stream->read(NonceSize, data); }
   void write(BitStream *stream) const { stream->write(NonceSize, data); }
   void getRandom() { Random::read(data, NonceSize); }
};

};

#endif
