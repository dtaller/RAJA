/*!
******************************************************************************
*
* \file
*
* \brief   Header file providing RAJA sort templates.
*
******************************************************************************
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_util_sort_HPP
#define RAJA_util_sort_HPP

#include "RAJA/config.hpp"

#include <iterator>

#include "RAJA/pattern/detail/algorithm.hpp"

#include "RAJA/util/macros.hpp"

#include "RAJA/util/concepts.hpp"

namespace RAJA
{

namespace detail
{

/*!
    \brief evaluate log base 2 of N rounded down to the nearest integer >= 0
*/
RAJA_HOST_DEVICE RAJA_INLINE
unsigned
ulog2(size_t N)
{
  unsigned val = 0;

  while (N > 1) {
    val += 1;
    N >>= 1;
  }

  return val;
}

/*!
    \brief unstable partition given range inplace using predicate function
    and using O(N) predicate evaluations and O(1) memory
*/
template <typename Iter, typename Predicate>
RAJA_HOST_DEVICE RAJA_INLINE
Iter
partition(Iter begin,
          Iter end,
          Predicate pred)
{
  using ::RAJA::safe_iter_swap;

  if (begin == end) {
    return begin;
  }

  // advance to first false
  Iter first_false = begin;
  for (; first_false != end; ++first_false) {

    if (!pred(first_false)) {
      break;
    }
  }

  // return if none were false
  if (first_false == end) {
    return first_false;
  }

  // advance through rest of list to find the next true
  for (Iter next_true = RAJA::next(first_false); next_true != end; ++next_true) {

    // find the end of a range of falses [first_false, next_true)
    if (pred(next_true)) {

      // shift the known range of falses forward
      // by swapping the true to the beginning of the range
      safe_iter_swap(first_false, next_true);
      ++first_false;
    }
  }

  return first_false;
}

/*!
    \brief stable insertion sort given range inplace using comparison function
    and using O(N^2) comparisons and O(1) memory
*/
template <typename Iter, typename Compare>
RAJA_HOST_DEVICE RAJA_INLINE
void
insertion_sort(Iter begin,
               Iter end,
               Compare comp)
{
  using ::RAJA::safe_iter_swap;

  if (begin == end) {
    return;
  }

  // for each unsorted item in the right side of the range
  for (Iter next_unsorted = RAJA::next(begin); next_unsorted != end; ++next_unsorted) {

    // insert unsorted item into the sorted left side of the range
    for (Iter to_insert = next_unsorted; to_insert != begin; --to_insert) {

      Iter next_sorted = RAJA::prev(to_insert);

      // compare with next item to left
      if (comp(*to_insert, *next_sorted)) {

        // swap down if should be before
        safe_iter_swap(next_sorted, to_insert);

      } else {

        // stop if in correct position
        break;
      }
    }
  }
}

/*!
    \brief get number of strides for shell sort
*/
RAJA_HOST_DEVICE RAJA_INLINE
constexpr size_t num_shell_strides()
{
  return 39;
}

/*!
    \brief get strides for shell sort
*/
RAJA_HOST_DEVICE RAJA_INLINE
constexpr long long unsigned get_shell_stride(int i)
{
  using array_type = long long unsigned[num_shell_strides()];
  return (array_type{
      // strides from M. Ciura 2001
      1llu, 4llu, 10llu, 23llu, 57llu, 132llu, 301llu, 701llu, 1750llu,
      // extended up to 2^47 with strides[n] = floor(2.25*strides[n-1])
      3937llu, 8858llu, 19930llu, 44842llu, 100894llu, 227011llu, 510774llu,
      1149241llu, 2585792llu, 5818032llu, 13090572llu, 29453787llu, 66271020llu,
      149109795llu, 335497038llu, 754868335llu, 1698453753llu, 3821520944llu,
      8598422124llu, 19346449779llu, 43529512002llu, 97941402004llu,
      220368154509llu, 495828347645llu, 1115613782201llu, 2510131009952llu,
      5647794772392llu, 12707538237882llu, 28591961035234llu, 64331912329276llu
    })[i];
}

