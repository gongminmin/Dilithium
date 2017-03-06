/**
 * @file Compiler.hpp
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

#ifndef _DILITHIUM_COMPILER_HPP
#define _DILITHIUM_COMPILER_HPP

#pragma once

// Defines supported compilers
#if defined(__clang__)
	#if __cplusplus < 201402L
		#error "-std=c++14 must be turned on."
	#endif

	#define DILITHIUM_COMPILER_CLANG
	#define DILITHIUM_COMPILER_NAME clang

	#define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if defined(__APPLE__)
		#if CLANG_VERSION >= 61
			#define DILITHIUM_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install Apple clang++ 6.1 or up."
		#endif

		#define DILITHIUM_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define DILITHIUM_SYMBOL_IMPORT
	#elif defined(__ANDROID__)
		#if CLANG_VERSION >= 36
			#define DILITHIUM_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang++ 3.6 or up."
		#endif

		#define DILITHIUM_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define DILITHIUM_SYMBOL_IMPORT
	#elif defined(__c2__)
		#if CLANG_VERSION >= 36
			#define DILITHIUM_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang++ 3.6 or up."
		#endif

		#define DILITHIUM_COMPILER_CLANGC2

		#define DILITHIUM_SYMBOL_EXPORT __declspec(dllexport)
		#define DILITHIUM_SYMBOL_IMPORT __declspec(dllimport)

		#ifndef _CRT_SECURE_NO_DEPRECATE
			#define _CRT_SECURE_NO_DEPRECATE
		#endif
		#ifndef _SCL_SECURE_NO_DEPRECATE
			#define _SCL_SECURE_NO_DEPRECATE
		#endif
	#else
		#error "Clang++ on an unknown platform. Only Apple and Windows are supported."
	#endif
#elif defined(__GNUC__)
	#define DILITHIUM_COMPILER_GCC

	#define GCC_VERSION KFL_JOIN(__GNUC__, __GNUC_MINOR__)

	#if GCC_VERSION >= 51
		#define DILITHIUM_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 5.1 or up."
	#endif

	#if __cplusplus < 201402L
		#error "-std=c++14 must be turned on."
	#endif
	#if !defined(_GLIBCXX_HAS_GTHREADS)
		#error "_GLIBCXX_HAS_GTHREADS must be turned on."
	#endif

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define DILITHIUM_SYMBOL_EXPORT __attribute__((__dllexport__))
		#define DILITHIUM_SYMBOL_IMPORT __attribute__((__dllimport__))
	#else
		#define DILITHIUM_SYMBOL_EXPORT __attribute__((__visibility__("default")))
		#define DILITHIUM_SYMBOL_IMPORT
	#endif
#elif defined(_MSC_VER)
	#define DILITHIUM_COMPILER_MSVC
	#define DILITHIUM_COMPILER_NAME vc

	#if _MSC_VER >= 1910
		#define DILITHIUM_COMPILER_VERSION 141
	#elif _MSC_VER >= 1900
		#define DILITHIUM_COMPILER_VERSION 140
	#else
		#error "Unsupported compiler version. Please install vc14 or up."
	#endif

	#define DILITHIUM_SYMBOL_EXPORT __declspec(dllexport)
	#define DILITHIUM_SYMBOL_IMPORT __declspec(dllimport)

	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif
	#ifndef _SCL_SECURE_NO_DEPRECATE
		#define _SCL_SECURE_NO_DEPRECATE
	#endif
#else
	#error "Unknown compiler. Please install vc, g++ or clang."
#endif

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_unreachable)
	#define DILITHIUM_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
	#define DILITHIUM_BUILTIN_UNREACHABLE __assume(false)
#endif

#endif		//_DILITHIUM_COMPILER_HPP
