/**
 * @file Use.cpp
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

#include <Dilithium/Use.hpp>
#include <Dilithium/Value.hpp>

namespace Dilithium 
{
	Use::~Use()
	{
		if (val_)
		{
			this->RemoveFromList();
		}
	}

	Use& Use::operator=(Use const & rhs)
	{
		if (this != &rhs)
		{
			this->Set(rhs.val_);
		}
		return *this;
	}

	Use& Use::operator=(Use&& rhs)
	{
		if (this != &rhs)
		{
			this->Set(rhs.val_);
		}
		return *this;
	}

	void Use::Set(Value* val)
	{
		if (val_)
		{
			this->RemoveFromList();
		}
		val_ = val;
		if (val)
		{
			val->AddUse(*this);
		}
	}

	User* Use::GetUser() const
	{
		// TODO: Could be wrong, very careful of this.

		// The is_ref is encoded to the last bit of this pointer
		Use const * end = this->GetImpliedUser();
		bool is_ref = (reinterpret_cast<size_t>(end) & 1) ? true : false;
		return is_ref ? reinterpret_cast<User *>((reinterpret_cast<size_t>(end) & ~static_cast<size_t>(1)))
			: reinterpret_cast<User *>(const_cast<Use *>(end));
	}

	void Use::Swap(Use& rhs)
	{
		if (val_ != rhs.val_)
		{
			if (val_)
			{
				this->RemoveFromList();
			}

			Value* old_val = val_;
			if (rhs.val_)
			{
				rhs.RemoveFromList();
				val_ = rhs.val_;
				val_->AddUse(*this);
			}
			else
			{
				val_ = nullptr;
			}

			if (old_val)
			{
				rhs.val_ = old_val;
				rhs.val_->AddUse(rhs);
			}
			else
			{
				rhs.val_ = nullptr;
			}
		}
	}

	// Sets up the waymarking algorithm's tags for a series of Uses. See the
	// algorithm details here:
	// http://www.llvm.org/docs/ProgrammersManual.html#the-waymarking-algorithm
	//
	Use* Use::InitTags(Use* beg, Use* end)
	{
		ptrdiff_t done = 0;
		while (done < 20)
		{
			if (beg == end)
			{
				return beg;
			}
			-- end;
			static PrevPtrTag constexpr tags[] =
			{
				FullStopTag,  OneDigitTag,  StopTag,      OneDigitTag, OneDigitTag,
				StopTag,      ZeroDigitTag, OneDigitTag,  OneDigitTag, StopTag,
				ZeroDigitTag, OneDigitTag,  ZeroDigitTag, OneDigitTag, StopTag,
				OneDigitTag,  OneDigitTag,  OneDigitTag,  OneDigitTag, StopTag
			};
			end->tag_ = tags[done];
			++ done;
		}

		ptrdiff_t count = done;
		while (beg != end)
		{
			-- end;
			if (!count)
			{
				end->tag_ = StopTag;
				++ done;
				count = done;
			}
			else
			{
				end->tag_ = PrevPtrTag(count & 1);
				count >>= 1;
				++ done;
			}
		}

		return beg;
	}

	Use const * Use::GetImpliedUser() const
	{
		// TODO: Rewrite this, without bit tricks

		Use const * curr = this;

		for (;;)
		{
			uint32_t tag = curr->tag_;
			++ curr;
			switch (tag)
			{
			case ZeroDigitTag:
			case OneDigitTag:
				continue;

			case StopTag:
				{
					++ curr;
					ptrdiff_t offset = 1;
					for (;;)
					{
						tag = curr->tag_;
						switch (tag)
						{
						case ZeroDigitTag:
						case OneDigitTag:
							++ curr;
							offset = (offset << 1) + tag;
							continue;

						default:
							return curr + offset;
						}
					}
				}

			case FullStopTag:
				return curr;
			}
		}
	}

	void Use::AddToList(Use** node)
	{
		next_ = *node;
		if (next_)
		{
			next_->prev_ptr_ = &next_;
		}
		prev_ptr_ = node;
		*node = this;
	}

	void Use::RemoveFromList()
	{
		Use** stripped_prev = prev_ptr_;
		*stripped_prev = next_;
		if (next_)
		{
			next_->prev_ptr_ = stripped_prev;
		}
	}
}
