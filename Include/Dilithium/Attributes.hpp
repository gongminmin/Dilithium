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

#include <bitset>
#include <map>
#include <string>

namespace Dilithium
{
	class AttrBuilder;
	class AttributeImpl;
	class AttributeSetImpl;
	class AttributeSetNode;
	class LLVMContext;

	class Attribute
	{
	public:
		enum AttrKind
		{
			// IR-Level Attributes
			AK_None,					///< No attributes have been set
			AK_Alignment,				///< Alignment of parameter (5 bits)
										///< stored as log2 of alignment with +1 bias
										///< 0 means unaligned (different from align(1))
			AK_AlwaysInline,			///< inline=always
			AK_Builtin,					///< Callee is recognized as a builtin, despite
										///< nobuiltin attribute on its declaration.
			AK_ByVal,					///< Pass structure by value
			AK_InAlloca,				///< Pass structure in an alloca
			AK_Cold,					///< Marks function as being in a cold path.
			AK_Convergent,				///< Can only be moved to control-equivalent blocks
			AK_InlineHint,				///< Source said inlining was desirable
			AK_InReg,					///< Force argument to be passed in register
			AK_JumpTable,				///< Build jump-instruction tables and replace refs.
			AK_MinSize,					///< Function must be optimized for size first
			AK_Naked,					///< Naked function
			AK_Nest,					///< Nested function static chain
			AK_NoAlias,					///< Considered to not alias after call
			AK_NoBuiltin,				///< Callee isn't recognized as a builtin
			AK_NoCapture,				///< Function creates no aliases of pointer
			AK_NoDuplicate,				///< Call cannot be duplicated
			AK_NoImplicitFloat,			///< Disable implicit floating point insts
			AK_NoInline,				///< inline=never
			AK_NonLazyBind,				///< Function is called early and/or
										///< often, so lazy binding isn't worthwhile
			AK_NonNull,					///< Pointer is known to be not null
			AK_Dereferenceable,			///< Pointer is known to be dereferenceable
			AK_DereferenceableOrNull,	///< Pointer is either null or dereferenceable
			AK_NoRedZone,				///< Disable redzone
			AK_NoReturn,				///< Mark the function as not returning
			AK_NoUnwind,				///< Function doesn't unwind stack
			AK_OptimizeForSize,			///< opt_size
			AK_OptimizeNone,			///< Function must not be optimized.
			AK_ReadNone,				///< Function does not access memory
			AK_ReadOnly,				///< Function only reads from memory
			AK_ArgMemOnly,				///< Funciton can access memory only using pointers
										///< based on its arguments.
			AK_Returned,				///< Return value is always equal to this argument
			AK_ReturnsTwice,			///< Function can return twice
			AK_SExt,					///< Sign extended before/after call
			AK_StackAlignment,			///< Alignment of stack for function (3 bits)
										///< stored as log2 of alignment with +1 bias 0
										///< means unaligned (different from
										///< alignstack=(1))
			AK_StackProtect,			///< Stack protection.
			AK_StackProtectReq,			///< Stack protection required.
			AK_StackProtectStrong,		///< Strong Stack protection.
			AK_SafeStack,				///< Safe Stack protection.
			AK_StructRet,				///< Hidden pointer to structure to return
			AK_SanitizeAddress,			///< AddressSanitizer is on.
			AK_SanitizeThread,			///< ThreadSanitizer is on.
			AK_SanitizeMemory,			///< MemorySanitizer is on.
			AK_UWTable,					///< Function must be in a unwind table
			AK_ZExt,					///< Zero extended before/after call

			AK_EndAttrKinds				///< Sentinal value useful for loops
		};

	public:
		Attribute();

		static Attribute Get(LLVMContext& context, AttrKind kind, uint64_t val = 0);
		static Attribute Get(LLVMContext& context, std::string_view kind, std::string_view val = std::string_view());

		static Attribute GetWithAlignment(LLVMContext& context, uint64_t align);
		static Attribute GetWithStackAlignment(LLVMContext& context, uint64_t align);
		static Attribute GetWithDereferenceableBytes(LLVMContext& context, uint64_t bytes);
		static Attribute GetWithDereferenceableOrNullBytes(LLVMContext& context, uint64_t bytes);

		bool IsEnumAttribute() const;
		bool IsIntAttribute() const;
		bool IsStringAttribute() const;

		bool HasAttribute(AttrKind val) const;
		bool HasAttribute(std::string_view val) const;

		AttrKind KindAsEnum() const;
		uint64_t ValueAsInt() const;

		std::string_view KindAsString() const;
		std::string_view ValueAsString() const;

		uint32_t Alignment() const;
		uint32_t StackAlignment() const;
		uint64_t DereferenceableBytes() const;
		uint64_t DereferenceableOrNullBytes() const;
		std::string AsString(bool in_attr_grp = false) const;

