/**
 * @file CFG.hpp
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

#ifndef _DILITHIUM_CFG_HPP
#define _DILITHIUM_CFG_HPP

#pragma once

#include <Dilithium/Function.hpp>
#include <Dilithium/InstrTypes.hpp>
#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	// BasicBlock pred_iterator definition
	
	template <class Ptr, class USE_iterator> // Predecessor Iterator
	class PredIterator
	{
	public:
		typedef Ptr value_type;
		typedef ptrdiff_t difference_type;
		typedef Ptr* pointer;
		typedef Ptr* reference;
		typedef std::forward_iterator_tag iterator_category;

	public:
		PredIterator()
		{
		}
		explicit PredIterator(Ptr* bb)
			: iter_(bb->UserBegin())
		{
			this->AdvancePastNonTerminators();
		}
		PredIterator(Ptr* bb, bool)
			: iter_(bb->UserEnd())
		{
		}

		bool operator==(PredIterator const & rhs) const
		{
			return iter_ == rhs.iter_;
		}
		bool operator!=(PredIterator const & rhs) const
		{
			return !operator==(rhs);
		}

		reference operator*() const
		{
			BOOST_ASSERT_MSG(!iter_.AtEnd(), "pred_iterator out of range!");
			return cast<TerminatorInst>(*iter_)->Parent();
		}
		pointer* operator->() const
		{
			return &operator*();
		}

		PredIterator<Ptr, USE_iterator>& operator++()
		{
			// Preincrement
			BOOST_ASSERT_MSG(!iter_.AtEnd(), "pred_iterator out of range!");
			++ iter_;
			this->AdvancePastNonTerminators();
			return *this;
		}

		PredIterator<Ptr, USE_iterator> operator++(int)
		{
			// Postincrement
			auto tmp = *this;
			++ *this;
			return tmp;
		}

		/// GetOperandNo - Return the operand number in the predecessor's terminator of the successor.
		uint32_t GetOperandNo() const
		{
			return iter_.GetOperandNo();
		}

		/// GetUse - Return the operand Use in the predecessor's terminator of the successor.
		Use& GetUse() const
		{
			return iter_.GetUse();
		}

	private:
		void AdvancePastNonTerminators()
		{
			// Loop to ignore non-terminator uses (for example BlockAddresses).
			while (!iter_.AtEnd() && !isa<TerminatorInst>(*iter_))
			{
				++ iter_;
			}
		}

	private:
		USE_iterator iter_;
	};

	typedef PredIterator<BasicBlock, Value::user_iterator> pred_iterator;
	typedef PredIterator<BasicBlock const, Value::const_user_iterator> const_pred_iterator;
	typedef boost::iterator_range<pred_iterator> pred_range;
	typedef boost::iterator_range<const_pred_iterator> pred_const_range;

	inline pred_iterator pred_begin(BasicBlock* bb)
	{
		return pred_iterator(bb);
	}
	inline const_pred_iterator pred_begin(BasicBlock const * bb)
	{
		return const_pred_iterator(bb);
	}
	inline pred_iterator pred_end(BasicBlock* bb)
	{
		return pred_iterator(bb, true);
	}
	inline const_pred_iterator pred_end(BasicBlock const * bb)
	{
		return const_pred_iterator(bb, true);
	}
	inline bool pred_empty(BasicBlock const * bb)
	{
		return pred_begin(bb) == pred_end(bb);
	}
	inline pred_range predecessors(BasicBlock* bb)
	{
		return pred_range(pred_begin(bb), pred_end(bb));
	}
	inline pred_const_range predecessors(BasicBlock const * bb)
	{
		return pred_const_range(pred_begin(bb), pred_end(bb));
	}
}

#endif