/*!
    \brief unstable shell sort given range inplace using comparison function
    and using O(N^?) comparisons and O(1) memory
*/
template <typename Iter, typename Compare>
RAJA_HOST_DEVICE RAJA_INLINE
void
shell_sort(Iter begin,
           Iter end,
           Compare comp)
{
  using ::RAJA::safe_iter_swap;
  using diff_type = ::RAJA::detail::IterDiff<Iter>;

  diff_type n = end - begin;

  if (n <= static_cast<diff_type>(1)) {
    return;
  } else if (get_shell_stride(1) < static_cast<unsigned long long>(n)) {

    int i_stride = 2;
    // find first stride larger than n
    constexpr int num_strides = num_shell_strides();
    for (; i_stride < num_strides; ++i_stride) {
      if (get_shell_stride(i_stride) >= static_cast<unsigned long long>(n)) {
        break;
      }
    }
    // back up to first stride smaller than n
    i_stride -= 1;

    // for each stride size smaller than n, largest to smallest, not including 1
    // sort strided ranges with stride stride
    for (; i_stride > 0; --i_stride) {
      diff_type stride = static_cast<diff_type>(get_shell_stride(i_stride));

      // for each unsorted item in the right side of each strided range
      for (diff_type i_next_unsorted = stride; i_next_unsorted != n; ++i_next_unsorted) {

        // insert unsorted item into the sorted left side of the strided range
        for (diff_type i_to_insert = i_next_unsorted; i_to_insert >= stride; i_to_insert -= stride) {

          Iter to_insert = begin + i_to_insert;
          Iter next_sorted = to_insert - stride;

          // compare with next item to left
          if (comp(*to_insert, *next_sorted)) {

            // swap down if should be before
            safe_iter_swap(next_sorted, to_insert);

          } else {

            // stop if in correct position
            break;
          }
        }
      }
    }
  }

  // finish with stride size of 1, which is just normal insertion_sort
  RAJA::detail::insertion_sort(begin, end, comp);
}

/*!
    \brief insert the given element into the heaps below it
    using comparison function
    and using O(lg(N)) comparisons and O(1) memory
*/
template <typename Iter, typename Compare>
RAJA_HOST_DEVICE RAJA_INLINE
void
heapify(Iter begin,
        Iter root,
        Iter end,
        Compare comp)
{
  using RAJA::safe_iter_swap;

  auto N = end - begin;

  // heapify the root node into place
  // until this is a max heap again
  for (auto i = root - begin; 2*i+1 < N; i = root - begin) {

    // find the max item amongst the root, left child, and right child
    Iter maxit = root;

    // left child
    Iter child = begin + 2*i+1;
    if (comp(*maxit, *child)) {
      maxit = child;
    }

    // right child
    ++child;
    if (child != end && comp(*maxit, *child)) {
      maxit = child;
    }

    if (maxit == root) {
      // root is the max, done
      break;
    }

    // swap max child with root
    safe_iter_swap(root, maxit);
    // continue to heapify with the former max child
    root = maxit;
  }
}

/*!
    \brief unstable heap sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(1) memory
*/
template <typename Iter, typename Compare>
RAJA_HOST_DEVICE inline
void
heap_sort(Iter begin,
          Iter end,
          Compare comp)
{
  using RAJA::safe_iter_swap;

  auto N = end - begin;

  if (N < 2) {
    // already sorted
    return;
  }

  // make range into a max heap by
  // going through nodes with children one-by-one in reverse order
  for (Iter root = begin + (N-1)/2; root != begin; --root) {
    // heapify a sub-heap
    heapify(begin, root, end, comp);
  }
  // finish heapifying
  heapify(begin, begin, end, comp);

  // remove one element from max heap repeatedly until sorted
  for (--end; begin != end; --end) {

    // swap max element into sorted position at end of heap
    safe_iter_swap(begin, end);

    // fix top item of heap
    heapify(begin, begin, end, comp);
  }
}

