/**
 * @file Metadata.cpp
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
#include <Dilithium/Casting.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Metadata.hpp>
#include "LLVMContextImpl.hpp"

namespace
{
	using namespace Dilithium;

	static Metadata* CanonicalizeMetadataForValue(LLVMContext& context, Metadata* md)
	{
		if (!md)
		{
			return MDNode::Get(context, ArrayRef<Metadata*>());
		}
		else
		{
			auto mdn = dyn_cast<MDNode>(md);
			if (!mdn || mdn->NumOperands() != 1)
			{
				return md;
			}
			else
			{
				if (!mdn->Operand(0).Get())
				{
					return MDNode::Get(context, ArrayRef<Metadata*>());
				}
				else
				{
					auto c = dyn_cast<ConstantAsMetadata>(mdn->Operand(0));
					if (c)
					{
						return c;
					}
					else
					{
						return md;
					}
				}
			}
		}
	}

	Function* LocalFunction(Value* val)
	{
		BOOST_ASSERT_MSG(val, "Expected value");
		auto arg = dyn_cast<Argument>(val);
		if (arg)
		{
			return arg->Parent();
		}
		else
		{
			auto bb = cast<Instruction>(val)->Parent();
			if (bb)
			{
				return bb->Parent();
			}
			else
			{
				return nullptr;
			}
		}
	}

	bool IsOperandUnresolved(Metadata* op)
	{
		auto mdn = dyn_cast_or_null<MDNode>(op);
		if (mdn)
		{
			return !mdn->IsResolved();
		}
		return false;
	}

	bool HasSelfReference(MDNode* node)
	{
		for (Metadata* md : node->Operands())
		{
			if (md == node)
			{
				return true;
			}
		}
		return false;
	}

	template <typename T>
	static T* GetUniqued(std::unordered_map<uint64_t, T*>& Store, uint64_t key)
	{
		auto iter = Store.find(key);
		return iter == Store.end() ? nullptr : iter->second;
	}

	template <typename T>
	static T* UniquifyImpl(T* n, std::unordered_map<uint64_t, T*>& store)
	{
		uint64_t hash_val = std::hash<T>()(*n);
		T* u = GetUniqued(store, hash_val);
		if (u)
		{
			return u;
		}
		else
		{
			store.emplace(hash_val, n);
			return n;
		}
	}
}

namespace Dilithium 
{
	Metadata::Metadata(uint8_t id, StorageType storage)
		: subclass_id_(id), storage_(storage), subclass_data_16_(0), subclass_data_32_(0)
	{
	}
	
	Metadata::~Metadata()
	{
	}

	void Metadata::HandleChangedOperand(void* ref, Metadata* new_ref)
	{
		DILITHIUM_UNUSED(ref);
		DILITHIUM_UNUSED(new_ref);
		DILITHIUM_UNREACHABLE("Unimplemented in Metadata subclass");
	}


	MetadataAsValue::MetadataAsValue(Type* ty, Metadata* md)
		: Value(ty, MetadataAsValueVal), md_(md)
	{
		this->Track();
	}

	MetadataAsValue::~MetadataAsValue()
	{
		this->GetType()->Context().Impl().metadata_as_values.erase(md_);
		this->Untrack();
	}
	
	MetadataAsValue* MetadataAsValue::Get(LLVMContext& context, Metadata* md)
	{
		md = CanonicalizeMetadataForValue(context, md);
		auto& entry = context.Impl().metadata_as_values[md];
		if (!entry)
		{
			entry = new MetadataAsValue(Type::MetadataType(context), md);
		}
		return entry;
	}

	void MetadataAsValue::HandleChangedMetadata(Metadata* md)
	{
		auto& context = this->Context();
		md = CanonicalizeMetadataForValue(context, md);
		auto& store = context.Impl().metadata_as_values;

		store.erase(md_);
		this->Untrack();
		md_ = nullptr;

		auto& entry = store[md_];
		if (entry)
		{
			this->ReplaceAllUsesWith(entry);
			delete this;
		}
		else
		{
			md_ = md;
			this->Track();
			entry = this;
		}
	}

	void MetadataAsValue::Track()
	{
		if (md_)
		{
			MetadataTracking::track(&md_, *md_, *this);
		}
	}

	void MetadataAsValue::Untrack()
	{
		if (md_)
		{
			MetadataTracking::Untrack(md_);
		}
	}


	ReplaceableMetadataImpl::ReplaceableMetadataImpl(LLVMContext& context)
		: context_(context), next_index_(0)
	{
	}
	
	ReplaceableMetadataImpl::~ReplaceableMetadataImpl()
	{
		BOOST_ASSERT_MSG(use_map_.empty(), "Cannot destroy in-use replaceable metadata");
	}

	void ReplaceableMetadataImpl::ReplaceAllUsesWith(Metadata* md)
	{
		BOOST_ASSERT_MSG(!(md && isa<MDNode>(md) && cast<MDNode>(md)->IsTemporary()), "Expected non-temp node");

		if (!use_map_.empty())
		{
			typedef std::pair<void *, std::pair<OwnerTy, uint64_t>> UseTy;
			boost::container::small_vector<UseTy, 8> uses(use_map_.begin(), use_map_.end());
			std::sort(uses.begin(), uses.end(), [](UseTy const & lhs, UseTy const & rhs)
				{
					return lhs.second.second < rhs.second.second;
				});
			for (auto const & pair : uses)
			{
				if (use_map_.find(pair.first) == use_map_.end())
				{
					continue;
				}

				OwnerTy owner = pair.second.first;
				if (!owner)
				{
					auto& ref = *static_cast<Metadata **>(pair.first);
					ref = md;
					if (md)
					{
						MetadataTracking::Track(ref);
					}
					use_map_.erase(pair.first);
					continue;
				}

				if (owner.Is<MetadataAsValue*>())
				{
					owner.Get<MetadataAsValue*>()->HandleChangedMetadata(md);
					continue;
				}

				Metadata* owner_md = owner.Get<Metadata*>();
				switch (owner_md->MetadataId())
				{
#define HANDLE_METADATA_LEAF(CLASS)													\
				case Metadata::CLASS##Kind:											\
					cast<CLASS>(owner_md)->HandleChangedOperand(pair.first, md);	\
					continue;
#include "Dilithium/Metadata.inc"
				default:
					DILITHIUM_UNREACHABLE("Invalid metadata subclass");
				}
			}
			BOOST_ASSERT_MSG(use_map_.empty(), "Expected all uses to be replaced");
		}
	}

	void ReplaceableMetadataImpl::ResolveAllUses(bool resolve_users)
	{
		if (!use_map_.empty())
		{
			if (!resolve_users)
			{
				use_map_.clear();
				return;
			}

			typedef std::pair<void *, std::pair<OwnerTy, uint64_t>> UseTy;
			boost::container::small_vector<UseTy, 8> uses(use_map_.begin(), use_map_.end());
			std::sort(uses.begin(), uses.end(), [](UseTy const & lhs, UseTy const & rhs)
				{
					return lhs.second.second < rhs.second.second;
				});
			use_map_.clear();
			for (auto const & pair : uses)
			{
				auto owner = pair.second.first;
				if (!owner)
				{
					continue;
				}
				if (owner.Is<MetadataAsValue*>())
				{
					continue;
				}

				auto owner_md = dyn_cast<MDNode>(owner.Get<Metadata*>());
				if (!owner_md)
				{
					continue;
				}
				if (owner_md->IsResolved())
				{
					continue;
				}
				owner_md->DecrementUnresolvedOperandCount();
			}
		}
	}

	void ReplaceableMetadataImpl::AddRef(void* ref, OwnerTy owner)
	{
		bool was_inserted = use_map_.insert(std::make_pair(ref, std::make_pair(owner, next_index_))).second;
		DILITHIUM_UNUSED(was_inserted);
		BOOST_ASSERT_MSG(was_inserted, "Expected to add a reference");

		++ next_index_;
		BOOST_ASSERT_MSG(next_index_ != 0, "Unexpected overflow");
	}

	void ReplaceableMetadataImpl::DropRef(void* ref)
	{
		bool was_erased = use_map_.erase(ref);
		DILITHIUM_UNUSED(was_erased);
		BOOST_ASSERT_MSG(was_erased, "Expected to drop a reference");
	}

	void ReplaceableMetadataImpl::MoveRef(void* old_ref, void* new_ref, Metadata const & md)
	{
		auto iter = use_map_.find(old_ref);
		BOOST_ASSERT_MSG(iter != use_map_.end(), "Expected to move a reference");
		auto owner_and_index = iter->second;
		use_map_.erase(iter);
		bool was_inserted = use_map_.insert(std::make_pair(new_ref, owner_and_index)).second;
		DILITHIUM_UNUSED(was_inserted);
		BOOST_ASSERT_MSG(was_inserted, "Expected to add a reference");

		DILITHIUM_UNUSED(md);
		BOOST_ASSERT_MSG((owner_and_index.first || *static_cast<Metadata**>(old_ref) == &md), "Reference without owner must be direct");
		BOOST_ASSERT_MSG((owner_and_index.first || *static_cast<Metadata**>(new_ref) == &md), "Reference without owner must be direct");
	}

	ReplaceableMetadataImpl* ReplaceableMetadataImpl::Get(Metadata& md)
	{
		auto mdn = dyn_cast<MDNode>(&md);
		if (mdn)
		{
			return mdn->context_.ReplaceableUses();
		}
		return dyn_cast<ValueAsMetadata>(&md);
	}


	ValueAsMetadata::ValueAsMetadata(uint8_t id, Value* val)
		: Metadata(id, Uniqued), ReplaceableMetadataImpl(val->Context()),
			val_(val)
	{
		BOOST_ASSERT_MSG(val_, "Expected valid value");
	}

	ValueAsMetadata::~ValueAsMetadata()
	{
	}

	ValueAsMetadata* ValueAsMetadata::Get(Value* val)
	{
		BOOST_ASSERT_MSG(val, "Unexpected null Value");

		auto& context = val->Context();
		auto& entry = context.Impl().values_as_metadata[val];
		if (!entry)
		{
			BOOST_ASSERT_MSG(isa<Constant>(val) || isa<Argument>(val) || isa<Instruction>(val),
				"Expected constant or function-local value");
			BOOST_ASSERT_MSG(!val->is_used_by_md_, "Expected this to be the only metadata use");
			val->is_used_by_md_ = true;
			auto c = dyn_cast<Constant>(val);
			if (c)
			{
				entry = new ConstantAsMetadata(c);
			}
			else
			{
				entry = new LocalAsMetadata(val);
			}
		}

		return entry;
	}

	void ValueAsMetadata::HandleDeletion(Value* val)
	{
		BOOST_ASSERT_MSG(val, "Expected valid value");

		auto& store = val->GetType()->Context().Impl().values_as_metadata;
		auto iter = store.find(val);
		if (iter != store.end())
		{
			auto md = iter->second;
			BOOST_ASSERT_MSG(md, "Expected valid metadata");
			BOOST_ASSERT_MSG(md->GetValue() == val, "Expected valid mapping");
			store.erase(iter);

			md->ReplaceAllUsesWith(nullptr);
			delete md;
		}
	}

	void ValueAsMetadata::HandleRAUW(Value* from, Value* to)
	{
		BOOST_ASSERT_MSG(from, "Expected valid value");
		BOOST_ASSERT_MSG(to, "Expected valid value");
		BOOST_ASSERT_MSG(from != to, "Expected changed value");
		BOOST_ASSERT_MSG(from->GetType() == to->GetType(), "Unexpected type change");

		auto& context = from->GetType()->Context();
		auto& store = context.Impl().values_as_metadata;
		auto iter = store.find(from);
		if (iter == store.end())
		{
			BOOST_ASSERT_MSG(!from->is_used_by_md_, "Expected From not to be used by metadata");
			return;
		}
		else
		{
			BOOST_ASSERT_MSG(from->is_used_by_md_, "Expected From to be used by metadata");
			from->is_used_by_md_ = false;
			auto md = iter->second;
			BOOST_ASSERT_MSG(md, "Expected valid metadata");
			BOOST_ASSERT_MSG(md->GetValue() == from, "Expected valid mapping");
			store.erase(iter);

			if (isa<LocalAsMetadata>(md))
			{
				auto c = dyn_cast<Constant>(to);
				if (c)
				{
					// Local became a constant.
					md->ReplaceAllUsesWith(ConstantAsMetadata::Get(c));
					delete md;
					return;
				}
				if (LocalFunction(from) && LocalFunction(to) && (LocalFunction(from) != LocalFunction(to)))
				{
					md->ReplaceAllUsesWith(nullptr);
					delete md;
					return;
				}
			}
			else if (!isa<Constant>(to))
			{
				md->ReplaceAllUsesWith(nullptr);
				delete md;
				return;
			}

			auto& entry = store[to];
			if (entry)
			{
				md->ReplaceAllUsesWith(entry);
				delete md;
				return;
			}

			BOOST_ASSERT_MSG(!to->is_used_by_md_, "Expected this to be the only metadata use");
			to->is_used_by_md_ = true;
			md->val_ = to;
			entry = md;
		}
	}


	MDString::MDString()
		: Metadata(MDStringKind, Uniqued), string_hash_(0)
	{
	}

	MDString::MDString(MDString&& rhs)
		: Metadata(MDStringKind, Uniqued)
	{
		DILITHIUM_UNUSED(rhs);
	}

	MDString* MDString::Get(LLVMContext& context, std::string_view str)
	{
		auto& store = context.Impl().md_string_cache;
		uint64_t hash_val = boost::hash_value(str);
		auto iter = store.find(hash_val);
		if (iter == store.end())
		{
			auto mds = std::make_unique<MDString>();
			mds->string_ = std::string(str);
			mds->string_hash_ = hash_val;

			bool was_inserted;
			std::tie(iter, was_inserted) = store.emplace(hash_val, std::move(mds));
			DILITHIUM_UNUSED(was_inserted);
			BOOST_ASSERT_MSG(was_inserted, "Expected entry to be inserted");
		}
		return iter->second.get();
	}

	std::string_view MDString::String() const
	{
		BOOST_ASSERT_MSG(!string_.empty(), "Expected to find string map entry");
		return string_;
	}


	MDOperand::MDOperand()
		: md_(nullptr)
	{
	}

	MDOperand::MDOperand(MDOperand&& rhs)
		: md_(std::move(rhs.md_))
	{
	}

	MDOperand::~MDOperand()
	{
		this->Untrack();
	}

	void MDOperand::Reset()
	{
		this->Untrack();
		md_ = nullptr;
	}

	void MDOperand::Reset(Metadata* md, Metadata* owner)
	{
		this->Untrack();
		md_ = md;
		this->Track(owner);
	}

	MDOperand& MDOperand::operator=(MDOperand&& rhs)
	{
		if (this != &rhs)
		{
			md_ = std::move(rhs.md_);
		}
		return *this;
	}

	void MDOperand::Track(Metadata* owner)
	{
		if (md_)
		{
			if (owner)
			{
				MetadataTracking::Track(this, *md_, *owner);
			}
			else
			{
				MetadataTracking::Track(md_);
			}
		}
	}

	void MDOperand::Untrack()
	{
		if (md_)
		{
			MetadataTracking::Untrack(md_);
		}
	}


	ContextAndReplaceableUses::ContextAndReplaceableUses(LLVMContext& context)
		: ptr_(&context)
	{
	}

	ContextAndReplaceableUses::ContextAndReplaceableUses(std::unique_ptr<ReplaceableMetadataImpl> replaceable_uses)
		: ptr_(replaceable_uses.release())
	{
		BOOST_ASSERT_MSG(this->ReplaceableUses(), "Expected non-null replaceable uses");
	}

	ContextAndReplaceableUses::~ContextAndReplaceableUses()
	{
		delete this->ReplaceableUses();
	}

	LLVMContext& ContextAndReplaceableUses::Context() const
	{
		if (this->HasReplaceableUses())
		{
			return this->ReplaceableUses()->Context();
		}
		return *ptr_.Get<LLVMContext*>();
	}

	ReplaceableMetadataImpl* ContextAndReplaceableUses::ReplaceableUses() const
	{
		if (this->HasReplaceableUses())
		{
			return ptr_.Get<ReplaceableMetadataImpl*>();
		}
		return nullptr;
	}

	void ContextAndReplaceableUses::MakeReplaceable(std::unique_ptr<ReplaceableMetadataImpl> replaceable_uses)
	{
		BOOST_ASSERT_MSG(replaceable_uses, "Expected non-null replaceable uses");
		BOOST_ASSERT_MSG(&replaceable_uses->Context() == &this->Context(), "Expected same context");
		delete this->ReplaceableUses();
		ptr_ = replaceable_uses.release();
	}

	std::unique_ptr<ReplaceableMetadataImpl> ContextAndReplaceableUses::TakeReplaceableUses()
	{
		BOOST_ASSERT_MSG(this->HasReplaceableUses(), "Expected to own replaceable uses");
		std::unique_ptr<ReplaceableMetadataImpl> replaceable_uses(this->ReplaceableUses());
		ptr_ = &replaceable_uses->Context();
		return replaceable_uses;
	}


	void TempMDNodeDeleter::operator()(MDNode* node) const
	{
		DILITHIUM_UNUSED(node);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	MDNode::MDNode(LLVMContext& context, uint8_t id, StorageType storage, ArrayRef<Metadata*> ops1, ArrayRef<Metadata*> ops2)
		: Metadata(id, storage), operands_(ops1.size() + ops2.size()), num_unresolved_(0), context_(context)
	{
		uint32_t op = 0;
		for (auto md : ops1)
		{
			this->Operand(op, md);
			++ op;
		}
		for (auto md : ops2)
		{
			this->Operand(op, md);
			++ op;
		}

		if (!this->IsDistinct())
		{
			if (this->IsUniqued())
			{
				if (!this->CountUnresolvedOperands())
				{
					return;
				}
			}

			context_.MakeReplaceable(std::make_unique<ReplaceableMetadataImpl>(context));
		}
	}

	MDNode::~MDNode()
	{
	}

	MDTuple* MDNode::Get(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		return MDTuple::Get(context, mds);
	}

	MDTuple* MDNode::GetIfExists(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		return MDTuple::GetIfExists(context, mds);
	}

	MDTuple* MDNode::GetDistinct(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		return MDTuple::GetDistinct(context, mds);
	}

	void MDNode::ReplaceAllUsesWith(Metadata* md)
	{
		BOOST_ASSERT_MSG(this->IsTemporary(), "Expected temporary node");
		BOOST_ASSERT_MSG(!this->IsResolved(), "Expected RAUW support");
		context_.ReplaceableUses()->ReplaceAllUsesWith(md);
	}

	void MDNode::DropAllReferences()
	{
		for (uint32_t i = 0, e = static_cast<uint32_t>(operands_.size()); i != e; ++ i)
		{
			this->Operand(i, nullptr);
		}
		if (!this->IsResolved())
		{
			context_.ReplaceableUses()->ResolveAllUses(false);
			context_.TakeReplaceableUses();
		}
	}

	void MDNode::Operand(uint32_t idx, Metadata* md)
	{
		BOOST_ASSERT(idx < operands_.size());
		operands_[idx].Reset(md, this->IsUniqued() ? this : nullptr);
	}

	void MDNode::StoreDistinctInContext()
	{
		BOOST_ASSERT_MSG(this->IsResolved(), "Expected resolved nodes");
		storage_ = Distinct;

		switch (this->MetadataId())
		{
		default:
			DILITHIUM_UNREACHABLE("Invalid subclass of MDNode");

#define HANDLE_MDNODE_LEAF(CLASS)																\
		case CLASS##Kind:																		\
			{																					\
				std::integral_constant<bool, HasCachedHash<CLASS>::value> should_reset_hash;	\
				this->DispatchResetHash(cast<CLASS>(this), should_reset_hash);					\
				break;																			\
			}
#include "Dilithium/Metadata.inc"
		}

		context_.Context().Impl().distinct_md_nodes.insert(this);
	}

	template <typename T, typename StoreT>
	T* MDNode::StoreImpl(T* n, StorageType storage, StoreT& store)
	{
		switch (storage)
		{
		case Uniqued:
			store.emplace(std::hash<T>()(*n), n);
			break;
		case Distinct:
			n->StoreDistinctInContext();
			break;
		case Temporary:
			break;
		}
		return n;
	}

	void MDNode::HandleChangedOperand(void* ref, Metadata* new_op)
	{
		uint32_t op_idx = static_cast<uint32_t>(static_cast<MDOperand*>(ref) - this->OpBegin());
		BOOST_ASSERT_MSG(op_idx < this->NumOperands(), "Expected valid operand");

		if (!this->IsUniqued())
		{
			Operand(op_idx, new_op);
		}
		else
		{
			this->EraseFromStore();

			Metadata* old = this->Operand(op_idx);
			this->Operand(op_idx, new_op);

			if (new_op == this)
			{
				if (!this->IsResolved())
				{
					this->Resolve();
				}
				this->StoreDistinctInContext();
			}
			else
			{
				auto uniqued = this->Uniquify();
				if (uniqued == this)
				{
					if (!this->IsResolved())
					{
						this->ResolveAfterOperandChange(old, new_op);
					}
				}
				else
				{
					if (!this->IsResolved())
					{
						for (uint32_t j = 0, e = this->NumOperands(); j != e; ++j)
						{
							this->Operand(j, nullptr);
						}
						context_.ReplaceableUses()->ReplaceAllUsesWith(uniqued);
						this->DeleteAsSubclass();
					}
					else
					{
						this->StoreDistinctInContext();
					}
				}
			}
		}
	}

	void MDNode::Resolve()
	{
		BOOST_ASSERT_MSG(this->IsUniqued(), "Expected this to be uniqued");
		BOOST_ASSERT_MSG(!this->IsResolved(), "Expected this to be unresolved");

		auto uses = context_.TakeReplaceableUses();
		num_unresolved_ = 0;
		BOOST_ASSERT_MSG(this->IsResolved(), "Expected this to be resolved");

		uses->ResolveAllUses();
	}

	void MDNode::ResolveAfterOperandChange(Metadata* old, Metadata* new_op)
	{
		BOOST_ASSERT_MSG(num_unresolved_ != 0, "Expected unresolved operands");

		if (!IsOperandUnresolved(old))
		{
			if (IsOperandUnresolved(new_op))
			{
				++ num_unresolved_;
			}
		}
		else if (!IsOperandUnresolved(new_op))
		{
			this->DecrementUnresolvedOperandCount();
		}
	}

	void MDNode::DecrementUnresolvedOperandCount()
	{
		-- num_unresolved_;
		if (!num_unresolved_)
		{
			this->Resolve();
		}
	}

	uint32_t MDNode::CountUnresolvedOperands()
	{
		BOOST_ASSERT_MSG(num_unresolved_ == 0, "Expected unresolved ops to be uncounted");
		num_unresolved_ = static_cast<uint32_t>(std::count_if(operands_.begin(), operands_.end(), IsOperandUnresolved));
		return num_unresolved_;
	}

	void MDNode::DeleteAsSubclass()
	{
		switch (this->MetadataId())
		{
		default:
			DILITHIUM_UNREACHABLE("Invalid subclass of MDNode");

#define HANDLE_MDNODE_LEAF(CLASS)						\
		case CLASS##Kind:								\
			delete cast<CLASS>(this);					\
			break;
#include "Dilithium/Metadata.inc"
		}
	}

	MDNode* MDNode::Uniquify()
	{
		BOOST_ASSERT_MSG(!HasSelfReference(this), "Cannot uniquify a self-referencing node");

		switch (this->MetadataId())
		{
		default:
			DILITHIUM_UNREACHABLE("Invalid subclass of MDNode");

#define HANDLE_MDNODE_LEAF(CLASS)																	\
		case CLASS##Kind:																			\
			{																						\
				CLASS* subclass_this = cast<CLASS>(this);											\
				std::integral_constant<bool, HasCachedHash<CLASS>::value> should_recalculate_hash;	\
				this->DispatchRecalculateHash(subclass_this, should_recalculate_hash);				\
				return UniquifyImpl(subclass_this, context_.Context().Impl().CLASS##s);				\
			}
#include "Dilithium/Metadata.inc"
		}
	}

	void MDNode::EraseFromStore()
	{
	}


	MDTuple::MDTuple(LLVMContext& context, StorageType storage, uint32_t hash, ArrayRef<Metadata*> vals)
		: MDNode(context, MDTupleKind, storage, vals)
	{
		this->Hash(hash);
	}

	MDTuple::~MDTuple()
	{
		this->DropAllReferences();
	}

	void MDTuple::RecalculateHash()
	{
		this->Hash(static_cast<uint32_t>(boost::hash_range(this->OpBegin(), this->OpEnd())));
	}

	MDTuple* MDTuple::GetImpl(LLVMContext& context, ArrayRef<Metadata*> mds, StorageType storage, bool should_create)
	{
		uint32_t hash = 0;
		if (storage == Uniqued)
		{
			uint64_t hash_val = boost::hash_range(mds.begin(), mds.end());
			auto mdt = GetUniqued(context.Impl().MDTuples, hash_val);
			if (mdt)
			{
				return mdt;
			}
			if (!should_create)
			{
				return nullptr;
			}
			hash = static_cast<uint32_t>(hash_val);
		}
		else
		{
			BOOST_ASSERT_MSG(should_create, "Expected non-uniqued nodes to always be created");
		}

		return StoreImpl(new MDTuple(context, storage, hash, mds), storage, context.Impl().MDTuples);
	}


	NamedMDNode::NamedMDNode(std::string_view name)
		: name_(std::string(name)), parent_(nullptr)
	{
	}

	NamedMDNode::~NamedMDNode()
	{
		this->DropAllReferences();
	}

	void NamedMDNode::DropAllReferences()
	{
		operands_.clear();
	}

	MDNode* NamedMDNode::Operand(uint32_t idx) const
	{
		BOOST_ASSERT_MSG(idx < this->NumOperands(), "Invalid Operand number!");
		auto n = operands_[idx].Get();
		return cast_or_null<MDNode>(n);
	}

	void NamedMDNode::Operand(uint32_t idx, MDNode* new_op)
	{
		BOOST_ASSERT_MSG(idx < this->NumOperands(), "Invalid operand number");
		operands_[idx].Reset(new_op);
	}

	uint32_t NamedMDNode::NumOperands() const
	{
		return static_cast<uint32_t>(operands_.size());
	}

	void NamedMDNode::AddOperand(MDNode* mdn)
	{
		operands_.emplace_back(mdn);
	}
}
