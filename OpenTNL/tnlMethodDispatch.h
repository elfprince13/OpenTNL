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

#ifndef _TNL_METHODDISPATCH_H_
#define _TNL_METHODDISPATCH_H_

#include <tuple>
#include <utility>

#ifndef _TNL_TYPES_H_
#include "tnlTypes.h"
#endif

#ifndef _TNL_VECTOR_H_
#include "tnlVector.h"
#endif

#ifndef _TNL_BITSTREAM_H_
#include "tnlBitStream.h"
#endif

#ifndef _TNL_NETSTRINGTABLE_H_
#include "tnlNetStringTable.h"
#endif

#ifndef _TNL_STRING_H_
#include "tnlString.h"
#endif

namespace Types
{
   enum {
      VectorSizeBitSize = 8,
      ByteBufferSizeBitSize = 10,
   };

   /// Reads a string from a BitStream.
   extern void read(TNL::BitStream &s, TNL::StringPtr *val);
   /// Rrites a string into a BitStream.
   extern void write(TNL::BitStream &s, TNL::StringPtr &val);
   /// Reads a ByteBuffer from a BitStream.
   extern void read(TNL::BitStream &s, TNL::ByteBufferPtr *val);
   /// Writes a ByteBuffer into a BitStream.
   extern void write(TNL::BitStream &s, TNL::ByteBufferPtr &val);
   /// Reads an IP address from a BitStream.
   extern void read(TNL::BitStream &s, TNL::IPAddress *val);
   /// Writes an IP address into a BitStream.
   extern void write(TNL::BitStream &s, TNL::IPAddress &val);

   /// Reads a StringTableEntry from a BitStream.
   inline void read(TNL::BitStream &s, TNL::StringTableEntry *val)
   {
      s.readStringTableEntry(val);
   }
   /// Writes a StringTableEntry into a BitStream.
   inline void write(TNL::BitStream &s, TNL::StringTableEntry &val)
   {
      s.writeStringTableEntry(val);
   }

   /// Reads a generic object from a BitStream.  This can be used for any
   /// type supported by BitStream::read.
   template <typename T> inline void read(TNL::BitStream &s, T *val)
   { 
      s.read(val);
   }
   /// Writes a generic object into a BitStream.  This can be used for any
   /// type supported by BitStream::write.
   template <typename T> inline void write(TNL::BitStream &s, T &val)
   { 
      s.write(val);
   }
   /// Reads a Vector of objects from a BitStream.
   template <typename T> inline void read(TNL::BitStream &s, TNL::Vector<T> *val)
   {
      TNL::U32 size = s.readInt(VectorSizeBitSize);
      val->setSize(size);
      for(TNL::S32 i = 0; i < val->size(); i++)
         read(s, &((*val)[i]));
   }
   /// Writes a Vector of objects into a BitStream.
   template <typename T> void write(TNL::BitStream &s, TNL::Vector<T> &val)
   {
      s.writeInt(val.size(), VectorSizeBitSize);
      for(TNL::S32 i = 0; i < val.size(); i++)
         write(s, val[i]);
   }
   /// Reads a bit-compressed integer from a BitStream.
   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::Int<BitCount> *val)
   {
      val->value = s.readInt(BitCount);
   }
   /// Writes a bit-compressed integer into a BitStream.
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::Int<BitCount> &val)
   {
      s.writeInt(val.value, BitCount);
   }

   /// Reads a bit-compressed signed integer from a BitStream.
   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::SignedInt<BitCount> *val)
   {
      val->value = s.readSignedInt(BitCount);
   }
   /// Writes a bit-compressed signed integer into a BitStream.
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::SignedInt<BitCount> &val)
   {
      s.writeSignedInt(val.value, BitCount);
   }

   /// Reads a bit-compressed RangedU32 from a BitStream.
   template <TNL::U32 MinValue, TNL::U32 MaxValue> inline void read(TNL::BitStream &s, TNL::RangedU32<MinValue,MaxValue> *val)
   {
      val->value = s.readRangedU32(MinValue, MaxValue);
   }

   /// Writes a bit-compressed RangedU32 into a BitStream.
   template <TNL::U32 MinValue, TNL::U32 MaxValue> inline void write(TNL::BitStream &s,TNL::RangedU32<MinValue,MaxValue> &val)
   {
      s.writeRangedU32(val.value, MinValue, MaxValue);
   }

   /// Reads a bit-compressed SignedFloat (-1 to 1) from a BitStream.
   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::Float<BitCount> *val)
   {
      val->value = s.readFloat(BitCount);
   }
   /// Writes a bit-compressed SignedFloat (-1 to 1) into a BitStream.
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::Float<BitCount> &val)
   {
      s.writeFloat(val.value, BitCount);
   }
   /// Reads a bit-compressed Float (0 to 1) from a BitStream.
   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::SignedFloat<BitCount> *val)
   {
      val->value = s.readSignedFloat(BitCount);
   }
   /// Writes a bit-compressed Float (0 to 1) into a BitStream.
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::SignedFloat<BitCount> &val)
   {
      s.writeSignedFloat(val.value, BitCount);
   }
};

namespace TNL {

/// Base class for FunctorDecl template classes.  The Functor objects
/// store the parameters and member function pointer for the invocation
/// of some class member function.  Functor is used in TNL by the
/// RPC mechanism, the journaling system and the ThreadQueue to store
/// a function for later transmission and dispatch, either to a remote
/// host, a journal file, or another thread in the process.
struct Functor {
   /// Construct the Functor.
   Functor() {}
   /// Destruct the Functor.
   virtual ~Functor() {}
   /// Reads this Functor from a BitStream.
   virtual void read(BitStream &stream) = 0;
   /// Writes this Functor to a BitStream.
   virtual void write(BitStream &stream) = 0;
   /// Dispatch the function represented by the Functor.
   virtual void dispatch(void *t) = 0;
};

/// FunctorDecl template class.  This class is specialized based on the
/// member function call signature of the method it represents.  Other
/// specializations hold specific member function pointers and slots
/// for each of the function arguments.
template <class T> 
struct FunctorDecl : public Functor {
   FunctorDecl() {}
   void set() {}
   void read(BitStream &stream) {}
   void write(BitStream &stream) {}
   void dispatch(void *t) { }
};
template <class T, typename ...ArgTs>
struct FunctorDecl<void (T::*)(ArgTs... args)> : public Functor {
   typedef void (T::*FuncPtr)(ArgTs... args);
   FuncPtr ptr;
	std::tuple<ArgTs ...> argT;
   FunctorDecl(FuncPtr p) : ptr(p) {}
	void set(ArgTs &...args) { argT = std::make_tuple(args ...);}
   void read(BitStream &stream) {}
   void write(BitStream &stream) {}
	void dispatch(void *t) { dispatchHelper((T*)t, typename gens<sizeof...(ArgTs)>::type()); }
private:
	template<size_t ...> struct seq {};
	
	template<size_t N, size_t ...S> struct gens : gens<N-1, N-1, S...> {};
	
	template<size_t ...I> struct gens<0, I...>{ typedef seq<I...> type; };
	
	template<size_t ...I>
	void dispatchHelper(T* t, seq<I...>)  {
		(t->*ptr)(std::get<I>(argT) ...);
	}
};

};

#endif

