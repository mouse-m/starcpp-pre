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

#ifndef STAR_MATH_H
#define STAR_MATH_H

#include<algorithm>
#include<cmath>

namespace STAR_CPP {
	
	template <typename T>
	class Math {
	public:
		struct Point { T x, y; };
		
	private:
		static T mul_mod(T a, T b, T mod) {
			T res = 0;
			a %= mod;
			while (b > 0) {
				if (b & 1) res = (res + a) % mod;
				a = (a + a) % mod;
				b >>= 1;
			}
			return res;
		}
		
	public:
		static T dist_sq(const Point& a, const Point& b) {
			T dx = a.x - b.x;
			T dy = a.y - b.y;
			return dx * dx + dy * dy;
		}
		
		static double dist(const Point& a, const Point& b) {
			return std::sqrt((double)dist_sq(a, b));
		}
		
		static T manhattan_dist(const Point& a, const Point& b) {
			T ax = a.x < b.x ? b.x - a.x : a.x - b.x;
			T ay = a.y < b.y ? b.y - a.y : a.y - b.y;
			return ax + ay;
		}
		
		static T cross(const Point& a, const Point& b, const Point& c) {
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		
		static T dot(const Point& a, const Point& b, const Point& c) {
			return (b.x - a.x) * (c.x - a.x) + (b.y - a.y) * (c.y - a.y);
		}
		
		static double polygon_area(const std::vector<Point>& p) {
			double area = 0.0;
			int n = p.size();
			for (int i = 0; i < n; ++i) {
				area += (double)p[i].x * p[(i + 1) % n].y;
				area -= (double)p[i].y * p[(i + 1) % n].x;
			}
			return area < 0 ? -area / 2.0 : area / 2.0;
		}
		
		static std::vector<Point> convex_hull(std::vector<Point> p) {
			int n = p.size();
			if (n <= 1) return p;
			std::sort(p.begin(), p.end(), [](const Point& a, const Point& b) {
				return a.x == b.x ? a.y < b.y : a.x < b.x;
			});
			std::vector<Point> hull(n * 2);
			int k = 0;
			for (int i = 0; i < n; ++i) {
				while (k > 1 && cross(hull[k - 2], hull[k - 1], p[i]) <= 0) k--;
				hull[k++] = p[i];
			}
			for (int i = n - 2, t = k; i >= 0; --i) {
				while (k > t && cross(hull[k - 2], hull[k - 1], p[i]) <= 0) k--;
				hull[k++] = p[i];
			}
			hull.resize(k - 1);
			return hull;
		}
		
		static T gcd(T a, T b) {
			while (b) {
				T r = a % b;
				a = b;
				b = r;
			}
			return a;
		}
		
		static T lcm(T a, T b) {
			if (a == 0 || b == 0) return 0;
			T g = gcd(a, b);
			return (a / g) * b;
		}
		
		static T qpow(T base, T exp) {
			T res = 1;
			while (exp > 0) {
				if (exp & 1) res *= base;
				base *= base;
				exp >>= 1;
			}
			return res;
		}
		
		static T qpow_mod(T base, T exp, T mod) {
			T res = 1 % mod;
			base %= mod;
			while (exp > 0) {
				if (exp & 1) res = mul_mod(res, base, mod);
				base = mul_mod(base, base, mod);
				exp >>= 1;
			}
			return res;
		}
		
		static T inv_mod(T a, T mod) {
			return qpow_mod(a, mod - 2, mod);
		}
		
		static long long factorial(int n) {
			if (n < 0) return -1;
			if (n > 20) return -1;
			long long res = 1;
			for (int i = 2; i <= n; ++i) res *= i;
			return res;
		}
		
		static long long P(int n, int m) {
			if (m < 0 || n < 0 || m > n) return 0;
			if (n > 20) return -1;
			long long res = 1;
			for (int i = 0; i < m; ++i) res *= (n - i);
			return res;
		}
		
		static long long C(int n, int m) {
			if (m < 0 || n < 0 || m > n) return 0;
			if (m > n - m) m = n - m;
			if (n > 60) return -1;
			long long res = 1;
			for (int i = 1; i <= m; ++i) {
				res = res * (n - m + i) / i;
			}
			return res;
		}
		
		static std::vector<T> factorials_mod(int n, T mod) {
			std::vector<T> fac(n + 1);
			fac[0] = 1 % mod;
			for (int i = 1; i <= n; ++i) {
				fac[i] = mul_mod(fac[i - 1], (T)i, mod);
			}
			return fac;
		}
		
		static std::vector<T> inv_factorials_mod(int n, T mod, const std::vector<T>& fac) {
			std::vector<T> inv_fac(n + 1);
			inv_fac[n] = inv_mod(fac[n], mod);
			for (int i = n - 1; i >= 0; --i) {
				inv_fac[i] = mul_mod(inv_fac[i + 1], (T)(i + 1), mod);
			}
			return inv_fac;
		}
		
		static T C_mod(int n, int m, T mod, const std::vector<T>& fac, const std::vector<T>& inv_fac) {
			if (m < 0 || m > n) return 0;
			T res = mul_mod(fac[n], inv_fac[m], mod);
			return mul_mod(res, inv_fac[n - m], mod);
		}
		
		static T C_mod_direct(int n, int m, T mod) {
			if (m < 0 || m > n) return 0;
			m = m < n - m ? m : n - m;
			T res = 1;
			for (int i = 1; i <= m; ++i) {
				T numerator = (n - m + i) % mod;
				T denominator = inv_mod((T)i, mod);
				res = mul_mod(res, numerator, mod);
				res = mul_mod(res, denominator, mod);
			}
			return res;
		}
		
		static bool is_prime(long long n) {
			if (n < 2) return false;
			if (n == 2 || n == 3) return true;
			if (n % 2 == 0) return false;
			long long d = n - 1;
			int s = 0;
			while ((d & 1) == 0) {
				d >>= 1;
				s++;
			}
			static const std::vector<int> bases = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };
			for (int a : bases) {
				if (n == a) return true;
				if (n % a == 0) return false;
				long long x = qpow_mod((long long)a, d, (long long)n);
				if (x == 1 || x == n - 1) continue;
				bool composite = true;
				for (int r = 1; r < s; ++r) {
					x = mul_mod(x, x, (long long)n);
					if (x == n - 1) {
						composite = false;
						break;
					}
				}
				if (composite) return false;
			}
			return true;
		}
		
		static long long next_prime(long long n) {
			if (n < 2) return 2;
			long long res = (n % 2 == 0) ? n + 1 : n + 2;
			while (!is_prime(res)) {
				res += 2;
			}
			return res;
		}
		
		static std::vector<int> sieve_primes(int n) {
			std::vector<bool> is_composite(n + 1, false);
			std::vector<int> primes;
			for (int i = 2; i <= n; ++i) {
				if (!is_composite[i]) primes.push_back(i);
				for (int p : primes) {
					if ((long long)i * p > n) break;
					is_composite[i * p] = true;
					if (i % p == 0) break;
				}
			}
			return primes;
		}
	};
	
}

#endif

