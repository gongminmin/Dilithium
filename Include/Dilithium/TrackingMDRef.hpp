/**
 * @file TrackingMDRef.hpp
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

#ifndef _DILITHIUM_TRACKING_MD_REF_HPP
#define _DILITHIUM_TRACKING_MD_REF_HPP

#pragma once

#include <Dilithium/Casting.hpp>
#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/MetadataTracking.hpp>
#include <Dilithium/Util.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	class Metadata;
	class MDNode;
	class ValueAsMetadata;

	class TrackingMDRef
	{
	public:
		TrackingMDRef()
			: metadata_(nullptr)
		{
		}
		explicit TrackingMDRef(Metadata* md)
			: metadata_(md)
		{
			this->Track();
		}
		TrackingMDRef(TrackingMDRef&& rhs)
			: metadata_(rhs.metadata_)
		{
			this->Retrack(rhs);
		}
		TrackingMDRef(TrackingMDRef const & rhs)
			: metadata_(rhs.metadata_)
		{
			this->Track();
		}
		~TrackingMDRef()
		{
			this->Untrack();
		}

		TrackingMDRef& operator=(TrackingMDRef&& rhs)
		{
			if (this != &rhs)
			{
				this->Untrack();
				metadata_ = rhs.metadata_;
				this->Retrack(rhs);
			}
			return *this;
		}
		TrackingMDRef& operator=(TrackingMDRef const & rhs)
		{
			if (this != &rhs)
			{
				this->Untrack();
				metadata_ = rhs.metadata_;
				this->Track();
			}
			return *this;
		}

		Metadata* Get() const
		{
			return metadata_;
		}
		Metadata* operator->() const
		{
			return this->Get();
		}
		Metadata& operator*() const
		{
			return *this->Get();
		}

		void Reset()
		{
			this->Untrack();
			metadata_ = nullptr;
		}
		void Reset(Metadata* md)
		{
			this->Untrack();
			metadata_ = md;
			this->Track();
		}

		bool HasTrivialDestructor() const
		{
			return !metadata_ || !MetadataTracking::IsReplaceable(*metadata_);
		}

		bool operator==(TrackingMDRef const & rhs) const
		{
			return metadata_ == rhs.metadata_;
		}
		bool operator!=(TrackingMDRef const & rhs) const
		{
			return metadata_ != rhs.metadata_;
		}

	private:
		void Track()
		{
			if (metadata_)
			{
				MetadataTracking::Track(metadata_);
			}
		}
		void Untrack()
		{
			if (metadata_)
			{
				MetadataTracking::Untrack(metadata_);
			}
		}
		void Retrack(TrackingMDRef& rhs)
		{
			BOOST_ASSERT_MSG(metadata_ == rhs.metadata_, "Expected values to match");
			if (rhs.metadata_)
			{
				MetadataTracking::Retrack(rhs.metadata_, metadata_);
				rhs.metadata_ = nullptr;
			}
		}

	private:
		Metadata* metadata_;
	};

	template <typename T>
	class TypedTrackingMDRef
	{
	public:
		TypedTrackingMDRef()
		{
		}
		explicit TypedTrackingMDRef(T* md)
			: ref_(static_cast<Metadata*>(md))
		{
		}
		TypedTrackingMDRef(TypedTrackingMDRef&& rhs)
			: ref_(std::move(rhs.ref))
		{
		}
		TypedTrackingMDRef(TypedTrackingMDRef const & rhs)
			: ref_(rhs.ref_)
		{
		}
		
		TypedTrackingMDRef& operator=(TypedTrackingMDRef&& rhs)
		{
			if (this != &rhs)
			{
				ref_ = std::move(rhs.ref);
			}
			return *this;
		}
		TypedTrackingMDRef& operator=(TypedTrackingMDRef const & rhs)
		{
			if (this != &rhs)
			{
				ref_ = rhs.ref_;
			}
			return *this;
		}

		T* Get() const
		{
			return static_cast<T*>(ref_.Get());
		}
		// TODO: Consider making it explicit
		operator T*() const
		{
			return this->Get();
		}
		T* operator->() const
		{
			return this->Get();
		}
		T& operator*() const
		{
			return *this->Get();
		}

		bool operator==(TypedTrackingMDRef const & rhs) const
		{
			return ref_ == rhs.ref_;
		}
		bool operator!=(TypedTrackingMDRef const & rhs) const
		{
			return ref_ != rhs.ref_;
		}

		void reset()
		{
			ref_.reset();
		}
		void reset(T* md)
		{
			ref.reset(static_cast<Metadata*>(md));
		}

		bool HasTrivialDestructor() const
		{
			return ref_.HasTrivialDestructor();
		}

	private:
		TrackingMDRef ref_;
	};

	typedef TypedTrackingMDRef<MDNode> TrackingMDNodeRef;
	typedef TypedTrackingMDRef<ValueAsMetadata> TrackingValueAsMetadataRef;

	// Expose the underlying metadata to casting.
	template <>
	struct simplify_type<TrackingMDRef>
	{
		typedef Metadata *SimpleType;
		static SimpleType SimplifiedValue(TrackingMDRef& md)
		{
			return md.Get();
		}
	};

	template <>
	struct simplify_type<TrackingMDRef const>
	{
		typedef Metadata *SimpleType;
		static SimpleType SimplifiedValue(TrackingMDRef const & md)
		{
			return md.Get();
		}
	};

	template <typename T>
	struct simplify_type<TypedTrackingMDRef<T>>
	{
		typedef T* SimpleType;
		static SimpleType SimplifiedValue(TypedTrackingMDRef<T> &md)
		{
			return md.Get();
		}
	};

	template <typename T>
	struct simplify_type<TypedTrackingMDRef<T> const>
	{
		typedef T* SimpleType;
		static SimpleType SimplifiedValue(TypedTrackingMDRef<T> const & md)
		{
			return md.Get();
		}
	};
}

#endif		// _DILITHIUM_TRACKING_MD_REF_HPP
