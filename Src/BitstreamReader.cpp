/**
 * @file BitstreamReader.cpp
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

#include <Dilithium/BitStreamReader.hpp>

namespace
{
	using namespace Dilithium;

	uint64_t ReadAbbreviatedField(BitStreamCursor& cursor, BitCodeAbbrevOp const & op)
	{
		BOOST_ASSERT_MSG(!op.IsLiteral(), "Not to be used with literals!");

		switch (op.Encoding())
		{
		case BitCodeAbbrevOp::BitCodeEncoding::Array:
		case BitCodeAbbrevOp::BitCodeEncoding::Blob:
			DILITHIUM_UNREACHABLE("Should not reach here");
		case BitCodeAbbrevOp::BitCodeEncoding::Fixed:
			BOOST_ASSERT(op.EncodingData() <= cursor.MAX_CHUNK_SIZE);
			return cursor.Read(static_cast<uint32_t>(op.EncodingData()));
		case BitCodeAbbrevOp::BitCodeEncoding::VBR:
			BOOST_ASSERT(op.EncodingData() <= cursor.MAX_CHUNK_SIZE);
			return cursor.ReadVBR64(static_cast<uint32_t>(op.EncodingData()));
		case BitCodeAbbrevOp::BitCodeEncoding::Char6:
			return BitCodeAbbrevOp::DecodeChar6(static_cast<uint32_t>(cursor.Read(6)));

		default:
			DILITHIUM_UNREACHABLE("invalid abbreviation encoding");
		}
	}

	void SkipAbbreviatedField(BitStreamCursor& cursor, BitCodeAbbrevOp const & op)
	{
		BOOST_ASSERT_MSG(!op.IsLiteral(), "Not to be used with literals!");

		switch (op.Encoding())
		{
		case BitCodeAbbrevOp::BitCodeEncoding::Fixed:
			BOOST_ASSERT(op.EncodingData() <= cursor.MAX_CHUNK_SIZE);
			cursor.Read(static_cast<uint32_t>(op.EncodingData()));
			break;
		case BitCodeAbbrevOp::BitCodeEncoding::VBR:
			BOOST_ASSERT(op.EncodingData() <= cursor.MAX_CHUNK_SIZE);
			cursor.ReadVBR64(static_cast<uint32_t>(op.EncodingData()));
			break;
		case BitCodeAbbrevOp::BitCodeEncoding::Char6:
			cursor.Read(6);
			break;

		case BitCodeAbbrevOp::BitCodeEncoding::Array:
		case BitCodeAbbrevOp::BitCodeEncoding::Blob:
		default:
			DILITHIUM_UNREACHABLE("Should not reach here");
		}
	}
}

namespace Dilithium
{
	BitStreamReader::BitStreamReader(uint8_t const * beg, uint8_t const * end)
	{
		BOOST_ASSERT_MSG(((end - beg) & 3) == 0, "Bitcode stream not a multiple of 4 bytes");
		bitcode_buff_ = std::make_unique<MemStreamBuf>(beg, end);
		bitcode_stream_ = std::make_unique<std::istream>(bitcode_buff_.get());
		bitcode_begin_ = beg;
		bitcode_size_ = static_cast<uint32_t>(end - beg);
	}

	BitStreamReader::BitStreamReader(BitStreamReader&& rhs)
	{
		bitcode_buff_ = std::move(rhs.bitcode_buff_);
		bitcode_stream_ = std::move(rhs.bitcode_stream_);
		bitcode_begin_ = rhs.bitcode_begin_;
		bitcode_size_ = rhs.bitcode_size_;
		block_info_records_ = std::move(rhs.block_info_records_);
	}

	BitStreamReader& BitStreamReader::operator=(BitStreamReader&& rhs)
	{
		if (this != &rhs)
		{
			bitcode_buff_ = std::move(rhs.bitcode_buff_);
			bitcode_stream_ = std::move(rhs.bitcode_stream_);
			bitcode_begin_ = rhs.bitcode_begin_;
			bitcode_size_ = rhs.bitcode_size_;
			block_info_records_ = std::move(rhs.block_info_records_);
		}
		return *this;
	}

	BitStreamReader::BlockInfo* BitStreamReader::GetBlockInfo(uint32_t block_id)
	{
		if (!block_info_records_.empty() && (block_info_records_.back().block_id == block_id))
		{
			return &block_info_records_.back();
		}

		for (uint32_t i = 0, e = static_cast<uint32_t>(block_info_records_.size()); i != e; ++i)
		{
			if (block_info_records_[i].block_id == block_id)
			{
				return &block_info_records_[i];
			}
		}

		return nullptr;
	}

	BitStreamReader::BlockInfo& BitStreamReader::GetOrCreateBlockInfo(uint32_t block_id)
	{
		auto bi = this->GetBlockInfo(block_id);
		if (bi)
		{
			return *bi;
		}
		else
		{
			block_info_records_.emplace_back();
			block_info_records_.back().block_id = block_id;
			return block_info_records_.back();
		}
	}

	void BitStreamReader::TakeBlockInfo(BitStreamReader&& rhs)
	{
		BOOST_ASSERT(!this->HasBlockInfoRecords());
		block_info_records_ = std::move(rhs.block_info_records_);
	}


	BitStreamEntry BitStreamEntry::GetError()
	{
		BitStreamEntry ret;
		ret.kind = Error;
		return ret;
	}

	BitStreamEntry BitStreamEntry::GetEndBlock()
	{
		BitStreamEntry ret;
		ret.kind = EndBlock;
		return ret;
	}

	BitStreamEntry BitStreamEntry::GetSubBlock(uint32_t id)
	{
		BitStreamEntry ret;
		ret.kind = SubBlock;
		ret.id = id;
		return ret;
	}

	BitStreamEntry BitStreamEntry::GetRecord(uint32_t abbrev_id)
	{
		BitStreamEntry ret;
		ret.kind = Record;
		ret.id = abbrev_id;
		return ret;
	}


	void BitStreamCursor::Init(BitStreamReader* rhs)
	{
		this->FreeState();

		bit_stream_ = rhs;
		next_char_ = 0;
		size_ = 0;
		bits_in_curr_word_ = 0;
		curr_code_size_ = 2;
	}

	void BitStreamCursor::FreeState()
	{
		curr_abbrevs_.clear();
		block_scope_.clear();
	}

	bool BitStreamCursor::AtEndOfStream()
	{
		if (bits_in_curr_word_ != 0)
		{
			return false;
		}
		if (size_ != 0)
		{
			return size_ == next_char_;
		}
		this->FillCurrWord();
		return bits_in_curr_word_ == 0;
	}

	BitStreamEntry BitStreamCursor::Advance(uint32_t flags)
	{
		for (;;)
		{
			uint32_t code = this->ReadCode();
			if (code == BitCode::FixedAbbrevId::EndBlock)
			{
				if (!(flags & AF_DontPopBlockAtEnd) && this->ReadBlockEnd())
				{
					return BitStreamEntry::GetError();
				}
				return BitStreamEntry::GetEndBlock();
			}

			if (code == BitCode::FixedAbbrevId::EnterSubblock)
			{
				return BitStreamEntry::GetSubBlock(ReadSubBlockID());
			}

			if ((code == BitCode::FixedAbbrevId::DefineAbbrev) && !(flags & AF_DontAutoprocessAbbrevs))
			{
				this->ReadAbbrevRecord();
				continue;
			}

			return BitStreamEntry::GetRecord(code);
		}
	}

	BitStreamEntry BitStreamCursor::AdvanceSkippingSubblocks(uint32_t flags)
	{
		for (;;)
		{
			BitStreamEntry entry = this->Advance(flags);
			if (entry.kind != BitStreamEntry::SubBlock)
			{
				return entry;
			}

			// If we found a sub-block, just skip over it and check the next entry.
			if (this->SkipBlock())
			{
				return BitStreamEntry::GetError();
			}
		}
	}

	void BitStreamCursor::JumpToBit(uint64_t bit_no)
	{
		size_t byte_no = static_cast<size_t>(bit_no / 8) & ~(sizeof(word_t) - 1);
		uint32_t word_bit_no = static_cast<uint32_t>(bit_no & (sizeof(word_t) * 8 - 1));
		BOOST_ASSERT_MSG(this->CanSkipToPos(byte_no), "Invalid location");

		bit_stream_->BitcodeStream().seekg(byte_no);
		next_char_ = bit_stream_->BitcodeStream().tellg();
		bits_in_curr_word_ = 0;

		if (word_bit_no > 0)
		{
			this->Read(word_bit_no);
		}
	}

	void BitStreamCursor::FillCurrWord()
	{
		if (size_ != 0 && (next_char_ >= size_))
		{
			ReportFatalError("Unexpected end of file");
		}

		char data[sizeof(word_t)] = { 0 };

		bit_stream_->BitcodeStream().read(data, sizeof(data));
		auto bytes_read = bit_stream_->BitcodeStream().gcount();
		next_char_ = bit_stream_->BitcodeStream().tellg();

		if (bytes_read == 0)
		{
			size_ = next_char_;
			return;
		}

		curr_word_ = boost::endian::little_to_native(*reinterpret_cast<word_t*>(data));
		bits_in_curr_word_ = static_cast<uint32_t>(bytes_read * 8);
	}

	BitStreamCursor::word_t BitStreamCursor::Read(uint32_t num_bits)
	{
		BOOST_ASSERT_MSG(num_bits && num_bits <= MAX_CHUNK_SIZE, "Cannot return zero or more than BitsInWord bits!");

		static uint32_t constexpr MASK = sizeof(word_t) > 4 ? 0x3F : 0x1F;

		if (bits_in_curr_word_ >= num_bits)
		{
			word_t ret = curr_word_ & (~static_cast<word_t>(0) >> (MAX_CHUNK_SIZE - num_bits));

			curr_word_ >>= (num_bits & MASK);

			bits_in_curr_word_ -= num_bits;
			return ret;
		}
		else
		{
			word_t ret = bits_in_curr_word_ ? curr_word_ : 0;
			uint32_t bits_left = num_bits - bits_in_curr_word_;

			this->FillCurrWord();

			if (bits_left > bits_in_curr_word_)
			{
				return 0;
			}
			else
			{
				word_t ret2 = curr_word_ & (~word_t(0) >> (MAX_CHUNK_SIZE - bits_left));

				curr_word_ >>= (bits_left & MASK);

				bits_in_curr_word_ -= bits_left;

				ret |= ret2 << (num_bits - bits_left);

				return ret;
			}
		}
	}

	uint32_t BitStreamCursor::ReadVBR(uint32_t num_bits)
	{
		uint32_t piece = static_cast<uint32_t>(this->Read(num_bits));
		if ((piece & (1U << (num_bits - 1))) == 0)
		{
			return piece;
		}
		else
		{
			uint32_t result = 0;
			uint32_t next_bit = 0;
			for (;;)
			{
				result |= (piece & ((1U << (num_bits - 1)) - 1)) << next_bit;

				if ((piece & (1U << (num_bits - 1))) == 0)
				{
					return result;
				}

				next_bit += num_bits - 1;
				piece = static_cast<uint32_t>(this->Read(num_bits));
			}
		}
	}

	uint64_t BitStreamCursor::ReadVBR64(uint32_t num_bits)
	{
		uint32_t piece = static_cast<uint32_t>(this->Read(num_bits));
		if ((piece & (1U << (num_bits - 1))) == 0)
		{
			return static_cast<uint64_t>(piece);
		}
		else
		{
			uint64_t result = 0;
			uint32_t next_bit = 0;
			for (;;)
			{
				result |= static_cast<uint64_t>(piece & ((1U << (num_bits - 1)) - 1)) << next_bit;

				if ((piece & (1U << (num_bits - 1))) == 0)
				{
					return result;
				}

				next_bit += num_bits - 1;
				piece = static_cast<uint32_t>(this->Read(num_bits));
			}
		}
	}

	bool BitStreamCursor::SkipBlock()
	{
		// Read and ignore the codelen value.  Since we are skipping this block, we
		// don't care what code widths are used inside of it.
		this->ReadVBR(BitCode::StandardWidth::CodeLenWidth);
		this->SkipToFourByteBoundary();
		uint32_t num_four_bytes = static_cast<uint32_t>(this->Read(BitCode::StandardWidth::BlockSizeWidth));

		// Check that the block wasn't partially defined, and that the offset isn't bogus.
		size_t skip_to = this->CurrBitNo() + num_four_bytes * 4 * 8;
		if (this->AtEndOfStream() || !this->CanSkipToPos(skip_to / 8))
		{
			return true;
		}

		this->JumpToBit(skip_to);
		return false;
	}

	bool BitStreamCursor::EnterSubBlock(uint32_t block_id, uint32_t* num_words_ptr)
	{
		block_scope_.push_back(Block(curr_code_size_));
		block_scope_.back().prev_abbrevs.swap(curr_abbrevs_);

		auto info = bit_stream_->GetBlockInfo(block_id);
		if (info)
		{
			curr_abbrevs_.insert(curr_abbrevs_.end(), info->abbrevs.begin(), info->abbrevs.end());
		}

		curr_code_size_ = this->ReadVBR(BitCode::StandardWidth::CodeLenWidth);
		if (curr_code_size_ > MAX_CHUNK_SIZE)
		{
			return true;
		}

		this->SkipToFourByteBoundary();
		uint32_t num_words = static_cast<uint32_t>(this->Read(BitCode::StandardWidth::BlockSizeWidth));
		if (num_words_ptr)
		{
			*num_words_ptr = num_words;
		}

		return (curr_code_size_ == 0) || this->AtEndOfStream();
	}

	bool BitStreamCursor::ReadBlockEnd()
	{
		if (block_scope_.empty())
		{
			return true;
		}
		else
		{
			// Block tail: [END_BLOCK, <align4bytes>]
			this->SkipToFourByteBoundary();

			this->PopBlockScope();
			return false;
		}
	}

	BitCodeAbbrev const * BitStreamCursor::GetAbbrev(uint32_t abbrev_id)
	{
		uint32_t abbrev_no = abbrev_id - BitCode::FixedAbbrevId::FirstApplicationAbbrev;
		if (abbrev_no >= curr_abbrevs_.size())
		{
			ReportFatalError("Invalid abbrev number");
		}
		return curr_abbrevs_[abbrev_no].get();
	}

	uint32_t BitStreamCursor::ReadRecord(uint32_t abbrev_id, boost::container::small_vector_base<uint64_t>& vals)
	{
		if (abbrev_id == BitCode::FixedAbbrevId::UnabbrevRecord)
		{
			uint32_t code = this->ReadVBR(6);
			uint32_t num_elems = this->ReadVBR(6);
			for (uint32_t i = 0; i != num_elems; ++ i)
			{
				vals.push_back(this->ReadVBR64(6));
			}
			return code;
		}

		auto abbv = this->GetAbbrev(abbrev_id);

		BOOST_ASSERT_MSG(abbv->NumOperandInfos() != 0, "no record code in abbreviation?");
		auto const & code_op = abbv->OperandInfo(0);
		uint32_t code;
		if (code_op.IsLiteral())
		{
			code = static_cast<uint32_t>(code_op.LiteralValue());
		}
		else
		{
			if ((code_op.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Array)
				|| (code_op.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Blob))
			{
				ReportFatalError("Abbreviation starts with an Array or a Blob");
			}
			code = static_cast<uint32_t>(ReadAbbreviatedField(*this, code_op));
		}

		for (uint32_t i = 1, e = abbv->NumOperandInfos(); i != e; ++ i)
		{
			auto const & op = abbv->OperandInfo(i);
			if (op.IsLiteral())
			{
				vals.push_back(op.LiteralValue());
				continue;
			}

			if ((op.Encoding() != BitCodeAbbrevOp::BitCodeEncoding::Array)
				&& (op.Encoding() != BitCodeAbbrevOp::BitCodeEncoding::Blob))
			{
				vals.push_back(ReadAbbreviatedField(*this, op));
				continue;
			}

			if (op.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Array)
			{
				uint32_t num_elems = this->ReadVBR(6);

				if (i + 2 != e)
				{
					ReportFatalError("Array op not second to last");
				}
				++ i;
				auto const & elem_enc = abbv->OperandInfo(i);
				if (!elem_enc.IsEncoding())
				{
					ReportFatalError("Array element type has to be an encoding of a type");
				}
				if ((elem_enc.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Array)
					|| (elem_enc.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Blob))
				{
					ReportFatalError("Array element type can't be an Array or a Blob");
				}

				// Read all the elements.
				for (; num_elems; --num_elems)
				{
					vals.push_back(ReadAbbreviatedField(*this, elem_enc));
				}

				continue;
			}

			BOOST_ASSERT(op.Encoding() == BitCodeAbbrevOp::BitCodeEncoding::Blob);

			uint32_t num_elems = this->ReadVBR(6);
			this->SkipToFourByteBoundary();  // 32-bit alignment

			// Figure out where the end of this blob will be including tail padding.
			size_t curr_bit_pos = this->CurrBitNo();
			size_t new_end = curr_bit_pos + ((num_elems + 3) & ~3) * 8;

			if (!this->CanSkipToPos(new_end / 8))
			{
				vals.insert(vals.end(), num_elems, 0);
				bit_stream_->BitcodeStream().seekg(0, std::ios_base::end);
				next_char_ = bit_stream_->BitcodeStream().tellg();
				break;
			}

			uint8_t const * ptr = bit_stream_->BitcodeStart() + curr_bit_pos / 8;

			for (; num_elems; -- num_elems)
			{
				vals.push_back(*ptr);
				++ ptr;
			}

			// Skip over tail padding.
			this->JumpToBit(new_end);
		}

		return code;
	}

	void BitStreamCursor::ReadAbbrevRecord()
	{
		auto abbv = std::make_shared<BitCodeAbbrev>();
		uint32_t num_op_info = this->ReadVBR(5);
		for (uint32_t i = 0; i != num_op_info; ++ i)
		{
			bool is_literal = this->Read(1);
			if (is_literal)
			{
				abbv->Add(BitCodeAbbrevOp(this->ReadVBR64(8)));
				continue;
			}

			BitCodeAbbrevOp::BitCodeEncoding enc = static_cast<BitCodeAbbrevOp::BitCodeEncoding>(this->Read(3));
			if (BitCodeAbbrevOp::HasEncodingData(enc))
			{
				uint64_t data = this->ReadVBR64(5);

				if (((enc == BitCodeAbbrevOp::BitCodeEncoding::Fixed) || (enc == BitCodeAbbrevOp::BitCodeEncoding::VBR))
					&& (data == 0))
				{
					abbv->Add(BitCodeAbbrevOp(0));
					continue;
				}

				if (((enc == BitCodeAbbrevOp::BitCodeEncoding::Fixed) || (enc == BitCodeAbbrevOp::BitCodeEncoding::VBR))
					&& (data > MAX_CHUNK_SIZE))
				{
					ReportFatalError("Fixed or VBR abbrev record with size > MaxChunkData");
				}

				abbv->Add(BitCodeAbbrevOp(enc, data));
			}
			else
			{
				abbv->Add(BitCodeAbbrevOp(enc));
			}
		}

		if (abbv->NumOperandInfos() == 0)
		{
			ReportFatalError("Abbrev record with no operands");
		}
		curr_abbrevs_.push_back(abbv);
	}

	bool BitStreamCursor::ReadBlockInfoBlock()
	{
		if (bit_stream_->HasBlockInfoRecords())
		{
			return this->SkipBlock();
		}

		if (EnterSubBlock(BitCode::StandardBlockId::BlockInfoBlockId))
		{
			return true;
		}

		boost::container::small_vector<uint64_t, 64> record;
		BitStreamReader::BlockInfo* curr_block_info = nullptr;

		// Read all the records for this module.
		for (;;)
		{
			BitStreamEntry entry = this->AdvanceSkippingSubblocks(AF_DontAutoprocessAbbrevs);

			switch (entry.kind)
			{
			case BitStreamEntry::SubBlock: // Handled for us already.
			case BitStreamEntry::Error:
				return true;
			case BitStreamEntry::EndBlock:
				return false;
			case BitStreamEntry::Record:
				// The interesting case.
				break;
			}

			if (entry.id == BitCode::FixedAbbrevId::DefineAbbrev)
			{
				if (!curr_block_info)
				{
					return true;
				}
				this->ReadAbbrevRecord();

				curr_block_info->abbrevs.push_back(std::move(curr_abbrevs_.back()));
				curr_abbrevs_.pop_back();
				continue;
			}

			// Read a record.
			record.clear();
			switch (this->ReadRecord(entry.id, record))
			{
			case BitCode::BlockInfoCode::SetBlockId:
				if (record.size() < 1)
				{
					return true;
				}
				curr_block_info = &bit_stream_->GetOrCreateBlockInfo(static_cast<uint32_t>(record[0]));
				break;
			case BitCode::BlockInfoCode::BlockName:
				if (!curr_block_info)
				{
					return true;
				}
				// Ignore name.
				break;
			case BitCode::BlockInfoCode::SetRecordName:
				if (!curr_block_info)
				{
					return true;
				}
				// Ignore name.
				break;

			default:
				// Default behavior, ignore unknown content.
				break;
			}
		}
	}

	void BitStreamCursor::SkipToFourByteBoundary()
	{
		// If word_t is 64-bits and if we've read less than 32 bits, just dump the bits we have up to the next 32-bit boundary.
		if ((sizeof(word_t) > 4) && (bits_in_curr_word_ >= 32))
		{
			curr_word_ >>= bits_in_curr_word_ - 32;
			bits_in_curr_word_ = 32;
			return;
		}

		bits_in_curr_word_ = 0;
	}

	void BitStreamCursor::PopBlockScope()
	{
		curr_code_size_ = block_scope_.back().prev_code_size;

		curr_abbrevs_ = std::move(block_scope_.back().prev_abbrevs);
		block_scope_.pop_back();
	}
}