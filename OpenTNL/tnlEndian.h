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
	
template<typename T> inline T convertHostToLEndian(T i){ return i; }
template<typename T> inline T convertLEndianToHost(T i){ return i; }
template<typename T> inline T convertHostToBEndian(T i){ return endianSwap(i); }
template<typename T> inline T convertBEndianToHost(T i){ return endianSwap(i); }

#elif defined(TNL_BIG_ENDIAN)

template<typename T> inline T convertHostToLEndian(T i){ return endianSwap(i); }
template<typename T> inline T convertLEndianToHost(T i){ return endianSwap(i); }
template<typename T> inline T convertHostToBEndian(T i){ return i; }
template<typename T> inline T convertBEndianToHost(T i){ return i; }

#else
#error "Endian define not set!"
#endif


};

#endif
