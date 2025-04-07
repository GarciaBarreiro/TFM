#pragma once

#include <type_traits>

#include "cheesemap/maps/Mixed2D.hpp"
#include "cheesemap/maps/Mixed3D.hpp"

namespace chs
{
	template<std::size_t Dim>
	struct MixedValidDim
	{
		static_assert(Dim == 2 or Dim == 3, "chs::Mixed<> dimension must be 2 or 3");
		static constexpr bool value = true;
	};

	template<typename Point_type, std::size_t Dim = 3, typename = std::enable_if<MixedValidDim<Dim>::value>>
	using Mixed = std::conditional_t<Dim == 2, Mixed2D<Point_type>, Mixed3D<Point_type>>;
} // namespace chs