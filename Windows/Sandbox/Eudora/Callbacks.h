//**************** callback.hpp **********************
// Copyright 1994 Rich Hickey
/* Permission to use, copy, modify, distribute and sell this software
 * for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Rich Hickey makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
*/

// 08/31/96 Rich Hickey
// Added ==, != and <
//  They are not inline, source is in file callback.cpp
//  Note: You must compile and link in callback.obj if you use them
// C++ doesn't allow ptr-to-func to void * anymore -> changed to void (*)(void)
// Added compiler workarounds for MS VC++ 4.2
// Prefixed all macros with RHCB
// Note: derivation from CBFunctorBase is now public, and access functions
// (for func, callee etc) are provided >>for implementation use only<<

// 06/12/94 Rich Hickey
// 3rd major revision
// Now functors are concrete classes, and should be held by value
// Virtual function mechanism removed
// Generic makeCallback() mechanism added for building functors
// from both stand-alone functions and object/ptr-to-mem-func pairs

#ifndef CALLBACK_HPP
#define CALLBACK_HPP

/*
To use:

If you wish to build a component that provides/needs a callback, simply
specify and hold a CBFunctor of the type corresponding to the args
you wish to pass and the return value you need. There are 10 Functors
from which to choose:

Callback0
Callback1<P1>
Callback2<P1,P2>
Callback3<P1,P2,P3>
Callback4<P1,P2,P3,P4>
Callback0wRet<RT>
Callback1wRet<P1,RT>
Callback2wRet<P1,P2,RT>
Callback3wRet<P1,P2,P3,RT>
Callback4wRet<P1,P2,P3,P4,RT>

These are parameterized by their args and return value if any. Each has
a default ctor and an operator() with the corresponding signature.

They can be treated and used just like ptr-to-functions.

If you want to be a client of a component that uses callbacks, you
create a CBFunctor by calling makeCallback().

There are three flavors of makeCallback - you can create a functor from:

a ptr-to-stand-alone function
an object and a pointer-to-member function.
a pointer-to-member function (which will be called on first arg of functor)

Note: this last was not covered in the article - see CBEXAM3.CPP

The current iteration of this library requires you pass makeCallback()
a dummy first argument of type ptr-to-the-Functor-type-you-want-to-create.
Simply cast 0 to provide this argument:

makeCallback((target-functor*)0,ptr-to-function)
makeCallback((target-functor*)0,reference-to-object,ptr-to-member-function)
makeCallback((target-functor*)0,ptr-to-member-function)

Future versions will drop this requirement once member templates are
available.

The functor system is 100% type safe. It is also type flexible. You can
build a functor out of a function that is 'type compatible' with the
target functor - it need not have an exactly matching signature. By
type compatible I mean a function with the same number of arguments, of
types reachable from the functor's argument types by implicit conversion.
The return type of the function must be implicitly convertible to the
return type of the functor. A functor with no return can be built from a
function with a return - the return value is simply ignored.
(See ethel example below)

All the correct virtual function behavior is preserved. (see ricky
example below).

If you somehow try to create something in violation
of the type system you will get a compile-time or template-instantiation-
time error.

The CBFunctor base class and translator
classes are artifacts of this implementation. You should not write
code that relies upon their existence. Nor should you rely on the return
value of makeCallback being anything in particular.

All that is guaranteed is that the Functor classes have a default ctor,
a ctor that can accept 0 as an initializer,
an operator() with the requested argument types and return type, an
operator that will allow it to be evaluated in a conditional (with
'true' meaning the functor is set, 'false' meaning it is not), and that
Functors can be constructed from the result of makeCallback(), given
you've passed something compatible to makeCallback(). In addition you
can compare 2 functors with ==, !=, and <. 2 functors with the same
'callee' (function, object and/or ptr-to-mem-func) shall compare
equal. op < forms an ordering relation across all callee types -> the
actual order is not meaningful or to be depended upon.

/////////////////////// BEGIN Example 1 //////////////////////////
#include <iostream.h>
#include "callback.hpp"

//do5Times() is a function that takes a functor and invokes it 5 times

void do5Times(const Callback1<int> &doIt)
	{
	for(int i=0;i<5;i++)
		doIt(i);
	}

//Here are some standalone functions

void fred(int i){cout << "fred: " << i<<endl;}
int ethel(long l){cout << "ethel: " << l<<endl;return l;}

//Here is a class with a virtual function, and a derived class

class B{
public:
	virtual void ricky(int i)
	   {cout << "B::ricky: " << i<<endl;}
};

class D:public B{
public:
	void ricky(int i)
	   {cout << "D::ricky: " << i<<endl;}
};

void main()
	{
	//create a typedef of the functor type to simplify dummy argument
	typedef Callback1<int> *FtorType;

	Callback1<int> ftor;	//a functor variable
	//make a functor from ptr-to-function
	ftor = makeCallback((FtorType)0,fred);
	do5Times(ftor);
	//note ethel is not an exact match - ok, is compatible
	ftor = makeCallback((FtorType)0,ethel);
	do5Times(ftor);

	//create a D object to be a callback target
	D myD;
	//make functor from object and ptr-to-member-func
	ftor = makeCallback((FtorType)0,myD,&B::ricky);
	do5Times(ftor);
	}
/////////////////////// END of example 1 //////////////////////////

/////////////////////// BEGIN Example 2 //////////////////////////
#include <iostream.h>
#include "callback.hpp"

//Button is a component that provides a functor-based
//callback mechanism, so you can wire it up to whatever you wish

class Button{
public:
	//ctor takes a functor and stores it away in a member

	Button(const Callback0 &uponClickDoThis):notify(uponClickDoThis)
		{}
	void click()
		{
		//invoke the functor, thus calling back client
		notify();
		}
private:
	//note this is a data member with a verb for a name - matches its
	//function-like usage
	Callback0 notify;
};

class CDPlayer{
public:
	void play()
		{cout << "Playing"<<endl;}
	void stop()
		{cout << "Stopped"<<endl;}
};

void main()
	{
	CDPlayer myCD;
	Button playButton(makeCallback((Callback0*)0,myCD,&CDPlayer::play));
	Button stopButton(makeCallback((Callback0*)0,myCD,&CDPlayer::stop));
	playButton.click();	//calls myCD.play()
	stopButton.click();  //calls myCD.stop()
	}
/////////////////////// END of example 2 //////////////////////////

*/