/*!
    \brief unstable intro sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(lg(N)) memory
*/
template <typename Iter, typename Compare>
RAJA_HOST_DEVICE inline
void
intro_sort(Iter begin,
           Iter end,
           Compare comp,
           unsigned depth)
{
  using RAJA::safe_iter_swap;

  auto N = end - begin;

  // cutoff to use insertion sort
  static constexpr camp::decay<decltype(N)> insertion_sort_cutoff = 16;

  if (N < 2) {

    // already sorted

  } else if (N < insertion_sort_cutoff) {

    // use insertion sort for small inputs
    detail::insertion_sort(begin, end, comp);

  } else if (depth == 0) {

    // use heap sort if recurse too deep
    detail::heap_sort(begin, end, comp);

  } else {

    // use quick sort
    // choose pivot with median of 3 (N >= insertion_sort_cutoff)
    Iter mid = begin + N/2;
    Iter last = end-1;
    Iter pivot = comp(*begin, *mid)
                    ? ( comp(*mid, *last)
                           ? mid
                           : ( comp(*begin, *last)
                                  ? last
                                  : begin ) )
                    : ( comp(*mid, *last)
                           ? ( comp(*begin, *last)
                                  ? begin
                                  : last )
                           : mid );

    // swap pivot to last
    if (pivot != last) {
      safe_iter_swap(pivot, last);
      pivot = last;
    }

    // partition
    mid = partition(begin, last, [&](Iter it){ return comp(*it, *pivot); });

    // swap pivot to sorted position
    if (mid != pivot) {
      safe_iter_swap(mid, pivot);
      pivot = mid;
    }

    // recurse to sort first and second parts, ignoring already sorted pivot
    // by construction pivot is always in the range [begin, last]
    detail::intro_sort(begin, pivot, comp, depth-1);
    detail::intro_sort(RAJA::next(pivot), end, comp, depth-1);
  }

}

/*!
    \brief merge given two ranges using comparison function
    while copies are outside, somewhat follows STL API
*/
template <typename Iter1, typename Iter2, typename OutIter, typename Compare>
//constexpr OutIter // <-- std:: return value
void
RAJA_INLINE RAJA_HOST_DEVICE
merge_like_std( Iter1 first1,
                Iter1 last1,
                Iter2 first2,
                Iter2 last2,
                OutIter d_first,  // using this as direct access to result
                Compare comp)
{
  using ::RAJA::safe_iter_swap;

  if ( first1 == last2 )  // should never need to do this
  {
    return;
  }

  if ( (last2 - first1) == 1 ) // only 2 elements, simple swap
  {
    if ( !comp(*d_first, *(d_first+1)) )
    {
      safe_iter_swap( d_first, d_first+1 );
    }
    return;
  }

  while ( first1 <= last1 || first2 <= last2 )
  {
    if ( first1 > last1 ) // first half done
    {
      *d_first = *first2;
      ++first2;
    }
    else if ( first2 > last2 )  // second half done
    {
      *d_first = *first1;
      ++first1;
    }
    else  // neither half done
    {
      if ( comp( *first1, *first2 ) )
      {
        *d_first = *first1;
        ++first1;
      }
      else
      {
        *d_first = *first2;
        ++first2;
      }
    }

    ++d_first;  // advance output
  }

  return;
}

