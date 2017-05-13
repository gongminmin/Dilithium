/**
 * @file Attributes.cpp
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
#include <Dilithium/Attributes.hpp>
#include <Dilithium/Hashing.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/MathExtras.hpp>
#include "AttributeImpl.hpp"
#include "LLVMContextImpl.hpp"

namespace Dilithium 
{
	Attribute::Attribute()
		: impl_(nullptr)
	{
	}

	Attribute::Attribute(AttributeImpl* ai)
		: impl_(ai)
	{
	}

	Attribute Attribute::Get(LLVMContext& context, AttrKind kind, uint64_t val)
	{
		auto& context_impl = context.Impl();
		size_t hash_val = boost::hash_value(kind);
		if (val)
		{
			boost::hash_combine(hash_val, val);
		}

		auto iter = context_impl.attrs_set.find(hash_val);
		if (iter == context_impl.attrs_set.end())
		{
			std::unique_ptr<AttributeImpl> pa;
			if (!val)
			{
				pa = std::make_unique<EnumAttributeImpl>(kind);
			}
			else
			{
				pa = std::make_unique<IntAttributeImpl>(kind, val);
			}
			iter = context_impl.attrs_set.emplace(hash_val, std::move(pa)).first;
		}

		return Attribute(iter->second.get());
	}

	Attribute Attribute::Get(LLVMContext& context, std::string_view kind, std::string_view val)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(kind);
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Attribute Attribute::GetWithAlignment(LLVMContext& context, uint64_t align)
	{
		BOOST_ASSERT_MSG(IsPowerOfTwo32(static_cast<uint32_t>(align)), "Alignment must be a power of two.");
		BOOST_ASSERT_MSG(align <= 0x40000000, "Alignment too large.");
		return Attribute::Get(context, AK_Alignment, align);
	}

	Attribute Attribute::GetWithStackAlignment(LLVMContext& context, uint64_t align)
	{
		BOOST_ASSERT_MSG(IsPowerOfTwo32(static_cast<uint32_t>(align)), "Alignment must be a power of two.");
		BOOST_ASSERT_MSG(align <= 0x100, "Alignment too large.");
		return Attribute::Get(context, AK_StackAlignment, align);
	}

	Attribute Attribute::GetWithDereferenceableBytes(LLVMContext& context, uint64_t bytes)
	{
		BOOST_ASSERT_MSG(bytes, "Bytes must be non-zero.");
		return Attribute::Get(context, AK_Dereferenceable, bytes);
	}

	Attribute Attribute::GetWithDereferenceableOrNullBytes(LLVMContext& context, uint64_t bytes)
	{
		BOOST_ASSERT_MSG(bytes, "Bytes must be non-zero.");
		return Attribute::Get(context, AK_DereferenceableOrNull, bytes);
	}

	bool Attribute::IsEnumAttribute() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Attribute::IsIntAttribute() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Attribute::IsStringAttribute() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Attribute::HasAttribute(AttrKind kind) const
	{
		return (impl_ && impl_->HasAttribute(kind)) || (!impl_ && (kind == AK_None));
	}

	bool Attribute::HasAttribute(std::string_view val) const
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Attribute::AttrKind Attribute::KindAsEnum() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint64_t Attribute::ValueAsInt() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	std::string_view Attribute::KindAsString() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	std::string_view Attribute::ValueAsString() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint32_t Attribute::Alignment() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint32_t Attribute::StackAlignment() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint64_t Attribute::DereferenceableBytes() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint64_t Attribute::DereferenceableOrNullBytes() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	std::string Attribute::AsString(bool in_attr_grp) const
	{
		DILITHIUM_UNUSED(in_attr_grp);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Attribute::operator<(Attribute const & rhs) const
	{
		if (!impl_ && !rhs.impl_)
		{
			return false;
		}
		if (!impl_)
		{
			return true;
		}
		if (!rhs.impl_)
		{
			return false;
		}
		return *impl_ < *rhs.impl_;
	}


	AttributeSet::AttributeSet()
		: impl_(nullptr)
	{
	}

	AttributeSet::AttributeSet(AttributeSetImpl* asi)
		: impl_(asi)
	{
	}

	AttributeSet AttributeSet::Get(LLVMContext& context, ArrayRef<AttributeSet> attrs)
	{
		if (attrs.empty())
		{
			return AttributeSet();
		}
		if (attrs.size() == 1)
		{
			return attrs[0];
		}

		boost::container::small_vector<std::pair<uint32_t, AttributeSetNode*>, 8> attr_node_vec;
		AttributeSetImpl* asi = attrs[0].impl_;
		if (asi)
		{
			attr_node_vec.insert(attr_node_vec.end(), asi->Node(0), asi->Node(asi->NumAttributes()));
		}
		for (size_t i = 1, e = attrs.size(); i != e; ++ i)
		{
			asi = attrs[i].impl_;
			if (!asi)
			{
				continue;
			}
			auto anvi = attr_node_vec.begin();
			auto anve = attr_node_vec.end();
			for (auto ai = asi->Node(0), ae = asi->Node(asi->NumAttributes()); ai != ae; ++ ai)
			{
				anve = attr_node_vec.end();
				while ((anvi != anve) && (anvi->first <= ai->first))
				{
					++ anvi;
				}
				anvi = attr_node_vec.insert(anvi, *ai) + 1;
			}
		}

		return GetImpl(context, attr_node_vec);
	}
	
	AttributeSet AttributeSet::Get(LLVMContext& context, uint32_t index, ArrayRef<Attribute::AttrKind> kind)
	{
		boost::container::small_vector<std::pair<unsigned, Attribute>, 8> attrs;
		for (auto iter = kind.begin(), end_iter = kind.end(); iter != end_iter; ++ iter)
		{
			attrs.push_back(std::make_pair(index, Attribute::Get(context, *iter)));
		}
		return AttributeSet::Get(context, attrs);
	}

	AttributeSet AttributeSet::Get(LLVMContext& context, uint32_t index, AttrBuilder const & ab)
	{
		if (!ab.HasAttributes())
		{
			return AttributeSet();
		}
		else
		{
			boost::container::small_vector<std::pair<unsigned, Attribute>, 8> attrs;
			for (uint32_t i = Attribute::AK_None; i != Attribute::AK_EndAttrKinds; ++ i)
			{
				auto kind = static_cast<Attribute::AttrKind>(i);
				if (ab.Contains(kind))
				{
					switch (kind)
					{
					case Attribute::AK_Alignment:
						attrs.push_back(std::make_pair(index, Attribute::GetWithAlignment(context, ab.Alignment())));
						break;
					case Attribute::AK_StackAlignment:
						attrs.push_back(std::make_pair(index, Attribute::GetWithStackAlignment(context, ab.StackAlignment())));
						break;
					case Attribute::AK_Dereferenceable:
						attrs.push_back(std::make_pair(index,
							Attribute::GetWithDereferenceableBytes(context, ab.DereferenceableBytes())));
						break;
					case Attribute::AK_DereferenceableOrNull:
						attrs.push_back(std::make_pair(index,
							Attribute::GetWithDereferenceableOrNullBytes(context, ab.DereferenceableOrNullBytes())));
						break;

					default:
						attrs.push_back(std::make_pair(index, Attribute::Get(context, kind)));
					}
				}
			}

			for (auto const & tda : ab.TDAttrs())
			{
				attrs.push_back(std::make_pair(index, Attribute::Get(context, tda.first, tda.second)));
			}

			return AttributeSet::Get(context, attrs);
		}
	}

	AttributeSet AttributeSet::Get(LLVMContext& context, ArrayRef<std::pair<uint32_t, Attribute>> attrs)
	{
		if (attrs.empty())
		{
			return AttributeSet();
		}
		else
		{
#ifdef DILITHIUM_DEBUG
			for (size_t i = 0, e = attrs.size(); i != e; ++ i)
			{
				BOOST_ASSERT_MSG(!i || (attrs[i - 1].first <= attrs[i].first), "Misordered Attributes list!");
				BOOST_ASSERT_MSG(!attrs[i].second.HasAttribute(Attribute::AK_None), "Pointless attribute!");
			}
#endif

			boost::container::small_vector<std::pair<unsigned, AttributeSetNode*>, 8> attr_pair_vec;
			for (auto iter = attrs.begin(), end_iter = attrs.end(); iter != end_iter;)
			{
				uint32_t index = iter->first;
				boost::container::small_vector<Attribute, 4> attr_vec;
				while ((iter != end_iter) && (iter->first == index))
				{
					attr_vec.push_back(iter->second);
					++ iter;
				}

				attr_pair_vec.push_back(std::make_pair(index, AttributeSetNode::Get(context, attr_vec)));
			}

			return GetImpl(context, attr_pair_vec);
		}
	}

	AttributeSet AttributeSet::Get(LLVMContext& context, ArrayRef<std::pair<uint32_t, AttributeSetNode*>> attrs)
	{
		if (attrs.empty())
		{
			return AttributeSet();
		}
		else
		{
			return GetImpl(context, attrs);
		}
	}

	AttributeSet AttributeSet::GetImpl(LLVMContext& context, ArrayRef<std::pair<uint32_t, AttributeSetNode*>> attrs)
	{
		auto& context_impl = context.Impl();

		size_t hash_val = 0;
		for (auto iter = attrs.begin(), end_iter = attrs.end(); iter != end_iter; ++ iter)
		{
			boost::hash_combine(hash_val, iter->first);
			boost::hash_combine(hash_val, iter->second);
		}

		auto iter = context_impl.attrs_lists.find(hash_val);
		if (iter == context_impl.attrs_lists.end())
		{
			auto pa = std::make_unique<AttributeSetImpl>(context, attrs);
			iter = context_impl.attrs_lists.emplace(hash_val, std::move(pa)).first;
		}

		return AttributeSet(iter->second.get());
	}

	AttributeSet::iterator AttributeSet::Begin(uint32_t slot) const
	{
		DILITHIUM_UNUSED(slot);

		if (!impl_)
		{
			return ArrayRef<Attribute>().begin();
		}
		return impl_->Begin(slot);
	}

	AttributeSet::iterator AttributeSet::End(uint32_t slot) const
	{
		DILITHIUM_UNUSED(slot);

		if (!impl_)
		{
			return ArrayRef<Attribute>().end();
		}
		return impl_->End(slot);
	}

	uint32_t AttributeSet::NumSlots() const
	{
		return impl_ ? impl_->NumAttributes() : 0;
	}

	uint32_t AttributeSet::SlotIndex(uint32_t slot) const
	{
		BOOST_ASSERT_MSG(impl_ && slot < impl_->NumAttributes(), "Slot # out of range!");
		return impl_->SlotIndex(slot);
	}


	AttrBuilder::AttrBuilder()
		: attrs_(0), alignment_(0), stack_alignment_(0), deref_bytes_(0), deref_or_null_bytes_(0)
	{
	}

	AttrBuilder::AttrBuilder(uint64_t val)
		: attrs_(0), alignment_(0), stack_alignment_(0), deref_bytes_(0), deref_or_null_bytes_(0)
	{
		this->AddRawValue(val);
	}

	AttrBuilder::AttrBuilder(Attribute const & attr)
		: attrs_(0), alignment_(0), stack_alignment_(0), deref_bytes_(0), deref_or_null_bytes_(0)
	{
		this->AddAttribute(attr);
	}

	AttrBuilder::AttrBuilder(AttributeSet const & as, uint32_t index)
		: attrs_(0), alignment_(0), stack_alignment_(0), deref_bytes_(0), deref_or_null_bytes_(0)
	{
		auto impl = as.impl_;
		if (!impl)
		{
			return;
		}

		for (uint32_t i = 0, e = impl->NumAttributes(); i != e; ++ i)
		{
			if (impl->SlotIndex(i) == index)
			{
				for (auto iter = impl->Begin(i), end_iter = impl->End(i); iter != end_iter; ++ iter)
				{
					this->AddAttribute(*iter);
				}
				break;
			}
		}
	}

	AttrBuilder& AttrBuilder::AddAttribute(Attribute::AttrKind val)
	{
		BOOST_ASSERT_MSG(val < Attribute::AK_EndAttrKinds, "Attribute out of range!");
		BOOST_ASSERT_MSG((val != Attribute::AK_Alignment) && (val != Attribute::AK_StackAlignment)
			&& (val != Attribute::AK_Dereferenceable), "Adding integer attribute without adding a value!");
		attrs_[val] = true;
		return *this;
	}

	AttrBuilder& AttrBuilder::AddAttribute(Attribute const & attr)
	{
		DILITHIUM_UNUSED(attr);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddAttribute(std::string_view attr, std::string_view val)
	{
		DILITHIUM_UNUSED(attr);
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool AttrBuilder::Contains(std::string_view attr) const
	{
		return target_dep_attrs_.find(attr.to_string()) != target_dep_attrs_.end();
	}

	bool AttrBuilder::HasAttributes() const
	{
		return !attrs_.none() || !target_dep_attrs_.empty();
	}

	bool AttrBuilder::HasAttributes(AttributeSet const & as, uint64_t index) const
	{
		uint32_t slot = ~0U;
		for (uint32_t i = 0, e = as.NumSlots(); i != e; ++ i)
		{
			if (as.SlotIndex(i) == index)
			{
				slot = i;
				break;
			}
		}

		BOOST_ASSERT_MSG(slot != ~0U, "Couldn't find the index!");

		for (auto iter = as.Begin(slot), end_iter = as.End(slot); iter != end_iter; ++ iter)
		{
			Attribute attr = *iter;
			if (attr.IsEnumAttribute() || attr.IsIntAttribute())
			{
				if (attrs_[iter->KindAsEnum()])
				{
					return true;
				}
			}
			else
			{
				BOOST_ASSERT_MSG(attr.IsStringAttribute(), "Invalid attribute kind!");
				return target_dep_attrs_.find(attr.KindAsString().to_string()) != target_dep_attrs_.end();
			}
		}

		return false;
	}

	AttrBuilder& AttrBuilder::AddAlignmentAttr(uint32_t align)
	{
		DILITHIUM_UNUSED(align);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddStackAlignmentAttr(uint32_t align)
	{
		DILITHIUM_UNUSED(align);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddDereferenceableAttr(uint64_t bytes)
	{
		DILITHIUM_UNUSED(bytes);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddDereferenceableOrNullAttr(uint64_t bytes)
	{
		DILITHIUM_UNUSED(bytes);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddRawValue(uint64_t val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
