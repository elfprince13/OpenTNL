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
   virtual void dispatch(Object *t) = 0;
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
   void dispatch(Object *t) { }
};
template <class T> 
struct FunctorDecl<void (T::*)()> : public Functor {
   typedef void (T::*FuncPtr)();
   FuncPtr ptr;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set() {}
   void read(BitStream &stream) {}
   void write(BitStream &stream) {}
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(); }
};
template <class T, class A> 
struct FunctorDecl<void (T::*)(A)> : public Functor {
   typedef void (T::*FuncPtr)(A);
   FuncPtr ptr; A a;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a) { a = _a; }
   void read(BitStream &stream) { Types::read(stream, &a); }
   void write(BitStream &stream) { Types::write(stream, a); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a); }
};
template <class T, class A, class B>
struct FunctorDecl<void (T::*)(A,B)>: public Functor {
   typedef void (T::*FuncPtr)(A,B);
   FuncPtr ptr; A a; B b;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b) { a = _a; b = _b;}
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b); }
};

template <class T, class A, class B, class C>
struct FunctorDecl<void (T::*)(A,B,C)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C);
   FuncPtr ptr; A a; B b; C c;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c) { a = _a; b = _b; c = _c;}
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c); }
};

template <class T, class A, class B, class C, class D>
struct FunctorDecl<void (T::*)(A,B,C,D)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D);
   FuncPtr ptr; A a; B b; C c; D d;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d) { a = _a; b = _b; c = _c; d = _d; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d); }
};

template <class T, class A, class B, class C, class D, class E>
struct FunctorDecl<void (T::*)(A,B,C,D,E)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E);
   FuncPtr ptr; A a; B b; C c; D d; E e;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e) { a = _a; b = _b; c = _c; d = _d; e = _e; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e); }
};

template <class T, class A, class B, class C, class D, class E, class F>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e, f); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e, f, g); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e, f, g, h); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H,I)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H,I);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h, I &_i) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; i = _i; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); Types::read(stream, &i); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); Types::write(stream, i); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e, f, g, h, i); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H,I,J)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H,I,J);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h, I &_i, J &_j) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; i = _i; j = _j; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); Types::read(stream, &i); Types::read(stream, &j); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); Types::write(stream, i); Types::write(stream, j); }
   void dispatch(Object *t) { (static_cast<T *>(t)->*ptr)(a, b, c, d, e, f, g, h, i, j); }
};

};

#endif