		bool operator==(Attribute const & rhs) const
		{
			return impl_ == rhs.impl_;
		}
		bool operator!=(Attribute const & rhs) const
		{
			return impl_ != rhs.impl_;
		}

		bool operator<(Attribute const & rhs) const;

		void* RawPointer() const
		{
			return impl_;
		}

	private:
		AttributeImpl* impl_;
		Attribute(AttributeImpl* ai);
	};

	class AttributeSet
	{
		friend class AttrBuilder;
		friend class AttributeSetImpl;

	public:
		enum AttrIndex : uint32_t
		{
			AI_ReturnIndex = 0U,
			AI_FunctionIndex = ~0U
		};

		typedef ArrayRef<Attribute>::iterator iterator;

	public:
		AttributeSet();

		static AttributeSet Get(LLVMContext& context, ArrayRef<AttributeSet> attrs);
		static AttributeSet Get(LLVMContext& context, uint32_t index, ArrayRef<Attribute::AttrKind> kind);
		static AttributeSet Get(LLVMContext& context, uint32_t index, AttrBuilder const & ab);

		iterator Begin(uint32_t slot) const;
		iterator End(uint32_t slot) const;

		bool operator==(AttributeSet const & rhs) const
		{
			return impl_ == rhs.impl_;
		}
		bool operator!=(AttributeSet const & rhs) const
		{
			return impl_ != rhs.impl_;
		}

		uint32_t NumSlots() const;
		uint32_t SlotIndex(uint32_t slot) const;

	private:
		explicit AttributeSet(AttributeSetImpl* asi);

		static AttributeSet Get(LLVMContext& context, ArrayRef<std::pair<uint32_t, Attribute>> attrs);
		static AttributeSet Get(LLVMContext& context, ArrayRef<std::pair<uint32_t, AttributeSetNode*>> attrs);

	private:
		AttributeSetImpl* impl_;
	};

	class AttrBuilder
	{
	public:
		typedef std::pair<std::string, std::string> td_type;
		typedef std::map<std::string, std::string>::iterator td_iterator;
		typedef std::map<std::string, std::string>::const_iterator td_const_iterator;
		typedef boost::iterator_range<td_iterator> td_range;
		typedef boost::iterator_range<td_const_iterator> td_const_range;

	public:
		AttrBuilder();
		explicit AttrBuilder(uint64_t val);
		AttrBuilder(Attribute const & attr);
		AttrBuilder(AttributeSet const & as, uint32_t index);

		AttrBuilder& AddAttribute(Attribute::AttrKind val);
		AttrBuilder& AddAttribute(Attribute const & attr);
		AttrBuilder& AddAttribute(std::string_view attr, std::string_view val = std::string_view());

		bool Contains(Attribute::AttrKind kind) const
		{
			return attrs_[kind];
		}
		bool Contains(std::string_view attr) const;

		bool HasAttributes() const;
		bool HasAttributes(AttributeSet const & as, uint64_t index) const;

		uint64_t Alignment() const
		{
			return alignment_;
		}
		uint64_t StackAlignment() const
		{
			return stack_alignment_;
		}
		uint64_t DereferenceableBytes() const
		{
			return deref_bytes_;
		}
		uint64_t DereferenceableOrNullBytes() const
		{
			return deref_or_null_bytes_;
		}

		AttrBuilder& AddAlignmentAttr(uint32_t align);
		AttrBuilder& AddStackAlignmentAttr(uint32_t align);
		AttrBuilder& AddDereferenceableAttr(uint64_t bytes);
		AttrBuilder& AddDereferenceableOrNullAttr(uint64_t bytes);

		td_iterator TDBegin()
		{
			return target_dep_attrs_.begin();
		}
		td_const_iterator TDBegin() const
		{
			return target_dep_attrs_.begin();
		}		
		td_iterator TDEnd()
		{
			return target_dep_attrs_.end();
		}
		td_const_iterator TDEnd() const
		{
			return target_dep_attrs_.end();
		}

		td_const_range TDAttrs() const
		{
			return td_const_range(this->TDBegin(), this->TDEnd());
		}
		td_range TDAttrs()
		{
			return td_range(this->TDBegin(), this->TDEnd());
		}

		// FIXME: Remove this in 4.0.

		AttrBuilder& AddRawValue(uint64_t val);

	private:
		std::bitset<Attribute::AK_EndAttrKinds> attrs_;
		std::map<std::string, std::string> target_dep_attrs_;
		uint64_t alignment_;
		uint64_t stack_alignment_;
		uint64_t deref_bytes_;
		uint64_t deref_or_null_bytes_;
	};
}

#endif		// _DILITHIUM_ATTRIBUTES_HPP
