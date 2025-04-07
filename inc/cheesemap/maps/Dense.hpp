#pragma once

#include <array>
#include <execution>
#include <utility>
#include <vector>

#include <range/v3/all.hpp>

#include "cheesemap/concepts/concepts.hpp"
#include "cheesemap/kernels/kernels.hpp"

#include "cheesemap/utils/Box.hpp"
#include "cheesemap/utils/Cell.hpp"
#include "cheesemap/utils/sorted_vector.hpp"

#include "cheesemap/utils/arithmetic.hpp"
#include "cheesemap/utils/Cartesian.hpp"
#include "cheesemap/utils/flags.hpp"
#include "cheesemap/utils/type_traits.hpp"

namespace chs
{
	template<typename Point_type, std::size_t Dim = 3>
	class Dense
	{
		protected:
		using resolution_type = double;
		using dimensions_type = chs::type_traits::tuple<resolution_type, Dim>;
		using indices_type    = chs::type_traits::tuple<std::size_t, Dim>;
		using cell_type       = Cell<Point_type>;

		static constexpr dimensions_type DEFAULT_RESOLUTIONS = chs::n_tuple<Dim>(resolution_type{ 1 });

		// Dimension of each cell
		dimensions_type resolutions_ = DEFAULT_RESOLUTIONS;

		// Bounding box of the map
		Box box_;

		// Number of cells of the map on each dimension
		indices_type sizes_;

		// Cells of the map
		std::vector<cell_type> cells_;

		template<std::size_t... Is>
		[[nodiscard]] inline auto indices2global(const auto & indices, std::index_sequence<Is...>) const
		{
			std::size_t idx = 0;

			((idx = idx * std::get<Is>(sizes_) + std::get<Is>(indices)), ...);

			return idx;
		}

		[[nodiscard]] inline auto indices2global(const auto & indices) const
		{
			return indices2global(indices, std::make_index_sequence<Dim>{});
		}

		template<std::size_t... Is>
		[[nodiscard]] inline auto idx2box(const auto & idx, std::index_sequence<Is...>) const
		{
			Point min = box_.min();
			Point max = box_.max();

			(((min[Is] = box_.min()[Is] +
			             static_cast<resolution_type>(std::get<Is>(idx)) * std::get<Is>(resolutions_)),
			  (max[Is] = min[Is] + std::get<Is>(resolutions_))),
			 ...);

			return Box(std::make_pair(min, max));
		}

		[[nodiscard]] inline auto idx2box(const auto & idx) const
		{
			return idx2box(idx, std::make_index_sequence<Dim>{});
		}

		template<std::size_t... Is>
		[[nodiscard]] inline auto coord2indices(const Point & p, std::index_sequence<Is...>) const
		{
			indices_type idx;

			resolution_type diff;

			(((diff = p[Is] - box_.min()[Is]),
			  (diff = std::clamp(diff, 0.0, box_.max()[Is] - box_.min()[Is])),
			  (std::get<Is>(idx) = static_cast<std::size_t>(diff / std::get<Is>(resolutions_)))),
			 ...);

			return idx;
		}

		[[nodiscard]] inline auto coord2indices(const Point & p) const
		{
			return coord2indices(p, std::make_index_sequence<Dim>{});
		}

		[[nodiscard]] inline auto & at(const auto & indices) { return cells_[indices2global(indices)]; }

		[[nodiscard]] inline auto & at(const auto & indices) const { return cells_[indices2global(indices)]; }

		public:
		Dense() = delete;

		template<ranges::range Points_rng>
		Dense(Points_rng & points, const resolution_type res, chs::flags::build::flags_t flags = {}) :
		        Dense(points, dimensions_type{ n_tuple<Dim>(res) }, flags)
		{}

		template<ranges::range Points_rng> // requires to be sortable
		Dense(Points_rng & points, dimensions_type res, chs::flags::build::flags_t flags = {}) :
		        resolutions_(res), box_(Box::mbb(points))
		{
			// Number of cells in each dimension
			[&]<std::size_t... Is>(std::index_sequence<Is...>) {
				((std::get<Is>(sizes_) =
				          static_cast<std::size_t>(std::floor((box_.max()[Is] - box_.min()[Is]) /
				                                              std::get<Is>(resolutions_))) +
				          1),
				 ...);
			}(std::make_index_sequence<Dim>{});

			// Create cells
			const auto num_cells = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
				std::size_t acc = 1;
				((acc *= std::get<Is>(sizes_)), ...);
				return acc;
			}(std::make_index_sequence<Dim>{});

			cells_.resize(num_cells);

			// Sort points by global idx (should improve locality when querying)
			if (flags & chs::flags::build::REORDER)
			{
				const auto proj = [&](const auto & p) { return indices2global(coord2indices(p)); };
				const auto cmp  = [&](const auto & a, const auto & b) { return proj(a) < proj(b); };
				if (flags & chs::flags::build::PARALLEL)
				{
					std::sort(std::execution::par_unseq, points.begin(), points.end(), cmp);
				}
				else { std::sort(points.begin(), points.end(), cmp); }
			}

