/**
 * @file BitCodes.hpp
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

#ifndef _DILITHIUM_BIT_CODES_HPP
#define _DILITHIUM_BIT_CODES_HPP

#pragma once

#include <Dilithium/Dilithium.hpp>

#include <boost/assert.hpp>
#include <boost/container/small_vector.hpp>

namespace Dilithium
{
	namespace BitCode
	{
		// Majorly from LLVM

		struct StandardWidth
		{
			enum
			{
				BlockIdWidth = 8,  // We use VBR-8 for block IDs.
				CodeLenWidth = 4,  // Codelen are VBR-4.
				BlockSizeWidth = 32  // BlockSize up to 2^32 32-bit words = 16GB per block.
			};
		};

		// The standard abbrev namespace always has a way to exit a block, enter a
		// nested block, define abbrevs, and define an unabbreviated record.
		struct FixedAbbrevId
		{
			enum
			{
				EndBlock = 0,  // Must be zero to guarantee termination for broken bitcode.
				EnterSubblock = 1,

				/// DefineAbbrev - Defines an abbrev for the current block.  It consists
				/// of a vbr5 for # operand infos.  Each operand info is emitted with a
				/// single bit to indicate if it is a literal encoding.  If so, the value is
				/// emitted with a vbr8.  If not, the encoding is emitted as 3 bits followed
				/// by the info value as a vbr5 if needed.
				DefineAbbrev = 2,

				// UnabbrevRecord are emitted with a vbr6 for the record code, followed by
				// a vbr6 for the # operands, followed by vbr6's for each operand.
				UnabbrevRecord = 3,

				// This is not a code, this is a marker for the first abbrev assignment.
				FirstApplicationAbbrev = 4
			};
		};

		/// StandardBlockId - All bitcode files can optionally include a BLOCKINFO
		/// block, which contains metadata about other blocks in the file.
		struct StandardBlockId
		{
			enum
			{
				/// BlockInfoBlockId is used to define metadata about blocks, for example,
				/// standard abbrevs that should be available to all blocks of a specified
				/// ID.
				BlockInfoBlockId = 0,

				// Block IDs 1-7 are reserved for future expansion.
				FirstApplicationBlockId = 8
			};
		};

		/// BlockInfoCodes - The blockinfo block contains metadata about user-defined
		/// blocks.
		struct BlockInfoCode
		{
			enum
			{
				// DEFINE_ABBREV has magic semantics here, applying to the current SETBID'd
				// block, instead of the BlockInfo block.

				SetBlockId = 1, // SetBlockId: [blockid#]
				BlockName = 2, // BlockName: [name]
				SetRecordName = 3  // SetRecordName: [id, name]
			};
		};
	}

	/// BitCodeAbbrevOp - This describes one or more operands in an abbreviation.
	/// This is actually a union of two different things:
	///   1. It could be a literal integer value ("the operand is always 17").
	///   2. It could be an encoding specification ("this operand encoded like so").
	///
	class BitCodeAbbrevOp
	{
	public:
		enum class BitCodeEncoding : uint8_t
		{
			Fixed = 1,	// A fixed width field, Val specifies number of bits.
			VBR = 2,	// A VBR field where Val specifies the width of each chunk.
			Array = 3,	// A sequence of fields, next field species elt encoding.
			Char6 = 4,	// A 6-bit fixed field which maps to [a-zA-Z0-9._].
			Blob = 5	// 32-bit aligned array of 8-bit characters.
		};

		explicit BitCodeAbbrevOp(uint64_t V)
			: val_(V), is_literal_(true)
		{
		}
		explicit BitCodeAbbrevOp(BitCodeEncoding enc, uint64_t Data = 0)
			: val_(Data), is_literal_(false), enc_(enc)
		{
		}

		bool IsLiteral() const
		{
			return is_literal_;
		}
		bool IsEncoding() const
		{
			return !is_literal_;
		}

		// Accessors for literals.
		uint64_t LiteralValue() const
		{
			BOOST_ASSERT(this->IsLiteral());
			return val_;
		}

		// Accessors for encoding info.
		BitCodeEncoding Encoding() const
		{
			BOOST_ASSERT(this->IsEncoding());
			return static_cast<BitCodeEncoding>(enc_);
		}
		uint64_t EncodingData() const
		{
			BOOST_ASSERT(this->IsEncoding() && this->HasEncodingData());
			return val_;
		}

		bool HasEncodingData() const
		{
			return BitCodeAbbrevOp::HasEncodingData(this->Encoding());
		}

		static bool HasEncodingData(BitCodeEncoding enc)
		{
			switch (enc)
			{
			case BitCodeEncoding::Fixed:
			case BitCodeEncoding::VBR:
				return true;
			
			case BitCodeEncoding::Array:
			case BitCodeEncoding::Char6:
			case BitCodeEncoding::Blob:
				return false;

			default:
				DILITHIUM_UNREACHABLE("Invalid encoding");
			}
		}

		static char DecodeChar6(uint32_t v)
		{
			BOOST_ASSERT_MSG((v & ~63) == 0, "Not a Char6 encoded character!");
			if (v < 26)
			{
				return static_cast<char>(v + 'a');
			}
			if (v < 26 + 26)
			{
				return static_cast<char>(v - 26 + 'A');
			}
			if (v < 26 + 26 + 10)
			{
				return static_cast<char>(v - 26 - 26 + '0');
			}
			if (v == 62)
			{
				return '.';
			}
			if (v == 63)
			{
				return '_';
			}
			DILITHIUM_UNREACHABLE("Not a value Char6 character!");
		}

	private:
		uint64_t val_;			// A literal value or data for an encoding.
		bool is_literal_ : 1;	// Indicate whether this is a literal value or not.
		BitCodeEncoding enc_ : 3;		// The encoding to use.
	};

	/// BitCodeAbbrev - This class represents an abbreviation record.  An
	/// abbreviation allows a complex record that has redundancy to be stored in a
	/// specialized format instead of the fully-general, fully-vbr, format.
	class BitCodeAbbrev
	{
	public:
		~BitCodeAbbrev() = default;

		uint32_t NumOperandInfos() const
		{
			return static_cast<uint32_t>(operand_list_.size());
		}

		BitCodeAbbrevOp const & OperandInfo(uint32_t i) const
		{
			return operand_list_[i];
		}

		void Add(BitCodeAbbrevOp const & op_info)
		{
			operand_list_.push_back(op_info);
		}

	private:
		boost::container::small_vector<BitCodeAbbrevOp, 32> operand_list_;
	};
}

#endif
