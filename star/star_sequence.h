/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.

*/

#ifndef STAR_SEQUENCE_H
#define STAR_SEQUENCE_H

#include "star_random.h"
namespace STAR_CPP {

	template <typename Engine>
	class Sequence {
	private:
		Random<Engine> rng;

	public:
		explicit Sequence(unsigned seed = 0) : rng(seed) {}

		template <typename Type>
		std::vector<Type> range(Type start, Type end, Type step = 1) {
			if (step == 0) throw std::invalid_argument("Step cannot be zero");

			std::vector<Type> seq;

			if constexpr (std::is_integral_v<Type>) {
				seq.reserve(static_cast<size_t>((end - start) / step) + 10);
			}

			if (step > 0) {
				for (Type i = start; i <= end; i += step) seq.push_back(i);
			} else {
				for (Type i = start; i >= end; i += step) seq.push_back(i);
			}

			return seq;
		}

		template <typename Type, typename Dist>
		std::vector<Type> randomStep(Type start, Type end, Dist dist) {
			std::vector<Type> seq;
			Type current = start;
			const Type direction = (start <= end) ? Type(1) : Type(-1);

			while ((direction > 0 && current <= end) || (direction < 0 && current >= end)) {
				seq.push_back(current);

				Type step = rng(dist);

				if constexpr (std::is_signed_v<Type>) {
					Type abs_step = (step == 0) ? Type(1) : std::abs(step);
					current += direction * abs_step;
				} else {
					current += step;
				}
			}

			return seq;
		}

		template <typename Type>
		std::vector<Type> randomStep(Type start, Type end, Type min_step, Type max_step) {
			if constexpr (std::is_integral_v<Type>) {
				std::uniform_int_distribution<Type> dist(min_step, max_step);
				return randomStep(start, end, dist);
			} else {
				std::uniform_real_distribution<Type> dist(min_step, max_step);
				return randomStep(start, end, dist);
			}
		}

		template <typename Type>
		std::vector<Type> sample(const std::vector<Type>& population, int k) {
			if (population.empty()) return {};

			int n = static_cast<int>(population.size());

			if (k >= n) {
				std::vector<Type> result = population;
				rng.shuffle(result);
				return result;
			}

			return rng.choices(population, k);
		}

		template <typename Type>
		std::vector<Type> sample(Type start, Type end, int k) {
			if constexpr(std::is_integral_v<Type>) {
				if (start > end) std::swap(start, end);

				std::vector<Type> full_pop = range(start, end);
				return sample(full_pop, k);
			} else {
				throw std::invalid_argument("Sample from range only supports integral types.");
			}
		}

		template <typename Type>
		std::vector<Type> weighted_sample(const std::vector<Type>& population, int k, const std::vector<double>& weights) {
			if (population.empty() || weights.empty()) return {};

			if (population.size() != weights.size()) throw std::invalid_argument("Size mismatch");

			std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
			std::vector<Type> results;
			results.reserve(k);

			for (int i = 0; i < k; ++i) {
				results.push_back(population[dist(rng)]);
			}

			return results;
		}

		template <typename Type>
		void shuffle(std::vector<Type>& container) {
			rng.shuffle(container);
		}
	};

}

#endif

