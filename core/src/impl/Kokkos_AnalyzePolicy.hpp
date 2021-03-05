/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 3.0
//       Copyright (2020) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Christian R. Trott (crtrott@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#ifndef KOKKOS_IMPL_ANALYZE_POLICY_HPP
#define KOKKOS_IMPL_ANALYZE_POLICY_HPP

#include <Kokkos_Core_fwd.hpp>
#include <Kokkos_Concepts.hpp>
#include <impl/Kokkos_Tags.hpp>
#include <impl/Kokkos_GraphImpl_fwd.hpp>
#include <impl/Kokkos_Error.hpp>
#include <traits/Kokkos_Traits_fwd.hpp>
#include <traits/Kokkos_PolicyTraitAdaptor.hpp>

#include <traits/Kokkos_ExecutionSpaceTrait.hpp>
#include <traits/Kokkos_GraphKernelTrait.hpp>
#include <traits/Kokkos_IndexTypeTrait.hpp>
#include <traits/Kokkos_IterationPatternTrait.hpp>
#include <traits/Kokkos_LaunchBoundsTrait.hpp>
#include <traits/Kokkos_OccupancyControlTrait.hpp>
#include <traits/Kokkos_ScheduleTrait.hpp>
#include <traits/Kokkos_WorkItemPropertyTrait.hpp>
#include <traits/Kokkos_WorkTagTrait.hpp>

namespace Kokkos {
namespace Impl {

//------------------------------------------------------------------------------

using execution_policy_trait_specifications =
    type_list<ExecutionSpaceTrait, GraphKernelTrait, IndexTypeTrait,
              IterationPatternTrait, LaunchBoundsTrait, OccupancyControlTrait,
              ScheduleTrait, WorkItemPropertyTrait, WorkTagTrait>;

//------------------------------------------------------------------------------
// Ignore void for backwards compatibility purposes, though hopefully no one is
// using this in application code
template <class... Traits>
struct AnalyzeExecPolicy<void, void, Traits...>
    : AnalyzeExecPolicy<void, Traits...> {
  using base_t = AnalyzeExecPolicy<void, Traits...>;
  using base_t::base_t;
};

//------------------------------------------------------------------------------
// Mix in the defaults (base_traits) for the traits that aren't yet handled

template <class TraitSpecList>
struct AnalyzeExecPolicyBaseTraits;
template <class... TraitSpecifications>
struct AnalyzeExecPolicyBaseTraits<type_list<TraitSpecifications...>>
    : TraitSpecifications::base_traits... {};

template <>
struct AnalyzeExecPolicy<void>
    : AnalyzeExecPolicyBaseTraits<execution_policy_trait_specifications> {
  using work_tag = void;

  // Ensure default constructibility since a converting constructor causes it to
  // be deleted.
  AnalyzeExecPolicy() = default;

  // Base converting constructor and assignment operator: unless an individual
  // policy analysis deletes a constructor, assume it's convertible
  template <class Other>
  AnalyzeExecPolicy(ExecPolicyTraitsWithDefaults<Other> const&) {}

  template <class Other>
  AnalyzeExecPolicy& operator=(ExecPolicyTraitsWithDefaults<Other> const&) {
    return *this;
  }
};

//------------------------------------------------------------------------------
// Used for defaults that depend on other analysis results
template <class AnalysisResults>
struct ExecPolicyTraitsWithDefaults : AnalysisResults {
  using base_t = AnalysisResults;
  using base_t::base_t;
  // The old code turned this into an integral type for backwards compatibility,
  // so that's what we're doing here. The original comment was:
  //   nasty hack to make index_type into an integral_type
  //   instead of the wrapped IndexType<T> for backwards compatibility
  using index_type = typename std::conditional_t<
      base_t::index_type_is_defaulted,
      Kokkos::IndexType<typename base_t::execution_space::size_type>,
      typename base_t::index_type>::type;
};

//------------------------------------------------------------------------------
template <typename... Traits>
struct PolicyTraits
    : ExecPolicyTraitsWithDefaults<AnalyzeExecPolicy<void, Traits...>> {
  using base_t =
      ExecPolicyTraitsWithDefaults<AnalyzeExecPolicy<void, Traits...>>;
  using base_t::base_t;
};

}  // namespace Impl
}  // namespace Kokkos

#endif  // KOKKOS_IMPL_ANALYZE_POLICY_HPP
