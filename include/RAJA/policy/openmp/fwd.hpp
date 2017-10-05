//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For additional details, please also read RAJA/LICENSE.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA segment template methods for
 *          execution via CUDA kernel launch.
 *
 *          These methods should work on any platform that supports
 *          CUDA devices.
 *
 ******************************************************************************
 */

#ifndef RAJA_forward_openmp_HXX
#define RAJA_forward_openmp_HXX

#include <type_traits>

#include "RAJA/config.hpp"

#include "RAJA/policy/openmp/policy.hpp"

namespace RAJA
{

namespace impl
{


template <typename Iterable, typename Func, typename InnerPolicy>
RAJA_INLINE void forall(const omp_parallel_exec<InnerPolicy>&,
                        Iterable&&,
                        Func&&);

template <typename Iterable,
          typename IndexType,
          typename Func,
          typename InnerPolicy>
RAJA_INLINE typename std::enable_if<std::is_integral<IndexType>::value>::type
forall_Icount(const omp_parallel_exec<InnerPolicy>&,
              Iterable&&,
              IndexType,
              Func&&);
///
/// OpenMP for nowait policy implementation
///

template <typename Iterable, typename Func>
RAJA_INLINE void forall(const omp_for_nowait_exec&, Iterable&&, Func&&);

template <typename Iterable, typename IndexType, typename Func>
RAJA_INLINE typename std::enable_if<std::is_integral<IndexType>::value>::type
forall_Icount(const omp_for_nowait_exec&, Iterable&&, IndexType, Func&&);

///
/// OpenMP for dependence graph policy implementation
///

template <typename Iterable, typename Func>
RAJA_INLINE void forall(const omp_for_dependence_graph&, Iterable&&, Func&&);

template <typename Iterable, typename IndexType, typename Func>
RAJA_INLINE typename std::enable_if<std::is_integral<IndexType>::value>::type
forall_Icount(const omp_for_dependence_graph&, Iterable&&, IndexType, Func&&);

///
/// OpenMP parallel for policy implementation
///

template <typename Iterable, typename Func>
RAJA_INLINE void forall(const omp_for_exec&, Iterable&&, Func&&);

template <typename Iterable, typename IndexType, typename Func>
RAJA_INLINE typename std::enable_if<std::is_integral<IndexType>::value>::type
forall_Icount(const omp_for_exec&, Iterable&&, IndexType, Func&&);
///
/// OpenMP parallel for static policy implementation
///

template <typename Iterable, typename Func, size_t ChunkSize>
RAJA_INLINE void forall(const omp_for_static<ChunkSize>&, Iterable&&, Func&&);

template <typename Iterable,
          typename IndexType,
          typename Func,
          size_t ChunkSize>
RAJA_INLINE typename std::enable_if<std::is_integral<IndexType>::value>::type
forall_Icount(const omp_for_static<ChunkSize>&, Iterable&&, IndexType, Func&&);

}  // closing brace for impl namespace

}  // closing brace for RAJA namespace

#endif  // closing endif for header file include guard