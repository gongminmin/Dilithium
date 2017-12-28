/**
 * @file Metadata.hpp
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

#ifndef _DILITHIUM_METADATA_HPP
#define _DILITHIUM_METADATA_HPP

#pragma once

#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/Casting.hpp>
#include <Dilithium/MetadataTracking.hpp>
#include <Dilithium/TrackingMDRef.hpp>
#include <Dilithium/Value.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/core/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	class MDNode;
	class MDTuple;

	class Metadata
	{
		friend class ReplaceableMetadataImpl;

	public:
		enum MetadataKind
		{
			MDTupleKind,
			ConstantAsMetadataKind,
			LocalAsMetadataKind,
			MDStringKind
		};

	public:
		uint32_t MetadataId() const
		{
			return subclass_id_;
		}

	protected:
		enum StorageType
		{
			Uniqued,
			Distinct,
			Temporary
		};

	protected:
		Metadata(uint8_t id, StorageType storage);
		~Metadata();

		void HandleChangedOperand(void* ref, Metadata* new_ref);

	protected:
		uint32_t storage_ : 2;

		uint16_t subclass_data_16_;
		uint32_t subclass_data_32_;

	private:
		uint8_t const subclass_id_;

		// DILITHIUM_NOT_IMPLEMENTED
	};

#define HANDLE_METADATA(CLASS) class CLASS;
#include "Metadata.inc"

#define HANDLE_METADATA_LEAF(CLASS)								\
	template <>													\
	struct isa_impl<CLASS, Metadata>							\
	{															\
		static bool doit(Metadata const & md)					\
		{														\
			return md.MetadataId() == Metadata::CLASS##Kind;	\
		}														\
	};
#include "Metadata.inc"

	class MetadataAsValue : public Value
	{
		friend class ReplaceableMetadataImpl;

	public:
		static MetadataAsValue* Get(LLVMContext& context, Metadata* md);
		Metadata* GetMetadata() const
		{
			return md_;
		}

		static bool classof(Value const * v)
		{
			return v->GetValueId() == MetadataAsValueVal;
		}

	private:
		MetadataAsValue(Type* ty, Metadata* md);
		~MetadataAsValue() override;

		void HandleChangedMetadata(Metadata* md);
		void Track();
		void Untrack();

	private:
		Metadata* md_;

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ReplaceableMetadataImpl
	{
		friend class MetadataTracking;

	public:
		typedef MetadataTracking::OwnerTy OwnerTy;

	public:
		ReplaceableMetadataImpl(LLVMContext& context);
		~ReplaceableMetadataImpl();

		LLVMContext& Context() const
		{
			return context_;
		}

		void ReplaceAllUsesWith(Metadata* md);
		void ResolveAllUses(bool resolve_users = true);

	private:
		void AddRef(void* ref, OwnerTy owner);
		void DropRef(void* ref);
		void MoveRef(void* old_ref, void* new_ref, Metadata const & md);

		static ReplaceableMetadataImpl* Get(Metadata& md);

	private:
		LLVMContext& context_;
		uint64_t next_index_;
		// TODO: Normally less than 4
		std::unordered_map<void*, std::pair<OwnerTy, uint64_t>> use_map_;
	};

	class ValueAsMetadata : public Metadata, ReplaceableMetadataImpl
	{
		friend class ReplaceableMetadataImpl;

	public:
		static ValueAsMetadata* Get(Value* val);
		static ConstantAsMetadata* GetConstant(Value* c)
		{
			return cast<ConstantAsMetadata>(Get(c));
		}
		static LocalAsMetadata* GetLocal(Value* local)
		{
			return cast<LocalAsMetadata>(Get(local));
		}

		static ValueAsMetadata* GetIfExists(Value* val);
		static ConstantAsMetadata* GetConstantIfExists(Value* c)
		{
			return cast_or_null<ConstantAsMetadata>(ValueAsMetadata::GetIfExists(c));
		}
		static LocalAsMetadata* GetLocalIfExists(Value* local)
		{
			return cast_or_null<LocalAsMetadata>(ValueAsMetadata::GetIfExists(local));
		}

		Value* GetValue() const
		{
			return val_;
		}
		Type* GetType() const
		{
			return val_->GetType();
		}
		LLVMContext& Context() const
		{
			return val_->Context();
		}

		static void HandleDeletion(Value* val);
		static void HandleRAUW(Value* from, Value* to);

		static bool classof(Metadata const * md)
		{
			return (md->MetadataId() == LocalAsMetadataKind) || (md->MetadataId() == ConstantAsMetadataKind);
		}

	protected:
		ValueAsMetadata(uint8_t id, Value* val);
		~ValueAsMetadata();

	private:
		void DropUsers()
		{
			ReplaceableMetadataImpl::ResolveAllUses(false);
		}

	private:
		Value* val_;
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ConstantAsMetadata : public ValueAsMetadata
	{
		friend class ValueAsMetadata;

	public:
		static ConstantAsMetadata* Get(Constant* c)
		{
			return ValueAsMetadata::GetConstant(c);
		}
		static ConstantAsMetadata* GetIfExists(Constant* c)
		{
			return ValueAsMetadata::GetConstantIfExists(c);
		}

		Constant* GetValue() const
		{
			return cast<Constant>(ValueAsMetadata::GetValue());
		}

		static bool classof(Metadata const * md)
		{
			return md->MetadataId() == ConstantAsMetadataKind;
		}

	private:
		explicit ConstantAsMetadata(Constant* c)
			: ValueAsMetadata(ConstantAsMetadataKind, c)
		{
		}
	};

	class LocalAsMetadata : public ValueAsMetadata
	{
		friend class ValueAsMetadata;

	public:
		static LocalAsMetadata* Get(Value* local)
		{
			return ValueAsMetadata::GetLocal(local);
		}
		static LocalAsMetadata* GetIfExists(Value* local)
		{
			return ValueAsMetadata::GetLocalIfExists(local);
		}

		static bool classof(Metadata const * md)
		{
			return md->MetadataId() == LocalAsMetadataKind;
		}

	private:
		explicit LocalAsMetadata(Value* local)
			: ValueAsMetadata(LocalAsMetadataKind, local)
		{
			BOOST_ASSERT_MSG(!isa<Constant>(local), "Expected local value");
		}
	};

	class MDString : boost::noncopyable, public Metadata
	{
		MDString& operator=(MDString&&) = delete;

	public:
		typedef std::string_view::iterator iterator;

	public:
		MDString();
		MDString(MDString&& rhs);

		static MDString* Get(LLVMContext& context, std::string_view str);

		std::string_view String() const;
		size_t Size() const
		{
			return string_.size();
		}

		iterator begin() const
		{
			return this->String().begin();
		}
		iterator end() const
		{
			return this->String().end();
		}

		static bool classof(const Metadata *MD)
		{
			return MD->MetadataId() == MDStringKind;
		}

	private:
		std::string string_;
		uint64_t string_hash_;
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class MDOperand : boost::noncopyable
	{
	public:
		MDOperand();
		MDOperand(MDOperand&& rhs);
		~MDOperand();

		Metadata* Get() const
		{
			return md_;
		}
		operator Metadata*() const
		{
			return this->Get();
		}
		Metadata* operator->() const
		{
			return this->Get();
		}
		Metadata& operator*() const
		{
			return *this->Get();
		}

		void Reset();
		void Reset(Metadata* md, Metadata* owner);

		MDOperand& operator=(MDOperand&& rhs);

	private:
		void Track(Metadata* owner);
		void Untrack();

	private:
		Metadata* md_;
	};

	class ContextAndReplaceableUses : boost::noncopyable
	{
		ContextAndReplaceableUses() = delete;
		ContextAndReplaceableUses(ContextAndReplaceableUses&&) = delete;
		ContextAndReplaceableUses& operator=(ContextAndReplaceableUses&&) = delete;

	public:
		explicit ContextAndReplaceableUses(LLVMContext& context);
		explicit ContextAndReplaceableUses(std::unique_ptr<ReplaceableMetadataImpl> replaceable_uses);
		~ContextAndReplaceableUses();

		bool HasReplaceableUses() const
		{
			return ptr_.Is<ReplaceableMetadataImpl*>();
		}
		LLVMContext& Context() const;
		ReplaceableMetadataImpl* ReplaceableUses() const;

		void MakeReplaceable(std::unique_ptr<ReplaceableMetadataImpl> replaceable_uses);
		std::unique_ptr<ReplaceableMetadataImpl> TakeReplaceableUses();

	private:
		PointerUnion<LLVMContext*, ReplaceableMetadataImpl*> ptr_;
	};

	struct TempMDNodeDeleter
	{
		void operator()(MDNode* node) const;
	};

#define HANDLE_MDNODE_LEAF(CLASS) typedef std::unique_ptr<CLASS, TempMDNodeDeleter> Temp##CLASS;
#define HANDLE_MDNODE_BRANCH(CLASS) HANDLE_MDNODE_LEAF(CLASS)
#include "Metadata.inc"

	class MDNode : boost::noncopyable, public Metadata
	{
		friend class ReplaceableMetadataImpl;

	public:
		static MDTuple* Get(LLVMContext& context, ArrayRef<Metadata*> mds);
		static MDTuple* GetIfExists(LLVMContext& context, ArrayRef<Metadata*> mds);
		static MDTuple* GetDistinct(LLVMContext& context, ArrayRef<Metadata*> mds);
		//static TempMDTuple GetTemporary(LLVMContext& context, ArrayRef<Metadata*> mds);

		void ReplaceOperandWith(uint32_t idx, Metadata* new_md);

		bool IsResolved() const
		{
			return !context_.HasReplaceableUses();
		}

		bool IsUniqued() const
		{
			return storage_ == Uniqued;
		}
		bool IsDistinct() const
		{
			return storage_ == Distinct;
		}
		bool IsTemporary() const
		{
			return storage_ == Temporary;
		}

		void ReplaceAllUsesWith(Metadata* md);

		typedef MDOperand const * op_iterator;
		typedef boost::iterator_range<op_iterator> op_range;

		op_iterator OpBegin() const
		{
			return operands_.data();
		}
		op_iterator OpEnd() const
		{
			return operands_.data() + operands_.size();
		}
		op_range Operands() const
		{
			return op_range(this->OpBegin(), this->OpEnd());
		}

		MDOperand const & Operand(uint32_t idx) const
		{
			return operands_[idx];
		}

		uint32_t NumOperands() const
		{
			return static_cast<uint32_t>(operands_.size());
		}

		static bool classof(Metadata const * md)
		{
			switch (md->MetadataId()) {
			default:
				return false;
#define HANDLE_MDNODE_LEAF(CLASS)			\
			case CLASS##Kind:				\
				return true;
#include "Metadata.inc"
			}
		}

	protected:
		MDNode(LLVMContext& context, uint8_t id, StorageType storage, ArrayRef<Metadata*> ops1, ArrayRef<Metadata*> ops2 = {});
		~MDNode();

		void DropAllReferences();

		MDOperand* MutableBegin()
		{
			return operands_.data();
		}
		MDOperand* MutableEnd()
		{
			return operands_.data() + operands_.size();
		}

		typedef boost::iterator_range<MDOperand*> mutable_op_range;
		mutable_op_range MutableOperands()
		{
			return mutable_op_range(this->MutableBegin(), this->MutableEnd());
		}

		void Operand(uint32_t idx, Metadata* md);

		void StoreDistinctInContext();
		template <typename T, typename StoreT>
		static T* StoreImpl(T* n, StorageType storage, StoreT& store);

		template <typename NodeTy>
		struct HasCachedHash
		{
			typedef char Yes[1];
			typedef char No[2];
			template <typename U, U Val>
			struct SFINAE
			{
			};

			template <typename U>
			static Yes& Check(SFINAE<void (U::*)(uint32_t), &U::Hash>*);
			template <typename U>
			static No& Check(...);

			static bool const value = sizeof(Check<NodeTy>(nullptr)) == sizeof(Yes);
		};


		template <typename NodeTy>
		static void DispatchRecalculateHash(NodeTy* node, std::true_type)
		{
			node->RecalculateHash();
		}
		template <typename NodeTy>
		static void DispatchRecalculateHash(NodeTy* node, std::false_type)
		{
		}
		template <typename NodeTy>
		static void DispatchResetHash(NodeTy* node, std::true_type)
		{
			node->Hash(0);
		}
		template <typename NodeTy>
		static void DispatchResetHash(NodeTy* node, std::false_type)
		{
		}

	protected:
		ContextAndReplaceableUses context_;

	private:
		void HandleChangedOperand(void* ref, Metadata* new_op);

		void Resolve();
		void ResolveAfterOperandChange(Metadata* old, Metadata* new_op);
		void DecrementUnresolvedOperandCount();
		uint32_t CountUnresolvedOperands();

		void DeleteAsSubclass();
		MDNode* Uniquify();
		void EraseFromStore();

	private:
		uint32_t num_unresolved_;
		std::vector<MDOperand> operands_;
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class MDTuple : public MDNode
	{
		friend class MDNode;

	public:
		uint32_t Hash() const
		{
			return subclass_data_32_;
		}

		static MDTuple* Get(LLVMContext& context, ArrayRef<Metadata*> mds)
		{
			return GetImpl(context, mds, Uniqued);
		}
		static MDTuple* GetIfExists(LLVMContext& context, ArrayRef<Metadata*> mds)
		{
			return GetImpl(context, mds, Uniqued, false);
		}
		static MDTuple* GetDistinct(LLVMContext& context, ArrayRef<Metadata*> mds)
		{
			return GetImpl(context, mds, Distinct);
		}

		static bool classof(Metadata const * md)
		{
			return md->MetadataId() == MDTupleKind;
		}

	private:
		MDTuple(LLVMContext& context, StorageType storage, uint32_t hash, ArrayRef<Metadata*> vals);
		~MDTuple();

		void Hash(uint32_t hash)
		{
			subclass_data_32_ = hash;
		}
		void RecalculateHash();

		static MDTuple* GetImpl(LLVMContext& context, ArrayRef<Metadata*> mds, StorageType storage, bool should_create = true);
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class NamedMDNode : boost::noncopyable
	{
		friend struct LLVMContextImpl;
		friend class LLVMModule;

	public:
		explicit NamedMDNode(std::string_view name);
		~NamedMDNode();

		void DropAllReferences();

		LLVMModule const * Parent() const
		{
			return parent_;
		}
		LLVMModule* Parent()
		{
			return parent_;
		}

		MDNode* Operand(uint32_t idx) const;
		void Operand(uint32_t idx, MDNode* new_op);
		uint32_t NumOperands() const;
		void AddOperand(MDNode* mn);

	private:
		void Parent(LLVMModule* mod)
		{
			parent_ = mod;
		}

	private:
		std::string name_;
		LLVMModule* parent_;
		boost::container::small_vector<TrackingMDRef, 4> operands_;
		// DILITHIUM_NOT_IMPLEMENTED
	};

	template <>
	struct simplify_type<MDOperand>
	{
		typedef Metadata* SimpleType;
		static SimpleType SimplifiedValue(MDOperand& md)
		{
			return md.Get();
		}
	};

	template <>
	struct simplify_type<MDOperand const>
	{
		typedef Metadata* SimpleType;
		static SimpleType SimplifiedValue(MDOperand const & md)
		{
			return md.Get();
		}
	};
}

namespace boost
{
	inline std::size_t hash_value(Dilithium::MDOperand const & v)
	{
		return hash_value(v.Get());
	}
}

namespace std
{
	template <>
	struct hash<Dilithium::MDTuple>
	{
		typedef std::size_t result_type;
		typedef Dilithium::MDTuple argument_type;

		result_type operator()(argument_type const & rhs) const
		{
			return boost::hash_range(rhs.OpBegin(), rhs.OpEnd());
		}
	};
}

#endif		// _DILITHIUM_METADATA_HPP
