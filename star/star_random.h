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

#ifndef STAR_RANDOM_H
#define STAR_RANDOM_H

#include<mutex>
#include<random>
#include<stdexcept>
#include<algorithm>
#include "extend/StarExtendedRandomEngine.h"

namespace STAR_CPP {
	
	template <typename Engine>
	class Random {
	private:
		Engine engine;
		mutable std::mutex mtx;
		
		static const std::string CHARSET_ALPHA;
		static const std::string CHARSET_NUMERIC;
		static const std::string CHARSET_HEX;
		static const std::string CHARSET_ALPHANUMERIC;
		
	public:
		explicit Random(unsigned seed = 0)
		: engine(typename Engine::result_type(seed)) {}
		
		Random() : engine(typename Engine::result_type(std::chrono::steady_clock::now().time_since_epoch().count())) {}
		
		int randint(int min, int max) {
			std::lock_guard<std::mutex> lock(mtx);
			std::uniform_int_distribution<int> dist(min, max);
			return dist(engine);
		}
		
		double random(double min = 0.0, double max = 1.0) {
			std::lock_guard<std::mutex> lock(mtx);
			std::uniform_real_distribution<double> dist(min, max);
			return dist(engine);
		}
		
		std::string randstr(int length, const std::string& charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {
			std::lock_guard<std::mutex> lock(mtx);
			
			if (charset.empty()) return "";
			
			std::string result;
			result.reserve(length);
			std::uniform_int_distribution<int> dist(0, static_cast<int>(charset.length()) - 1);
			
			for (int i = 0; i < length; ++i) {
				result += charset[dist(engine)];
			}
			
			return result;
		}
		
		std::string randAlphaStr(int length) {
			return randstr(length, CHARSET_ALPHA);
		}
		
		std::string randPalindromeStr(int length) {
			std::string front = randstr(length / 2);
			std::string back = front;
			std::reverse(back.begin(), back.end());
			
			if (length % 2 == 0) {
				return front + back;
			}
			else {
				std::string middle_char = randstr(1, CHARSET_ALPHA);
				return front + middle_char + back;
			}
		}
		
		std::string randNumericStr(int length) {
			return randstr(length, CHARSET_NUMERIC);
		}
		
		std::string randHexStr(int length) {
			return randstr(length, CHARSET_HEX);
		}
		
		std::string randDistinctStr(int length, int distinct_count) {
			std::lock_guard<std::mutex> lock(mtx);
			if (distinct_count <= 0 || distinct_count > (int)CHARSET_ALPHANUMERIC.length()) return "";
			std::string sub_charset = CHARSET_ALPHANUMERIC.substr(0, distinct_count);
			std::string result;
			result.reserve(length);
			std::uniform_int_distribution<int> dist(0, static_cast<int>(sub_charset.length()) - 1);
			for (int i = 0; i < length; ++i) {
				result += sub_charset[dist(engine)];
			}
			return result;
		}
		
		std::string randPeriodStr(int length, int period) {
			if (period <= 0) period = length;
			std::string base = randAlphaStr(period);
			std::string result;
			result.reserve(length);
			for (int i = 0; i < length; ++i) {
				result += base[i % period];
			}
			return result;
		}
		
		std::vector<int> randperm(int n) {
			std::lock_guard<std::mutex> lock(mtx);
			std::vector<int> result(n);
			
			for (int i = 0; i < n; ++i) result[i] = i + 1;
			
			std::shuffle(result.begin(), result.end(), engine);
			return result;
		}
		
		std::vector<int> randperm(int min, int max) {
			std::lock_guard<std::mutex> lock(mtx);
			
			if (min > max) std::swap(min, max);
			
			int n = max - min + 1;
			std::vector<int> result(n);
			
			for (int i = 0; i < n; ++i) result[i] = min + i;
			
			std::shuffle(result.begin(), result.end(), engine);
			return result;
		}
		
		template<typename T>
		T choice(const std::vector<T>& container) {
			std::lock_guard<std::mutex> lock(mtx);
			
			if (container.empty()) throw std::invalid_argument("Cannot choice from empty container");
			
			std::uniform_int_distribution<size_t> dist(0, container.size() - 1);
			return container[dist(engine)];
		}
		
		template<typename T>
		std::vector<T> choices(const std::vector<T>& container, int k) {
			std::lock_guard<std::mutex> lock(mtx);
			
			if (container.empty()) throw std::invalid_argument("Empty container");
			
			int n = static_cast<int>(container.size());
			
			if (k > n) k = n;
			
			if (k <= 0) return {};
			
			std::vector<T> result = container;
			
			for (int i = 0; i < k; ++i) {
				std::uniform_int_distribution<int> dist(i, n - 1);
				int j = dist(engine);
				std::swap(result[i], result[j]);
			}
			
			return std::vector<T>(result.begin(), result.begin() + k);
		}
		
		template<typename T>
		T weightedChoice(const std::vector<T>& items, const std::vector<double>& weights) {
			std::lock_guard<std::mutex> lock(mtx);
			
			if (items.empty() || weights.empty()) throw std::invalid_argument("Empty inputs");
			
			if (items.size() != weights.size()) throw std::invalid_argument("Size mismatch");
			
			std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
			return items[dist(engine)];
		}
		
		template<typename T>
		void shuffle(std::vector<T>& container) {
			std::lock_guard<std::mutex> lock(mtx);
			std::shuffle(container.begin(), container.end(), engine);
		}
		
		template <typename Distribution>
		auto operator()(Distribution& dist) -> typename Distribution::result_type {
			std::lock_guard<std::mutex> lock(mtx);
			return dist(engine);
		}
	};
	
	template <typename Engine>
	const std::string Random<Engine>::CHARSET_ALPHA =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	template <typename Engine>
	const std::string Random<Engine>::CHARSET_NUMERIC =
	"0123456789";
	
	template <typename Engine>
	const std::string Random<Engine>::CHARSET_HEX =
	"0123456789abcdef";
	
	template <typename Engine>
	const std::string Random<Engine>::CHARSET_ALPHANUMERIC =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	
}

#endif