//******************************************************************
///////////////////////////////////////////////////////////////////*
//WARNING - no need to read past this point, lest confusion ensue. *
//Only the curious need explore further - but remember				 *
//about that cat!																	 *
///////////////////////////////////////////////////////////////////*
//******************************************************************


// MS VC++ 4.2 still has many bugs relating to templates
// This version works around them as best I can - however note that
// MS will allow 'void (T::*)()const' to bind to a non-const member function
// of T. In addition, they do not support overloading template functions
// based on constness of ptr-to-mem-funcs.
// When _MSC_VER is defined I provide only the const versions,however it is on
// the user's head, when calling makeCallback with a const T, to make sure
// that the pointed-to member function is also const since MS won't enforce it!

// Other than that the port is completely functional under VC++ 4.2

// One MS bug you may encounter during _use_ of the callbacks:
// If you pass them by reference you can't call op() on the reference
// Workaround is to pass by value.

/*
// MS unable to call operator() on template class reference
template <class T>
class Functor{
public:
	void operator()(T t)const{};
};

void foo(const Functor<int> &f)
	{
	f(1);	//error C2064: term does not evaluate to a function

	//works when f is passed by value
	}
*/

// Note: if you are porting to another compiler that is having trouble you
// can try defining some of these flags as well:


#if defined(_MSC_VER)	
//#define RHCB_CANT_PASS_MEMFUNC_BY_REFERENCE	//like it says
//#define RHCB_CANT_OVERLOAD_ON_CONSTNESS		//of mem funcs
#define RHCB_CANT_INIT_REFERENCE_CTOR_STYLE	//int i;int &ir(i); //MS falls down
//#define RHCB_WONT_PERFORM_PTR_CONVERSION		//of 0 to ptr-to-any-type
#endif


// Don't touch this stuff
#if defined(RHCB_CANT_PASS_MEMFUNC_BY_REFERENCE)
#define RHCB_CONST_REF
#else
#define RHCB_CONST_REF const &
#endif

#if defined(RHCB_CANT_INIT_REFERENCE_CTOR_STYLE)
#define RHCB_CTOR_STYLE_INIT =
#else
#define RHCB_CTOR_STYLE_INIT
#endif

#if defined(RHCB_WONT_PERFORM_PTR_CONVERSION)
#define RHCB_DUMMY_INIT int
#else
#define RHCB_DUMMY_INIT DummyInit *
#endif

////////////////////////////// THE CODE //////////////////////////

#include <string.h> //for memstuff
#include <stddef.h> //for size_t

//typeless representation of a function and optional object

class CBFunctorBase{
public:
	//Note: ctors are protected

	//for evaluation in conditionals - can the functor be called?
	operator bool()const{return callee||func;}

	//Now you can put them in containers _and_ remove them!
	//Source for these 3 is in callback.cpp
	friend bool
		operator==(const CBFunctorBase &lhs,const CBFunctorBase &rhs);
	friend bool
		operator!=(const CBFunctorBase &lhs,const CBFunctorBase &rhs);
	friend bool
		operator<(const CBFunctorBase &lhs,const CBFunctorBase &rhs);

   //The rest below for implementation use only !

	class DummyInit{
	};

	typedef void (CBFunctorBase::*PMemFunc)();
	typedef void (*PFunc)();
	enum {MEM_FUNC_SIZE = sizeof(PMemFunc)};

	PFunc	getFunc() const {return func;}
	void *getCallee() const {return callee;}
	const char *getMemFunc() const {return memFunc;}

protected:
////////////////////////////////////////////////////////////////
// Note: this code depends on all ptr-to-mem-funcs being same size
// If that is not the case then make memFunc as large as largest
////////////////////////////////////////////////////////////////
	union{
	PFunc func;
	char memFunc[MEM_FUNC_SIZE];
	};
	void *callee;

	CBFunctorBase():callee(0),func(0){}
	CBFunctorBase(const void *c,PFunc f, const void *mf,size_t sz):
		callee((void *)c)
		{
		if(c)	//must be callee/memfunc
			{
			memcpy(memFunc,mf,sz);
			if(sz<MEM_FUNC_SIZE)	//zero-out the rest, if any, so comparisons work
				{
				memset(memFunc+sz,0,MEM_FUNC_SIZE-sz);
				}
			}
		else	//must be ptr-to-func
			{
			func = f;
			}
		}
};


