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

#ifndef _TNL_ENDIAN_H_
#define _TNL_ENDIAN_H_

namespace TNL {

/**
   Convert the byte ordering on the U16 to and from big/little endian format.
   @param in_swap Any U16
   @returns swapped U16.
 */
inline U16 endianSwap(const U16 in_swap)
{
   return U16(((in_swap >> 8) & 0x00ff) |
              ((in_swap << 8) & 0xff00));
}

/**
   Convert the byte ordering on the U32 to and from big/little endian format.
   @param in_swap Any U32
   @returns swapped U32.
 */
inline U32 endianSwap(const U32 in_swap)
{
   return U32(((in_swap >> 24) & 0x000000ff) |
              ((in_swap >>  8) & 0x0000ff00) |
              ((in_swap <<  8) & 0x00ff0000) |
              ((in_swap << 24) & 0xff000000));
}

//------------------------------------------------------------------------------
// Endian conversions
#ifdef TNL_LITTLE_ENDIAN

inline U8  convertHostToLEndian(U8 i)  { return i; }
inline U8  convertLEndianToHost(U8 i)  { return i; }
inline U16 convertHostToLEndian(U16 i) { return i; }
inline U16 convertLEndianToHost(U16 i) { return i; }
inline U32 convertHostToLEndian(U32 i) { return i; }
inline U32 convertLEndianToHost(U32 i) { return i; }
inline S8  convertHostToLEndian(S8 i)  { return i; }
inline S8  convertLEndianToHost(S8 i)  { return i; }
inline S16 convertHostToLEndian(S16 i) { return i; }
inline S16 convertLEndianToHost(S16 i) { return i; }
inline S32 convertHostToLEndian(S32 i) { return i; }
inline S32 convertLEndianToHost(S32 i) { return i; }

inline F32 convertHostToLEndian(F32 i) { return i; }
inline F32 convertLEndianToHost(F32 i) { return i; }
inline F64 convertHostToLEndian(F64 i) { return i; }
inline F64 convertLEndianToHost(F64 i) { return i; }

inline U8  convertHostToBEndian(U8 i)  { return i; }
inline U8  convertBEndianToHost(U8 i)  { return i; }
inline S8  convertHostToBEndian(S8 i)  { return i; }
inline S8  convertBEndianToHost(S8 i)  { return i; }

inline U16 convertHostToBEndian(U16 i)
{
   return U16((i << 8) | (i >> 8));
}

inline U16 convertBEndianToHost(U16 i)
{
   return U16((i << 8) | (i >> 8));
}

inline S16 convertHostToBEndian(S16 i)
{
   return S16(convertHostToBEndian(U16(i)));
}

inline S16 convertBEndianToHost(S16 i)
{
   return S16(convertBEndianToHost(U16(i)));
}

inline U32 convertHostToBEndian(U32 i)
{
   return ((i << 24) & 0xff000000) |
          ((i <<  8) & 0x00ff0000) |
          ((i >>  8) & 0x0000ff00) |
          ((i >> 24) & 0x000000ff);
}

inline U32 convertBEndianToHost(U32 i)
{
   return ((i << 24) & 0xff000000) |
          ((i <<  8) & 0x00ff0000) |
          ((i >>  8) & 0x0000ff00) |
          ((i >> 24) & 0x000000ff);
}

inline S32 convertHostToBEndian(S32 i)
{
   return S32(convertHostToBEndian(U32(i)));
}

inline S32 convertBEndianToHost(S32 i)
{
   return S32(convertBEndianToHost(S32(i)));
}


#elif defined(TNL_BIG_ENDIAN)

inline U8  convertHostToBEndian(U8 i)  { return i; }
inline U8  convertBEndianToHost(U8 i)  { return i; }
inline S8  convertHostToBEndian(S8 i)  { return i; }
inline S8  convertBEndianToHost(S8 i)  { return i; }
inline U16 convertHostToBEndian(U16 i) { return i; }
inline U16 convertBEndianToHost(U16 i) { return i; }
inline U32 convertHostToBEndian(U32 i) { return i; }
inline U32 convertBEndianToHost(U32 i) { return i; }
inline S16 convertHostToBEndian(S16 i) { return i; }
inline S16 convertBEndianToHost(S16 i) { return i; }
inline S32 convertHostToBEndian(S32 i) { return i; }
inline S32 convertBEndianToHost(S32 i) { return i; }

inline U8  convertHostToLEndian(U8 i)  { return i; }
inline U8  convertLEndianToHost(U8 i)  { return i; }
inline S8  convertHostToLEndian(S8 i)  { return i; }
inline S8  convertLEndianToHost(S8 i)  { return i; }

inline U16 convertHostToLEndian(U16 i)
{
   return (i << 8) | (i >> 8);
}
inline U16 convertLEndianToHost(U16 i)
{
   return (i << 8) | (i >> 8);
}
inline U32 convertHostToLEndian(U32 i)
{
   return ((i << 24) & 0xff000000) |
          ((i <<  8) & 0x00ff0000) |
          ((i >>  8) & 0x0000ff00) |
          ((i >> 24) & 0x000000ff);
}
inline U32 convertLEndianToHost(U32 i)
{
   return ((i << 24) & 0xff000000) |
          ((i <<  8) & 0x00ff0000) |
          ((i >>  8) & 0x0000ff00) |
          ((i >> 24) & 0x000000ff);
}


inline F32 convertHostToLEndian(F32 i)
{
   U32 result = convertHostToLEndian( *reinterpret_cast<U32*>(&i) );
   return *reinterpret_cast<F32*>(&result);
}

inline F32 convertLEndianToHost(F32 i)
{
   U32 result = convertLEndianToHost( *reinterpret_cast<U32*>(&i) );
   return *reinterpret_cast<F32*>(&result);
}

inline F64 convertHostToLEndian(F64 i)
{
   F64 ret;
   reinterpret_cast<U32 *>(&ret)[0] = 
      convertLEndianToHost( reinterpret_cast<U32*>(&i)[1] );
   reinterpret_cast<U32 *>(&ret)[1] = 
      convertLEndianToHost( reinterpret_cast<U32*>(&i)[0] );
   return ret;
}

inline F64 convertLEndianToHost(F64 i)
{
   F64 ret;
   reinterpret_cast<U32 *>(&ret)[0] = 
      convertLEndianToHost( reinterpret_cast<U32*>(&i)[1] );
   reinterpret_cast<U32 *>(&ret)[1] = 
      convertLEndianToHost( reinterpret_cast<U32*>(&i)[0] );
   return ret;
}

inline S16 convertHostToLEndian(S16 i) { return S16(convertHostToLEndian(U16(i))); }
inline S16 convertLEndianToHost(S16 i) { return S16(convertLEndianToHost(U16(i))); }
inline S32 convertHostToLEndian(S32 i) { return S32(convertHostToLEndian(U32(i))); }
inline S32 convertLEndianToHost(S32 i) { return S32(convertLEndianToHost(U32(i))); }

#else
#error "Endian define not set!"
#endif


};

#endif
