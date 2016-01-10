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
	
template <typename T>
inline	T endianSwap(T u)
	{
		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;
		source.u = u;
		for(size_t k = 0; k < sizeof(T); k++)
			dest.u8[k] = source.u8[sizeof(T) - k -1];
		return dest.u;
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
	TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(size_t)
	TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(ssize_t)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(U64)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(S64)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(F32)
TNL_DECLARE_TEMPLATIZED_ENDIAN_CONV(F64)

};

#endif
