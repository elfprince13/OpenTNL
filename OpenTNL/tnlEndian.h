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

inline U8 endianSwap(const U8 in_swap)
{
   return in_swap;
}

inline S8 endianSwap(const S8 in_swap)
{
   return in_swap;
}

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

inline S16 endianSwap(const S16 in_swap)
{
   return S16(endianSwap(U16(in_swap)));
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

inline S32 endianSwap(const S32 in_swap)
{
   return S32(endianSwap(U32(in_swap)));
}

inline U64 endianSwap(const U64 in_swap)
{
   U32 *inp = (U32 *) &in_swap;
   U64 ret;
   U32 *outp = (U32 *) &ret;
   outp[0] = endianSwap(inp[1]);
   outp[1] = endianSwap(inp[0]);
   return ret;
}

inline S64 endianSwap(const S64 in_swap)
{
   return S64(endianSwap(U64(in_swap)));
}

inline F32 endianSwap(const F32 in_swap)
{
   U32 result = endianSwap(* ((U32 *) &in_swap) );
   return * ((F32 *) &result);
}

inline F64 endianSwap(const F64 in_swap)
{
   U64 result = endianSwap(* ((U64 *) &in_swap) );
   return * ((F64 *) &result);
}

//------------------------------------------------------------------------------
// Endian conversions
#ifdef TNL_LITTLE_ENDIAN

#define TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(type) \
   inline type convertHostToLEndian(type i) { return i; } \
   inline type convertLEndianToHost(type i) { return i; } \
   inline type convertHostToBEndian(type i) { return endianSwap(i); } \
   inline type convertBEndianToHost(type i) { return endianSwap(i); }

#elif defined(TNL_BIG_ENDIAN)

#define TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(type) \
   inline type convertHostToLEndian(type i) { return endianSwap(i); } \
   inline type convertLEndianToHost(type i) { return endianSwap(i); } \
   inline type convertHostToBEndian(type i) { return i; } \
   inline type convertBEndianToHost(type i) { return i; }

#else
#error "Endian define not set!"
#endif


TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(U8)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(S8)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(U16)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(S16)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(U32)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(S32)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(U64)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(S64)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(F32)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(F64)

};

#endif
