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

#include <Dilithium/BitcodeReader.hpp>

#include <Dilithium/BasicBlock.hpp>
#include <Dilithium/BitstreamReader.hpp>
#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/GVMaterializer.hpp>
#include <Dilithium/LLVMBitCodes.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/Util.hpp>

#include <deque>
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

	class BitcodeReader : boost::noncopyable, public GVMaterializer
	{
	public:
		BitcodeReader(uint8_t const * data, uint32_t data_length, std::shared_ptr<LLVMContext> const & context)
			: context_(context), buffer_(data), buffer_length_(data_length)
		{
		}
		~BitcodeReader() override
		{
			buffer_ = nullptr;
			buffer_length_ = 0;
		}

		void Materialize(GlobalValue* gv) override
		{
			DILITHIUM_UNUSED(gv);

			DILITHIUM_NOT_IMPLEMENTED;
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
			DILITHIUM_NOT_IMPLEMENTED;
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

		void ParseModule(bool resume, bool should_lazy_load_metadata = false)
		{
			DILITHIUM_UNUSED(resume);
			DILITHIUM_UNUSED(should_lazy_load_metadata);

			DILITHIUM_NOT_IMPLEMENTED;
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

	private:
		std::shared_ptr<LLVMContext> const & context_;

		LLVMModule* the_module_ = nullptr;
		uint8_t const * buffer_ = nullptr;
		uint32_t buffer_length_ = 0;
		std::unique_ptr<BitStreamReader> stream_file_;
		BitStreamCursor stream_cursor_;
		uint64_t next_unread_bit_ = 0;

		std::unordered_map<Function*, std::vector<BasicBlock*>> basic_block_fwd_refs_;
		std::deque<Function*> basic_block_fwd_ref_queue_;

		bool will_materialize_all_forward_refs_ = false;
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
