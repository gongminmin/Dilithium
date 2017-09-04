/**
 * @file DataLayout.cpp
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
#include <Dilithium/DataLayout.hpp>
#include <Dilithium/DerivedType.hpp>

namespace
{
	using namespace Dilithium;

	static LayoutAlignElem const default_alignments[] =
	{
		{ INTEGER_ALIGN, 1, 1, 1 },    // i1
		{ INTEGER_ALIGN, 8, 1, 1 },    // i8
		{ INTEGER_ALIGN, 16, 2, 2 },   // i16
		{ INTEGER_ALIGN, 32, 4, 4 },   // i32
		{ INTEGER_ALIGN, 64, 4, 8 },   // i64
		{ FLOAT_ALIGN, 16, 2, 2 },     // half
		{ FLOAT_ALIGN, 32, 4, 4 },     // float
		{ FLOAT_ALIGN, 64, 8, 8 },     // double
		{ FLOAT_ALIGN, 128, 16, 16 },  // ppcf128, quad, ...
		{ VECTOR_ALIGN, 64, 8, 8 },    // v2i32, v1i64, ...
		{ VECTOR_ALIGN, 128, 16, 16 }, // v16i8, v8i16, v4i32, ...
		{ AGGREGATE_ALIGN, 0, 0, 8 }   // struct
	};

	std::pair<std::string_view, std::string_view> Split(std::string_view str, char separator)
	{
		BOOST_ASSERT_MSG(!str.empty(), "Parse error, string can't be empty here");

		std::pair<std::string_view, std::string_view> split;
		size_t index = str.find(separator);
		if (index == std::string_view::npos)
		{
			split = std::make_pair(str, std::string_view());
		}
		else
		{
			split = std::make_pair(str.substr(0, index), str.substr(index + 1));
		}

		if (split.second.empty() && (split.first != str))
		{
			TERROR("Trailing separator in datalayout string");
		}
		if (!split.second.empty() && split.first.empty())
		{
			TERROR("Expected token before separator in datalayout string");
		}
		return split;
	}

	uint32_t ToInt(std::string_view sv)
	{
		return std::stoi(std::string(sv));
	}

	uint32_t InBytes(uint32_t bits)
	{
		if (bits & 7)
		{
			TERROR("number of bits must be a byte width multiple");
		}
		return bits / 8;
	}
}

namespace Dilithium 
{
	LayoutAlignElem LayoutAlignElem::Get(AlignTypeEnum align_type, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width)
	{
		BOOST_ASSERT_MSG(abi_align <= pref_align, "Preferred alignment worse than ABI!");
		LayoutAlignElem retval;
		retval.align_type = align_type;
		retval.abi_align = abi_align;
		retval.pref_align = pref_align;
		retval.type_bit_width = type_byte_width;
		return retval;
	}

	bool LayoutAlignElem::operator==(LayoutAlignElem const & rhs) const
	{
		return (align_type == rhs.align_type)
			&& (abi_align == rhs.abi_align)
			&& (pref_align == rhs.pref_align)
			&& (type_bit_width == rhs.type_bit_width);
	}


	PointerAlignElem PointerAlignElem::Get(uint32_t addr_space, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width)
	{
		BOOST_ASSERT_MSG(abi_align <= pref_align, "Preferred alignment worse than ABI!");
		PointerAlignElem retval;
		retval.addr_space = addr_space;
		retval.abi_align = abi_align;
		retval.pref_align = pref_align;
		retval.type_byte_width = type_byte_width;
		return retval;
	}

	bool PointerAlignElem::operator==(PointerAlignElem const & rhs) const
	{
		return (abi_align == rhs.abi_align)
			&& (addr_space == rhs.addr_space)
			&& (pref_align == rhs.pref_align)
			&& (type_byte_width == rhs.type_byte_width);
	}


	StructLayout::StructLayout(StructType* st, DataLayout const & dl)
		: struct_size_(0), struct_alignment_(0), member_offsets_(st->NumElements())
	{
		BOOST_ASSERT_MSG(!st->IsOpaque(), "Cannot get layout of opaque structs");

		for (size_t i = 0, e = member_offsets_.size(); i != e; ++ i)
		{
			Type* ty = st->ElementType(static_cast<uint32_t>(i));
			uint32_t ty_align = st->IsPacked() ? 1 : dl.ABITypeAlignment(ty);

			if ((struct_size_ & (ty_align - 1)) != 0)
			{
				struct_size_ = RoundUpToAlignment(struct_size_, ty_align);
			}

			struct_alignment_ = std::max(ty_align, struct_alignment_);

			member_offsets_[i] = struct_size_;
			struct_size_ += dl.TypeAllocSize(ty);
		}

		if (struct_alignment_ == 0)
		{
			struct_alignment_ = 1;
		}

		if ((struct_size_ & (struct_alignment_ - 1)) != 0)
		{
			struct_size_ = RoundUpToAlignment(struct_size_, struct_alignment_);
		}
	}

	uint32_t StructLayout::ElementContainingOffset(uint64_t offset) const
	{
		auto si = std::upper_bound(member_offsets_.begin(), member_offsets_.end(), offset);
		BOOST_ASSERT_MSG(si != member_offsets_.begin(), "Offset not in structure type!");
		-- si;
		BOOST_ASSERT_MSG(*si <= offset, "upper_bound didn't work");
		BOOST_ASSERT_MSG(((si == member_offsets_.begin()) || (*(si - 1) <= offset))
			&& ((si + 1 == member_offsets_.end()) || (*(si + 1) > offset)),
			"Upper bound didn't work!");

		return static_cast<uint32_t>(si - member_offsets_.begin());
	}


	DataLayout::DataLayout(std::string_view desc)
	{
		this->Reset(desc);
	}

	DataLayout::DataLayout(DataLayout const & rhs)
		: big_endian_(rhs.big_endian_),
			stack_natural_align_(rhs.stack_natural_align_),
			mangling_mode_(rhs.mangling_mode_),
			legal_int_widths_(rhs.legal_int_widths_),
			alignments_(rhs.alignments_),
			string_representation_(rhs.string_representation_),
			pointers_(rhs.pointers_)
	{
	}

	DataLayout::~DataLayout()
	{
		this->Clear();
	}

	DataLayout& DataLayout::operator=(DataLayout const & rhs)
	{
		if (this != &rhs)
		{
			this->Clear();

			big_endian_ = rhs.big_endian_;
			stack_natural_align_ = rhs.stack_natural_align_;
			mangling_mode_ = rhs.mangling_mode_;
			legal_int_widths_ = rhs.legal_int_widths_;
			alignments_ = rhs.alignments_;
			string_representation_ = rhs.string_representation_;
			pointers_ = rhs.pointers_;
		}
		return *this;
	}

	void DataLayout::Reset(std::string_view desc)
	{
		this->Clear();

		big_endian_ = false;
		stack_natural_align_ = 0;
		mangling_mode_ = MM_None;

		for (auto const & e : default_alignments)
		{
			this->Alignment(static_cast<AlignTypeEnum>(e.align_type), e.abi_align, e.pref_align, e.type_bit_width);
		}
		this->PointerAlignment(0, 8, 8, 8);
		this->ParseSpecifier(desc);
	}

	bool DataLayout::operator==(DataLayout const & rhs) const
	{
		return (big_endian_ == rhs.big_endian_)
			&& (stack_natural_align_ == rhs.stack_natural_align_)
			&& (mangling_mode_ == rhs.mangling_mode_)
			&& (legal_int_widths_ == rhs.legal_int_widths_)
			&& (alignments_ == rhs.alignments_)
			&& (pointers_ == rhs.pointers_);
	}

	uint32_t DataLayout::PointerABIAlignment(uint32_t addr_space) const
	{
		auto iter = this->FindPointerLowerBound(addr_space);
		if ((iter == pointers_.end()) || (iter->addr_space != addr_space))
		{
			iter = this->FindPointerLowerBound(0);
			BOOST_ASSERT(iter->addr_space == 0);
		}
		return iter->abi_align;
	}

	uint32_t DataLayout::PointerPrefAlignment(uint32_t addr_space) const
	{
		auto iter = this->FindPointerLowerBound(addr_space);
		if ((iter == pointers_.end()) || (iter->addr_space != addr_space))
		{
			iter = this->FindPointerLowerBound(0);
			BOOST_ASSERT(iter->addr_space == 0);
		}
		return iter->pref_align;
	}

	uint32_t DataLayout::PointerSize(uint32_t addr_space) const
	{
		auto iter = this->FindPointerLowerBound(addr_space);
		if ((iter == pointers_.end()) || (iter->addr_space != addr_space))
		{
			iter = this->FindPointerLowerBound(0);
			BOOST_ASSERT(iter->addr_space == 0);
		}
		return iter->type_byte_width;
	}

	uint64_t DataLayout::TypeSizeInBits(Type* ty) const
	{
		BOOST_ASSERT_MSG(ty->IsSized(), "Cannot getTypeInfo() on a type that is unsized!");

		switch (ty->GetTypeId())
		{
		case Type::TID_Label:
			return this->PointerSizeInBits(0);
		case Type::TID_Pointer:
			return this->PointerSizeInBits(ty->PointerAddressSpace());
		case Type::TID_Array:
			{
				ArrayType* arr_ty = cast<ArrayType>(ty);
				return arr_ty->NumElements() * this->TypeAllocSizeInBits(arr_ty->ElementType());
			}
		case Type::TID_Struct:
			return this->GetStructLayout(cast<StructType>(ty))->SizeInBits();
		case Type::TID_Integer:
			return ty->IntegerBitWidth();
		case Type::TID_Half:
			return 16;
		case Type::TID_Float:
			return 32;
		case Type::TID_Double:
		case Type::TID_X86Mmx:
			return 64;
		case Type::TID_Fp128:
		case Type::TID_PpcFp128:
			return 128;
		case Type::TID_X86Fp80:
			// In memory objects this is always aligned to a higher boundary, but
			// only 80 bits contain information.
			return 80;
		case Type::TID_Vector:
			{
				VectorType* vec_ty = cast<VectorType>(ty);
				return vec_ty->NumElements() * this->TypeAllocSizeInBits(vec_ty->ElementType());
			}

		default:
			DILITHIUM_UNREACHABLE("Unsupported type");
		}
	}

	StructLayout const * DataLayout::GetStructLayout(StructType* ty) const
	{
		auto& sl = layout_map_[ty];
		if (!sl)
		{
			sl = std::make_unique<StructLayout>(ty, *this);
		}
		return sl.get();
	}

	DataLayout::PointersTy::iterator DataLayout::FindPointerLowerBound(uint32_t addr_space)
	{
		return std::lower_bound(pointers_.begin(), pointers_.end(), addr_space,
			[](PointerAlignElem const & pae, uint32_t addr_space)
			{
				return pae.addr_space < addr_space;
			});
	}

	void DataLayout::Alignment(AlignTypeEnum align_type, uint32_t abi_align, uint32_t pref_align, uint32_t bit_width)
	{
		if (!IsUInt<24>(bit_width))
		{
			TERROR("Invalid bit width, must be a 24bit integer");
		}
		if (!IsUInt<16>(abi_align))
		{
			TERROR("Invalid ABI alignment, must be a 16bit integer");
		}
		if (!IsUInt<16>(pref_align))
		{
			TERROR("Invalid preferred alignment, must be a 16bit integer");
		}
		if (abi_align != 0 && !IsPowerOfTwo64(abi_align))
		{
			TERROR("Invalid ABI alignment, must be a power of 2");
		}
		if (pref_align != 0 && !IsPowerOfTwo64(pref_align))
		{
			TERROR("Invalid preferred alignment, must be a power of 2");
		}

		if (pref_align < abi_align)
		{
			TERROR("Preferred alignment cannot be less than the ABI alignment");
		}

		for (auto& elem : alignments_)
		{
			if ((elem.align_type == static_cast<uint32_t>(align_type)) && (elem.type_bit_width == bit_width))
			{
				elem.abi_align = abi_align;
				elem.pref_align = pref_align;
				return;
			}
		}

		alignments_.push_back(LayoutAlignElem::Get(align_type, abi_align, pref_align, bit_width));
	}

	uint32_t DataLayout::Alignment(Type* ty, bool abi_or_pref) const
	{
		int align_type = -1;

		BOOST_ASSERT_MSG(ty->IsSized(), "Cannot process a type that is unsized!");
		switch (ty->GetTypeId())
		{
		case Type::TID_Label:
			return abi_or_pref ? this->PointerABIAlignment(0) : this->PointerPrefAlignment(0);
		case Type::TID_Pointer:
			{
				uint32_t addr_space = cast<PointerType>(ty)->AddressSpace();
				return abi_or_pref ? this->PointerABIAlignment(addr_space) : this->PointerPrefAlignment(addr_space);
			}
		case Type::TID_Array:
			return this->Alignment(cast<ArrayType>(ty)->ElementType(), abi_or_pref);

		case Type::TID_Struct:
			{
				if (cast<StructType>(ty)->IsPacked() && abi_or_pref)
				{
					return 1;
				}

				auto layout = this->GetStructLayout(cast<StructType>(ty));
				uint32_t align = this->AlignmentInfo(AGGREGATE_ALIGN, 0, abi_or_pref, ty);
				return std::max(align, layout->Alignment());
			}
		case Type::TID_Integer:
			align_type = INTEGER_ALIGN;
			break;
		case Type::TID_Half:
		case Type::TID_Float:
		case Type::TID_Double:
		case Type::TID_Fp128:
		case Type::TID_PpcFp128:
		case Type::TID_X86Fp80:
			align_type = FLOAT_ALIGN;
			break;
		case Type::TID_X86Mmx:
		case Type::TID_Vector:
			align_type = VECTOR_ALIGN;
			return this->Alignment(ty->VectorElementType(), abi_or_pref);
		default:
			DILITHIUM_UNREACHABLE("Invalid type");
		}

		return this->AlignmentInfo(static_cast<AlignTypeEnum>(align_type), static_cast<uint32_t>(this->TypeSizeInBits(ty)),
			abi_or_pref, ty);
	}

	void DataLayout::PointerAlignment(uint32_t addr_space, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width)
	{
		if (pref_align < abi_align)
		{
			TERROR("Preferred alignment cannot be less than the ABI alignment");
		}

		auto iter = this->FindPointerLowerBound(addr_space);
		if ((iter == pointers_.end()) || (iter->addr_space != addr_space))
		{
			pointers_.insert(iter, PointerAlignElem::Get(addr_space, abi_align, pref_align, type_byte_width));
		}
		else
		{
			iter->abi_align = abi_align;
			iter->pref_align = pref_align;
			iter->type_byte_width = type_byte_width;
		}
	}

	uint32_t DataLayout::AlignmentInfo(AlignTypeEnum align_type, uint32_t bit_width, bool abi_info, Type* ty) const
	{
		int best_match_idx = -1;
		int largest_int = -1;
		for (size_t i = 0, e = alignments_.size(); i != e; ++ i)
		{
			if ((alignments_[i].align_type == static_cast<uint32_t>(align_type)) && (alignments_[i].type_bit_width == bit_width))
			{
				return abi_info ? alignments_[i].abi_align : alignments_[i].pref_align;
			}

			if ((align_type == INTEGER_ALIGN) && (alignments_[i].align_type == INTEGER_ALIGN))
			{
				if ((alignments_[i].type_bit_width > bit_width)
					&& ((best_match_idx == -1) || (alignments_[i].type_bit_width < alignments_[best_match_idx].type_bit_width)))
				{
					best_match_idx = static_cast<int>(i);
				}
				if ((largest_int == -1) || (alignments_[i].type_bit_width > alignments_[largest_int].type_bit_width))
				{
					largest_int = static_cast<int>(i);
				}
			}
		}

		if (best_match_idx == -1)
		{
			if (align_type == INTEGER_ALIGN)
			{
				best_match_idx = largest_int;
			}
			else if (align_type == VECTOR_ALIGN)
			{
				uint32_t align = static_cast<uint32_t>(this->TypeAllocSize(cast<VectorType>(ty)->ElementType()));
				align *= cast<VectorType>(ty)->NumElements();
				if (align & (align - 1))
				{
					align = static_cast<uint32_t>(NextPowerOf2(align));
				}
				return align;
			}
		}

		if (best_match_idx == -1)
		{
			uint32_t align = static_cast<uint32_t>(this->TypeStoreSize(ty));
			if (align & (align - 1))
			{
				align = static_cast<uint32_t>(NextPowerOf2(align));
			}
			return align;
		}

		return abi_info ? alignments_[best_match_idx].abi_align : alignments_[best_match_idx].pref_align;
	}

	void DataLayout::ParseSpecifier(std::string_view desc)
	{
		string_representation_ = std::string(desc);
		while (!desc.empty())
		{
			std::pair<std::string_view, std::string_view> ss = Split(desc, '-');
			desc = ss.second;

			ss = Split(ss.first, ':');

			auto& tok = ss.first;
			auto& rest = ss.second;

			char specifier = tok.front();
			tok = tok.substr(1);

			switch (specifier)
			{
			case 's':
				// Ignored for backward compatibility.
				// FIXME: remove this on LLVM 4.0.
				break;
			case 'E':
				big_endian_ = true;
				break;
			case 'e':
				big_endian_ = false;
				break;
			case 'p':
				{
					uint32_t addr_space = tok.empty() ? 0 : ToInt(tok);
					if (!IsUInt<24>(addr_space))
					{
						TERROR("Invalid address space, must be a 24bit integer");
					}

					if (rest.empty())
					{
						TERROR("Missing size specification for pointer in datalayout string");
					}
					ss = Split(rest, ':');
					uint32_t pointer_mem_size = InBytes(ToInt(tok));
					if (!pointer_mem_size)
					{
						TERROR("Invalid pointer size of 0 bytes");
					}

					if (rest.empty())
					{
						TERROR("Missing alignment specification for pointer in datalayout string");
					}
					ss = Split(rest, ':');
					uint32_t pointer_abi_align = InBytes(ToInt(tok));
					if (!IsPowerOfTwo64(pointer_abi_align))
					{
						TERROR("Pointer ABI alignment must be a power of 2");
					}

					uint32_t pointer_pref_align = pointer_abi_align;
					if (!rest.empty())
					{
						ss = Split(rest, ':');
						pointer_pref_align = InBytes(ToInt(tok));
						if (!IsPowerOfTwo64(pointer_pref_align))
						{
							TERROR("Pointer preferred alignment must be a power of 2");
						}
					}

					this->PointerAlignment(addr_space, pointer_abi_align, pointer_pref_align, pointer_mem_size);
					break;
				}
			case 'i':
			case 'v':
			case 'f':
			case 'a':
				{
					AlignTypeEnum align_type;
					switch (specifier)
					{
					default:
					case 'i':
						align_type = INTEGER_ALIGN;
						break;
					case 'v':
						align_type = VECTOR_ALIGN;
						break;
					case 'f':
						align_type = FLOAT_ALIGN;
						break;
					case 'a':
						align_type = AGGREGATE_ALIGN;
						break;
					}

					uint32_t size = tok.empty() ? 0 : ToInt(tok);

					if ((align_type == AGGREGATE_ALIGN) && (size != 0))
					{
						TERROR("Sized aggregate specification in datalayout string");
					}

					if (rest.empty())
					{
						TERROR("Missing alignment specification in datalayout string");
					}
					ss = Split(rest, ':');
					uint32_t abi_align = InBytes(ToInt(tok));
					if (align_type != AGGREGATE_ALIGN && !abi_align)
					{
						TERROR("ABI alignment specification must be >0 for non-aggregate types");
					}

					uint32_t pref_align = abi_align;
					if (!rest.empty())
					{
						ss = Split(rest, ':');
						pref_align = InBytes(ToInt(tok));
					}

					this->Alignment(align_type, abi_align, pref_align, size);
					break;
				}
			case 'n':
				// Native integer types.
				for (;;)
				{
					uint32_t width = ToInt(tok);
					if (width == 0)
					{
						TERROR("Zero width native integer type in datalayout string");
					}
					legal_int_widths_.push_back(static_cast<uint8_t>(width));
					if (rest.empty())
					{
						break;
					}
					ss = Split(rest, ':');
				}
				break;
			case 'S':
				stack_natural_align_ = InBytes(ToInt(tok));
				break;
			case 'm':
				if (!tok.empty())
				{
					TERROR("Unexpected trailing characters after mangling specifier in datalayout string");
				}
				if (rest.empty())
				{
					TERROR("Expected mangling specifier in datalayout string");
				}
				if (rest.size() > 1)
				{
					TERROR("Unknown mangling specifier in datalayout string");
				}
				switch (rest[0])
				{
				case 'e':
					mangling_mode_ = MM_ELF;
					break;
				case 'o':
					mangling_mode_ = MM_MachO;
					break;
				case 'm':
					mangling_mode_ = MM_Mips;
					break;
				case 'w':
					mangling_mode_ = MM_WinCOFF;
					break;
				case 'x':
					mangling_mode_ = MM_WinCOFFX86;
					break;

				default:
					TERROR("Unknown mangling in datalayout string");
				}
				break;

			default:
				DILITHIUM_UNREACHABLE("Unknown specifier in datalayout string");
			}
		}
	}

	void DataLayout::Clear()
	{
		legal_int_widths_.clear();
		alignments_.clear();
		pointers_.clear();
		layout_map_.clear();
	}
}