/************************* no arg - no return *******************/
class Callback0:public CBFunctorBase{
public:
	Callback0(RHCB_DUMMY_INIT = 0){}
	void operator()()const
		{
		thunk(*this);
		}
protected:
	typedef void (*Thunk)(const CBFunctorBase &);
	Callback0(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class Callee, class MemFunc>
class CBMemberTranslator0:public Callback0{
public:
	CBMemberTranslator0(Callee &c,const MemFunc &m):
		Callback0(thunk,&c,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(callee->*memFunc)();
		}
};

template <class Func>
class CBFunctionTranslator0:public Callback0{
public:
	CBFunctionTranslator0(Func f):Callback0(thunk,0,(PFunc)f,0,0){}
	static void thunk(const CBFunctorBase &ftor)
		{
		(Func(ftor.getFunc()))();
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class Callee,class TRT,class CallType>
inline CBMemberTranslator0<Callee,TRT (CallType::*)()>
makeCallback(Callback0 *,Callee &c,TRT (CallType::* RHCB_CONST_REF f)())
	{
	typedef TRT (CallType::*MemFunc)();
	return CBMemberTranslator0<Callee,MemFunc>(c,f);
	}
#endif

template <class Callee,class TRT,class CallType>
inline CBMemberTranslator0<const Callee,TRT (CallType::*)()const>
makeCallback(Callback0 *,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)()const)
	{
	typedef TRT (CallType::*MemFunc)()const;
	return CBMemberTranslator0<const Callee,MemFunc>(c,f);
	}

template <class TRT>
inline CBFunctionTranslator0<TRT (*)()>
makeCallback(Callback0 *,TRT (*f)())
	{
	return CBFunctionTranslator0<TRT (*)()>(f);
	}
/************************* no arg - with return *******************/
template <class RT>
class Callback0wRet:public CBFunctorBase{
public:
	Callback0wRet(RHCB_DUMMY_INIT = 0){}
	RT operator()()const
		{
		return thunk(*this);
		}
protected:
	typedef RT (*Thunk)(const CBFunctorBase &);
	Callback0wRet(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class RT,class Callee, class MemFunc>
class CBMemberTranslator0wRet:public Callback0wRet<RT>{
public:
	CBMemberTranslator0wRet(Callee &c,const MemFunc &m):
		Callback0wRet<RT>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (callee->*memFunc)();
		}
};

template <class RT,class Func>
class CBFunctionTranslator0wRet:public Callback0wRet<RT>{
public:
	CBFunctionTranslator0wRet(Func f):Callback0wRet<RT>(thunk,0,(PFunc)f,0,0){}
	static RT thunk(const CBFunctorBase &ftor)
		{
		return (Func(ftor.getFunc()))();
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class RT,class Callee,class TRT,class CallType>
inline CBMemberTranslator0wRet<RT,Callee,TRT (CallType::*)()>
makeCallback(Callback0wRet<RT>*,Callee &c,TRT (CallType::* RHCB_CONST_REF f)())
	{
	typedef TRT (CallType::*MemFunc)();
	return CBMemberTranslator0wRet<RT,Callee,MemFunc>(c,f);
	}
#endif

template <class RT,class Callee,class TRT,class CallType>
inline CBMemberTranslator0wRet<RT,const Callee,TRT (CallType::*)()const>
makeCallback(Callback0wRet<RT>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)()const)
	{
	typedef TRT (CallType::*MemFunc)()const;
	return CBMemberTranslator0wRet<RT,const Callee,MemFunc>(c,f);
	}

template <class RT,class TRT>
inline CBFunctionTranslator0wRet<RT,TRT (*)()>
makeCallback(Callback0wRet<RT>*,TRT (*f)())
	{
	return CBFunctionTranslator0wRet<RT,TRT (*)()>(f);
	}
/************************* one arg - no return *******************/
template <class P1>
class Callback1:public CBFunctorBase{
public:
	Callback1(RHCB_DUMMY_INIT = 0){}
	void operator()(P1 p1)const
		{
		thunk(*this,p1);
		}
	//for STL
	typedef P1 argument_type;
	typedef void result_type;
protected:
	typedef void (*Thunk)(const CBFunctorBase &,P1);
	Callback1(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class Callee, class MemFunc>
class CBMemberTranslator1:public Callback1<P1>{
public:
	CBMemberTranslator1(Callee &c,const MemFunc &m):
		Callback1<P1>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(callee->*memFunc)(p1);
		}
};

template <class P1,class Func>
class CBFunctionTranslator1:public Callback1<P1>{
public:
	CBFunctionTranslator1(Func f):Callback1<P1>(thunk,0,(PFunc)f,0,0){}
	static void thunk(const CBFunctorBase &ftor,P1 p1)
		{
		(Func(ftor.getFunc()))(p1);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class Callee,class TRT,class CallType,class TP1>
inline CBMemberTranslator1<P1,Callee,TRT (CallType::*)(TP1)>
makeCallback(Callback1<P1>*,Callee &c,TRT (CallType::* RHCB_CONST_REF f)(TP1))
	{
	typedef TRT (CallType::*MemFunc)(TP1);
	return CBMemberTranslator1<P1,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class Callee,class TRT,class CallType,class TP1>
inline CBMemberTranslator1<P1,const Callee,TRT (CallType::*)(TP1)const>
makeCallback(Callback1<P1>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1)const;
	return CBMemberTranslator1<P1,const Callee,MemFunc>(c,f);
	}

template <class P1, class TRT,class TP1>
inline CBFunctionTranslator1<TP1,TRT (*)(TP1)>
makeCallback(Callback1<P1>*, TRT (*f)(TP1))
	{
	return CBFunctionTranslator1<P1,TRT (*)(TP1)>(f);
	}

template <class P1,class MemFunc>
class CBMemberOf1stArgTranslator1:public Callback1<P1>{
public:
	CBMemberOf1stArgTranslator1(const MemFunc &m):
		Callback1<P1>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(p1.*memFunc)();
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class TRT,class CallType>
inline CBMemberOf1stArgTranslator1<P1,TRT (CallType::*)()>
makeCallback(Callback1<P1>*,TRT (CallType::* RHCB_CONST_REF f)())
	{
	typedef TRT (CallType::*MemFunc)();
	return CBMemberOf1stArgTranslator1<P1,MemFunc>(f);
	}
#endif

template <class P1,class TRT,class CallType>
inline CBMemberOf1stArgTranslator1<P1,TRT (CallType::*)()const>
makeCallback(Callback1<P1>*,TRT (CallType::* RHCB_CONST_REF f)()const)
	{
	typedef TRT (CallType::*MemFunc)()const;
	return CBMemberOf1stArgTranslator1<P1,MemFunc>(f);
	}


/************************* one arg - with return *******************/
template <class P1,class RT>
class Callback1wRet:public CBFunctorBase{
public:
	Callback1wRet(RHCB_DUMMY_INIT = 0){}
	RT operator()(P1 p1)const
		{
		return thunk(*this,p1);
		}
	//for STL
	typedef P1 argument_type;
	typedef RT result_type;
protected:
	typedef RT (*Thunk)(const CBFunctorBase &,P1);
	Callback1wRet(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class RT,class Callee, class MemFunc>
class CBMemberTranslator1wRet:public Callback1wRet<P1,RT>{
public:
	CBMemberTranslator1wRet(Callee &c,const MemFunc &m):
		Callback1wRet<P1,RT>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (callee->*memFunc)(p1);
		}
};

template <class P1,class RT,class Func>
class CBFunctionTranslator1wRet:public Callback1wRet<P1,RT>{
public:
	CBFunctionTranslator1wRet(Func f):
		Callback1wRet<P1,RT>(thunk,0,(PFunc)f,0,0){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1)
		{
		return (Func(ftor.getFunc()))(p1);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class RT,
	class Callee,class TRT,class CallType,class TP1>
inline CBMemberTranslator1wRet<P1,RT,Callee,TRT (CallType::*)(TP1)>
makeCallback(Callback1wRet<P1,RT>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1))
	{
	typedef TRT (CallType::*MemFunc)(TP1);
	return CBMemberTranslator1wRet<P1,RT,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class RT,
	class Callee,class TRT,class CallType,class TP1>
inline CBMemberTranslator1wRet<P1,RT,
	const Callee,TRT (CallType::*)(TP1)const>
makeCallback(Callback1wRet<P1,RT>*,
	const Callee &c,TRT (CallType::* RHCB_CONST_REF f)(TP1)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1)const;
	return CBMemberTranslator1wRet<P1,RT,const Callee,MemFunc>(c,f);
	}

template <class P1,class RT,class TRT,class TP1>
inline CBFunctionTranslator1wRet<P1,RT,TRT (*)(TP1)>
makeCallback(Callback1wRet<P1,RT>*,TRT (*f)(TP1))
	{
	return CBFunctionTranslator1wRet<P1,RT,TRT (*)(TP1)>(f);
	}

template <class P1,class RT,class MemFunc>
class CBMemberOf1stArgTranslator1wRet:public Callback1wRet<P1,RT>{
public:
	CBMemberOf1stArgTranslator1wRet(const MemFunc &m):
		Callback1wRet<P1,RT>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (p1.*memFunc)();
	}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class RT,class TRT,class CallType>
inline CBMemberOf1stArgTranslator1wRet<P1,RT,TRT (CallType::*)()>
makeCallback(Callback1wRet<P1,RT>*,TRT (CallType::* RHCB_CONST_REF f)())
	{
	typedef TRT (CallType::*MemFunc)();
	return CBMemberOf1stArgTranslator1wRet<P1,RT,MemFunc>(f);
	}
#endif

template <class P1,class RT,class TRT,class CallType>
inline CBMemberOf1stArgTranslator1wRet<P1,RT,TRT (CallType::*)()const>
makeCallback(Callback1wRet<P1,RT>*,TRT (CallType::* RHCB_CONST_REF f)()const)
	{
	typedef TRT (CallType::*MemFunc)()const;
	return CBMemberOf1stArgTranslator1wRet<P1,RT,MemFunc>(f);
	}


/************************* two args - no return *******************/
template <class P1,class P2>
class Callback2:public CBFunctorBase{
public:
	Callback2(RHCB_DUMMY_INIT = 0){}
	void operator()(P1 p1,P2 p2)const
		{
		thunk(*this,p1,p2);
		}
	//for STL
	typedef P1 first_argument_type;
	typedef P2 second_argument_type;
	typedef void result_type;
protected:
	typedef void (*Thunk)(const CBFunctorBase &,P1,P2);
	Callback2(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class Callee, class MemFunc>
class CBMemberTranslator2:public Callback2<P1,P2>{
public:
	CBMemberTranslator2(Callee &c,const MemFunc &m):
		Callback2<P1,P2>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(callee->*memFunc)(p1,p2);
		}
};

template <class P1,class P2,class Func>
class CBFunctionTranslator2:public Callback2<P1,P2>{
public:
	CBFunctionTranslator2(Func f):Callback2<P1,P2>(thunk,0,(PFunc)f,0,0){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		(Func(ftor.getFunc()))(p1,p2);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class Callee,
	class TRT,class CallType,class TP1,class TP2>
inline CBMemberTranslator2<P1,P2,Callee,TRT (CallType::*)(TP1,TP2)>
makeCallback(Callback2<P1,P2>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2);
	return CBMemberTranslator2<P1,P2,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class Callee,
	class TRT,class CallType,class TP1,class TP2>
inline CBMemberTranslator2<P1,P2,const Callee,
	TRT (CallType::*)(TP1,TP2)const>
makeCallback(Callback2<P1,P2>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2)const;
	return CBMemberTranslator2<P1,P2,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class TRT,class TP1,class TP2>
inline CBFunctionTranslator2<P1,P2,TRT (*)(TP1,TP2)>
makeCallback(Callback2<P1,P2>*,TRT (*f)(TP1,TP2))
	{
	return CBFunctionTranslator2<P1,P2,TRT (*)(TP1,TP2)>(f);
	}

template <class P1,class P2,class MemFunc>
class CBMemberOf1stArgTranslator2:public Callback2<P1,P2>{
public:
	CBMemberOf1stArgTranslator2(const MemFunc &m):
		Callback2<P1,P2>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(p1.*memFunc)(p2);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class TRT,class CallType,class TP1>
inline CBMemberOf1stArgTranslator2<P1,P2,TRT (CallType::*)(TP1)>
makeCallback(Callback2<P1,P2>*,TRT (CallType::* RHCB_CONST_REF f)(TP1))
	{
	typedef TRT (CallType::*MemFunc)(TP1);
	return CBMemberOf1stArgTranslator2<P1,P2,MemFunc>(f);
	}
#endif

template <class P1,class P2,class TRT,class CallType,class TP1>
inline CBMemberOf1stArgTranslator2<P1,P2,TRT (CallType::*)(TP1)const>
makeCallback(Callback2<P1,P2>*,TRT (CallType::* RHCB_CONST_REF f)(TP1)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1)const;
	return CBMemberOf1stArgTranslator2<P1,P2,MemFunc>(f);
	}


/************************* two args - with return *******************/
template <class P1,class P2,class RT>
class Callback2wRet:public CBFunctorBase{
public:
	Callback2wRet(RHCB_DUMMY_INIT = 0){}
	RT operator()(P1 p1,P2 p2)const
		{
		return thunk(*this,p1,p2);
		}
	//for STL
	typedef P1 first_argument_type;
	typedef P2 second_argument_type;
	typedef RT result_type;
protected:
	typedef RT (*Thunk)(const CBFunctorBase &,P1,P2);
	Callback2wRet(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class RT,class Callee, class MemFunc>
class CBMemberTranslator2wRet:public Callback2wRet<P1,P2,RT>{
public:
	CBMemberTranslator2wRet(Callee &c,const MemFunc &m):
		Callback2wRet<P1,P2,RT>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (callee->*memFunc)(p1,p2);
		}
};

template <class P1,class P2,class RT,class Func>
class CBFunctionTranslator2wRet:public Callback2wRet<P1,P2,RT>{
public:
	CBFunctionTranslator2wRet(Func f):
		Callback2wRet<P1,P2,RT>(thunk,0,(PFunc)f,0,0){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		return (Func(ftor.getFunc()))(p1,p2);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2>
inline CBMemberTranslator2wRet<P1,P2,RT,Callee,
	TRT (CallType::*)(TP1,TP2)>
makeCallback(Callback2wRet<P1,P2,RT>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2);
	return CBMemberTranslator2wRet<P1,P2,RT,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2>
inline CBMemberTranslator2wRet<P1,P2,RT,const Callee,
	TRT (CallType::*)(TP1,TP2)const>
makeCallback(Callback2wRet<P1,P2,RT>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2)const;
	return CBMemberTranslator2wRet<P1,P2,RT,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class RT,class TRT,class TP1,class TP2>
inline CBFunctionTranslator2wRet<P1,P2,RT,TRT (*)(TP1,TP2)>
makeCallback(Callback2wRet<P1,P2,RT>*,TRT (*f)(TP1,TP2))
	{
	return CBFunctionTranslator2wRet<P1,P2,RT,TRT (*)(TP1,TP2)>(f);
	}

template <class P1,class P2,class RT,class MemFunc>
class CBMemberOf1stArgTranslator2wRet:public Callback2wRet<P1,P2,RT>{
public:
	CBMemberOf1stArgTranslator2wRet(const MemFunc &m):
		Callback2wRet<P1,P2,RT>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (p1.*memFunc)(p2);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class RT,class TRT,class CallType,class TP1>
inline CBMemberOf1stArgTranslator2wRet<P1,P2,RT,TRT (CallType::*)(TP1)>
makeCallback(Callback2wRet<P1,P2,RT>*,TRT (CallType::* RHCB_CONST_REF f)(TP1))
	{
	typedef TRT (CallType::*MemFunc)(TP1);
	return CBMemberOf1stArgTranslator2wRet<P1,P2,RT,MemFunc>(f);
	}
#endif

template <class P1,class P2,class RT,class TRT,class CallType,class TP1>
inline CBMemberOf1stArgTranslator2wRet<P1,P2,RT,TRT (CallType::*)(TP1)const>
makeCallback(Callback2wRet<P1,P2,RT>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1)const;
	return CBMemberOf1stArgTranslator2wRet<P1,P2,RT,MemFunc>(f);
	}


/************************* three args - no return *******************/
template <class P1,class P2,class P3>
class Callback3:public CBFunctorBase{
public:
	Callback3(RHCB_DUMMY_INIT = 0){}
	void operator()(P1 p1,P2 p2,P3 p3)const
		{
		thunk(*this,p1,p2,p3);
		}
protected:
	typedef void (*Thunk)(const CBFunctorBase &,P1,P2,P3);
	Callback3(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class P3,class Callee, class MemFunc>
class CBMemberTranslator3:public Callback3<P1,P2,P3>{
public:
	CBMemberTranslator3(Callee &c,const MemFunc &m):
		Callback3<P1,P2,P3>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(callee->*memFunc)(p1,p2,p3);
		}
};

template <class P1,class P2,class P3,class Func>
class CBFunctionTranslator3:public Callback3<P1,P2,P3>{
public:
	CBFunctionTranslator3(Func f):Callback3<P1,P2,P3>(thunk,0,(PFunc)f,0,0){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		(Func(ftor.getFunc()))(p1,p2,p3);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3>
inline CBMemberTranslator3<P1,P2,P3,Callee,
	TRT (CallType::*)(TP1,TP2,TP3)>
makeCallback(Callback3<P1,P2,P3>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3);
	return CBMemberTranslator3<P1,P2,P3,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class P3,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3>
inline CBMemberTranslator3<P1,P2,P3,const Callee,
	TRT (CallType::*)(TP1,TP2,TP3)const>
makeCallback(Callback3<P1,P2,P3>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3)const;
	return CBMemberTranslator3<P1,P2,P3,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class P3,
	class TRT,class TP1,class TP2,class TP3>
inline CBFunctionTranslator3<P1,P2,P3,TRT (*)(TP1,TP2,TP3)>
makeCallback(Callback3<P1,P2,P3>*,TRT (*f)(TP1,TP2,TP3))
	{
	return CBFunctionTranslator3<P1,P2,P3,TRT (*)(TP1,TP2,TP3)>(f);
	}

template <class P1,class P2,class P3,class MemFunc>
class CBMemberOf1stArgTranslator3:public Callback3<P1,P2,P3>{
public:
	CBMemberOf1stArgTranslator3(const MemFunc &m):
		Callback3<P1,P2,P3>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(p1.*memFunc)(p2,p3);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class TRT,class CallType,
	class TP1,class TP2>
inline CBMemberOf1stArgTranslator3<P1,P2,P3,TRT (CallType::*)(TP1,TP2)>
makeCallback(Callback3<P1,P2,P3>*,TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2);
	return CBMemberOf1stArgTranslator3<P1,P2,P3,MemFunc>(f);
	}
#endif

template <class P1,class P2,class P3,class TRT,class CallType,
	class TP1,class TP2>
inline CBMemberOf1stArgTranslator3<P1,P2,P3,TRT (CallType::*)(TP1,TP2)const>
makeCallback(Callback3<P1,P2,P3>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2)const;
	return CBMemberOf1stArgTranslator3<P1,P2,P3,MemFunc>(f);
	}


/************************* three args - with return *******************/
template <class P1,class P2,class P3,class RT>
class Callback3wRet:public CBFunctorBase{
public:
	Callback3wRet(RHCB_DUMMY_INIT = 0){}
	RT operator()(P1 p1,P2 p2,P3 p3)const
		{
		return thunk(*this,p1,p2,p3);
		}
protected:
	typedef RT (*Thunk)(const CBFunctorBase &,P1,P2,P3);
	Callback3wRet(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class P3,
	class RT,class Callee, class MemFunc>
class CBMemberTranslator3wRet:public Callback3wRet<P1,P2,P3,RT>{
public:
	CBMemberTranslator3wRet(Callee &c,const MemFunc &m):
		Callback3wRet<P1,P2,P3,RT>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (callee->*memFunc)(p1,p2,p3);
		}
};

template <class P1,class P2,class P3,class RT,class Func>
class CBFunctionTranslator3wRet:public Callback3wRet<P1,P2,P3,RT>{
public:
	CBFunctionTranslator3wRet(Func f):
		Callback3wRet<P1,P2,P3,RT>(thunk,0,(PFunc)f,0,0){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		return (Func(ftor.getFunc()))(p1,p2,p3);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3>
inline CBMemberTranslator3wRet<P1,P2,P3,RT,Callee,
	TRT (CallType::*)(TP1,TP2,TP3)>
makeCallback(Callback3wRet<P1,P2,P3,RT>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3);
	return CBMemberTranslator3wRet<P1,P2,P3,RT,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class P3,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3>
inline CBMemberTranslator3wRet<P1,P2,P3,RT,const Callee,
	TRT (CallType::*)(TP1,TP2,TP3)const>
makeCallback(Callback3wRet<P1,P2,P3,RT>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3)const;
	return CBMemberTranslator3wRet<P1,P2,P3,RT,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class P3,class RT,
	class TRT,class TP1,class TP2,class TP3>
inline CBFunctionTranslator3wRet<P1,P2,P3,RT,TRT (*)(TP1,TP2,TP3)>
makeCallback(Callback3wRet<P1,P2,P3,RT>*,TRT (*f)(TP1,TP2,TP3))
	{
	return CBFunctionTranslator3wRet<P1,P2,P3,RT,TRT (*)(TP1,TP2,TP3)>(f);
	}

template <class P1,class P2,class P3,class RT,class MemFunc>
class CBMemberOf1stArgTranslator3wRet:public Callback3wRet<P1,P2,P3,RT>{
public:
	CBMemberOf1stArgTranslator3wRet(const MemFunc &m):
		Callback3wRet<P1,P2,P3,RT>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (p1.*memFunc)(p2,p3);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class RT,class TRT,class CallType,
	class TP1,class TP2>
inline CBMemberOf1stArgTranslator3wRet<P1,P2,P3,RT,TRT (CallType::*)(TP1,TP2)>
makeCallback(Callback3wRet<P1,P2,P3,RT>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2);
	return CBMemberOf1stArgTranslator3wRet<P1,P2,P3,RT,MemFunc>(f);
	}
#endif

template <class P1,class P2,class P3,class RT,class TRT,class CallType,
	class TP1,class TP2>
inline CBMemberOf1stArgTranslator3wRet<P1,P2,P3,RT,
	TRT (CallType::*)(TP1,TP2)const>
makeCallback(Callback3wRet<P1,P2,P3,RT>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2)const;
	return CBMemberOf1stArgTranslator3wRet<P1,P2,P3,RT,MemFunc>(f);
	}


/************************* four args - no return *******************/
template <class P1,class P2,class P3,class P4>
class Callback4:public CBFunctorBase{
public:
	Callback4(RHCB_DUMMY_INIT = 0){}
	void operator()(P1 p1,P2 p2,P3 p3,P4 p4)const
		{
		thunk(*this,p1,p2,p3,p4);
		}
protected:
	typedef void (*Thunk)(const CBFunctorBase &,P1,P2,P3,P4);
	Callback4(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class P3,class P4,
	class Callee, class MemFunc>
class CBMemberTranslator4:public Callback4<P1,P2,P3,P4>{
public:
	CBMemberTranslator4(Callee &c,const MemFunc &m):
		Callback4<P1,P2,P3,P4>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(callee->*memFunc)(p1,p2,p3,p4);
		}
};

template <class P1,class P2,class P3,class P4,class Func>
class CBFunctionTranslator4:public Callback4<P1,P2,P3,P4>{
public:
	CBFunctionTranslator4(Func f):
		Callback4<P1,P2,P3,P4>(thunk,0,(PFunc)f,0,0){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		(Func(ftor.getFunc()))(p1,p2,p3,p4);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class P4,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3,class TP4>
inline CBMemberTranslator4<P1,P2,P3,P4,Callee,
	TRT (CallType::*)(TP1,TP2,TP3,TP4)>
makeCallback(Callback4<P1,P2,P3,P4>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3,TP4))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3,TP4);
	return CBMemberTranslator4<P1,P2,P3,P4,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class P3,class P4,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3,class TP4>
inline CBMemberTranslator4<P1,P2,P3,P4,const Callee,
	TRT (CallType::*)(TP1,TP2,TP3,TP4)const>
makeCallback(Callback4<P1,P2,P3,P4>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3,TP4)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3,TP4)const;
	return CBMemberTranslator4<P1,P2,P3,P4,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class P3,class P4,
	class TRT,class TP1,class TP2,class TP3,class TP4>
inline CBFunctionTranslator4<P1,P2,P3,P4,TRT (*)(TP1,TP2,TP3,TP4)>
makeCallback(Callback4<P1,P2,P3,P4>*,TRT (*f)(TP1,TP2,TP3,TP4))
	{
	return CBFunctionTranslator4<P1,P2,P3,P4,TRT (*)(TP1,TP2,TP3,TP4)>(f);
	}

template <class P1,class P2,class P3,class P4,class MemFunc>
class CBMemberOf1stArgTranslator4:public Callback4<P1,P2,P3,P4>{
public:
	CBMemberOf1stArgTranslator4(const MemFunc &m):
		Callback4<P1,P2,P3,P4>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static void thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		(p1.*memFunc)(p2,p3,p4);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class P4,class TRT,class CallType,
	class TP1,class TP2,class TP3>
inline CBMemberOf1stArgTranslator4<P1,P2,P3,P4,TRT (CallType::*)(TP1,TP2,TP3)>
makeCallback(Callback4<P1,P2,P3,P4>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3);
	return CBMemberOf1stArgTranslator4<P1,P2,P3,P4,MemFunc>(f);
	}
#endif

template <class P1,class P2,class P3,class P4,class TRT,class CallType,
	class TP1,class TP2,class TP3>
inline CBMemberOf1stArgTranslator4<P1,P2,P3,P4,
	TRT (CallType::*)(TP1,TP2,TP3)const>
makeCallback(Callback4<P1,P2,P3,P4>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3)const;
	return CBMemberOf1stArgTranslator4<P1,P2,P3,P4,MemFunc>(f);
	}


/************************* four args - with return *******************/
template <class P1,class P2,class P3,class P4,class RT>
class Callback4wRet:public CBFunctorBase{
public:
	Callback4wRet(RHCB_DUMMY_INIT = 0){}
	RT operator()(P1 p1,P2 p2,P3 p3,P4 p4)const
		{
		return thunk(*this,p1,p2,p3,p4);
		}
protected:
	typedef RT (*Thunk)(const CBFunctorBase &,P1,P2,P3,P4);
	Callback4wRet(Thunk t,const void *c,PFunc f,const void *mf,size_t sz):
		CBFunctorBase(c,f,mf,sz),thunk(t){}
private:
	Thunk thunk;
};

template <class P1,class P2,class P3,class P4,class RT,
	class Callee, class MemFunc>
class CBMemberTranslator4wRet:public Callback4wRet<P1,P2,P3,P4,RT>{
public:
	CBMemberTranslator4wRet(Callee &c,const MemFunc &m):
		Callback4wRet<P1,P2,P3,P4,RT>(thunk,&c,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		Callee *callee = (Callee *)ftor.getCallee();
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (callee->*memFunc)(p1,p2,p3,p4);
		}
};

template <class P1,class P2,class P3,class P4,class RT,class Func>
class CBFunctionTranslator4wRet:public Callback4wRet<P1,P2,P3,P4,RT>{
public:
	CBFunctionTranslator4wRet(Func f):
		Callback4wRet<P1,P2,P3,P4,RT>(thunk,0,(PFunc)f,0,0){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		return (Func(ftor.getFunc()))(p1,p2,p3,p4);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class P4,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3,class TP4>
inline CBMemberTranslator4wRet<P1,P2,P3,P4,RT,Callee,
	TRT (CallType::*)(TP1,TP2,TP3,TP4)>
makeCallback(Callback4wRet<P1,P2,P3,P4,RT>*,Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3,TP4))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3,TP4);
	return CBMemberTranslator4wRet<P1,P2,P3,P4,RT,Callee,MemFunc>(c,f);
	}
#endif

template <class P1,class P2,class P3,class P4,class RT,class Callee,
	class TRT,class CallType,class TP1,class TP2,class TP3,class TP4>
inline CBMemberTranslator4wRet<P1,P2,P3,P4,RT,const Callee,
	TRT (CallType::*)(TP1,TP2,TP3,TP4)const>
makeCallback(Callback4wRet<P1,P2,P3,P4,RT>*,const Callee &c,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3,TP4)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3,TP4)const;
	return CBMemberTranslator4wRet<P1,P2,P3,P4,RT,const Callee,MemFunc>(c,f);
	}

template <class P1,class P2,class P3,class P4,class RT,
	class TRT,class TP1,class TP2,class TP3,class TP4>
inline CBFunctionTranslator4wRet<P1,P2,P3,P4,RT,TRT (*)(TP1,TP2,TP3,TP4)>
makeCallback(Callback4wRet<P1,P2,P3,P4,RT>*,TRT (*f)(TP1,TP2,TP3,TP4))
	{
	return CBFunctionTranslator4wRet
		<P1,P2,P3,P4,RT,TRT (*)(TP1,TP2,TP3,TP4)>(f);
	}


template <class P1,class P2,class P3,class P4,class RT,class MemFunc>
class CBMemberOf1stArgTranslator4wRet:public Callback4wRet<P1,P2,P3,P4,RT>{
public:
	CBMemberOf1stArgTranslator4wRet(const MemFunc &m):
		Callback4wRet<P1,P2,P3,P4,RT>(thunk,(void *)1,0,&m,sizeof(MemFunc)){}
	static RT thunk(const CBFunctorBase &ftor,P1 p1,P2 p2,P3 p3,P4 p4)
		{
		MemFunc &memFunc RHCB_CTOR_STYLE_INIT
			(*(MemFunc*)(void *)(ftor.getMemFunc()));
		return (p1.*memFunc)(p2,p3,p4);
		}
};

#if !defined(RHCB_CANT_OVERLOAD_ON_CONSTNESS)
template <class P1,class P2,class P3,class P4,class RT,class TRT,
	class CallType,class TP1,class TP2,class TP3>
inline CBMemberOf1stArgTranslator4wRet<P1,P2,P3,P4,RT,
	TRT (CallType::*)(TP1,TP2,TP3)>
makeCallback(Callback4wRet<P1,P2,P3,P4,RT>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3))
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3);
	return CBMemberOf1stArgTranslator4wRet<P1,P2,P3,P4,RT,MemFunc>(f);
	}
#endif

template <class P1,class P2,class P3,class P4,class RT,class TRT,
	class CallType,class TP1,class TP2,class TP3>
inline CBMemberOf1stArgTranslator4wRet<P1,P2,P3,P4,RT,
	TRT (CallType::*)(TP1,TP2,TP3)const>
makeCallback(Callback4wRet<P1,P2,P3,P4,RT>*,
	TRT (CallType::* RHCB_CONST_REF f)(TP1,TP2,TP3)const)
	{
	typedef TRT (CallType::*MemFunc)(TP1,TP2,TP3)const;
	return CBMemberOf1stArgTranslator4wRet<P1,P2,P3,P4,RT,MemFunc>(f);
	}

#endif //CALLBACK_HPP

