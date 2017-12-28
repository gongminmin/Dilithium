/**
 * @file AttributeImpl.cpp
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

#include <Dilithium/Dilithium.hpp>
#include "AttributeImpl.hpp"
#include <Dilithium/Hashing.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/MathExtras.hpp>
#include "LLVMContextImpl.hpp"

namespace Dilithium
{
	AttributeImpl::AttributeImpl(AttrEntryKind kind_id)
		: kind_id_(static_cast<uint8_t>(kind_id))
	{
	}

	AttributeImpl::~AttributeImpl()
	{
	}

	bool AttributeImpl::HasAttribute(Attribute::AttrKind ak) const
	{
		if (this->IsStringAttribute())
		{
			return false;
		}
		return this->KindAsEnum() == ak;
	}

	bool AttributeImpl::HasAttribute(std::string_view kind) const
	{
		if (!this->IsStringAttribute())
		{
			return false;
		}
		return this->KindAsString() == kind;
	}

	Attribute::AttrKind AttributeImpl::KindAsEnum() const
	{
		BOOST_ASSERT(this->IsEnumAttribute() || this->IsIntAttribute());
		return static_cast<EnumAttributeImpl const *>(this)->EnumKind();
	}

	uint64_t AttributeImpl::ValueAsInt() const
	{
		BOOST_ASSERT(this->IsIntAttribute());
		return static_cast<const IntAttributeImpl *>(this)->Value();
	}

	std::string_view AttributeImpl::KindAsString() const
	{
		BOOST_ASSERT(this->IsStringAttribute());
		return static_cast<const StringAttributeImpl *>(this)->StringKind();
	}

	std::string_view AttributeImpl::ValueAsString() const
	{
		BOOST_ASSERT(this->IsStringAttribute());
		return static_cast<const StringAttributeImpl *>(this)->StringValue();
	}

	bool AttributeImpl::operator<(AttributeImpl const & rhs) const
	{
		// The order followers the one in Attribute::AttrKinds

		if (this->IsEnumAttribute())
		{
			if (rhs.IsEnumAttribute())
			{
				return this->KindAsEnum() < rhs.KindAsEnum();
			}
			if (rhs.IsIntAttribute())
			{
				return true;
			}
			if (rhs.IsStringAttribute())
			{
				return true;
			}
		}

		if (this->IsIntAttribute())
		{
			if (rhs.IsEnumAttribute())
			{
				return false;
			}
			if (rhs.IsIntAttribute())
			{
				return this->ValueAsInt() < rhs.ValueAsInt();
			}
			if (rhs.IsStringAttribute())
			{
				return true;
			}
		}

		if (rhs.IsEnumAttribute())
		{
			return false;
		}
		if (rhs.IsIntAttribute())
		{
			return false;
		}
		if (this->KindAsString() == rhs.KindAsString())
		{
			return this->ValueAsString() < rhs.ValueAsString();
		}

		return this->KindAsString() < rhs.KindAsString();
	}

	uint64_t AttributeImpl::AttrMask(Attribute::AttrKind val)
	{
		// FIXME: Remove this.
		switch (val)
		{
		case Attribute::AK_None:
			return 0;
		case Attribute::AK_ZExt:
			return 1U << 0;
		case Attribute::AK_SExt:
			return 1U << 1;
		case Attribute::AK_NoReturn:
			return 1U << 2;
		case Attribute::AK_InReg:
			return 1U << 3;
		case Attribute::AK_StructRet:
			return 1U << 4;
		case Attribute::AK_NoUnwind:
			return 1U << 5;
		case Attribute::AK_NoAlias:
			return 1U << 6;
		case Attribute::AK_ByVal:
			return 1U << 7;
		case Attribute::AK_Nest:
			return 1U << 8;
		case Attribute::AK_ReadNone:
			return 1U << 9;
		case Attribute::AK_ReadOnly:
			return 1U << 10;
		case Attribute::AK_NoInline:
			return 1U << 11;
		case Attribute::AK_AlwaysInline:
			return 1U << 12;
		case Attribute::AK_OptimizeForSize:
			return 1U << 13;
		case Attribute::AK_StackProtect:
			return 1U << 14;
		case Attribute::AK_StackProtectReq:
			return 1U << 15;
		case Attribute::AK_Alignment:
			return 31U << 16;
		case Attribute::AK_NoCapture:
			return 1U << 21;
		case Attribute::AK_NoRedZone:
			return 1U << 22;
		case Attribute::AK_NoImplicitFloat:
			return 1U << 23;
		case Attribute::AK_Naked:
			return 1U << 24;
		case Attribute::AK_InlineHint:
			return 1U << 25;
		case Attribute::AK_StackAlignment:
			return 7U << 26;
		case Attribute::AK_ReturnsTwice:
			return 1U << 29;
		case Attribute::AK_UWTable:
			return 1U << 30;
		case Attribute::AK_NonLazyBind:
			return 1U << 31;
		case Attribute::AK_SanitizeAddress:
			return 1ULL << 32;
		case Attribute::AK_MinSize:
			return 1ULL << 33;
		case Attribute::AK_NoDuplicate:
			return 1ULL << 34;
		case Attribute::AK_StackProtectStrong:
			return 1ULL << 35;
		case Attribute::AK_SanitizeThread:
			return 1ULL << 36;
		case Attribute::AK_SanitizeMemory:
			return 1ULL << 37;
		case Attribute::AK_NoBuiltin:
			return 1ULL << 38;
		case Attribute::AK_Returned:
			return 1ULL << 39;
		case Attribute::AK_Cold:
			return 1ULL << 40;
		case Attribute::AK_Builtin:
			return 1ULL << 41;
		case Attribute::AK_OptimizeNone:
			return 1ULL << 42;
		case Attribute::AK_InAlloca:
			return 1ULL << 43;
		case Attribute::AK_NonNull:
			return 1ULL << 44;
		case Attribute::AK_JumpTable:
			return 1ULL << 45;
		case Attribute::AK_Convergent:
			return 1ULL << 46;
		case Attribute::AK_SafeStack:
			return 1ULL << 47;

		case Attribute::AK_Dereferenceable:
			DILITHIUM_UNREACHABLE("dereferenceable attribute not supported in raw format");
			break;
		case Attribute::AK_DereferenceableOrNull:
			DILITHIUM_UNREACHABLE("dereferenceable_or_null attribute not supported in raw format");
			break;
		case Attribute::AK_ArgMemOnly:
			DILITHIUM_UNREACHABLE("argmemonly attribute not supported in raw format");
			break;
		case Attribute::AK_EndAttrKinds:
			DILITHIUM_UNREACHABLE("Synthetic enumerators which should never get here");

		default:
			DILITHIUM_UNREACHABLE("Unsupported attribute type");
		}
	}


	EnumAttributeImpl::EnumAttributeImpl(AttrEntryKind id, Attribute::AttrKind kind)
		: AttributeImpl(id), kind_(kind)
	{
	}

	EnumAttributeImpl::EnumAttributeImpl(Attribute::AttrKind kind)
		: EnumAttributeImpl(EnumAttrEntry, kind)
	{
	}


	IntAttributeImpl::IntAttributeImpl(Attribute::AttrKind kind, uint64_t val)
		: EnumAttributeImpl(IntAttrEntry, kind), val_(val)
	{
		BOOST_ASSERT_MSG((kind == Attribute::AK_Alignment) || (kind == Attribute::AK_StackAlignment)
			|| (kind == Attribute::AK_Dereferenceable) || (kind == Attribute::AK_DereferenceableOrNull),
			"Wrong kind for int attribute!");
	}


	StringAttributeImpl::StringAttributeImpl(std::string_view kind, std::string_view val)
		: AttributeImpl(StringAttrEntry), kind_(kind), val_(val)
	{
	}


	AttributeSetNode::AttributeSetNode(ArrayRef<Attribute> attrs)
		: attrs_(attrs.ToVector())
	{
	}

	AttributeSetNode* AttributeSetNode::Get(LLVMContext& context, ArrayRef<Attribute> attrs)
	{
		if (attrs.empty())
		{
			return nullptr;
		}
		else
		{
			auto& context_impl = context.Impl();

			boost::container::small_vector<Attribute, 8> SortedAttrs(attrs.begin(), attrs.end());
			std::sort(SortedAttrs.begin(), SortedAttrs.end());

			size_t hash_val = 0;
			for (auto iter = SortedAttrs.begin(), end_iter = SortedAttrs.end(); iter != end_iter; ++iter)
			{
				boost::hash_combine(hash_val, iter->RawPointer());
			}

			auto iter = context_impl.attrs_set_nodes.find(hash_val);
			if (iter == context_impl.attrs_set_nodes.end())
			{
				auto pa = std::make_unique<AttributeSetNode>(SortedAttrs);
				iter = context_impl.attrs_set_nodes.emplace(hash_val, std::move(pa)).first;
			}

			return iter->second.get();
		}
	}

	bool AttributeSetNode::HasAttribute(Attribute::AttrKind kind) const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(kind))
			{
				return true;
			}
		}
		return false;
	}

	bool AttributeSetNode::HasAttribute(std::string_view kind) const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(kind))
			{
				return true;
			}
		}
		return false;
	}

	Attribute AttributeSetNode::GetAttribute(Attribute::AttrKind kind) const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(kind))
			{
				return attr;
			}
		}
		return Attribute();
	}

	Attribute AttributeSetNode::GetAttribute(std::string_view kind) const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(kind))
			{
				return attr;
			}
		}
		return Attribute();
	}

	uint32_t AttributeSetNode::Alignment() const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(Attribute::AK_Alignment))
			{
				return attr.Alignment();
			}
		}
		return 0;
	}

	uint32_t AttributeSetNode::StackAlignment() const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(Attribute::AK_StackAlignment))
			{
				return attr.StackAlignment();
			}
		}
		return 0;
	}

	uint64_t AttributeSetNode::DereferenceableBytes() const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(Attribute::AK_Dereferenceable))
			{
				return attr.DereferenceableBytes();
			}
		}
		return 0;
	}

	uint64_t AttributeSetNode::DereferenceableOrNullBytes() const
	{
		for (auto const & attr : *this)
		{
			if (attr.HasAttribute(Attribute::AK_DereferenceableOrNull))
			{
				return attr.DereferenceableOrNullBytes();
			}
		}
		return 0;
	}

	std::string AttributeSetNode::GetAsString(bool in_attr_grp) const
	{
		std::string ret;
		for (auto iter = this->begin(), end_iter = this->end(); iter != end_iter; ++ iter)
		{
			if (iter != this->begin())
			{
				ret += ' ';
			}
			ret += iter->GetAsString(in_attr_grp);
		}
		return ret;
	}


	AttributeSetImpl::AttributeSetImpl(LLVMContext& context, ArrayRef<std::pair<uint32_t, AttributeSetNode*>> attrs)
		: context_(context), attrs_(attrs.ToVector())
	{
#ifdef DILITHIUM_DEBUG
		if (attrs.size() >= 2)
		{
			for (auto iter = attrs.begin() + 1, end_iter = attrs.end(); iter != end_iter; ++iter)
			{
				BOOST_ASSERT_MSG((iter - 1)->first <= iter->first, "Attribute set not ordered!");
			}
		}
#endif
	}

	uint64_t AttributeSetImpl::Raw(uint32_t index) const
	{
		for (uint32_t i = 0, e = this->NumAttributes(); i != e; ++ i)
		{
			if (this->SlotIndex(i) != index)
			{
				continue;
			}

			AttributeSetNode const * asn = this->SlotNode(i);
			uint64_t mask = 0;

			for (auto iter = asn->begin(), end_iter = asn->end(); iter != end_iter; ++ iter)
			{
				Attribute const & attr = *iter;

				if (attr.IsStringAttribute())
				{
					continue;
				}

				Attribute::AttrKind kind = attr.KindAsEnum();

				if (kind == Attribute::AK_Alignment)
				{
					mask |= (static_cast<uint64_t>(Log2_32(asn->Alignment())) + 1) << 16;
				}
				else if (kind == Attribute::AK_StackAlignment)
				{
					mask |= (static_cast<uint64_t>(Log2_32(asn->StackAlignment())) + 1) << 26;
				}
				else if (kind == Attribute::AK_Dereferenceable)
				{
					DILITHIUM_UNREACHABLE("Dereferenceable not supported in bit mask");
				}
				else
				{
					mask |= AttributeImpl::AttrMask(kind);
				}
			}

			return mask;
		}

		return 0;
	}
}
