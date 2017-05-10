/**
 * @file Attributes.hpp
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

#ifndef _DILITHIUM_ATTRIBUTES_HPP
#define _DILITHIUM_ATTRIBUTES_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/ArrayRef.hpp>

namespace Dilithium
{
	class AttrBuilder;
	class LLVMContext;

	class Attribute
	{
	public:
		enum AttrKind
		{
			// IR-Level Attributes
			None,                  ///< No attributes have been set
			Alignment,             ///< Alignment of parameter (5 bits)
								   ///< stored as log2 of alignment with +1 bias
								   ///< 0 means unaligned (different from align(1))
			AlwaysInline,          ///< inline=always
			Builtin,               ///< Callee is recognized as a builtin, despite
								   ///< nobuiltin attribute on its declaration.
			ByVal,                 ///< Pass structure by value
			InAlloca,              ///< Pass structure in an alloca
			Cold,                  ///< Marks function as being in a cold path.
			Convergent,            ///< Can only be moved to control-equivalent blocks
			InlineHint,            ///< Source said inlining was desirable
			InReg,                 ///< Force argument to be passed in register
			JumpTable,             ///< Build jump-instruction tables and replace refs.
			MinSize,               ///< Function must be optimized for size first
			Naked,                 ///< Naked function
			Nest,                  ///< Nested function static chain
			NoAlias,               ///< Considered to not alias after call
			NoBuiltin,             ///< Callee isn't recognized as a builtin
			NoCapture,             ///< Function creates no aliases of pointer
			NoDuplicate,           ///< Call cannot be duplicated
			NoImplicitFloat,       ///< Disable implicit floating point insts
			NoInline,              ///< inline=never
			NonLazyBind,           ///< Function is called early and/or
								   ///< often, so lazy binding isn't worthwhile
			NonNull,               ///< Pointer is known to be not null
			Dereferenceable,       ///< Pointer is known to be dereferenceable
			DereferenceableOrNull, ///< Pointer is either null or dereferenceable
			NoRedZone,             ///< Disable redzone
			NoReturn,              ///< Mark the function as not returning
			NoUnwind,              ///< Function doesn't unwind stack
			OptimizeForSize,       ///< opt_size
			OptimizeNone,          ///< Function must not be optimized.
			ReadNone,              ///< Function does not access memory
			ReadOnly,              ///< Function only reads from memory
			ArgMemOnly,            ///< Funciton can access memory only using pointers
								   ///< based on its arguments.
			Returned,              ///< Return value is always equal to this argument
			ReturnsTwice,          ///< Function can return twice
			SExt,                  ///< Sign extended before/after call
			StackAlignment,        ///< Alignment of stack for function (3 bits)
								   ///< stored as log2 of alignment with +1 bias 0
								   ///< means unaligned (different from
								   ///< alignstack=(1))
			StackProtect,          ///< Stack protection.
			StackProtectReq,       ///< Stack protection required.
			StackProtectStrong,    ///< Strong Stack protection.
			SafeStack,             ///< Safe Stack protection.
			StructRet,             ///< Hidden pointer to structure to return
			SanitizeAddress,       ///< AddressSanitizer is on.
			SanitizeThread,        ///< ThreadSanitizer is on.
			SanitizeMemory,        ///< MemorySanitizer is on.
			UWTable,               ///< Function must be in a unwind table
			ZExt,                  ///< Zero extended before/after call

			EndAttrKinds           ///< Sentinal value useful for loops
		};
	};

	class AttributeSet
	{
		friend class AttrBuilder;

	public:
		static AttributeSet Get(LLVMContext& context, ArrayRef<AttributeSet> attrs);
		static AttributeSet Get(LLVMContext& context, uint32_t index, ArrayRef<Attribute::AttrKind> kind);
		static AttributeSet Get(LLVMContext& context, uint32_t index, AttrBuilder const & ab);
	};

	class AttrBuilder
	{
	public:
		AttrBuilder& AddAttribute(Attribute::AttrKind val);
		AttrBuilder& AddAttribute(Attribute attr);
		AttrBuilder& AddAttribute(std::string_view attr, std::string_view val = std::string_view());

		AttrBuilder& AddAlignmentAttr(uint32_t align);
		AttrBuilder& AddStackAlignmentAttr(uint32_t align);
		AttrBuilder& AddDereferenceableAttr(uint64_t bytes);
		AttrBuilder& AddDereferenceableOrNullAttr(uint64_t bytes);

		// FIXME: Remove this in 4.0.

		AttrBuilder& AddRawValue(uint64_t val);
	};
}

#endif		// _DILITHIUM_ATTRIBUTES_HPP
