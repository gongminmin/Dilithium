/**
 * @file DataLayout.hpp
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
 
#ifndef _DILITHIUM_DATA_LAYOUT_HPP
#define _DILITHIUM_DATA_LAYOUT_HPP

#pragma once

#include <Dilithium/MathExtras.hpp>

#include <string>
#include <unordered_map>

#include <boost/container/small_vector.hpp>

namespace Dilithium
{
	class DataLayout;
	class StructType;
	class Type;

	enum AlignTypeEnum
	{
		INVALID_ALIGN = 0,
		INTEGER_ALIGN = 'i',
		VECTOR_ALIGN = 'v',
		FLOAT_ALIGN = 'f',
		AGGREGATE_ALIGN = 'a'
	};

	struct LayoutAlignElem
	{
		uint32_t align_type : 8;
		uint32_t type_bit_width : 24;
		uint32_t abi_align : 16;
		uint32_t pref_align : 16;

		static LayoutAlignElem Get(AlignTypeEnum align_type, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width);
		bool operator==(LayoutAlignElem const & rhs) const;
	};

	struct PointerAlignElem
	{
		uint32_t abi_align;
		uint32_t pref_align;
		uint32_t type_byte_width;
		uint32_t addr_space;

		static PointerAlignElem Get(uint32_t AddressSpace, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width);
		bool operator==(PointerAlignElem const & rhs) const;
	};

	class StructLayout
	{
	public:
		StructLayout(StructType* st, DataLayout const & dl);

		uint64_t SizeInBytes() const
		{
			return struct_size_;
		}
		uint64_t SizeInBits() const
		{
			return struct_size_ * 8;
		}
		uint32_t Alignment() const
		{
			return struct_alignment_;
		}

		uint32_t ElementContainingOffset(uint64_t offset) const;
		uint64_t ElementOffset(uint32_t index) const
		{
			return member_offsets_[index];
		}
		uint64_t ElementOffsetInBits(uint32_t index) const
		{
			return this->ElementOffset(index) * 8;
		}

	private:
		uint64_t struct_size_;
		uint32_t struct_alignment_;
		std::vector<uint64_t> member_offsets_;
	};

	class DataLayout
	{
		typedef boost::container::small_vector<PointerAlignElem, 8> PointersTy;

	public:
		explicit DataLayout(std::string_view desc);
		DataLayout(DataLayout const & rhs);
		~DataLayout();

		DataLayout& operator=(DataLayout const & rhs);

		void Reset(std::string_view desc);

		bool operator==(DataLayout const & rhs) const;
		bool operator!=(DataLayout const & rhs) const
		{
			return !(*this == rhs);
		}

		uint32_t PointerABIAlignment(uint32_t addr_space) const;
		uint32_t PointerPrefAlignment(uint32_t addr_space) const;
		uint32_t PointerSize(uint32_t addr_space) const;
		uint32_t PointerSizeInBits(uint32_t addr_space) const
		{
			return PointerSize(addr_space) * 8;
		}

		uint64_t TypeSizeInBits(Type* ty) const;
		uint64_t TypeStoreSize(Type* ty) const
		{
			return (this->TypeSizeInBits(ty) + 7) / 8;
		}
		uint64_t TypeStoreSizeInBits(Type* ty) const
		{
			return this->TypeStoreSize(ty) * 8;
		}
		uint64_t TypeAllocSize(Type* ty) const
		{
			return RoundUpToAlignment(this->TypeStoreSize(ty), this->ABITypeAlignment(ty));
		}
		uint64_t TypeAllocSizeInBits(Type* ty) const
		{
			return this->TypeAllocSize(ty) * 8;
		}

		uint32_t ABITypeAlignment(Type* ty) const
		{
			return this->Alignment(ty, true);
		}

		StructLayout const * GetStructLayout(StructType* ty) const;

	private:
		PointersTy::const_iterator FindPointerLowerBound(uint32_t addr_space) const
		{
			return const_cast<DataLayout *>(this)->FindPointerLowerBound(addr_space);
		}
		PointersTy::iterator FindPointerLowerBound(uint32_t addr_space);

		void Alignment(AlignTypeEnum align_type, uint32_t abi_align, uint32_t pref_align, uint32_t bit_width);
		uint32_t Alignment(Type* ty, bool abi_or_pref) const;
		void PointerAlignment(uint32_t addr_space, uint32_t abi_align, uint32_t pref_align, uint32_t type_byte_width);
		uint32_t AlignmentInfo(AlignTypeEnum align_type, uint32_t bit_width, bool abi_align, Type* ty) const;

		void ParseSpecifier(std::string_view desc);

		void Clear();

	private:
		bool big_endian_;

		uint32_t stack_natural_align_;

		enum ManglingMode
		{
			MM_None,
			MM_ELF,
			MM_MachO,
			MM_WinCOFF,
			MM_WinCOFFX86,
			MM_Mips
		};
		ManglingMode mangling_mode_;

		boost::container::small_vector<uint8_t, 8> legal_int_widths_;

		boost::container::small_vector<LayoutAlignElem, 16> alignments_;

		std::string string_representation_;

		PointersTy pointers_;

		static const LayoutAlignElem invalid_alignment_elem_;
		static const PointerAlignElem invalid_pointer_elem_;

		mutable std::unordered_map<StructType*, std::unique_ptr<StructLayout>> layout_map_;
	};
}

#endif		// _DILITHIUM_DATA_LAYOUT_HPP
