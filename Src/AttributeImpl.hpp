/**
 * @file AttributeImpl.hpp
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

#ifndef _DILITHIUM_ATTRIBUTE_IMPL_HPP
#define _DILITHIUM_ATTRIBUTE_IMPL_HPP

#pragma once

#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/Attributes.hpp>
#include <string>
#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class AttributeImpl : boost::noncopyable
	{
	public:
		virtual ~AttributeImpl();

		bool IsEnumAttribute() const
		{
			return kind_id_ == EnumAttrEntry;
		}
		bool IsIntAttribute() const
		{
			return kind_id_ == IntAttrEntry;
		}
		bool IsStringAttribute() const
		{
			return kind_id_ == StringAttrEntry;
		}

		bool HasAttribute(Attribute::AttrKind ak) const;
		bool HasAttribute(std::string_view kind) const;

		Attribute::AttrKind KindAsEnum() const;
		uint64_t ValueAsInt() const;

		std::string_view KindAsString() const;
		std::string_view ValueAsString() const;

		bool operator<(AttributeImpl const & rhs) const;

		static uint64_t AttrMask(Attribute::AttrKind val);

	protected:
		enum AttrEntryKind
		{
			EnumAttrEntry,
			IntAttrEntry,
			StringAttrEntry
		};

		AttributeImpl(AttrEntryKind kind_id);

	private:
		uint8_t kind_id_;
	};


	class EnumAttributeImpl : public AttributeImpl
	{
	public:
		EnumAttributeImpl(Attribute::AttrKind kind);

		Attribute::AttrKind EnumKind() const
		{
			return kind_;
		}

	protected:
		EnumAttributeImpl(AttrEntryKind id, Attribute::AttrKind kind);

	private:
		Attribute::AttrKind kind_;
	};

	class IntAttributeImpl : public EnumAttributeImpl
	{
	public:
		IntAttributeImpl(Attribute::AttrKind kind, uint64_t val);

		uint64_t Value() const
		{
			return val_;
		}

	private:
		uint64_t val_;
	};

	class StringAttributeImpl : public AttributeImpl
	{
	public:
		StringAttributeImpl(std::string_view kind, std::string_view val = std::string_view());

		std::string_view StringKind() const
		{
			return kind_;
		}
		std::string_view StringValue() const
		{
			return val_;
		}

	private:
		std::string kind_;
		std::string val_;
	};

	class AttributeSetNode : boost::noncopyable
	{
	public:
		typedef Attribute const * iterator;

	public:
		explicit AttributeSetNode(ArrayRef<Attribute> attrs);

		static AttributeSetNode* Get(LLVMContext& context, ArrayRef<Attribute> attrs);

		bool HasAttribute(Attribute::AttrKind kind) const;
		bool HasAttribute(std::string_view kind) const;
		bool HasAttributes() const
		{
			return !attrs_.empty();
		}

		Attribute GetAttribute(Attribute::AttrKind kind) const;
		Attribute GetAttribute(std::string_view kind) const;

		uint32_t Alignment() const;
		uint32_t StackAlignment() const;
		uint64_t DereferenceableBytes() const;
		uint64_t DereferenceableOrNullBytes() const;
		std::string GetAsString(bool in_attr_grp) const;

		iterator begin() const
		{
			return attrs_.data();
		}
		iterator end() const
		{
			return attrs_.data() + attrs_.size();
		}

	private:
		std::vector<Attribute> attrs_;
	};

	class AttributeSetImpl : boost::noncopyable
	{
		friend class AttributeSet;

	public:
		typedef std::pair<uint32_t, AttributeSetNode*> IndexAttrPair;
		typedef AttributeSetNode::iterator iterator;

	public:
		AttributeSetImpl(LLVMContext& context, ArrayRef<std::pair<uint32_t, AttributeSetNode*>> attrs);

		LLVMContext& Context()
		{
			return context_;
		}

		uint32_t NumAttributes() const
		{
			return static_cast<uint32_t>(attrs_.size());
		}

		uint32_t SlotIndex(uint32_t slot) const
		{
			return this->Node(slot)->first;
		}
		AttributeSet SlotAttributes(uint32_t slot) const
		{
			return AttributeSet::Get(context_, *this->Node(slot));
		}
		AttributeSetNode* SlotNode(uint32_t slot) const
		{
			return this->Node(slot)->second;
		}

		iterator Begin(uint32_t slot) const
		{
			return this->SlotNode(slot)->begin();
		}
		iterator End(uint32_t slot) const
		{
			return this->SlotNode(slot)->end();
		}

		// FIXME: This atrocity is temporary.
		uint64_t Raw(uint32_t index) const;

	private:
		IndexAttrPair const * Node(uint32_t slot) const
		{
			return &attrs_[slot];
		}

	private:
		LLVMContext& context_;
		std::vector<IndexAttrPair> attrs_;
	};
}

#endif		// _DILITHIUM_ATTRIBUTE_IMPL_HPP
