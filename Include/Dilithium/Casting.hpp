/**
 * @file Casting.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of Dilithium
 * For the latest info, see https://github.com/gongminmin/Dilithium
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Minmin Gong. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _DILITHIUM_CASTING_HPP
#define _DILITHIUM_CASTING_HPP

#include <Dilithium/Compiler.hpp>
#include <Dilithium/TypeTraits.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	// From LLVM
	// TODO: Try to replace them by RTTI

	//===----------------------------------------------------------------------===//
	//                          isa<x> Support Templates
	//===----------------------------------------------------------------------===//

	// Define a template that can be specialized by smart pointers to reflect the
	// fact that they are automatically dereferenced, and are not involved with the
	// template selection process...  the default implementation is a noop.
	//
	template <typename From>
	struct simplify_type
	{
		typedef From SimpleType;        // The real type this represents...

		// An accessor to get the real value...
		static SimpleType& SimplifiedValue(From& val)
		{
			return val;
		}
	};

	template <typename From>
	struct simplify_type<From const>
	{
		typedef typename simplify_type<From>::SimpleType NonConstSimpleType;
		typedef typename add_const_past_pointer<NonConstSimpleType>::type SimpleType;
		typedef typename add_lvalue_reference_if_not_pointer<SimpleType>::type RetType;
		static RetType SimplifiedValue(From const & val)
		{
			return simplify_type<From>::SimplifiedValue(const_cast<From&>(val));
		}
	};

	// The core of the implementation of isa<X> is here; To and From should be
	// the names of classes.  This template can be specialized to customize the
	// implementation of isa<> without rewriting it from scratch.
	template <typename To, typename From, typename Enabler = void>
	struct isa_impl
	{
		static bool doit(From const & val)
		{
			return To::classof(&val);
		}
	};

	/// \brief Always allow upcasts, and perform no dynamic check for them.
	template <typename To, typename From>
	struct isa_impl<To, From, typename std::enable_if<std::is_base_of<To, From>::value>::type>
	{
		static bool doit(From const &)
		{
			return true;
		}
	};

	template <typename To, typename From>
	struct isa_impl_cl
	{
		static bool doit(From const & val)
		{
			return isa_impl<To, From>::doit(val);
		}
	};

	template <typename To, typename From>
	struct isa_impl_cl<To, From const>
	{
		static bool doit(From const & val)
		{
			return isa_impl<To, From>::doit(val);
		}
	};

	template <typename To, typename From>
	struct isa_impl_cl<To, From*>
	{
		static bool doit(From const * val)
		{
			BOOST_ASSERT_MSG(val, "isa<> used on a null pointer");
			return isa_impl<To, From>::doit(*val);
		}
	};

	template <typename To, typename From>
	struct isa_impl_cl<To, From* const>
	{
		static bool doit(From const * val)
		{
			BOOST_ASSERT_MSG(val, "isa<> used on a null pointer");
			return isa_impl<To, From>::doit(*val);
		}
	};

	template <typename To, typename From>
	struct isa_impl_cl<To, From const *>
	{
		static bool doit(From const * val)
		{
			BOOST_ASSERT_MSG(val, "isa<> used on a null pointer");
			return isa_impl<To, From>::doit(*val);
		}
	};

	template <typename To, typename From> struct isa_impl_cl<To, From const * const>
	{
		static bool doit(From const * val)
		{
			BOOST_ASSERT_MSG(val, "isa<> used on a null pointer");
			return isa_impl<To, From>::doit(*val);
		}
	};

	template <typename To, typename From, typename SimpleFrom>
	struct isa_impl_wrap
	{
		// When From != SimplifiedType, we can simplify the type some more by using
		// the simplify_type template.
		static bool doit(From const & val)
		{
			return isa_impl_wrap<To, SimpleFrom, typename simplify_type<SimpleFrom>::SimpleType>::doit(
					simplify_type<From const>::SimplifiedValue(val));
		}
	};

	template <typename To, typename FromTy>
	struct isa_impl_wrap<To, FromTy, FromTy>
	{
		// When From == SimpleType, we are as simple as we are going to get.
		static bool doit(FromTy const & val)
		{
			return isa_impl_cl<To, FromTy>::doit(val);
		}
	};

	// isa<X> - Return true if the parameter to the template is an instance of the
	// template type argument.  Used like this:
	//
	//  if (isa<Type>(myVal)) { ... }
	//
	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline bool isa(Y const & val)
	{
		return isa_impl_wrap<X, Y const, typename simplify_type<Y const>::SimpleType>::doit(val);
	}

	//===----------------------------------------------------------------------===//
	//                          cast<x> Support Templates
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	template <typename To, typename From>
	struct cast_retty;


	// Calculate what type the 'cast' function should return, based on a requested
	// type of To and a source type of From.
	template <typename To, typename From>
	struct cast_retty_impl
	{
		typedef To& ret_type;         // Normal case, return Ty&
	};
	template <typename To, typename From>
	struct cast_retty_impl<To, From const>
	{
		typedef To const & ret_type;   // Normal case, return Ty&
	};

	template <typename To, typename From>
	struct cast_retty_impl<To, From*>
	{
		typedef To* ret_type;         // Pointer arg case, return Ty*
	};

	template <typename To, typename From>
	struct cast_retty_impl<To, From const *>
	{
		typedef To const * ret_type;   // Constant pointer arg case, return Ty const *
	};

	template <typename To, typename From>
	struct cast_retty_impl<To, From const * const>
	{
		typedef To const * ret_type;   // Constant pointer arg case, return Ty const *
	};


	template <typename To, typename From, typename SimpleFrom>
	struct cast_retty_wrap
	{
		// When the simplified type and the from type are not the same, use the type
		// simplifier to reduce the type, then reuse cast_retty_impl to get the
		// resultant type.
		typedef typename cast_retty<To, SimpleFrom>::ret_type ret_type;
	};

	template <typename To, typename FromTy>
	struct cast_retty_wrap<To, FromTy, FromTy>
	{
		// When the simplified type is equal to the from type, use it directly.
		typedef typename cast_retty_impl<To, FromTy>::ret_type ret_type;
	};

	template <typename To, typename From>
	struct cast_retty
	{
		typedef typename cast_retty_wrap<To, From, typename simplify_type<From>::SimpleType>::ret_type ret_type;
	};

	// Ensure the non-simple values are converted using the simplify_type template
	// that may be specialized by smart pointers...
	//
	template <typename To, typename From, typename SimpleFrom>
	struct cast_convert_val
	{
		// This is not a simple type, use the template to simplify it...
		static typename cast_retty<To, From>::ret_type doit(From &val)
		{
			return cast_convert_val<To, SimpleFrom, typename simplify_type<SimpleFrom>::SimpleType>::doit(
					simplify_type<From>::SimplifiedValue(val));
		}
	};

	template <typename To, typename FromTy>
	struct cast_convert_val<To, FromTy, FromTy>
	{
		// This _is_ a simple type, just cast it.
		static typename cast_retty<To, FromTy>::ret_type doit(FromTy const & val)
		{
			return static_cast<typename cast_retty<To, FromTy>::ret_type>(const_cast<FromTy&>(val));
		}
	};

	template <typename X>
	struct is_simple_type
	{
		static bool constexpr value = std::is_same<X, typename simplify_type<X>::SimpleType>::value;
	};

	// cast<X> - Return the argument parameter cast to the specified type.  This
	// casting operator asserts that the type is correct, so it does not return null
	// on failure.  It does not allow a null argument (use cast_or_null for that).
	// It is typically used like this:
	//
	//  cast<Instruction>(myVal)->getParent()
	//
	template <typename X, typename Y>
	inline typename std::enable_if<!is_simple_type<Y>::value, typename cast_retty<X, Y const>::ret_type>::type cast(Y const & val)
	{
		BOOST_ASSERT_MSG(isa<X>(val), "cast<Ty>() argument of incompatible type!");
		return cast_convert_val<X, Y const, typename simplify_type<Y const>::SimpleType>::doit(val);
	}

	template <typename X, typename Y>
	inline typename cast_retty<X, Y>::ret_type cast(Y& val)
	{
		BOOST_ASSERT_MSG(isa<X>(val), "cast<Ty>() argument of incompatible type!");
		return cast_convert_val<X, Y, typename simplify_type<Y>::SimpleType>::doit(val);
	}

	template <typename X, typename Y>
	inline typename cast_retty<X const, Y const *>::ret_type cast(Y const * val)
	{
		BOOST_ASSERT_MSG(isa<X>(val), "cast<Ty>() argument of incompatible type!");
		return cast_convert_val<X, Y const *, typename simplify_type<Y const *>::SimpleType>::doit(val);
	}

	template <typename X, typename Y>
	inline typename cast_retty<X, Y *>::ret_type cast(Y *val)
	{
		BOOST_ASSERT_MSG(isa<X>(val), "cast<Ty>() argument of incompatible type!");
		return cast_convert_val<X, Y*, typename simplify_type<Y*>::SimpleType>::doit(val);
	}

	// cast_or_null<X> - Functionally identical to cast, except that a null value is
	// accepted.
	//
	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename std::enable_if<
		!is_simple_type<Y>::value, typename cast_retty<X, Y const>::ret_type>::type cast_or_null(Y const & val)
	{
		if (!val)
		{
			return nullptr;
		}

		BOOST_ASSERT_MSG(isa<X>(val), "cast_or_null<Ty>() argument of incompatible type!");
		return cast<X>(val);
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename std::enable_if<
		!is_simple_type<Y>::value, typename cast_retty<X, Y>::ret_type>::type cast_or_null(Y& val)
	{
		if (!val)
		{
			return nullptr;
		}

		BOOST_ASSERT_MSG(isa<X>(val), "cast_or_null<Ty>() argument of incompatible type!");
		return cast<X>(val);
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X const, Y const *>::ret_type cast_or_null(Y const * val)
	{
		if (!val)
		{
			return nullptr;
		}

		BOOST_ASSERT_MSG(isa<X>(val), "cast_or_null<Ty>() argument of incompatible type!");
		return cast<X>(val);
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X, Y *>::ret_type cast_or_null(Y *val)
	{
		if (!val)
		{
			return nullptr;
		}

		BOOST_ASSERT_MSG(isa<X>(val), "cast_or_null<Ty>() argument of incompatible type!");
		return cast<X>(val);
	}


	// dyn_cast<X> - Return the argument parameter cast to the specified type.  This
	// casting operator returns null if the argument is of the wrong type, so it can
	// be used to test for a type as well as cast if successful.  This should be
	// used in the context of an if statement like this:
	//
	//  if (Instruction const * I = dyn_cast<Instruction>(myVal)) { ... }
	//

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename std::enable_if<
		!is_simple_type<Y>::value, typename cast_retty<X, Y const>::ret_type>::type dyn_cast(Y const & val)
	{
		return isa<X>(val) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X, Y>::ret_type dyn_cast(Y& val)
	{
		return isa<X>(val) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X const, Y const *>::ret_type dyn_cast(Y const * val)
	{
		return isa<X>(val) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X, Y *>::ret_type dyn_cast(Y* val)
	{
		return isa<X>(val) ? cast<X>(val) : nullptr;
	}

	// dyn_cast_or_null<X> - Functionally identical to dyn_cast, except that a null
	// value is accepted.
	//
	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename std::enable_if<
		!is_simple_type<Y>::value, typename cast_retty<X, Y const>::ret_type>::type dyn_cast_or_null(Y const & val)
	{
		return (val && isa<X>(val)) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename std::enable_if<
		!is_simple_type<Y>::value, typename cast_retty<X, Y>::ret_type>::type dyn_cast_or_null(Y& val)
	{
		return (val && isa<X>(val)) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X const, Y const *>::ret_type dyn_cast_or_null(Y const * val)
	{
		return (val && isa<X>(val)) ? cast<X>(val) : nullptr;
	}

	template <typename X, typename Y>
	DILITHIUM_ATTRIBUTE_UNUSED_RESULT inline typename cast_retty<X, Y *>::ret_type dyn_cast_or_null(Y* val)
	{
		return (val && isa<X>(val)) ? cast<X>(val) : nullptr;
	}
}

#endif		// _DILITHIUM_CASTING_HPP
