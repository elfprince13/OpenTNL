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

#include "tnlByteBuffer.h"
#include <mycrypt.h>

namespace TNL {

RefPtr<ByteBuffer> ByteBuffer::encodeBase64() const
{
   unsigned long outLen = ((getBufferSize() / 3) + 1) * 4 + 4 + 1;
   ByteBuffer *ret = new ByteBuffer(outLen);
   base64_encode(getBuffer(), getBufferSize(), ret->getBuffer(), &outLen);
   ret->resize(outLen+1);
   ret->getBuffer()[outLen] = 0;
   return ret;
}

RefPtr<ByteBuffer> ByteBuffer::decodeBase64() const
{
   unsigned long outLen = getBufferSize();
   ByteBuffer *ret = new ByteBuffer(outLen);
   base64_decode(getBuffer(), getBufferSize(), ret->getBuffer(), &outLen);
   ret->resize(outLen);
   return ret;
}

U32 ByteBuffer::calculateCRC(U32 start, U32 end, U32 crcVal) const
{
   static U32 crcTable[256];
   static bool crcTableValid = false;

   if(!crcTableValid)
   {
      U32 val;

      for(S32 i = 0; i < 256; i++)
      {
         val = i;
         for(S32 j = 0; j < 8; j++)
         {
            if(val & 0x01)
               val = 0xedb88320 ^ (val >> 1);
            else
               val = val >> 1;
         }
         crcTable[i] = val;
      }
      crcTableValid = true;
   }
   
   if(start >= mBufSize)
      return 0;
   if(end > mBufSize)
      end = mBufSize;

   // now calculate the crc
   const U8 * buf = getBuffer();
   for(U32 i = start; i < end; i++)
      crcVal = crcTable[(crcVal ^ buf[i]) & 0xff] ^ (crcVal >> 8);
   return(crcVal);
}

};