/*!
    \brief stable merge sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(N) memory
*/
template <typename Iter, typename Compare>
RAJA_INLINE RAJA_HOST_DEVICE
void
merge_sort(Iter begin,
           Iter end,
           Compare comp)
{
  // iterative mergesort (bottom up) for future parallelism

  // min helper
  auto minlam = [] (int a, int b) {return (a < b) ? a : b;};

  // insertion sort for sizes <= 16
  auto len = end - begin;
  static constexpr camp::decay<decltype(len)> insertion_sort_cutoff = 16;
  if ( len <= insertion_sort_cutoff && len > 0 )
  {
    detail::insertion_sort( begin, end, comp );
  }
  else
  {
    // insertion sort on 16-element chunks, then merge
    for ( int start = 0; start < len; start += insertion_sort_cutoff )
    {
      int lastchunk = minlam( insertion_sort_cutoff, len - start );
      detail::insertion_sort( begin + start, begin + start + lastchunk, comp );
    }

    // merge

    // copy input
    using itertype = camp::decay<decltype(*begin)>;
    itertype * copyarr = RAJA::allocate_aligned_type<itertype>( RAJA::DATA_ALIGN, len * sizeof(itertype) );
    for ( int cc = 0; cc < len; ++cc )
    {
      copyarr[cc] = *(begin + cc);
    }

    //for ( int midpoint = 1; midpoint < len; midpoint *= 2 )  // O(log n) loop
    for ( int midpoint = 16; midpoint < len; midpoint *= 2 )  // O(log n) loop
    {
      for ( int start = 0; start < len; start += midpoint * 2 )  // O(n) merging loop (can be parallelized)
      {
        int finish = minlam( start + midpoint * 2 - 1, len - 1 );
        RAJA_ASSERT( finish >= (midpoint + start) );  // sanity check

        if ( start + midpoint > len - 1 )
        {
          break;  // skip merge if no second half exists
        }

        detail::merge_like_std( copyarr + start, copyarr + start + midpoint - 1, copyarr + start + midpoint, copyarr + finish, begin + start, comp );
      }

      // update copy
      for ( int cc = 0; cc < len; ++cc )
      {
        copyarr[cc] = *(begin + cc);
      }
    }

    RAJA::free_aligned( copyarr );
  }
  //else
  //{
      // Possible TBD: in-place mergesort
      // Would shift (like insertion sort) when performing merge.
      // PRO - Can use on GPU, O(1) storage required.
      // CON - Shifting would cause slowdown O(n^2 log n).
  //}
}

}  // namespace detail

/*!
    \brief stable insertion sort given range inplace using comparison function
    and using O(N^2) comparisons and O(1) memory
*/
template <typename Iter,
          typename Compare = operators::less<detail::IterVal<Iter>>>
RAJA_HOST_DEVICE RAJA_INLINE
concepts::enable_if<type_traits::is_iterator<Iter>>
insertion_sort(Iter begin,
               Iter end,
               Compare comp = Compare{})
{
  auto N = end - begin;

  if (N > 1) {

    detail::insertion_sort(begin, end, comp);
  }
}

/*!
    \brief unstable shell sort given range inplace using comparison function
    and using O(N^?) comparisons and O(1) memory
*/
template <typename Iter,
          typename Compare = operators::less<detail::IterVal<Iter>>>
RAJA_HOST_DEVICE RAJA_INLINE
concepts::enable_if<type_traits::is_iterator<Iter>>
shell_sort(Iter begin,
               Iter end,
               Compare comp = Compare{})
{
  auto N = end - begin;

  if (N > 1) {

    detail::shell_sort(begin, end, comp);
  }
}

/*!
    \brief unstable heap sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(1) memory
*/
template <typename Iter,
          typename Compare = operators::less<detail::IterVal<Iter>>>
RAJA_HOST_DEVICE RAJA_INLINE
concepts::enable_if<type_traits::is_iterator<Iter>>
heap_sort(Iter begin,
          Iter end,
          Compare comp = Compare{})
{
  auto N = end - begin;

  if (N > 1) {

    detail::heap_sort(begin, end, comp);
  }
}

/*!
    \brief unstable intro sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(lg(N)) memory
*/
template <typename Iter,
          typename Compare = operators::less<detail::IterVal<Iter>>>
RAJA_HOST_DEVICE RAJA_INLINE
concepts::enable_if<type_traits::is_iterator<Iter>>
intro_sort(Iter begin,
           Iter end,
           Compare comp = Compare{})
{
  auto N = end - begin;

  if (N > 1) {

    // sset max depth to 2*lg(N)
    unsigned max_depth = 2*detail::ulog2(N);

    detail::intro_sort(begin, end, comp, max_depth);
  }
}

/*!
    \brief stable merge sort given range inplace using comparison function
    and using O(N*lg(N)) comparisons and O(N) memory
*/
template <typename Iter,
          typename Compare = operators::less<detail::IterVal<Iter>>>
RAJA_HOST_DEVICE RAJA_INLINE
concepts::enable_if<type_traits::is_iterator<Iter>>
merge_sort(Iter begin,
           Iter end,
           Compare comp = Compare{})
{
  auto N = end - begin;

  if (N > 1) {

    detail::merge_sort(begin, end, comp);
  }
}

}  // namespace RAJA

#endif