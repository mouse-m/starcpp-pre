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

#ifndef STAR_RAND_GENE_H
#define STAR_RAND_GENE_H

#include "..\utils\StarError.h"
namespace STAR_CPP {
	namespace extend {
		class MinstdLCG {
		public:
			using result_type = uint32_t;
			explicit MinstdLCG(result_type seed): state(seed) {
				if (state == 0 || state == M) {
					state = 1;
				}
			}

			result_type operator ()() {
				const uint64_t product = static_cast<uint64_t>(state) * A;
				state = static_cast<result_type>(product % M);
				return state;
			}

			static constexpr result_type max() {
				return M - 1;
			}

			static constexpr result_type min() {
				return 1;
			}

			static constexpr result_type modulus() {
				return M;
			}

			static constexpr result_type multiplier() {
				return A;
			}

		private:
			result_type state;
			static constexpr result_type M = 2147483647;
			static constexpr result_type A = 48271;
			static constexpr result_type C = 0;
		};

		class SplitMix64Rand {
		public:
			using result_type = uint64_t;

			static constexpr result_type min() {
				return 0;
			}

			static constexpr result_type max() {
				return 18446744073709551615ULL;
			}

			explicit SplitMix64Rand(uint64_t seed) : state(seed) {}

			result_type operator()() {
				uint64_t z = (state += 0x9e3779b97f4a7c15);
				z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
				z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
				return z ^ (z >> 31);
			}

		private:
			uint64_t state;
		};

		class XSH_RR_PCG {
		public:
			using result_type = uint32_t;

			static constexpr result_type min() {
				return 0;
			}

			static constexpr result_type max() {
				return 4294967295U;
			}

			explicit XSH_RR_PCG(uint64_t seed) {
				state = 0;
				increment = (seed << 1) | 1;
				state += seed;
				(*this)();
			}

			result_type operator()() {
				uint64_t oldstate = state;

				state = oldstate * 6364136223846793005ULL + increment;

				uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
				uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);

				return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
			}

		private:
			uint64_t state;
			uint64_t increment;
		};
	}
}

#endif
