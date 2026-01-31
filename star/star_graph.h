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

#ifndef STAR_GRAPH_H
#define STAR_GRAPH_H

#include "star_random.h"
#include<set>
#include<map>
#include<queue>

namespace STAR_CPP {
	
	template <typename Engine, typename Node = int, typename Weight = long long>
	class Graph {
	private:
		Random<Engine> rng;
		
		std::vector<Node> selectNodes(Node limit, Node count) {
			if (limit < 1 || count < 1 || count > limit) return {};
			std::vector<Node> pool;
			for (Node i = 1; i <= limit; i++) pool.push_back(i);
			rng.shuffle(pool);
			return std::vector<Node>(pool.begin(), pool.begin() + (int)count);
		}
		
	public:
		using Edge = std::pair<Node, Node>;
		struct FlowEdge { Node u, v; Weight cap; };
		struct CostFlowEdge { Node u, v; Weight cap, cost; };
		
		explicit Graph(unsigned seed = 0) : rng(seed) {}
		
		std::vector<Edge> exactKAryTree(Node n, int k) {
			if (n <= 0 || k < 1) return {};
			if (n == 1) return {};
			std::vector<Edge> edges;
			edges.reserve((int)n - 1);
			std::vector<std::vector<Node>> layers;
			layers.push_back({ Node(1) });
			
			Node current = 2;
			for (int l = 0; l < (int)layers.size(); ++l) {
				if (current > n) break;
				std::vector<Node> next_layer;
				for (Node parent : layers[l]) {
					for (int child = 0; child < k && current <= n; ++child) {
						edges.push_back({ parent, current });
						next_layer.push_back(current);
						current++;
					}
				}
				if (!next_layer.empty()) layers.push_back(next_layer);
			}
			return edges;
		}
		
		std::vector<Edge> randomForest(Node n, int components) {
			if (n <= 0 || components <= 0 || components > n) return {};
			std::vector<Node> pool(n);
			for (Node i = 0; i < n; ++i) pool[i] = i + 1;
			rng.shuffle(pool);
			
			std::vector<int> sizes(components);
			std::fill(sizes.begin(), sizes.end(), 1);
			int remaining = n - components;
			for (int i = 0; i < remaining; ++i) {
				sizes[rng.randint(0, components - 1)]++;
			}
			
			std::vector<Edge> all_edges;
			Node id_offset = 1;
			for (int i = 0; i < components; ++i) {
				int sz = sizes[i];
				if (sz == 1) {
					id_offset++;
					continue;
				}
				std::vector<Node> component_nodes;
				for (int j = 0; j < sz; ++j) component_nodes.push_back(pool[(int)id_offset - 1 + j]);
				
				for (int idx = 1; idx < sz; idx++) {
					int p_idx = rng.randint(0, idx - 1);
					all_edges.push_back({ component_nodes[idx], component_nodes[p_idx] });
				}
				id_offset += sz;
			}
			return all_edges;
		}
		
		std::vector<Edge> grid(int rows, int cols) {
			std::vector<Edge> edges;
			if (rows <= 0 || cols <= 0) return edges;
			for (int r = 1; r <= rows; ++r) {
				for (int c = 1; c <= cols; ++c) {
					Node id = (Node)((r - 1) * cols + c);
					if (c < cols) edges.push_back({ id, id + 1 });
					if (r < rows) edges.push_back({ id, id + (Node)cols });
				}
			}
			return edges;
		}
		