			// Assign points to cells
			ranges::for_each(points, [&](auto & point) { at(coord2indices(point)).emplace_back(&point); });

			if (flags & chs::flags::build::SHRINK_TO_FIT)
			{
				ranges::for_each(cells_, [](auto & cell) { cell.shrink_to_fit(); });
			}
		}

		template<chs::concepts::Kernel<chs::Point> Kernel_t>
		[[nodiscard]] inline auto query(const Kernel_t & kernel) const
		{
			const auto dummy = []([[maybe_unused]] const auto &) { return true; };
			return query(kernel, dummy);
		}

		template<chs::concepts::Kernel<chs::Point> Kernel_t, chs::concepts::Filter<Point_type> Filter_t>
		[[nodiscard]] inline auto query(const Kernel_t & kernel, Filter_t && filter) const
		{
			std::vector<Point *> points;

			const auto min = coord2indices(kernel.box().min());
			const auto max = coord2indices(kernel.box().max());

			for (const auto indices : chs::cartesian<Dim>(min, max))
			{
				const auto & cell = at(indices);
				for (const auto & point : cell)
				{
					if (kernel.is_inside(*point) and filter(*point)) { points.emplace_back(point); }
				}
			}

			return points;
		}

		[[nodiscard]] inline auto knn(const std::integral auto k, const Point_type & p) const
		{
			// Store the points and the distance
			chs::sorted_vector<std::pair<double, Point_type *>> candidates(k);

			// Search radius starts within the cell containing p
			double search_radius = idx2box(coord2indices(p)).distance_to_wall(p, /* inside = */ true);

			auto candidates_within_sq_radius = [&] {
				const auto sq_radius = search_radius * search_radius;
				const auto it        = std::upper_bound(candidates.begin(), candidates.end(), sq_radius,
				                                        [](auto d, auto & pt) { return d < pt.first; });
				return it - candidates.begin();
			};

			// Taboo list (to avoid visiting the same cell twice)
			indices_type taboo_mins;
			indices_type taboo_maxs;

			auto is_taboo = [&](const auto & indices) {
				return chs::within_closed_bounds<Dim>(indices, taboo_mins, taboo_maxs);
			};

			// Do an increasing search
			const double default_radius_increment = chs::min<Dim>(resolutions_);

			// Explore first the neighbors of the cell containing p
			{
				const auto indices = coord2indices(p);
				taboo_mins         = indices;
				taboo_maxs         = indices;
				const auto & cell  = at(indices);
				for (const auto & point : cell)
				{
					candidates.insert({ chs::sq_distance(p, *point), point });
				}
			}

			while (
			        // not enough candidates or last candidate is outside the search radius
			        (std::cmp_less(candidates.size(), k) or
			         candidates.back().first > (search_radius * search_radius)) and
			        // we have not visited all the cells
			        not chs::all_visited<Dim>(taboo_mins, taboo_maxs, sizes_))
			{
				// Estimate the new required search radius -> k * density -> Saves time ~86% of the queries
				if (not candidates.empty() and search_radius > 0)
				{
					const auto density_based_radius = chs::radius_for_density(
					        candidates_within_sq_radius(), search_radius, k);
					search_radius = std::min(density_based_radius,
					                         search_radius + default_radius_increment);
				}
				else { search_radius += default_radius_increment; }

				const auto min = coord2indices(p - search_radius);
				const auto max = coord2indices(p + search_radius);

				// If min == taboo_mins and max == taboo_maxs, we have already visited all the cells
				if (chs::all_equal<Dim>(min, taboo_mins) and chs::all_equal<Dim>(max, taboo_maxs))
				{
					continue;
				}

				for (const auto indices : chs::cartesian<Dim>(min, max))
				{
					if (is_taboo(indices)) { continue; }

					const auto & cell = at(indices);
					for (const auto & point : cell)
					{
						candidates.insert({ chs::sq_distance(p, *point), point });
					}
				}

				taboo_mins = min;
				taboo_maxs = max;
			}

			ranges::for_each(candidates,
			                 [](auto & candidate) { candidate.first = std::sqrt(candidate.first); });

			return candidates;
		}

		[[nodiscard]] inline auto mem_footprint() const
		{
			std::size_t bytes = sizeof(*this);

			for (const auto & cell : cells_)
			{
				bytes += sizeof(cell);
				bytes += cell.capacity() * sizeof(Point_type *);
			}

			return bytes;
		}

		[[nodiscard]] inline auto get_num_cells() const
		{
			return cells_.size();
		}

		[[nodiscard]] inline auto get_empty_cells() const
		{
			int empty = 0;
			for (const auto & cell : cells_)
			{
				if (!cell.size()) empty++;
			}
			return empty;
		}
	};
} // namespace chs