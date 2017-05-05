/**
 * @file BitstreamReader.hpp
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

#ifndef _DILITHIUM_BITSTREAM_READER_HPP
#define _DILITHIUM_BITSTREAM_READER_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/BitCodes.hpp>
#include <Dilithium/MemStreamBuf.hpp>

#include <climits>
#include <memory>
#include <string>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/endian/conversion.hpp>

namespace Dilithium
{
	class BitStreamReader : boost::noncopyable
	{
	public:
		struct BlockInfo
		{
			uint32_t block_id;
			std::vector<std::shared_ptr<BitCodeAbbrev>> abbrevs;
			std::string name;

			std::vector<std::pair<uint32_t, std::string>> record_names;
		};

	public:
		BitStreamReader()
		{
		}

		BitStreamReader(uint8_t const * beg, uint8_t const * end);
		BitStreamReader(BitStreamReader&& rhs);

		BitStreamReader& operator=(BitStreamReader&& rhs);

		std::istream& BitcodeStream()
		{
			return *bitcode_stream_;
		}
		uint8_t const * BitcodeStart()
		{
			return bitcode_begin_;
		}
		uint32_t BitcodeSize() const
		{
			return bitcode_size_;
		}

		bool HasBlockInfoRecords() const
		{
			return !block_info_records_.empty();
		}

		BlockInfo* GetBlockInfo(uint32_t block_id);
		BlockInfo& GetOrCreateBlockInfo(uint32_t block_id);

		void TakeBlockInfo(BitStreamReader&& rhs);

	private:
		std::unique_ptr<std::streambuf> bitcode_buff_;
		std::unique_ptr<std::istream> bitcode_stream_;
		uint8_t const * bitcode_begin_;
		uint32_t bitcode_size_;

		std::vector<BlockInfo> block_info_records_;
	};

	struct BitStreamEntry
	{
		enum
		{
			Error,		// Malformed bitcode was found.
			EndBlock,	// We've reached the end of the current block, (or the end of the
						// file, which is treated like a series of EndBlock records.
			SubBlock,	// This is the start of a new subblock of a specific ID.
			Record		// This is a record with a specific AbbrevID.
		} kind;

		uint32_t id;

		static BitStreamEntry GetError();
		static BitStreamEntry GetEndBlock();
		static BitStreamEntry GetSubBlock(uint32_t id);
		static BitStreamEntry GetRecord(uint32_t abbrev_id);
	};

	class BitStreamCursor
	{
		typedef size_t word_t;

	public:
		static size_t constexpr MAX_CHUNK_SIZE = sizeof(word_t) * 8;

		// Flags that modify the behavior of Advance().
		enum
		{
			// If this flag is used, the Advance() method does not automatically pop
			// the block scope when the end of a block is reached.
			AF_DontPopBlockAtEnd = 1,

			// If this flag is used, abbrev entries are returned just like normal
			// records.
			AF_DontAutoprocessAbbrevs = 2
		};

	public:
		BitStreamCursor()
		{
			this->Init(nullptr);
		}

		explicit BitStreamCursor(BitStreamReader& rhs)
		{
			this->Init(&rhs);
		}

		void Init(BitStreamReader* rhs);

		void FreeState();

		bool CanSkipToPos(size_t pos) const
		{
			return (pos == 0) || (pos - 1 < bit_stream_->BitcodeSize());
		}

		bool AtEndOfStream();

		uint32_t AbbrevIdWidth() const
		{
			return curr_code_size_;
		}

		uint64_t CurrBitNo() const
		{
			return next_char_ * CHAR_BIT - bits_in_curr_word_;
		}

		BitStreamEntry Advance(uint32_t flags = 0);
		BitStreamEntry AdvanceSkippingSubblocks(uint32_t flags = 0);

		void JumpToBit(uint64_t bit_no);

		void FillCurrWord();

		word_t Read(uint32_t num_bits);
		uint32_t ReadVBR(uint32_t num_bits);
		uint64_t ReadVBR64(uint32_t num_bits);

		uint32_t ReadCode()
		{
			return static_cast<uint32_t>(this->Read(curr_code_size_));
		}

		uint32_t ReadSubBlockID()
		{
			return this->ReadVBR(BitCode::StandardWidth::BlockIdWidth);
		}

		bool SkipBlock();

		bool EnterSubBlock(uint32_t block_id, uint32_t* num_words_ptr = nullptr);
		bool ReadBlockEnd();

		BitCodeAbbrev const * GetAbbrev(uint32_t abbrev_id);

		uint32_t ReadRecord(uint32_t abbrev_id, boost::container::small_vector_base<uint64_t>& vals);

		void ReadAbbrevRecord();
		bool ReadBlockInfoBlock();

	private:
		void SkipToFourByteBoundary();
		void PopBlockScope();

	private:
		BitStreamReader* bit_stream_;
		size_t next_char_;

		size_t size_;

		word_t curr_word_;
		uint32_t bits_in_curr_word_;
		uint32_t curr_code_size_;

		std::vector<std::shared_ptr<BitCodeAbbrev>> curr_abbrevs_;

		struct Block
		{
			uint32_t prev_code_size;
			std::vector<std::shared_ptr<BitCodeAbbrev>> prev_abbrevs;

			explicit Block(uint32_t pcs)
				: prev_code_size(pcs)
			{
			}
		};

		boost::container::small_vector<Block, 8> block_scope_;
	};
}

#endif		// _DILITHIUM_BITSTREAM_READER_HPP