		std::vector<Edge> unicyclic(Node n) {
			if (n < 3) return randomTree(n);
			std::vector<Edge> edges = randomTree(n);
			std::set<Edge> edge_set(edges.begin(), edges.end());
			while (true) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v) continue;
				Edge e{ (Node)u, (Node)v };
				if (u > v) std::swap(e.first, e.second);
				if (edge_set.find(e) == edge_set.end()) {
					edges.push_back(e);
					break;
				}
			}
			return edges;
		}
		
		std::vector<Edge> strictConnected(Node n, int m) {
			if (n <= 0) return {};
			int min_edges = std::max(0, (int)n - 1);
			int max_edges = ((int)n) * ((int)n - 1) / 2;
			if (m < min_edges || m > max_edges) return {};
			std::vector<Edge> edges = randomTree(n);
			std::set<Edge> edge_set(edges.begin(), edges.end());
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v) continue;
				if (u > v) std::swap(u, v);
				Edge e{ (Node)u, (Node)v };
				if (edge_set.find(e) == edge_set.end()) {
					edges.push_back(e);
					edge_set.insert(e);
				}
			}
			return edges;
		}
		
		std::vector<Edge> graphEdges(Node n, int m) {
			if (n <= 0) return {};
			int max_edges = ((int)n) * ((int)n - 1) / 2;
			if (m < 0 || m > max_edges) return {};
			std::vector<Edge> edges;
			std::set<Edge> edge_set;
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v) continue;
				if (u > v) std::swap(u, v);
				Edge e{ (Node)u, (Node)v };
				if (edge_set.find(e) == edge_set.end()) {
					edges.push_back(e);
					edge_set.insert(e);
				}
			}
			return edges;
		}
		
		std::vector<Edge> strictLayered(Node n, int m, int k) {
			if (n <= 0 || k <= 1) return {};
			std::vector<int> layer((int)n + 1);
			for (int i = 1; i <= (int)n; ++i) layer[i] = rng.randint(0, k - 1);
			std::vector<int> count_in_layer(k, 0);
			for (int i = 1; i <= (int)n; ++i) count_in_layer[layer[i]]++;
			int max_possible = 0;
			for (int i = 0; i < k; ++i) {
				for (int j = i + 1; j < k; ++j) max_possible += count_in_layer[i] * count_in_layer[j];
			}
			if (m > max_possible) return graphEdges(n, m);
			std::vector<Edge> edges;
			std::set<Edge> seen;
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v || layer[u] >= layer[v]) continue;
				Edge e{ (Node)u, (Node)v };
				if (u > v) std::swap(e.first, e.second);
				if (seen.find(e) == seen.end()) {
					edges.push_back(e);
					seen.insert(e);
				}
			}
			return edges;
		}
		
		std::vector<Edge> degreeConstrained(Node n, double prob, int max_deg) {
			if (n <= 0 || max_deg <= 0) return {};
			std::vector<int> deg((int)n + 1, 0);
			std::vector<Edge> edges;
			for (Node u = 1; u <= n; ++u) {
				for (Node v = u + 1; v <= n; ++v) {
					if (rng.random(0.0, 1.0) < prob) {
						if (deg[(int)u] < max_deg && deg[(int)v] < max_deg) {
							edges.push_back({ u, v });
							deg[(int)u]++;
							deg[(int)v]++;
						}
					}
				}
			}
			return edges;
		}
		
		std::vector<CostFlowEdge> assignWeights(const std::vector<Edge>& edges, Weight min_w, Weight max_w) {
			std::vector<CostFlowEdge> res;
			for (auto& e : edges) {
				Weight w = (Weight)rng.randint((int)min_w, (int)max_w);
				res.push_back({ e.first, e.second, Weight(1), w });
			}
			return res;
		}
		
		std::vector<CostFlowEdge> metricWeights(Node n) {
			struct Point { int x, y; };
			std::vector<Point> pts((int)n + 1);
			for (int i = 1; i <= (int)n; ++i) {
				pts[i] = { rng.randint(0, 10000), rng.randint(0, 10000) };
			}
			std::vector<CostFlowEdge> res;
			for (Node u = 1; u <= n; ++u) {
				for (Node v = u + 1; v <= n; ++v) {
					int dx = pts[(int)u].x - pts[(int)v].x;
					int dy = pts[(int)u].y - pts[(int)v].y;
					Weight dist = (Weight)(dx * dx + dy * dy);
					res.push_back({ u, v, Weight(1), dist });
					res.push_back({ v, u, Weight(1), dist });
				}
			}
			return res;
		}
		
		std::vector<CostFlowEdge> graphWithNegativeCycle(Node n, int m) {
			if (n < 3) return {};
			std::vector<CostFlowEdge> edges;
			edges.push_back({ Node(1), Node(2), Weight(1), Weight(-10) });
			edges.push_back({ Node(2), Node(3), Weight(1), Weight(-10) });
			edges.push_back({ Node(3), Node(1), Weight(1), Weight(15) });
			std::set<std::pair<Node, Node>> used;
			used.insert({ Node(1), Node(2) }); used.insert({ Node(2), Node(3) }); used.insert({ Node(3), Node(1) });
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v) continue;
				if (used.count({ Node(u), Node(v) })) continue;
				Weight w = (Weight)rng.randint(1, 100);
				edges.push_back({ Node(u), Node(v), Weight(1), w });
				used.insert({ Node(u), Node(v) });
			}
			return edges;
		}
		
		bool checkConnectivity(Node n, const std::vector<Edge>& edges) {
			if (n <= 0) return true;
			std::vector<std::vector<Node>> adj((int)n + 1);
			for (auto& e : edges) {
				if (e.first <= n && e.second <= n) {
					adj[(int)e.first].push_back(e.second);
					adj[(int)e.second].push_back(e.first);
				}
			}
			std::vector<bool> visited((int)n + 1, false);
			std::queue<Node> q;
			Node start = 0;
			for (Node i = 1; i <= n; ++i) {
				bool has_edge = false;
				for (auto& e : edges) {
					if (e.first == i || e.second == i) {
						has_edge = true; break;
					}
				}
				if (has_edge || n == 1) { start = i; break; }
			}
			if (start == 0) return false;
			q.push(start); visited[(int)start] = true;
			while (!q.empty()) {
				Node u = q.front(); q.pop();
				for (Node v : adj[(int)u]) {
					if (!visited[(int)v]) {
						visited[(int)v] = true;
						q.push(v);
					}
				}
			}
			for (Node i = 1; i <= n; ++i) {
				if (!visited[(int)i]) return false;
			}
			return true;
		}
		
		bool checkNoMultiEdges(Node n, const std::vector<Edge>& edges) {
			std::set<Edge> seen;
			for (auto e : edges) {
				Edge normal = e;
				if (normal.first > normal.second) std::swap(normal.first, normal.second);
				if (seen.count(normal)) return false;
				seen.insert(normal);
			}
			return true;
		}
		
		bool checkDAG(Node n, const std::vector<Edge>& edges) {
			std::vector<std::vector<Node>> adj((int)n + 1);
			std::vector<int> in_degree((int)n + 1, 0);
			for (const auto& e : edges) {
				if (e.first <= n && e.second <= n) {
					adj[(int)e.first].push_back(e.second);
					in_degree[(int)e.second]++;
				}
			}
			std::queue<int> q;
			int visited_count = 0;
			for (int i = 1; i <= (int)n; ++i) if (in_degree[i] == 0) q.push(i);
			while (!q.empty()) {
				int u = q.front(); q.pop();
				visited_count++;
				for (Node v : adj[u]) {
					if (--in_degree[(int)v] == 0) q.push((int)v);
				}
			}
			return visited_count == (int)n;
		}
		
		std::vector<Edge> randomTree(Node id_limit, Node count) {
			if (count <= 0 || id_limit < count) return {};
			std::vector<Node> nodes = selectNodes(id_limit, count);
			std::vector<Edge> edges;
			edges.reserve((int)count - 1);
			for (int idx = 1; idx < (int)count; idx++) {
				int p_idx = rng.randint(0, idx - 1);
				edges.push_back({ nodes[idx], nodes[p_idx] });
			}
			return edges;
		}
		
		std::vector<Edge> nAryTree(Node id_limit, Node count, int k) {
			if (count <= 0 || id_limit < count || k < 1) return {};
			if (count == 1) return {};
			std::vector<Node> nodes = selectNodes(id_limit, count);
			std::vector<Edge> edges;
			edges.reserve((int)count - 1);
			std::vector<int> available_parents = { 0 };
			std::map<int, int> child_count;
			for (int i = 1; i < (int)count; i++) {
				if (available_parents.empty()) {
					edges.push_back({ nodes[i - 1], nodes[i] });
					continue;
				}
				int idx = rng.randint(0, (int)available_parents.size() - 1);
				int p_idx = available_parents[idx];
				edges.push_back({ nodes[p_idx], nodes[i] });
				child_count[p_idx]++;
				if (child_count[p_idx] >= k) {
					available_parents[idx] = available_parents.back();
					available_parents.pop_back();
				}
				available_parents.push_back(i);
			}
			return edges;
		}
		
		std::vector<Edge> randomGraph(Node id_limit, Node count, double prob) {
			if (count <= 0 || id_limit < count) return {};
			std::vector<Node> nodes = selectNodes(id_limit, count);
			std::vector<Edge> edges;
			for (int u = 0; u < (int)count; u++) {
				for (int v = u + 1; v < (int)count; v++) {
					if (rng.random(0.0, 1.0) < prob) {
						edges.push_back({ nodes[u], nodes[v] });
					}
				}
			}
			return edges;
		}
		
		std::vector<Edge> randomTree(Node n) {
			if (n <= 0) return {};
			std::vector<Edge> edges;
			edges.reserve((int)n - 1);
			for (Node i = 2; i <= n; i++) {
				Node p = (Node)rng.randint(1, (int)(i - 1));
				edges.push_back({ p, i });
			}
			return edges;
		}
		
		std::vector<Edge> nAryTree(Node n, int k) {
			if (n <= 0 || k < 1) return {};
			if (n == 1) return {};
			std::vector<Edge> edges;
			edges.reserve((int)n - 1);
			std::vector<Node> available_parents = { Node(1) };
			std::map<Node, int> child_count;
			for (Node i = 2; i <= n; i++) {
				if (available_parents.empty()) {
					edges.push_back({ i - Node(1), i });
					continue;
				}
				int idx = rng.randint(0, (int)available_parents.size() - 1);
				Node p = available_parents[idx];
				edges.push_back({ p, i });
				child_count[p]++;
				if (child_count[p] >= k) {
					available_parents[idx] = available_parents.back();
					available_parents.pop_back();
				}
				available_parents.push_back(i);
			}
			return edges;
		}
		
		std::vector<Edge> completeKAryTree(Node n, int k) {
			if (n <= 0 || k < 1) return {};
			std::vector<Edge> edges;
			edges.reserve((int)n - 1);
			for (Node i = 1; i <= n; i++) {
				Node start = (i - Node(1)) * Node(k) + Node(2);
				if (start > n) break;
				Node end = start + Node(k) - Node(1);
				if (end > n) end = n;
				for (Node child = start; child <= end; ++child) {
					edges.push_back({ i, child });
				}
			}
			return edges;
		}
		
		std::vector<Edge> randomChain(Node n) {
			std::vector<Edge> edges;
			for (Node i = 1; i < n; i++) edges.push_back({ i, i + 1 });
			return edges;
		}
		
		std::vector<Edge> randomCycle(Node n) {
			if (n < 2) return {};
			std::vector<Edge> edges = randomChain(n);
			edges.push_back({ n, Node(1) });
			return edges;
		}
		
		std::vector<Edge> randomStar(Node n) {
			std::vector<Edge> edges;
			int center = rng.randint(1, (int)n);
			for (Node i = 1; i <= n; i++) {
				if (i != center) edges.push_back({ Node(center), i });
			}
			return edges;
		}
		
		std::vector<Edge> randomGraph(Node n, double prob) {
			std::vector<Edge> edges;
			for (Node u = 1; u <= n; u++) {
				for (Node v = u + 1; v <= n; v++) {
					if (rng.random(0.0, 1.0) < prob) {
						edges.push_back({ u, v });
					}
				}
			}
			return edges;
		}
		
		std::vector<Edge> fixedDepthTree(Node n, int max_depth) {
			if (n <= 0 || max_depth <= 0) return {};
			std::vector<std::vector<Node>> layers(max_depth + 1);
			layers[0].push_back(Node(1));
			std::vector<Edge> edges;
			for (Node i = 2; i <= n; i++) {
				int d = rng.randint(0, max_depth - 1);
				while (layers[d].empty()) d = (d + 1) % max_depth;
				int sz = (int)layers[d].size();
				Node p = layers[d][rng.randint(0, sz - 1)];
				edges.push_back({ p, i });
				layers[d + 1].push_back(i);
			}
			return edges;
		}
		
		std::vector<Edge> randomCactus(Node n) {
			if (n <= 0) return {};
			std::vector<Edge> edges;
			edges.push_back({ Node(1), Node(2) });
			edges.push_back({ Node(2), Node(3) });
			edges.push_back({ Node(3), Node(1) });
			int nodes = 3;
			while (nodes < (int)n) {
				int u = rng.randint(1, nodes);
				int v = rng.randint(1, nodes);
				if (u == v) continue;
				int cycle_len = rng.randint(2, std::min(5, (int)n - nodes + 1));
				edges.push_back({ Node(u), Node(nodes + 1) });
				int last = nodes + 1;
				for (int k = 2; k < cycle_len; ++k) {
					edges.push_back({ Node(last), Node(nodes + k) });
					last = nodes + k;
				}
				edges.push_back({ Node(last), Node(v) });
				nodes += cycle_len;
			}
			return edges;
		}
		
		std::vector<Edge> randomEulerian(Node n, int m) {
			if (n < 2 || m < 0) return {};
			std::vector<int> degree((int)n + 1, 0);
			std::set<Edge> set_edges;
			std::vector<Edge> edges;
			auto add_edge = [&](Node u, Node v) {
				if (u > v) std::swap(u, v);
				if (set_edges.find({ u, v }) == set_edges.end()) {
					edges.push_back({ u, v });
					degree[(int)u]++; degree[(int)v]++;
					return true;
				}
				return false;
			};
			for (int i = 0; i < m; i++) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v) continue;
				add_edge(Node(u), Node(v));
			}
			std::vector<int> odds;
			for (int i = 1; i <= (int)n; i++) if (degree[i] % 2 == 1) odds.push_back(i);
			for (size_t i = 0; i + 1 < odds.size(); i += 2) add_edge(Node(odds[i]), Node(odds[i + 1]));
			return edges;
		}
		
		std::vector<Edge> randomBipartite(Node n, int m, double prob) {
			std::vector<Edge> edges;
			int split = (int)n / 2;
			for (Node u = 1; u <= split; u++) {
				for (Node v = split + 1; v <= n; v++) {
					if (rng.random(0.0, 1.0) < prob) edges.push_back({ u, v });
				}
			}
			return edges;
		}
		
		std::vector<Edge> randomDAG(Node n, int m) {
			if (n <= 0 || m > (int64_t)n * ((int)n - 1) / 2) return {};
			std::vector<Edge> edges;
			edges.reserve(m);
			std::set<Edge> existing_edges;
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n - 1);
				int v = rng.randint(u + 1, (int)n);
				if (existing_edges.insert({ Node(u), Node(v) }).second) edges.push_back({ Node(u), Node(v) });
			}
			return edges;
		}
		
		std::vector<Edge> randomLayered(Node n, int m, int k) {
			std::vector<int> layer((int)n + 1);
			for (int i = 1; i <= (int)n; i++) layer[i] = rng.randint(0, k - 1);
			std::vector<Edge> edges;
			while ((int)edges.size() < m) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u != v && layer[u] < layer[v]) edges.push_back({ Node(u), Node(v) });
			}
			return edges;
		}
		
		std::vector<Edge> randomChordal(Node n, double prob) {
			std::vector<Edge> edges;
			for (Node i = 1; i <= n; i++) {
				for (Node j = i + 1; j <= n; ++j) {
					if (rng.random(0.0, 1.0) < prob) edges.push_back({ i, j });
				}
				if (rng.random(0.0, 1.0) < 0.5) {
					int k = rng.randint(1, (int)(i - 1));
					edges.push_back({ Node(k), i });
				}
			}
			return edges;
		}
		
		std::vector<Edge> randomPlanar(Node n) {
			if (n < 3) return {};
			std::vector<Edge> edges = { {Node(1), Node(2)}, {Node(2), Node(3)}, {Node(3), Node(1)} };
			std::vector<std::vector<Node>> faces = { {Node(1), Node(2), Node(3)} };
			for (int i = 4; i <= (int)n; i++) {
				int f_idx = rng.randint(0, (int)faces.size() - 1);
				Node u = faces[f_idx][0], v = faces[f_idx][1], w = faces[f_idx][2];
				faces.erase(faces.begin() + f_idx);
				faces.push_back({ u, v, Node(i) }); faces.push_back({ v, w, Node(i) }); faces.push_back({ w, u, Node(i) });
				edges.push_back({ u, Node(i) }); edges.push_back({ v, Node(i) }); edges.push_back({ w, Node(i) });
			}
			return edges;
		}
		
		std::vector<FlowEdge> randomFlowNetwork(Node n, int m, Node s, Node t, Weight max_cap) {
			std::vector<FlowEdge> edges;
			std::set<std::pair<Node, Node>> existing;
			for (int i = 0; i < m; i++) {
				int u = rng.randint(1, (int)n);
				int v = rng.randint(1, (int)n);
				if (u == v || u == (int)s || v == (int)t) continue;
				if (existing.count({ Node(u), Node(v) })) continue;
				int cap_int = rng.randint(1, (int)max_cap);
				edges.push_back({ Node(u), Node(v), (Weight)cap_int });
				existing.insert({ Node(u), Node(v) });
			}
			return edges;
		}
		
		std::vector<Node> topologicalSort(Node n, const std::vector<Edge>& edges) {
			std::vector<std::vector<Node>> adj((int)n + 1);
			std::vector<int> in_degree((int)n + 1, 0);
			for (const auto& e : edges) {
				adj[(int)e.first].push_back(e.second);
				in_degree[(int)e.second]++;
			}
			std::vector<Node> result;
			for (int i = 1; i <= (int)n; i++) if (in_degree[i] == 0) result.push_back(Node(i));
			for (size_t idx = 0; idx < result.size(); ++idx) {
				Node u = result[idx];
				for (Node v : adj[(int)u]) if (--in_degree[(int)v] == 0) result.push_back(v);
			}
			return result;
		}
		
		std::vector<Edge> randomSteinerTreeGraph(Node n, int k) {
			if (n < k || k < 1) return {};
			std::set<Node> terminals;
			std::vector<Node> all_nodes;
			for (Node i = 1; i <= n; i++) all_nodes.push_back(i);
			std::vector<Node> t_vec = rng.sample(all_nodes, k);
			for (Node t : t_vec) terminals.insert(t);
			std::vector<Edge> edges;
			std::set<Node> connected;
			connected.insert(*terminals.begin());
			auto get_random_unused = [&]() -> Node {
				std::vector<Node> unused;
				for (Node i = 1; i <= n; i++) if (connected.find(i) == connected.end()) unused.push_back(i);
				if (unused.empty()) return Node(0);
				return unused[rng.randint(0, (int)unused.size() - 1)];
			};
			for (size_t i = 1; i < t_vec.size(); i++) {
				Node start = *connected.rbegin();
				Node target = t_vec[i];
				if (connected.count(target)) continue;
				Node current = start;
				while (current != target) {
					if (connected.count(target) || rng.random(0.0, 1.0) < 0.3) {
						edges.push_back({ current, target });
						connected.insert(target);
						break;
					}
					else {
						Node steiner = get_random_unused();
						if (steiner == Node(0)) {
							edges.push_back({ current, target });
							connected.insert(target);
							break;
						}
						edges.push_back({ current, steiner });
						connected.insert(steiner);
						current = steiner;
					}
				}
			}
			std::vector<Node> unused;
			for (Node i = 1; i <= n; i++) if (connected.find(i) == connected.end()) unused.push_back(i);
			for (Node u : unused) {
				int idx = rng.randint(0, (int)connected.size() - 1);
				auto it = connected.begin();
				std::advance(it, idx);
				Node v = *it;
				edges.push_back({ u, v });
				connected.insert(u);
			}
			return edges;
		}
		
		std::vector<Edge> randomHamiltonianGraph(Node n, double prob) {
			if (n < 3) return randomCycle(n);
			std::vector<Node> perm(n);
			for (int i = 0; i < n; i++) perm[i] = Node(i + 1);
			rng.shuffle(perm);
			std::vector<Edge> edges;
			for (int i = 0; i < n; i++) {
				edges.push_back({ perm[i], perm[(i + 1) % n] });
			}
			for (Node u = 1; u <= n; u++) {
				for (Node v = u + 1; v <= n; v++) {
					if (rng.random(0.0, 1.0) < prob) edges.push_back({ u, v });
				}
			}
			return edges;
		}
	};
	
}

#endif

