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


};
