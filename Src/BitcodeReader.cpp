/**
 * @file BitcodeReader.cpp
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
#include <Dilithium/BitcodeReader.hpp>

#include <Dilithium/Attributes.hpp>
#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/BasicBlock.hpp>
#include <Dilithium/BitstreamReader.hpp>
#include <Dilithium/Casting.hpp>
#include <Dilithium/Constants.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/GVMaterializer.hpp>
#include <Dilithium/Instructions.hpp>
#include <Dilithium/LLVMBitCodes.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/Mathextras.hpp>
#include <Dilithium/Metadata.hpp>
#include <Dilithium/SmallString.hpp>
#include <Dilithium/SymbolTableList.hpp>
#include <Dilithium/TrackingMDRef.hpp>
#include <Dilithium/Use.hpp>
#include <Dilithium/ValueHandle.hpp>

#include <deque>
#include <map>
#include <unordered_map>

#include <boost/assert.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/endian/conversion.hpp>

namespace
{
	using namespace Dilithium;

	enum class BitcodeError
	{
		InvalidBitcodeSignature = 1,
		CorruptedBitcode
	};

	std::error_category const & BitcodeErrorCategory()
	{
		class BitcodeErrorCategoryType : public std::error_category
		{
			char const * name() const noexcept override
			{
				return "llvm.bitcode";
			}

			std::string message(int ie) const override
			{
				BitcodeError e = static_cast<BitcodeError>(ie);
				switch (e)
				{
				case BitcodeError::InvalidBitcodeSignature:
					return "Invalid bitcode signature";
				case BitcodeError::CorruptedBitcode:
					return "Corrupted bitcode";

				default:
					DILITHIUM_UNREACHABLE("Unknown error type!");
				}
			}
		};

		static BitcodeErrorCategoryType error_category;
		return error_category;
	}

	std::error_code MakeErrorCode(BitcodeError e)
	{
		return std::error_code(static_cast<int>(e), BitcodeErrorCategory());
	}

	bool IsBitcodeWrapper(uint8_t const * buf_beg, uint8_t const * buf_end)
	{
		return (buf_beg != buf_end)
			&& (buf_beg[0] == 0xDE) && (buf_beg[1] == 0xC0) && (buf_beg[2] == 0x17) && (buf_beg[3] == 0x0B);
	}

	bool SkipBitcodeWrapperHeader(uint8_t const *& buf_beg, uint8_t const *& buf_end, bool verify_buff_size)
	{
		static uint32_t constexpr KNOWN_HEADER_SIZE = 4 * 4;
		static uint32_t constexpr OFFSET_FIELD = 2 * 4;
		static uint32_t constexpr SIZE_FIELD = 3 * 4;

		// Must contain the header!
		if (buf_end - buf_beg < KNOWN_HEADER_SIZE)
		{
			return true;
		}
		else
		{
			uint32_t offset = boost::endian::little_to_native(*reinterpret_cast<uint32_t const *>(&buf_beg[OFFSET_FIELD]));
			uint32_t size = boost::endian::little_to_native(*reinterpret_cast<uint32_t const *>(&buf_beg[SIZE_FIELD]));

			// Verify that offset + size fits in the file.
			if (verify_buff_size && (offset + size > static_cast<uint32_t>(buf_end - buf_beg)))
			{
				return true;
			}
			else
			{
				buf_beg += offset;
				buf_end = buf_beg + size;
				return false;
			}
		}
	}

	template <typename T>
	bool ConvertToString(ArrayRef<uint64_t> record, uint32_t idx, T& result)
	{
		if (idx > record.size())
		{
			return true;
		}

		for (uint32_t i = idx, e = static_cast<uint32_t>(record.size()); i != e; ++ i)
		{
			result += static_cast<char>(record[i]);
		}

		return false;
	}

	bool HasImplicitComdat(size_t val)
	{
		switch (val)
		{
		case 1:  // Old WeakAnyLinkage
		case 4:  // Old LinkOnceAnyLinkage
		case 10: // Old WeakODRLinkage
		case 11: // Old LinkOnceODRLinkage
			return true;

		default:
			return false;
		}
	}

	GlobalValue::LinkageTypes DecodedLinkage(uint32_t val)
	{
		switch (val)
		{
		case 0:
			return GlobalValue::ExternalLinkage;
		case 2:
			return GlobalValue::AppendingLinkage;
		case 3:
			return GlobalValue::InternalLinkage;
		case 5:
			return GlobalValue::ExternalLinkage; // Obsolete DLLImportLinkage
		case 6:
			return GlobalValue::ExternalLinkage; // Obsolete DLLExportLinkage
		case 7:
			return GlobalValue::ExternalWeakLinkage;
		case 8:
			return GlobalValue::CommonLinkage;
		case 9:
			return GlobalValue::PrivateLinkage;
		case 12:
			return GlobalValue::AvailableExternallyLinkage;
		case 13:
			return GlobalValue::PrivateLinkage; // Obsolete LinkerPrivateLinkage
		case 14:
			return GlobalValue::PrivateLinkage; // Obsolete LinkerPrivateWeakLinkage
		case 15:
			return GlobalValue::ExternalLinkage; // Obsolete LinkOnceODRAutoHideLinkage
		case 1: // Old value with implicit comdat.
		case 16:
			return GlobalValue::WeakAnyLinkage;
		case 10: // Old value with implicit comdat.
		case 17:
			return GlobalValue::WeakODRLinkage;
		case 4: // Old value with implicit comdat.
		case 18:
			return GlobalValue::LinkOnceAnyLinkage;
		case 11: // Old value with implicit comdat.
		case 19:
			return GlobalValue::LinkOnceODRLinkage;

		default: // Map unknown/new linkages to external
			return GlobalValue::ExternalLinkage;
		}
	}

	GlobalValue::VisibilityTypes DecodedVisibility(uint32_t val)
	{
		switch (val)
		{
		case 1:
			return GlobalValue::HiddenVisibility;
		case 2:
			return GlobalValue::ProtectedVisibility;

		default: // Map unknown visibilities to default.
		case 0:
			return GlobalValue::DefaultVisibility;
		}
	}

	GlobalValue::DLLStorageClassTypes DecodedDLLStorageClass(uint32_t val)
	{
		switch (val)
		{
		case 1:
			return GlobalValue::DLLImportStorageClass;
		case 2:
			return GlobalValue::DLLExportStorageClass;

		default: // Map unknown values to default.
		case 0:
			return GlobalValue::DefaultStorageClass;
		}
	}

	void UpgradeDLLImportExportLinkage(GlobalValue* gv, uint32_t val)
	{
		switch (val)
		{
		case 5:
			gv->DLLStorageClass(GlobalValue::DLLImportStorageClass);
			break;
		case 6:
			gv->DLLStorageClass(GlobalValue::DLLExportStorageClass);
			break;

		default:
			break;
		}
	}

	void DecodeLLVMAttributesForBitcode(AttrBuilder& ab, uint64_t encoded_attrs)
	{
		// FIXME: Remove in 4.0.

		// The alignment is stored as a 16-bit raw value from bits 31--16.  We shift
		// the bits above 31 down by 11 bits.
		uint32_t const alignment = (encoded_attrs & (0xFFFFULL << 16)) >> 16;
		BOOST_ASSERT_MSG(!alignment || IsPowerOfTwo32(alignment), "Alignment must be a power of two.");

		if (alignment)
		{
			ab.AddAlignmentAttr(alignment);
		}
		ab.AddRawValue(((encoded_attrs & (0xFFFFFULL << 32)) >> 11) | (encoded_attrs & 0xFFFF));
	}

	Attribute::AttrKind AttrFromCode(uint64_t Code)
	{
		switch (Code)
		{
		case BitCode::AttributeKindCode::Alignment:
			return Attribute::AK_Alignment;
		case BitCode::AttributeKindCode::AlwaysInline:
			return Attribute::AK_AlwaysInline;
		case BitCode::AttributeKindCode::ArgMemOnly:
			return Attribute::AK_ArgMemOnly;
		case BitCode::AttributeKindCode::Builtin:
			return Attribute::AK_Builtin;
		case BitCode::AttributeKindCode::ByVal:
			return Attribute::AK_ByVal;
		case BitCode::AttributeKindCode::InAlloca:
			return Attribute::AK_InAlloca;
		case BitCode::AttributeKindCode::Cold:
			return Attribute::AK_Cold;
		case BitCode::AttributeKindCode::Convergent:
			return Attribute::AK_Convergent;
		case BitCode::AttributeKindCode::InlineHint:
			return Attribute::AK_InlineHint;
		case BitCode::AttributeKindCode::InReg:
			return Attribute::AK_InReg;
		case BitCode::AttributeKindCode::JumpTable:
			return Attribute::AK_JumpTable;
		case BitCode::AttributeKindCode::MinSize:
			return Attribute::AK_MinSize;
		case BitCode::AttributeKindCode::Naked:
			return Attribute::AK_Naked;
		case BitCode::AttributeKindCode::Nest:
			return Attribute::AK_Nest;
		case BitCode::AttributeKindCode::NoAlias:
			return Attribute::AK_NoAlias;
		case BitCode::AttributeKindCode::NoBuiltin:
			return Attribute::AK_NoBuiltin;
		case BitCode::AttributeKindCode::NoCapture:
			return Attribute::AK_NoCapture;
		case BitCode::AttributeKindCode::NoDuplicate:
			return Attribute::AK_NoDuplicate;
		case BitCode::AttributeKindCode::NoImplicitFloat:
			return Attribute::AK_NoImplicitFloat;
		case BitCode::AttributeKindCode::NoInline:
			return Attribute::AK_NoInline;
		case BitCode::AttributeKindCode::NonLazyBind:
			return Attribute::AK_NonLazyBind;
		case BitCode::AttributeKindCode::NonNull:
			return Attribute::AK_NonNull;
		case BitCode::AttributeKindCode::Dereferenceable:
			return Attribute::AK_Dereferenceable;
		case BitCode::AttributeKindCode::DereferenceableOrNull:
			return Attribute::AK_DereferenceableOrNull;
		case BitCode::AttributeKindCode::NoRedZone:
			return Attribute::AK_NoRedZone;
		case BitCode::AttributeKindCode::NoReturn:
			return Attribute::AK_NoReturn;
		case BitCode::AttributeKindCode::NoUnwind:
			return Attribute::AK_NoUnwind;
		case BitCode::AttributeKindCode::OptimizeForSize:
			return Attribute::AK_OptimizeForSize;
		case BitCode::AttributeKindCode::OptimizeNone:
			return Attribute::AK_OptimizeNone;
		case BitCode::AttributeKindCode::ReadNone:
			return Attribute::AK_ReadNone;
		case BitCode::AttributeKindCode::ReadOnly:
			return Attribute::AK_ReadOnly;
		case BitCode::AttributeKindCode::Returned:
			return Attribute::AK_Returned;
		case BitCode::AttributeKindCode::ReturnsTwice:
			return Attribute::AK_ReturnsTwice;
		case BitCode::AttributeKindCode::SExt:
			return Attribute::AK_SExt;
		case BitCode::AttributeKindCode::StackAlignment:
			return Attribute::AK_StackAlignment;
		case BitCode::AttributeKindCode::StackProtect:
			return Attribute::AK_StackProtect;
		case BitCode::AttributeKindCode::StackProtectReq:
			return Attribute::AK_StackProtectReq;
		case BitCode::AttributeKindCode::StackProtectStrong:
			return Attribute::AK_StackProtectStrong;
		case BitCode::AttributeKindCode::SafeStack:
			return Attribute::AK_SafeStack;
		case BitCode::AttributeKindCode::StructRet:
			return Attribute::AK_StructRet;
		case BitCode::AttributeKindCode::SanitizeAddress:
			return Attribute::AK_SanitizeAddress;
		case BitCode::AttributeKindCode::SanitizeThread:
			return Attribute::AK_SanitizeThread;
		case BitCode::AttributeKindCode::SanitizeMemory:
			return Attribute::AK_SanitizeMemory;
		case BitCode::AttributeKindCode::UwTable:
			return Attribute::AK_UWTable;
		case BitCode::AttributeKindCode::ZExt:
			return Attribute::AK_ZExt;

		default:
			return Attribute::AK_None;
		}
	}

	class BitcodeReaderValueList : boost::noncopyable
	{
	public:
		explicit BitcodeReaderValueList(LLVMContext& context)
			: context_(context)
		{
		}

		// vector compatibility methods
		size_t size() const
		{
			return value_ptrs_.size();
		}
		void resize(size_t n)
		{
			value_ptrs_.resize(n);
		}
		void push_back(Value* v)
		{
			value_ptrs_.emplace_back(v);
		}

		void clear()
		{
			value_ptrs_.clear();
		}
		void shrink_to_fit()
		{
			value_ptrs_.shrink_to_fit();
		}

		Value* operator[](uint32_t i) const
		{
			BOOST_ASSERT(i < value_ptrs_.size());
			return value_ptrs_[i];
		}

		Value* back() const
		{
			return value_ptrs_.back();
		}
		void pop_back()
		{
			value_ptrs_.pop_back();
		}
		bool empty() const
		{
			return value_ptrs_.empty();
		}

		Value* ValueFwdRef(uint32_t idx, Type* ty)
		{
			if (idx == UINT_MAX)
			{
				return nullptr;
			}

			if (idx >= this->size())
			{
				this->resize(idx + 1);
			}

			Value* ret;

			auto v = value_ptrs_[idx];
			if (v)
			{
				if (ty && (ty != v->GetType()))
				{
					ret = nullptr;
				}
				ret = v;
			}
			else
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}

			return ret;
		}

		void AssignValue(Value* v, uint32_t idx)
		{
			if (idx == this->size())
			{
				this->push_back(v);
				return;
			}

			if (idx >= this->size())
			{
				this->resize(idx + 1);
			}

			auto& old_v = value_ptrs_[idx];
			if (!old_v)
			{
				old_v.Assign(v);
				return;
			}

			DILITHIUM_NOT_IMPLEMENTED;
		}

	private:
		std::vector<WeakVH> value_ptrs_;

		LLVMContext& context_;
	};

	class BitcodeReaderMDValueList : boost::noncopyable
	{
	public:
		BitcodeReaderMDValueList()
			: num_fwd_refs_(0)
		{
		}

		// vector compatibility methods
		size_t size() const
		{
			return md_value_ptrs_.size();
		}
		void resize(size_t n)
		{
			md_value_ptrs_.resize(n);
		}
		void push_back(Metadata* md)
		{
			md_value_ptrs_.emplace_back(md);
		}
		void clear()
		{
			md_value_ptrs_.clear();
		}
		void shrink_to_fit()
		{
			md_value_ptrs_.shrink_to_fit();
		}
		Metadata* back() const
		{
			return md_value_ptrs_.back().Get();
		}
		void pop_back()
		{
			md_value_ptrs_.pop_back();
		}
		bool empty() const
		{
			return md_value_ptrs_.empty();
		}

		Metadata* operator[](uint32_t i) const
		{
			BOOST_ASSERT(i < md_value_ptrs_.size());
			return md_value_ptrs_[i].Get();
		}

		Metadata* ValueFwdRef(uint32_t idx)
		{
			if (idx >= this->size())
			{
				this->resize(idx + 1);
			}

			Metadata* md = md_value_ptrs_[idx].Get();
			if (md)
			{
				return md;
			}

			DILITHIUM_NOT_IMPLEMENTED;
		}
		void AssignValue(Metadata* md, uint32_t idx)
		{
			if (idx == this->size())
			{
				this->push_back(md);
				return;
			}

			if (idx >= this->size())
			{
				this->resize(idx + 1);
			}

			auto& old_md = md_value_ptrs_[idx];
			if (!old_md.Get())
			{
				old_md.Reset(md);
				return;
			}

			// If there was a forward reference to this value, replace it.
			TempMDTuple prev_md(cast<MDTuple>(old_md.Get()));
			prev_md->ReplaceAllUsesWith(md);
			-- num_fwd_refs_;
		}

	private:
		uint32_t num_fwd_refs_;
		std::vector<TrackingMDRef> md_value_ptrs_;
	};

	class BitcodeReader : boost::noncopyable, public GVMaterializer
	{
	public:
		BitcodeReader(uint8_t const * data, uint32_t data_length, std::shared_ptr<LLVMContext> const & context)
			: context_(context), buffer_(data), buffer_length_(data_length), value_list_(*context)
		{
		}
		~BitcodeReader() override
		{
			buffer_ = nullptr;
			buffer_length_ = 0;
			type_list_.clear();
			type_list_.shrink_to_fit();
			value_list_.clear();
			value_list_.shrink_to_fit();
			md_value_list_.clear();
			md_value_list_.shrink_to_fit();

			std::vector<AttributeSet>().swap(m_attribs_);
			std::vector<BasicBlock*>().swap(func_bbs_);
			std::vector<Function*>().swap(func_with_bodies_);
			deferred_func_info_.clear();
			deferred_metadata_info_.clear();
			md_kind_map_.clear();

			BOOST_ASSERT_MSG(basic_block_fwd_refs_.empty(), "Unresolved blockaddress fwd references");
			basic_block_fwd_ref_queue_.clear();
		}

		void Materialize(GlobalValue* gv) override
		{
			this->MaterializeMetadata();

			auto func = dyn_cast<Function>(gv);
			if (!func || !func->IsMaterializable())
			{
				return;
			}

			auto dfii = deferred_func_info_.find(func);
			BOOST_ASSERT_MSG(dfii != deferred_func_info_.end(), "Deferred function not found!");
			if (dfii->second == 0)
			{
				this->FindFunctionInStream(*func, dfii);
			}

			stream_cursor_.JumpToBit(dfii->second);

			this->ParseFunctionBody(*func);
			func->IsMaterializable(false);

			// TODO: LLVM strips debug information for func here. We haven't implemented debug processing.

			// TODO: LLVM upgrades any old intrinsic calls in the function here. But it doesn't seems we need it for DXIL.

			this->MaterializeForwardReferencedFunctions();
		}

		void MaterializeModule(LLVMModule* mod) override
		{
			DILITHIUM_UNUSED(mod);
			BOOST_ASSERT_MSG(mod == the_module_, "Can only Materialize the Module this BitcodeReader is attached to.");

			this->MaterializeMetadata();

			// Promise to materialize all forward references.
			will_materialize_all_forward_refs_ = true;

			for (auto func_iter = the_module_->begin(), end_iter = the_module_->end(); func_iter != end_iter; ++ func_iter)
			{
				this->Materialize(func_iter->get());
			}
			if (next_unread_bit_)
			{
				this->ParseModule(true);
			}

			// Check that all block address forward references got resolved (as we promised above).
			if (!basic_block_fwd_refs_.empty())
			{
				this->Error("Never resolved function from blockaddress");
				return;
			}

			// TODO: LLVM upgrades any intrinsic calls that slipped through (should not happen!) and
			// delete the old functions to clean up. We can't do this unless the entire
			// module is materialized because there could always be another function body
			// with calls to the old function. But it doesn't seems we need it for DXIL.

			// TODO: LLVM upgrades any instructions with TBAATag. But DXIL instructions doesn't come with TBAATag.

			// TODO: LLVM upgrades debug information. We haven't implemented debug processing.
		}

		void MaterializeMetadata() override
		{
			for (auto bit_pos : deferred_metadata_info_)
			{
				stream_cursor_.JumpToBit(bit_pos);
				this->ParseMetadata();
			}
			deferred_metadata_info_.clear();
		}

		void ParseBitcodeInto(LLVMModule* mod, bool should_lazy_load_metadata = false)
		{
			DILITHIUM_UNUSED(should_lazy_load_metadata);

			the_module_ = mod;

			this->InitStream();

			// Sniff for the signature.
			if ((stream_cursor_.Read(8) != 'B')
				|| (stream_cursor_.Read(8) != 'C')
				|| (stream_cursor_.Read(8) != 0xC0)
				|| (stream_cursor_.Read(8) != 0xDE))
			{
				TERROR("Invalid bitcode signature");
			}

			// We expect a number of well-defined blocks, though we don't necessarily
			// need to understand them all.
			for (;;)
			{
				if (stream_cursor_.AtEndOfStream())
				{
					TERROR("Malformed IR file");
				}

				BitStreamEntry entry = stream_cursor_.Advance(BitStreamCursor::AF_DontAutoprocessAbbrevs);

				if (entry.kind != BitStreamEntry::SubBlock)
				{
					TERROR("Malformed block");
				}

				if (entry.id == BitCode::BlockId::Module)
				{
					this->ParseModule(false, should_lazy_load_metadata);
					break;
				}
				else
				{
					if (stream_cursor_.SkipBlock())
					{
						TERROR("Invalid record");
					}
				}
			}
		}

		static uint64_t DecodeSignRotatedValue(uint64_t v)
		{
			if ((v & 1) == 0)
			{
				return v >> 1;
			}
			if (v != 1)
			{
				return static_cast<uint64_t>(-static_cast<int64_t>(v >> 1));
			}
			// There is no such thing as -0 with integers. "-0" really means MININT.
			return 1ULL << 63;
		}

	private:
		void Error(BitcodeError err, char const * message)
		{
			TEC(MakeErrorCode(err), message);
		}
		void Error(BitcodeError err)
		{
			auto ec = MakeErrorCode(err);
			TEC(ec, ec.message().c_str());
		}
		void Error(char const * message)
		{
			this->Error(BitcodeError::CorruptedBitcode, message);
		}

		void MaterializeForwardReferencedFunctions()
		{
			if (will_materialize_all_forward_refs_)
			{
				return;
			}

			// Prevent recursion.
			will_materialize_all_forward_refs_ = true;

			while (!basic_block_fwd_ref_queue_.empty())
			{
				auto func = basic_block_fwd_ref_queue_.front();
				basic_block_fwd_ref_queue_.pop_front();
				BOOST_ASSERT_MSG(func, "Expected valid function");
				if (!basic_block_fwd_refs_.count(func))
				{
					continue;
				}

				if (!func->IsMaterializable())
				{
					this->Error("Never resolved function from blockaddress");
					return;
				}

				this->Materialize(func);
			}
			BOOST_ASSERT_MSG(basic_block_fwd_refs_.empty(), "Function missing from queue");

			will_materialize_all_forward_refs_ = false;
		}

		StructType* CreateIdentifiedStructType(LLVMContext& context)
		{
			auto ret = StructType::Create(context);
			identified_struct_types_.push_back(ret);
			return ret;
		}

		Type* TypeByID(uint32_t id)
		{
			if (id >= type_list_.size())
			{
				return nullptr;
			}

			Type* ty = type_list_[id];
			if (!ty)
			{
				ty = this->CreateIdentifiedStructType(*context_);
				type_list_[id] = ty;
			}

			return ty;
		}
		Value* FnValueByID(uint32_t id, Type* ty)
		{
			if (ty && ty->IsMetadataType())
			{
				return MetadataAsValue::Get(ty->Context(), this->FnMetadataByID(id));
			}
			return value_list_.ValueFwdRef(id, ty);
		}
		Metadata* FnMetadataByID(uint32_t id)
		{
			return md_value_list_.ValueFwdRef(id);
		}
		BasicBlock* GetBasicBlock(uint32_t id) const
		{
			if (id >= func_bbs_.size())
			{
				return nullptr;
			}
			return func_bbs_[id];
		}
		AttributeSet Attributes(uint32_t i) const
		{
			if (i - 1 < m_attribs_.size())
			{
				return m_attribs_[i - 1];
			}
			return AttributeSet();
		}

		bool ValueTypePair(ArrayRef<uint64_t> record, uint32_t& slot, uint32_t inst_num, Value*& res_val)
		{
			if (slot == record.size())
			{
				return true;
			}

			uint32_t val_no = static_cast<uint32_t>(record[slot]);
			++ slot;
			// Adjust the val_no, if it was encoded relative to the inst_num.
			if (use_relative_ids_)
			{
				val_no = inst_num - val_no;
			}
			if (val_no < inst_num)
			{
				// If this is not a forward reference, just return the value we already have.
				res_val = this->FnValueByID(val_no, nullptr);
				return res_val == nullptr;
			}
			if (slot == record.size())
			{
				return true;
			}

			uint32_t type_no = static_cast<uint32_t>(record[slot]);
			++ slot;
			res_val = this->FnValueByID(val_no, this->TypeByID(type_no));
			return res_val == nullptr;
		}

		Value* GetValue(ArrayRef<uint64_t> record, uint32_t slot, uint32_t inst_num, Type* ty)
		{
			if (slot == record.size())
			{
				return nullptr;
			}

			uint32_t val_no = static_cast<uint32_t>(record[slot]);
			// Adjust the val_no, if it was encoded relative to the inst_num.
			if (use_relative_ids_)
			{
				val_no = inst_num - val_no;
			}
			return this->FnValueByID(val_no, ty);
		}

		void ParseAlignmentValue(uint64_t exponent, uint32_t& alignment)
		{
			// Note: Alignment in bitcode files is incremented by 1, so that zero
			// can be used for default alignment.
			if (exponent > Value::MAX_ALIGNMENT_EXPONENT + 1)
			{
				this->Error("Invalid alignment value");
				return;
			}
			alignment = (1 << static_cast<uint32_t>(exponent)) >> 1;
		}
		void ParseAttrKind(uint64_t code, Attribute::AttrKind* kind)
		{
			*kind = AttrFromCode(code);
			if (*kind == Attribute::AK_None)
			{
				this->Error(("Unknown attribute kind (" + std::to_string(code) + ")").c_str());
				return;
			}
		}
		void ParseModule(bool resume, bool should_lazy_load_metadata = false)
		{
			if (resume)
			{
				stream_cursor_.JumpToBit(next_unread_bit_);
			}
			else if (stream_cursor_.EnterSubBlock(BitCode::BlockId::Module))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;
			std::vector<std::string> section_tab;

			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.Advance();
				switch (entry.kind)
				{
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					return;

				case BitStreamEntry::EndBlock:
					this->GlobalCleanup();
					return;

				case BitStreamEntry::SubBlock:
					switch (entry.id)
					{
					default:
						// Skip unknown content.
						if (stream_cursor_.SkipBlock())
						{
							this->Error("Invalid record");
							return;
						}
						break;

					case BitCode::StandardBlockId::BlockInfoBlockId:
						if (stream_cursor_.ReadBlockInfoBlock())
						{
							this->Error("Malformed block");
							return;
						}
						break;

					case BitCode::BlockId::ParamAttr:
						this->ParseAttributeBlock();
						break;

					case BitCode::BlockId::ParamAttrGroup:
						this->ParseAttributeGroupBlock();
						break;

					case BitCode::BlockId::Type:
						this->ParseTypeTable();
						break;

					case BitCode::BlockId::ValueSymTab:
						this->ParseValueSymbolTable();
						seen_value_sym_tab_ = true;
						break;

					case BitCode::BlockId::Constants:
						this->ParseConstants();
						this->ResolveGlobalAndAliasInits();
						break;

					case BitCode::BlockId::Metadata:
						if (should_lazy_load_metadata && !is_metadata_materialized_)
						{
							this->RememberAndSkipMetadata();
							break;
						}
						BOOST_ASSERT_MSG(deferred_metadata_info_.empty(), "Unexpected deferred metadata");
						this->ParseMetadata();
						break;

					case BitCode::BlockId::Function:
						// If this is the first function body we've seen, reverse the
						// func_with_bodies_ list.
						if (!seen_first_func_body_)
						{
							std::reverse(func_with_bodies_.begin(), func_with_bodies_.end());
							this->GlobalCleanup();
							seen_first_func_body_ = true;
						}

						this->RememberAndSkipFunctionBody();

						// Suspend parsing when we reach the function bodies. Subsequent
						// materialization calls will resume it when necessary. If the bitcode
						// file is old, the symbol table will be at the end instead and will not
						// have been seen yet. In this case, just finish the parse now.
						if (seen_value_sym_tab_)
						{
							next_unread_bit_ = stream_cursor_.CurrBitNo();
							return;
						}
						break;

					case BitCode::BlockId::UseList:
						this->ParseUseLists();
						break;
					}
					continue;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::ModuleCode::Version: // VERSION: [version#]
					{
						if (record.size() < 1)
						{
							this->Error("Invalid record");
							return;
						}
						// Only version #0 and #1 are supported so far.
						uint32_t module_version = static_cast<uint32_t>(record[0]);
						switch (module_version)
						{
						case 0:
							use_relative_ids_ = false;
							break;
						case 1:
							use_relative_ids_ = true;
							break;
						default:
							this->Error("Invalid value");
							return;
						}
					}
					break;

				case BitCode::ModuleCode::Triple: // TRIPLE: [strchr x N]
					{
						std::string str;
						if (ConvertToString(record, 0, str))
						{
							this->Error("Invalid record");
							return;
						}
						the_module_->SetTargetTriple(str);
					}
					break;

				case BitCode::ModuleCode::DataLayout: // DATALAYOUT: [strchr x N]
					{
						std::string str;
						if (ConvertToString(record, 0, str))
						{
							this->Error("Invalid record");
							return;
						}
						the_module_->SetDataLayout(str);
					}
					break;

				case BitCode::ModuleCode::Asm: // ASM: [strchr x N]
				case BitCode::ModuleCode::DepLib: // DEPLIB: [strchr x N]
				// FIXME: Remove in 4.0.
				case BitCode::ModuleCode::SectionName: // SECTIONNAME: [strchr x N]
				case BitCode::ModuleCode::GcName: // SECTIONNAME: [strchr x N]
				case BitCode::ModuleCode::Comdat: // COMDAT: [selection_kind, name]
				// GLOBALVAR: [pointer type, isconst, initid,
				//             linkage, alignment, section, visibility, threadlocal,
				//             unnamed_addr, externally_initialized, dllstorageclass,
				//             comdat]
				case BitCode::ModuleCode::GlobalVar:
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				// FUNCTION:  [type, callingconv, isproto, linkage, paramattr,
				//             alignment, section, visibility, gc, unnamed_addr,
				//             prologuedata, dllstorageclass, comdat, prefixdata]
				case BitCode::ModuleCode::Function:
					{
						if (record.size() < 8)
						{
							this->Error("Invalid record");
							return;
						}

						auto ty = this->TypeByID(static_cast<uint32_t>(record[0]));
						if (!ty)
						{
							this->Error("Invalid record");
							return;
						}
						auto pty = dyn_cast<PointerType>(ty);
						if (pty)
						{
							ty = pty->ElementType();
						}
						auto fty = dyn_cast<FunctionType>(ty);
						if (!fty)
						{
							this->Error("Invalid type for value");
							return;
						}

						auto func = Function::Create(fty, GlobalValue::ExternalLinkage, "", the_module_);

						func->SetCallingConv(static_cast<CallingConv::ID>(record[1]));
						if (func->GetCallingConv() != CallingConv::C)
						{
							DILITHIUM_NOT_IMPLEMENTED;
						}
						bool proto = record[2];
						uint32_t raw_linkage = static_cast<uint32_t>(record[3]);
						func->Linkage(DecodedLinkage(raw_linkage));
						func->SetAttributes(this->Attributes(static_cast<uint32_t>(record[4])));

						uint32_t alignment;
						this->ParseAlignmentValue(record[5], alignment);
						func->SetAlignment(alignment);
						if (record[6])
						{
							if (record[6] - 1 >= section_tab.size())
							{
								this->Error("Invalid ID");
								return;
							}
							func->SetSection(section_tab[record[6] - 1]);
						}
						// Local linkage must have default visibility.
						if (!func->HasLocalLinkage())
						{
							// FIXME: Change to an error if non-default in 4.0.
							func->Visibility(DecodedVisibility(static_cast<uint32_t>(record[7])));
						}
						if ((record.size() > 8) && record[8])
						{
							// GC
							DILITHIUM_NOT_IMPLEMENTED;
						}

						bool unnamed_addr = false;
						if (record.size() > 9)
						{
							unnamed_addr = record[9];
						}
						func->UnnamedAddr(unnamed_addr);
						if ((record.size() > 10) && (record[10] != 0))
						{
							func_prologues_.push_back(std::make_pair(func, static_cast<uint32_t>(record[10] - 1)));
						}

						if (record.size() > 11)
						{
							func->DLLStorageClass(DecodedDLLStorageClass(static_cast<uint32_t>(record[11])));
						}
						else
						{
							UpgradeDLLImportExportLinkage(func, raw_linkage);
						}

						if (record.size() > 12)
						{
							uint32_t comdat_id = static_cast<uint32_t>(record[12]);
							if (comdat_id)
							{
								DILITHIUM_NOT_IMPLEMENTED;
							}
						}
						else if (HasImplicitComdat(raw_linkage))
						{
							DILITHIUM_NOT_IMPLEMENTED;
						}

						if ((record.size() > 13) && (record[13] != 0))
						{
							func_prefixes_.push_back(std::make_pair(func, static_cast<uint32_t>(record[13] - 1)));
						}
						if ((record.size() > 14) && (record[14] != 0))
						{
							func_personality_fns_.push_back(std::make_pair(func, static_cast<uint32_t>(record[14] - 1)));
						}

						value_list_.push_back(func);

						// If this is a function with a body, remember the prototype we are
						// creating now, so that we can match up the body with them later.
						if (!proto)
						{
							func->IsMaterializable(true);
							func_with_bodies_.push_back(func);
							deferred_func_info_[func] = 0;
						}
					}
					break;

				// ALIAS: [alias type, aliasee val#, linkage]
				// ALIAS: [alias type, aliasee val#, linkage, visibility, dllstorageclass]
				case BitCode::ModuleCode::Alias:
				// ModuleCode::PurgeVals: [numvals]
				case BitCode::ModuleCode::PurgeVals:
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				default:
					break;
				}
				record.clear();
			}
		}
		void ParseAttributeBlock()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::ParamAttr))
			{
				this->Error("Invalid record");
			}

			if (!m_attribs_.empty())
			{
				this->Error("Invalid multiple blocks");
			}

			boost::container::small_vector<uint64_t, 64> record;
			boost::container::small_vector<AttributeSet, 8> attrs;

			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");

				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::ParamAttrCode::EntryOld: // ENTRY: [paramidx0, attr0, ...]
					{
						// FIXME: Remove in 4.0.
						if (record.size() & 1)
						{
							this->Error("Invalid record");
						}

						for (uint32_t i = 0, e = static_cast<uint32_t>(record.size()); i != e; i += 2)
						{
							AttrBuilder ab;
							DecodeLLVMAttributesForBitcode(ab, record[i + 1]);
							attrs.push_back(AttributeSet::Get(*context_, static_cast<uint32_t>(record[i]), ab));
						}

						m_attribs_.push_back(AttributeSet::Get(*context_, attrs));
						attrs.clear();
					}
					break;

				case BitCode::ParamAttrCode::Entry: // ENTRY: [attrgrp0, attrgrp1, ...]
					{
						for (uint32_t i = 0, e = static_cast<uint32_t>(record.size()); i != e; ++ i)
						{
							attrs.push_back(m_attrib_groups_[static_cast<uint32_t>(record[i])]);
						}

						m_attribs_.push_back(AttributeSet::Get(*context_, attrs));
						attrs.clear();
					}
					break;

				default:
					break;
				}
			}
		}
		void ParseAttributeGroupBlock()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::ParamAttrGroup))
			{
				this->Error("Invalid record");
			}

			if (!m_attrib_groups_.empty())
			{
				this->Error("Invalid multiple blocks");
			}

			boost::container::small_vector<uint64_t, 64> record;

			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					break;

				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::ParamAttrCode::GrpEntry: // ENTRY: [grpid, idx, a0, a1, ...]
					{
						if (record.size() < 3)
						{
							this->Error("Invalid record");
						}

						uint64_t grp_id = record[0];
						uint64_t idx = record[1]; // Index of the object this attribute refers to.

						AttrBuilder ab;
						for (uint32_t i = 2, e = static_cast<uint32_t>(record.size()); i != e; ++ i)
						{
							if (record[i] == 0)
							{
								Attribute::AttrKind kind;
								++ i;
								this->ParseAttrKind(record[i], &kind);
								ab.AddAttribute(kind);
							}
							else if (record[i] == 1)
							{
								Attribute::AttrKind kind;
								++ i;
								this->ParseAttrKind(record[i], &kind);
								switch (kind)
								{
								case Attribute::AK_Alignment:
									++ i;
									ab.AddAlignmentAttr(static_cast<uint32_t>(record[i]));
									break;
								case Attribute::AK_StackAlignment:
									++ i;
									ab.AddStackAlignmentAttr(static_cast<uint32_t>(record[i]));
									break;
								case Attribute::AK_Dereferenceable:
									++ i;
									ab.AddDereferenceableAttr(record[i]);
									break;
								case Attribute::AK_DereferenceableOrNull:
									++ i;
									ab.AddDereferenceableOrNullAttr(record[i]);
									break;

								default:
									break;
								}
							}
							else
							{
								BOOST_ASSERT_MSG((record[i] == 3) || (record[i] == 4), "Invalid attribute group entry");
								bool has_value = (record[i] == 4);
								++ i;
								SmallString<64> kind_str;
								SmallString<64> val_str;

								while ((record[i] != 0) && (i != e))
								{
									kind_str += static_cast<char>(record[i]);
									++ i;
								}
								BOOST_ASSERT_MSG(record[i] == 0, "Kind string not null terminated");

								if (has_value)
								{
									// Has a value associated with it.
									++ i; // Skip the '0' that terminates the "kind" string.
									while ((record[i] != 0) && (i != e))
									{
										val_str += static_cast<char>(record[i]);
										++ i;
									}
									BOOST_ASSERT_MSG(record[i] == 0, "Value string not null terminated");
								}

								ab.AddAttribute(kind_str.str(), val_str.str());
							}
						}

						m_attrib_groups_[static_cast<uint32_t>(grp_id)] = AttributeSet::Get(*context_, static_cast<uint32_t>(idx), ab);
					}
					break;

				default:
					break;
				}
			}
		}
		void ParseTypeTable()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::Type))
			{
				this->Error("Invalid record");
			}

			this->ParseTypeTableBody();
		}
		void ParseTypeTableBody()
		{
			if (!type_list_.empty())
			{
				this->Error("Invalid multiple blocks");
			}

			boost::container::small_vector<uint64_t, 64> record;
			uint32_t num_records = 0;
			SmallString<64> type_name;

			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					break;

				case BitStreamEntry::EndBlock:
					if (num_records != type_list_.size())
					{
						this->Error("Malformed block");
					}
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				Type* result_ty = nullptr;
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::TypeCode::NumEntry: // TypeCode::NumEntry: [numentries]
					// TypeCode::NumEntry contains a count of the number of types in the
					// type list. This allows us to reserve space.
					if (record.size() < 1)
					{
						this->Error("Invalid record");
						return;
					}

					type_list_.resize(record[0]);
					continue;

				case BitCode::TypeCode::Void:      // VOID
					result_ty = Type::VoidType(*context_);
					break;

				case BitCode::TypeCode::Half:     // HALF
					result_ty = Type::HalfType(*context_);
					break;

				case BitCode::TypeCode::Float:     // FLOAT
					result_ty = Type::FloatType(*context_);
					break;

				case BitCode::TypeCode::Double :    // DOUBLE
					result_ty = Type::DoubleType(*context_);
					break;

				case BitCode::TypeCode::X86Fp80:  // X86_FP80
				case BitCode::TypeCode::Fp128:     // FP128
				case BitCode::TypeCode::PpcFp128: // PPC_FP128
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::TypeCode::Label:     // LABEL
					result_ty = Type::LabelType(*context_);
					break;

				case BitCode::TypeCode::Metadata:  // METADATA
					result_ty = Type::MetadataType(*context_);
					break;

				case BitCode::TypeCode::X86Mmx:   // X86_MMX
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::TypeCode::Integer: // INTEGER: [width]
					{
						if (record.size() < 1)
						{
							this->Error("Invalid record");
							return;
						}

						uint64_t num_bits = record[0];
						if ((num_bits < IntegerType::MIN_INT_BITS) || (num_bits > IntegerType::MAX_INT_BITS))
						{
							this->Error("Bitwidth for integer type out of range");
						}
						result_ty = IntegerType::Get(*context_, static_cast<uint32_t>(num_bits));
					}
					break;

				case BitCode::TypeCode::Pointer: // POINTER: [pointee type] or [pointee type, address space]
					{
						if (record.size() < 1)
						{
							this->Error("Invalid record");
							return;
						}
				
						uint32_t addr_space = 0;
						if (record.size() == 2)
						{
							addr_space = static_cast<uint32_t>(record[1]);
						}
						result_ty = this->TypeByID(static_cast<uint32_t>(record[0]));
						if (!result_ty || !PointerType::IsValidElementType(result_ty))
						{
							this->Error("Invalid type");
							return;
						}
						result_ty = PointerType::Get(result_ty, addr_space);
					}
					break;

				case BitCode::TypeCode::FunctionOld:
					// FIXME: attrid is dead, remove it in LLVM 4.0
					// FUNCTION: [vararg, attrid, retty, paramty x N]
					if (record.size() < 3)
					{
						this->Error("Invalid record");
						return;
					}
					// TODO
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::TypeCode::Function: // FUNCTION: [vararg, retty, paramty x N]
					{
						if (record.size() < 2)
						{
							this->Error("Invalid record");
							break;
						}

						boost::container::small_vector<Type*, 8> arg_tys;
						for (uint32_t i = 2, e = static_cast<uint32_t>(record.size()); i != e; ++ i)
						{
							Type* t = this->TypeByID(static_cast<uint32_t>(record[i]));
							if (t)
							{
								if (!FunctionType::IsValidArgumentType(t))
								{
									this->Error("Invalid function argument type");
									return;
								}
								arg_tys.push_back(t);
							}
							else
							{
								break;
							}
						}

						result_ty = this->TypeByID(static_cast<uint32_t>(record[1]));
						if (!result_ty || (arg_tys.size() < record.size() - 2))
						{
							this->Error("Invalid type");
							return;
						}

						result_ty = FunctionType::Get(result_ty, arg_tys, record[0]);
					}
					break;

				case BitCode::TypeCode::StructAnon: // STRUCT: [ispacked, eltty x N]
					{
						if (record.size() < 1)
						{
							this->Error("Invalid record");
							return;
						}

						boost::container::small_vector<Type*, 8> elt_tys;
						for (uint32_t i = 1, e = static_cast<uint32_t>(record.size()); i != e; ++ i)
						{
							Type* t = this->TypeByID(static_cast<uint32_t>(record[i]));
							if (t)
							{
								elt_tys.push_back(t);
							}
							else
							{
								break;
							}
						}
						if (elt_tys.size() != record.size() - 1)
						{
							this->Error("Invalid type");
							return;
						}
						result_ty = StructType::Get(*context_, elt_tys, record[0]);
					}
					break;

				case BitCode::TypeCode::StructName: // STRUCT_NAME: [strchr x N]
					if (ConvertToString(record, 0, type_name))
					{
						this->Error("Invalid record");
						return;
					}
					continue;

				case BitCode::TypeCode::StructNamed: // STRUCT: [ispacked, eltty x N]
					if (record.size() < 1)
					{
						this->Error("Invalid record");
						return;
					}

					if (num_records >= type_list_.size())
					{
						this->Error("Invalid TYPE table");
						return;
					}

					// TODO
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::TypeCode::Opaque: // OPAQUE: []
					if (record.size() != 1)
					{
						this->Error("Invalid record");
						return;
					}

					if (num_records >= type_list_.size())
					{
						this->Error("Invalid TYPE table");
						return;
					}

					// TODO
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::TypeCode::Array: // ARRAY: [numelts, eltty]
					if (record.size() < 2)
					{
						this->Error("Invalid record");
						return;
					}

					result_ty = this->TypeByID(static_cast<uint32_t>(record[1]));
					if (!result_ty || !ArrayType::IsValidElementType(result_ty))
					{
						this->Error("Invalid type");
						return;
					}
					result_ty = ArrayType::Get(result_ty, record[0]);
					break;

				case BitCode::TypeCode::Vector: // VECTOR: [numelts, eltty]
					if (record.size() < 2)
					{
						this->Error("Invalid record");
						return;
					}
					if (record[0] == 0)
					{
						this->Error("Invalid vector length");
						return;
					}

					result_ty = this->TypeByID(static_cast<uint32_t>(record[1]));
					if (!result_ty || !StructType::IsValidElementType(result_ty))
					{
						this->Error("Invalid type");
						return;
					}
					result_ty = VectorType::Get(result_ty, static_cast<uint32_t>(record[0]));
					break;

				default:
					this->Error("Invalid value");
					return;
				}

				if (num_records >= type_list_.size())
				{
					this->Error("Invalid TYPE table");
					return;
				}
				if (type_list_[num_records])
				{
					this->Error("Invalid TYPE table: Only named structs can be forward referenced");
					return;
				}
				BOOST_ASSERT_MSG(result_ty, "Didn't read a type?");
				type_list_[num_records] = result_ty;
				++ num_records;
			}
		}
		void ParseValueSymbolTable()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::ValueSymTab))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;

			SmallString<128> value_name;
			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					break;

				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::ValueSymTabCode::Entry: // VST_ENTRY: [valueid, namechar x N]
					{
						if (ConvertToString(record, 1, value_name))
						{
							this->Error("Invalid record");
							return;
						}
						uint32_t value_id = static_cast<uint32_t>(record[0]);
						if ((value_id >= value_list_.size()) || !value_list_[value_id])
						{
							this->Error("Invalid record");
							return;
						}

						Value* v = value_list_[value_id];
						v->Name(std::string_view(value_name.data(), value_name.size()));
						value_name.clear();
					}
					break;

				case BitCode::ValueSymTabCode::BbEntry:
					{
						if (ConvertToString(record, 1, value_name))
						{
							this->Error("Invalid record");
							return;
						}
						BasicBlock* bb = this->GetBasicBlock(static_cast<uint32_t>(record[0]));
						if (!bb)
						{
							this->Error("Invalid record");
							return;
						}

						bb->Name(std::string_view(value_name.data(), value_name.size()));
						value_name.clear();
					}
					break;

				default:
					break;
				}
			}
		}
		void ParseConstants()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::Constants))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;

			Type* cur_ty = Type::Int32Type(*context_);
			uint32_t next_cst_no = static_cast<uint32_t>(value_list_.size());
			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					break;

				case BitStreamEntry::EndBlock:
					if (next_cst_no != value_list_.size())
					{
						this->Error("Invalid ronstant reference");
						break;
					}

					// Once all the constants have been read, go through and resolve forward references.
					// TODO: LLVM resolves constant forward references here. Do we need it?
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				Value* v = nullptr;
				uint32_t bit_code = stream_cursor_.ReadRecord(entry.id, record);
				switch (bit_code)
				{
				default:
				case BitCode::ConstantsCode::Undef:     // UNDEF
					v = UndefValue::Get(cur_ty);
					break;

				case BitCode::ConstantsCode::SetType:   // SETTYPE: [typeid]
					if (record.empty())
					{
						this->Error("Invalid record");
						return;
					}
					if ((record[0] >= type_list_.size()) || !type_list_[record[0]])
					{
						this->Error("Invalid record");
						return;
					}
					cur_ty = type_list_[record[0]];
					continue;  // Skip the value_list_ manipulation.
		
				case BitCode::ConstantsCode::Null:      // NULL
					v = Constant::NullValue(cur_ty);
					break;
		
				case BitCode::ConstantsCode::Integer:   // INTEGER: [intval]
					if (!cur_ty->IsIntegerType() || record.empty())
					{
						this->Error("Invalid record");
						return;
					}
					v = ConstantInt::Get(cur_ty, this->DecodeSignRotatedValue(record[0]));
					break;
		
				case BitCode::ConstantsCode::WideInteger:	// WIDE_INTEGER: [n x intval]
				case BitCode::ConstantsCode::Float:			// FLOAT: [fpval]
				case BitCode::ConstantsCode::Aggregate:		// AGGREGATE: [n x value number]
				case BitCode::ConstantsCode::String:		// STRING: [values]
				case BitCode::ConstantsCode::CString:		// CSTRING: [values]
				case BitCode::ConstantsCode::Data:			// DATA: [n x value]
				case BitCode::ConstantsCode::CeBinop:		// CE_BINOP: [opcode, opval, opval]
				case BitCode::ConstantsCode::CeCast:		// CE_CAST: [opcode, opty, opval]
				case BitCode::ConstantsCode::InboundsGep:
				case BitCode::ConstantsCode::CeGep:			// CE_GEP:        [n x operands]
				case BitCode::ConstantsCode::CeSelect:		// CE_SELECT: [opval#, opval#, opval#]
				case BitCode::ConstantsCode::CeExtractElt:	// CE_EXTRACTELT: [opty, opval, opty, opval]
				case BitCode::ConstantsCode::CeInsertElt:	// CE_INSERTELT: [opval, opval, opty, opval]
				case BitCode::ConstantsCode::CeShuffleVec:	// CE_SHUFFLEVEC: [opval, opval, opval]
				case BitCode::ConstantsCode::ShuffleVecEx:	// [opty, opval, opval, opval]
				case BitCode::ConstantsCode::CeCmp:			// CE_CMP: [opty, opval, opval, pred]
				// This maintains backward compatibility, pre-asm dialect keywords.
				// FIXME: Remove with the 4.0 release.
				case BitCode::ConstantsCode::InlineAsmOld:
				// This version adds support for the asm dialect keywords (e.g., inteldialect).
				case BitCode::ConstantsCode::InlineAsm:
				case BitCode::ConstantsCode::BlockAddress:
					DILITHIUM_NOT_IMPLEMENTED;
					break;
				}

				value_list_.AssignValue(v, next_cst_no);
				++ next_cst_no;
			}
		}
		void RememberAndSkipFunctionBody()
		{
			if (func_with_bodies_.empty())
			{
				this->Error("Insufficient function protos");
				return;
			}

			auto func = func_with_bodies_.back();
			func_with_bodies_.pop_back();

			uint64_t cur_bit = stream_cursor_.CurrBitNo();
			deferred_func_info_[func] = cur_bit;

			if (stream_cursor_.SkipBlock())
			{
				this->Error("Invalid record");
				return;
			}
		}
		void RememberAndSkipMetadata()
		{
			uint64_t cur_bit = stream_cursor_.CurrBitNo();
			deferred_metadata_info_.push_back(cur_bit);

			if (stream_cursor_.SkipBlock())
			{
				this->Error("Invalid record");
			}
		}
		void ParseFunctionBody(Function& func)
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::Function))
			{
				this->Error("Invalid record");
				return;
			}

			instruction_list_.clear();
			uint32_t module_value_list_size = static_cast<uint32_t>(value_list_.size());
			uint32_t module_md_value_list_size = static_cast<uint32_t>(md_value_list_.size());

			for (auto iter = func.ArgBegin(), end_iter = func.ArgEnd(); iter != end_iter; ++ iter)
			{
				value_list_.push_back(iter->get());
			}

			uint32_t next_value_no = static_cast<uint32_t>(value_list_.size());
			BasicBlock* cur_bb = nullptr;
			uint32_t cur_bb_no = 0;

			auto LastInstruction = [&]()->Instruction*
			{
				if (cur_bb && !cur_bb->empty())
				{
					return &cur_bb->back();
				}
				else if (cur_bb_no && func_bbs_[cur_bb_no - 1] && !func_bbs_[cur_bb_no - 1]->empty())
				{
					return &func_bbs_[cur_bb_no - 1]->back();
				}
				return nullptr;
			};

			boost::container::small_vector<uint64_t, 64> record;
			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.Advance();
				switch (entry.kind)
				{
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					return;

				case BitStreamEntry::EndBlock:
					goto OutOfRecordLoop;

				case BitStreamEntry::SubBlock:
					switch (entry.id)
					{
					default:
						if (stream_cursor_.SkipBlock())
						{
							this->Error("Invalid record");
							return;
						}
						break;

					case BitCode::BlockId::Constants:
						this->ParseConstants();
						next_value_no = static_cast<uint32_t>(value_list_.size());
						break;

					case BitCode::BlockId::ValueSymTab:
						this->ParseValueSymbolTable();
						break;

					case BitCode::BlockId::MetadataAttachment:
						this->ParseMetadataAttachment(func);
						break;

					case BitCode::BlockId::Metadata:
						this->ParseMetadata();
						break;

					case BitCode::BlockId::UseList:
						this->ParseUseLists();
						break;
					}
					continue;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				Instruction* inst = nullptr;
				uint32_t bit_code = stream_cursor_.ReadRecord(entry.id, record);
				switch (bit_code)
				{
				case BitCode::FunctionCode::DeclareBlocks: // DECLAREBLOCKS: [nblocks]
					{
						if ((record.size() < 1) || (record[0] == 0))
						{
							this->Error("Invalid record");
							return;
						}

						func_bbs_.resize(record[0]);

						auto bbfr_iter = basic_block_fwd_refs_.find(&func);
						if (bbfr_iter == basic_block_fwd_refs_.end())
						{
							for (uint32_t i = 0, e = static_cast<uint32_t>(func_bbs_.size()); i != e; ++ i)
							{
								func_bbs_[i] = BasicBlock::Create(*context_, "", &func);
							}
						}
						else
						{
							DILITHIUM_NOT_IMPLEMENTED;
						}

						cur_bb = func_bbs_[0];
					}
					continue;

				case BitCode::FunctionCode::DebugLocAgain:	// DEBUG_LOC_AGAIN
				case BitCode::FunctionCode::DebugLoc:		// DEBUG_LOC: [line, col, scope, ia]
					DILITHIUM_NOT_IMPLEMENTED;
					continue;

				case BitCode::FunctionCode::InstBinop:		// BINOP: [opval, ty, opval, opcode]
				case BitCode::FunctionCode::InstCast:		// CAST: [opval, opty, destty, castopc]
				case BitCode::FunctionCode::InstInboundsGepOld:
				case BitCode::FunctionCode::InstGepOld:
				case BitCode::FunctionCode::InstGep:		// GEP: type, [n x operands]
				case BitCode::FunctionCode::InstExtractVal:	// EXTRACTVAL: [opty, opval, n x indices]
				case BitCode::FunctionCode::InstInsertVal:	// INSERTVAL: [opty, opval, opty, opval, n x indices]
				case BitCode::FunctionCode::InstSelect:		// SELECT: [opval, ty, opval, opval]
				case BitCode::FunctionCode::InstVSelect:	// VSELECT: [ty,opval,opval,predty,pred]
				case BitCode::FunctionCode::InstExtractElt:	// EXTRACTELT: [opty, opval, opval]
				case BitCode::FunctionCode::InstInsertElt:	// INSERTELT: [ty, opval,opval,opval]
				case BitCode::FunctionCode::InstShuffleVec:	// SHUFFLEVEC: [opval,ty,opval,opval]
				case BitCode::FunctionCode::InstCmp:		// CMP: [opty, opval, opval, pred]
				case BitCode::FunctionCode::InstCmp2:		// CMP2: [opty, opval, opval, pred]
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::FunctionCode::InstRet: // RET: [opty,opval<optional>]
					if (record.empty())
					{
						inst = ReturnInst::Create(*context_);
						instruction_list_.push_back(inst);
						break;
					}

					DILITHIUM_NOT_IMPLEMENTED;
					break;
		
				case BitCode::FunctionCode::InstBr:			// BR: [bb#, bb#, opval] or [bb#]
				case BitCode::FunctionCode::InstSwitch:		// SWITCH: [opty, op0, op1, ...]
				case BitCode::FunctionCode::InstIndirectBr:	// INDIRECTBR: [opty, op0, op1, ...]
				case BitCode::FunctionCode::InstInvoke:		// INVOKE: [attrs, cc, normBB, unwindBB, fnty, op0,op1,op2, ...]
				case BitCode::FunctionCode::InstResume:		// RESUME: [opval]
				case BitCode::FunctionCode::InstUnreachable: // UNREACHABLE
				case BitCode::FunctionCode::InstPhi:		// PHI: [ty, val0,bb0, ...]
				case BitCode::FunctionCode::InstLandingPad:
				case BitCode::FunctionCode::InstLandingPadOld: // LANDINGPAD: [ty, val, val, num, (id0,val0 ...)?]
				case BitCode::FunctionCode::InstAlloca:		// ALLOCA: [instty, opty, op, align]
				case BitCode::FunctionCode::InstLoad:		// LOAD: [opty, op, align, vol]
				case BitCode::FunctionCode::InstLoadAtomic:	// LOADATOMIC: [opty, op, align, vol, ordering, synchscope]
				case BitCode::FunctionCode::InstStore:
				case BitCode::FunctionCode::InstStoreOld:	// STORE2:[ptrty, ptr, val, align, vol]
				case BitCode::FunctionCode::InstCmpXChgOld:
				case BitCode::FunctionCode::InstCmpXCHG:	// CMPXCHG:[ptrty, ptr, cmp, new, vol, successordering, synchscope, failureordering?, isweak?]
				case BitCode::FunctionCode::InstAtomicRmw:	// ATOMICRMW:[ptrty, ptr, val, op, vol, ordering, synchscope]
				case BitCode::FunctionCode::InstFence:		// FENCE:[ordering, synchscope]
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::FunctionCode::InstCall: // CALL: [paramattrs, cc, fnty, fnid, arg0, arg1...]
					{
						if (record.size() < 3)
						{
							this->Error("Invalid record");
							return;
						}

						uint32_t op_num = 0;
						AttributeSet pal = this->Attributes(static_cast<uint32_t>(record[op_num]));
						++ op_num;
						uint32_t cc_info = static_cast<uint32_t>(record[op_num]);
						++ op_num;

						FunctionType* fty = dyn_cast<FunctionType>(this->TypeByID(static_cast<uint32_t>(record[op_num])));
						if ((cc_info >> 15 & 1) && !fty)
						{
							this->Error("Explicit call type is not a function type");
							return;
						}
						++ op_num;

						Value* callee;
						if (this->ValueTypePair(record, op_num, next_value_no, callee))
						{
							this->Error("Invalid record");
							return;
						}

						auto op_ty = dyn_cast<PointerType>(callee->GetType());
						if (!op_ty)
						{
							this->Error("Callee is not a pointer type");
							return;
						}
						if (!fty)
						{
							fty = dyn_cast<FunctionType>(op_ty->ElementType());
							if (!fty)
							{
								this->Error("Callee is not of pointer to function type");
								return;
							}
						}
						else if (op_ty->ElementType() != fty)
						{
							this->Error("Explicit call type does not match pointee type of callee operand");
							return;
						}
				
						if (record.size() < fty->NumParams() + op_num)
						{
							this->Error("Insufficient operands to call");
							return;
						}

						boost::container::small_vector<Value*, 16> args;
						// Read the fixed params.
						for (uint32_t i = 0, e = fty->NumParams(); i != e; ++ i, ++ op_num)
						{
							if (fty->ParamType(i)->IsLabelType())
							{
								args.push_back(this->GetBasicBlock(static_cast<uint32_t>(record[op_num])));
							}
							else
							{
								args.push_back(this->GetValue(record, op_num, next_value_no, fty->ParamType(i)));
							}
							if (!args.back())
							{
								this->Error("Invalid record");
								return;
							}
						}

						// Read type/value pairs for varargs params.
						if (!fty->IsVarArg())
						{
							if (op_num != record.size())
							{
								this->Error("Invalid record");
								return;
							}
						}
						else
						{
							while (op_num != record.size())
							{
								Value* op;
								if (this->ValueTypePair(record, op_num, next_value_no, op))
								{
									this->Error("Invalid record");
									return;
								}
								args.push_back(op);
							}
						}

						inst = CallInst::Create(fty, callee, args);
						instruction_list_.push_back(inst);
						cast<CallInst>(inst)->SetCallingConv(static_cast<CallingConv::ID>((~(1U << 14) & cc_info) >> 1));
						CallInst::TailCallKind tck = CallInst::TCK_None;
						if (cc_info & 1)
						{
							tck = CallInst::TCK_Tail;
						}
						if (cc_info & (1 << 14))
						{
							tck = CallInst::TCK_MustTail;
						}
						cast<CallInst>(inst)->SetTailCallKind(tck);
						cast<CallInst>(inst)->SetAttributes(pal);
					}
					break;

				case BitCode::FunctionCode::InstVaArg: // VAARG: [valistty, valist, instty]
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				default:
					this->Error("Invalid value");
					return;
				}

				if (!cur_bb)
				{
					delete inst;
					this->Error("Invalid instruction with no BB");
					return;
				}
				// TODO: Store inst in a RAII manner to guarantee exception safty
				cur_bb->InstList().push_back(std::unique_ptr<Instruction>(inst));
				AddToSymbolTableList(inst, cur_bb);

				if (isa<TerminatorInst>(inst))
				{
					++ cur_bb_no;
					cur_bb = cur_bb_no < func_bbs_.size() ? func_bbs_[cur_bb_no] : nullptr;
				}

				if (inst && !inst->GetType()->IsVoidType())
				{
					value_list_.AssignValue(inst, next_value_no);
					++ next_value_no;
				}
			}

		OutOfRecordLoop:
			auto arg = dyn_cast<Argument>(value_list_.back());
			if (arg)
			{
				DILITHIUM_NOT_IMPLEMENTED;
			}

			// FIXME: Check for unresolved forward-declared metadata references
			// and clean up leaks.

			value_list_.resize(module_value_list_size);
			md_value_list_.resize(module_md_value_list_size);
			std::vector<BasicBlock*>().swap(func_bbs_);
		}
		void GlobalCleanup()
		{
			this->ResolveGlobalAndAliasInits();

			// TODO: LLVM looks for intrinsic functions which need to be upgraded at some point. But it doesn't seems we need it for DXIL.
#ifdef DILITHIUM_DEBUG
			for (auto& func : *the_module_)
			{
				BOOST_ASSERT((func->Name().size() <= 8) || (func->Name().find("llvm.") != 0));
			}
#endif

			// TODO: LLVM looks for global variables which need to be renamed. But it doesn't seems we need it for DXIL.
		}
		void ResolveGlobalAndAliasInits()
		{
			std::vector<std::pair<Function*, uint32_t>> func_prefix_worklist;
			std::vector<std::pair<Function*, uint32_t>> func_prologue_worklist;
			std::vector<std::pair<Function*, uint32_t>> func_personality_fn_worklist;

			func_prefix_worklist.swap(func_prefixes_);
			func_prologue_worklist.swap(func_prologues_);
			func_personality_fn_worklist.swap(func_personality_fns_);

			while (!func_prefix_worklist.empty())
			{
				uint32_t val_id = func_prefix_worklist.back().second;
				if (val_id >= value_list_.size())
				{
					func_prefixes_.push_back(func_prefix_worklist.back());
				}
				else
				{
					Constant* c = dyn_cast_or_null<Constant>(value_list_[val_id]);
					if (c)
					{
						func_prefix_worklist.back().first->SetPrefixData(c);
					}
					else
					{
						this->Error("Expected a constant");
					}
				}
				func_prefix_worklist.pop_back();
			}

			while (!func_prologue_worklist.empty())
			{
				uint32_t val_id = func_prologue_worklist.back().second;
				if (val_id >= value_list_.size())
				{
					func_prologues_.push_back(func_prologue_worklist.back());
				}
				else
				{
					Constant* c = dyn_cast_or_null<Constant>(value_list_[val_id]);
					if (c)
					{
						func_prologue_worklist.back().first->SetPrologueData(c);
					}
					else
					{
						this->Error("Expected a constant");
					}
				}
				func_prologue_worklist.pop_back();
			}

			while (!func_personality_fn_worklist.empty())
			{
				uint32_t val_id = func_personality_fn_worklist.back().second;
				if (val_id >= value_list_.size())
				{
					func_personality_fns_.push_back(func_personality_fn_worklist.back());
				}
				else
				{
					Constant* c = dyn_cast_or_null<Constant>(value_list_[val_id]);
					if (c)
					{
						func_personality_fn_worklist.back().first->SetPersonalityFn(c);
					}
					else
					{
						this->Error("Expected a constant");
					}
				}
				func_personality_fn_worklist.pop_back();
			}
		}
		void ParseMetadata()
		{
			is_metadata_materialized_ = true;
			uint32_t next_md_value_no = static_cast<uint32_t>(md_value_list_.size());

			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::Metadata))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;

			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					break;

				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				uint32_t code = stream_cursor_.ReadRecord(entry.id, record);
				bool distinct = false;
				switch (code)
				{
				case BitCode::MetadataCode::Name:
					{
						SmallString<8> name(record.begin(), record.end());
						record.clear();
						code = stream_cursor_.ReadCode();

						uint32_t next_bit_code = stream_cursor_.ReadRecord(code, record);
						if (next_bit_code != BitCode::MetadataCode::NamedNode)
						{
							this->Error("MetadataCode::Name not followed by MetadataCode::NamedNode");
							return;
						}

						uint32_t size = static_cast<uint32_t>(record.size());
						NamedMDNode* nmd = the_module_->GetOrInsertNamedMetadata(name);
						for (uint32_t i = 0; i != size; ++ i)
						{
							MDNode* md = dyn_cast_or_null<MDNode>(md_value_list_.ValueFwdRef(static_cast<uint32_t>(record[i])));
							if (!md)
							{
								this->Error("Invalid record");
								return;
							}
							nmd->AddOperand(md);
						}
					}
					break;

				case BitCode::MetadataCode::OldFnNode:
					// FIXME: Remove in 4.0.
					// This is a LocalAsMetadata record, the only type of function-local
					// metadata.
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::MetadataCode::OldNode:
					// FIXME: Remove in 4.0.
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::MetadataCode::Value:
					{
						if (record.size() != 2)
						{
							this->Error("Invalid record");
							return;
						}

						Type* ty = this->TypeByID(static_cast<uint32_t>(record[0]));
						if (ty->IsMetadataType() || ty->IsVoidType())
						{
							this->Error("Invalid record");
							return;
						}

						md_value_list_.AssignValue(
							ValueAsMetadata::Get(value_list_.ValueFwdRef(static_cast<uint32_t>(record[1]), ty)), next_md_value_no);
						++ next_md_value_no;
					}
					break;

				case BitCode::MetadataCode::DistinctNode:
					distinct = true;
					// fallthrough...
				case BitCode::MetadataCode::Node:
					{
						boost::container::small_vector<Metadata*, 8> elts;
						elts.reserve(record.size());
						for (auto id : record)
						{
							elts.push_back(id ? md_value_list_.ValueFwdRef(static_cast<uint32_t>(id - 1)) : nullptr);
						}
						md_value_list_.AssignValue(distinct ? MDNode::GetDistinct(*context_, elts) : MDNode::Get(*context_, elts),
							next_md_value_no);
						++ next_md_value_no;
					}
					break;

				case BitCode::MetadataCode::Location:
				case BitCode::MetadataCode::GenericDebug:
				case BitCode::MetadataCode::Subrange:
				case BitCode::MetadataCode::Enumerator:
				case BitCode::MetadataCode::BasicType:
				case BitCode::MetadataCode::DerivedType:
				case BitCode::MetadataCode::CompositeType:
				case BitCode::MetadataCode::SubroutineType:
				case BitCode::MetadataCode::Module:
				case BitCode::MetadataCode::File:
				case BitCode::MetadataCode::CompileUnit:
				case BitCode::MetadataCode::Subprogram:
				case BitCode::MetadataCode::LexicalBlock:
				case BitCode::MetadataCode::LexicalBlockFile:
				case BitCode::MetadataCode::Namespace:
				case BitCode::MetadataCode::TemplateType:
				case BitCode::MetadataCode::TemplateValue:
				case BitCode::MetadataCode::GlobalVar:
				case BitCode::MetadataCode::LocalVar:
				case BitCode::MetadataCode::Expression:
				case BitCode::MetadataCode::ObjCProperty:
				case BitCode::MetadataCode::ImportedEntity:
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				case BitCode::MetadataCode::String:
					{
						std::string str(record.begin(), record.end());
						// TODO: LLVM upgrades the MDStringConstant here. But it doesn't seems we need it for DXIL.
						BOOST_ASSERT(str != "llvm.vectorizer.unroll");
						BOOST_ASSERT(str.find("llvm.vectorizer.") != 0);
						Metadata* md = MDString::Get(*context_, str);
						md_value_list_.AssignValue(md, next_md_value_no);
						++ next_md_value_no;
					}
					break;

				case BitCode::MetadataCode::Kind:
					{
						if (record.size() < 2)
						{
							this->Error("Invalid record");
							return;
						}

						uint32_t kind = static_cast<uint32_t>(record[0]);
						SmallString<8> name(record.begin() + 1, record.end());
						uint32_t new_kind = the_module_->MdKindId(name.str());
						if (!md_kind_map_.insert(std::make_pair(kind, new_kind)).second)
						{
							this->Error("Conflicting MetadataCode::Kind records");
							return;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		void ParseMetadataAttachment(Function& func)
		{
			DILITHIUM_UNUSED(func);

			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::MetadataAttachment))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;
			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					return;
		
				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				record.clear();
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::MetadataCode::Attachment:
					DILITHIUM_NOT_IMPLEMENTED;
					break;

				default:
					break;
				}
			}
		}
		void ParseUseLists()
		{
			if (stream_cursor_.EnterSubBlock(BitCode::BlockId::UseList))
			{
				this->Error("Invalid record");
				return;
			}

			boost::container::small_vector<uint64_t, 64> record;
			for (;;)
			{
				BitStreamEntry entry = stream_cursor_.AdvanceSkippingSubblocks(0);

				switch (entry.kind)
				{
				case BitStreamEntry::SubBlock: // Handled for us already.
				case BitStreamEntry::Error:
					this->Error("Malformed block");
					return;

				case BitStreamEntry::EndBlock:
					return;

				case BitStreamEntry::Record:
					// The interesting case.
					break;
				}

				// Read a use list record.
				record.clear();
				bool bb = false;
				switch (stream_cursor_.ReadRecord(entry.id, record))
				{
				case BitCode::UseListCode::Bb:
					bb = true;
					// fallthrough
				case BitCode::UseListCode::Default:
					{
						uint32_t record_len = static_cast<uint32_t>(record.size());
						if (record_len < 3)
						{
							this->Error("Invalid record");
							return;
						}

						uint32_t id = static_cast<uint32_t>(record.back());
						record.pop_back();

						Value* v;
						if (bb)
						{
							BOOST_ASSERT_MSG(id < func_bbs_.size(), "Basic block not found");
							v = func_bbs_[id];
						}
						else
						{
							v = value_list_[id];
						}
						uint32_t num_uses = 0;
						// TODO: Normally less than 16
						std::unordered_map<Use const *, uint32_t> order;
						for (auto const & use : v->Uses())
						{
							++ num_uses;
							if (num_uses > record.size())
							{
								break;
							}
							order[&use] = static_cast<uint32_t>(record[num_uses - 1]);
						}
						if ((order.size() != record.size()) || (num_uses > record.size()))
						{
							// Mismatches can happen if the functions are being materialized lazily
							// (out-of-order), or a value has been upgraded.
							break;
						}

						v->SortUseList([&](Use const & lhs, Use const & rhs) { return order[&lhs] < order[&rhs]; });
					}
					break;

				default:  // Default behavior: unknown type.
					break;
				}
			}
		}

		void InitStream()
		{
			uint8_t const * buff_beg = buffer_;
			uint8_t const * buff_end = buff_beg + buffer_length_;

			if (buffer_length_ & 3)
			{
				TERROR("Invalid bitcode size"); // HLSL Change - bitcode size is the problem, not the signature per se
			}

			// If we have a wrapper header, parse it and ignore the non-bc file contents.
			// The magic number is 0x0B17C0DE stored in little endian.
			if (IsBitcodeWrapper(buff_beg, buff_end))
			{
				if (SkipBitcodeWrapperHeader(buff_beg, buff_end, true))
				{
					TERROR("Invalid bitcode wrapper header");
				}
			}

			stream_file_ = std::make_unique<BitStreamReader>(buff_beg, buff_end);
			stream_cursor_.Init(stream_file_.get());
		}
		void FindFunctionInStream(Function& func, std::unordered_map<Function*, uint64_t>::iterator deferred_func_info_iter)
		{
			DILITHIUM_UNUSED(func);

			while (deferred_func_info_iter->second == 0)
			{
				if (stream_cursor_.AtEndOfStream())
				{
					this->Error("Could not find function in stream");
					return;
				}
				// ParseModule will parse the next body in the stream and set its
				// position in the deferred_func_info_ map.
				this->ParseModule(true);
			}
		}

	private:
		std::shared_ptr<LLVMContext> const & context_;

		LLVMModule* the_module_ = nullptr;
		uint8_t const * buffer_ = nullptr;
		uint32_t buffer_length_ = 0;
		std::unique_ptr<BitStreamReader> stream_file_;
		BitStreamCursor stream_cursor_;
		uint64_t next_unread_bit_ = 0;
		bool seen_value_sym_tab_ = false;

		std::vector<Type*> type_list_;
		BitcodeReaderValueList value_list_;
		BitcodeReaderMDValueList md_value_list_;
		boost::container::small_vector<Instruction*, 64> instruction_list_;

		std::vector<std::pair<Function*, uint32_t>> func_prefixes_;
		std::vector<std::pair<Function*, uint32_t>> func_prologues_;
		std::vector<std::pair<Function*, uint32_t>> func_personality_fns_;

		std::vector<AttributeSet> m_attribs_;
		std::map<uint32_t, AttributeSet> m_attrib_groups_;

		std::vector<BasicBlock*> func_bbs_;
		std::vector<Function*> func_with_bodies_;

		std::unordered_map<uint32_t, uint32_t> md_kind_map_;

		bool seen_first_func_body_ = false;

		std::unordered_map<Function*, uint64_t> deferred_func_info_;
		std::vector<uint64_t> deferred_metadata_info_;

		std::unordered_map<Function*, std::vector<BasicBlock*>> basic_block_fwd_refs_;
		std::deque<Function*> basic_block_fwd_ref_queue_;
		bool use_relative_ids_ = false;
		bool will_materialize_all_forward_refs_ = false;
		bool is_metadata_materialized_ = false;
		std::vector<StructType*> identified_struct_types_;
	};
}

namespace Dilithium
{
	std::unique_ptr<LLVMModule> LoadLLVMModule(uint8_t const * data, uint32_t data_length, std::string const & name)
	{
		auto context = std::make_shared<LLVMContext>();
		auto reader = std::make_shared<BitcodeReader>(data, data_length, context);
		auto mod = std::make_unique<LLVMModule>(name, context);
		mod->Materializer(reader);
		reader->ParseBitcodeInto(mod.get(), false);
		mod->MaterializeAllPermanently();

		return std::move(mod);
	}
}
