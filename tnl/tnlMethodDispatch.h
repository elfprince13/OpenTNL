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

   extern void read(TNL::BitStream &s, TNL::StringPtr *val);
   extern void write(TNL::BitStream &s, TNL::StringPtr &val);
   extern void read(TNL::BitStream &s, TNL::ByteBufferPtr *val);
   extern void write(TNL::BitStream &s, TNL::ByteBufferPtr &val);
   extern void read(TNL::BitStream &s, TNL::IPAddress *val);
   extern void write(TNL::BitStream &s, TNL::IPAddress &val);

   inline void read(TNL::BitStream &s, TNL::StringTableEntry *val)
   {
      s.readStringTableEntry(val);
   }
   inline void write(TNL::BitStream &s, TNL::StringTableEntry &val)
   {
      s.writeStringTableEntry(val);
   }
   template <typename T> inline void read(TNL::BitStream &s, T *val)
   { 
      s.read(val);
   }
   template <typename T> inline void write(TNL::BitStream &s, T &val)
   { 
      s.write(val);
   }
   template <typename T> inline void read(TNL::BitStream &s, TNL::Vector<T> *val)
   {
      TNL::U32 size = s.readInt(VectorSizeBitSize);
      val->setSize(size);
      for(TNL::S32 i = 0; i < val->size(); i++)
         read(s, &((*val)[i]));
   }
   template <typename T> void write(TNL::BitStream &s, TNL::Vector<T> &val)
   {
      s.writeInt(val.size(), VectorSizeBitSize);
      for(TNL::S32 i = 0; i < val.size(); i++)
         write(s, val[i]);
   }

   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::Int<BitCount> *val)
   {
      val->value = s.readInt(BitCount);
   }
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::Int<BitCount> &val)
   {
      s.writeInt(val.value, BitCount);
   }

   template <TNL::U32 BitCount> inline void read(TNL::BitStream &s, TNL::SignedInt<BitCount> *val)
   {
      val->value = s.readSignedInt(BitCount);
   }
   template <TNL::U32 BitCount> inline void write(TNL::BitStream &s,TNL::SignedInt<BitCount> &val)
   {
      s.writeSignedInt(val.value, BitCount);
   }

   template <TNL::U32 T, TNL::U32 U> inline void read(TNL::BitStream &s, TNL::RangedU32<T,U> *val)
   {
      val->value = s.readRangedU32(T, U);
   }
   template <TNL::U32 T, TNL::U32 U> inline void write(TNL::BitStream &s,TNL::RangedU32<T,U> &val)
   {
      s.writeRangedU32(val.value, T, U);
   }

   template <TNL::U32 T> inline void read(TNL::BitStream &s, TNL::Float<T> *val)
   {
      val->value = s.readFloat(T);
   }
   template <TNL::U32 T> inline void write(TNL::BitStream &s,TNL::Float<T> &val)
   {
      s.writeFloat(val.value, T);
   }
   template <TNL::U32 T> inline void read(TNL::BitStream &s, TNL::SignedFloat<T> *val)
   {
      val->value = s.readSignedFloat(T);
   }
   template <TNL::U32 T> inline void write(TNL::BitStream &s,TNL::SignedFloat<T> &val)
   {
      s.writeSignedFloat(val.value, T);
   }
};

namespace TNL {

struct Functor {
   Functor() {}
   virtual ~Functor() {}
   virtual void read(BitStream &stream) = 0;
   virtual void write(BitStream &stream) = 0;
   virtual void dispatch(void *t) = 0;
};
template <class T> 
struct FunctorDecl : public Functor {
   FunctorDecl() {}
   void set() {}
   void read(BitStream &stream) {}
   void write(BitStream &stream) {}
   void dispatch(void *t) { }
};
template <class T> 
struct FunctorDecl<void (T::*)()> : public Functor {
   typedef void (T::*FuncPtr)();
   FuncPtr ptr;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set() {}
   void read(BitStream &stream) {}
   void write(BitStream &stream) {}
   void dispatch(void *t) { ((T *)t->*ptr)(); }
};
template <class T, class A> 
struct FunctorDecl<void (T::*)(A)> : public Functor {
   typedef void (T::*FuncPtr)(A);
   FuncPtr ptr; A a;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a) { a = _a; }
   void read(BitStream &stream) { Types::read(stream, &a); }
   void write(BitStream &stream) { Types::write(stream, a); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a); }
};
template <class T, class A, class B>
struct FunctorDecl<void (T::*)(A,B)>: public Functor {
   typedef void (T::*FuncPtr)(A,B);
   FuncPtr ptr; A a; B b;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b) { a = _a; b = _b;}
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b); }
};

template <class T, class A, class B, class C>
struct FunctorDecl<void (T::*)(A,B,C)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C);
   FuncPtr ptr; A a; B b; C c;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c) { a = _a; b = _b; c = _c;}
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c); }
};

template <class T, class A, class B, class C, class D>
struct FunctorDecl<void (T::*)(A,B,C,D)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D);
   FuncPtr ptr; A a; B b; C c; D d;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d) { a = _a; b = _b; c = _c; d = _d; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d); }
};

template <class T, class A, class B, class C, class D, class E>
struct FunctorDecl<void (T::*)(A,B,C,D,E)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E);
   FuncPtr ptr; A a; B b; C c; D d; E e;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e) { a = _a; b = _b; c = _c; d = _d; e = _e; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e); }
};

template <class T, class A, class B, class C, class D, class E, class F>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e, f); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e, f, g); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e, f, g, h); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H, class I>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H,I)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H,I);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h, I &_i) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; i = _i; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); Types::read(stream, &i); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); Types::write(stream, i); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e, f, g, h, i); }
};

template <class T, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
struct FunctorDecl<void (T::*)(A,B,C,D,E,F,G,H,I,J)>: public Functor {
   typedef void (T::*FuncPtr)(A,B,C,D,E,F,G,H,I,J);
   FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j;
   FunctorDecl(FuncPtr p) : ptr(p) {}
   void set(A &_a, B &_b, C &_c, D &_d, E &_e, F &_f, G &_g, H &_h, I &_i, J &_j) { a = _a; b = _b; c = _c; d = _d; e = _e; f = _f; g = _g; h = _h; i = _i; j = _j; }
   void read(BitStream &stream) { Types::read(stream, &a); Types::read(stream, &b); Types::read(stream, &c); Types::read(stream, &d); Types::read(stream, &e); Types::read(stream, &f); Types::read(stream, &g); Types::read(stream, &h); Types::read(stream, &i); Types::read(stream, &j); }
   void write(BitStream &stream) { Types::write(stream, a); Types::write(stream, b); Types::write(stream, c); Types::write(stream, d); Types::write(stream, e); Types::write(stream, f); Types::write(stream, g); Types::write(stream, h); Types::write(stream, i); Types::write(stream, j); }
   void dispatch(void *t) { (((T *)t)->*ptr)(a, b, c, d, e, f, g, h, i, j); }
};

};

#endif

