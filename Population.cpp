#include "Population.h"
#include <algorithm>
#include <ctime>
#include <random>
#include <cmath>
#include <numeric>
#include <iostream>
using namespace std;

# define M_PI 3.14159265358979323846264


Population::Population()
{
	std::random_device rd;
	gen.seed(rd());
}

void Population::Load_Data(const vector<City>& raw_cities)
{
	this->nodes = raw_cities;

	this->curr_score = DBL_MAX;
	this->last_score = DBL_MAX;
}

void Population::Init_Pop(const int& pop_size)
{
	curr_pop.clear();
	curr_pop.reserve(pop_size);

	vector<int> base_indices(nodes.size());
	iota(base_indices.begin(), base_indices.end(), 0);

	for (int i = 0; i < pop_size; i++)
	{
		vector<int> individual = base_indices;
		shuffle(individual.begin(), individual.end(),this->gen);
		curr_pop.emplace_back(individual);
	}
}


void Population::SetNeighbours(const vector<vector<int>>& neighbors)
{
	this->neighbor_list = neighbors;
}

void Population::Print_pop()
{
	for (const auto& i : curr_pop)
	{
		for (const auto& j : i)
		{
			cout << j << ' ';
		}
		cout << '\n';
	}
	cout << '\n';
}

void Population::fitness(const vector<int>& chromosome, double& fit)
{
	fit = 0.0;

	if (chromosome.empty()) return;

	for (int i = 0; i < chromosome.size() - 1; i++)
	{
		fit += dist(chromosome[i], chromosome[i + 1]);
	}
	fit += dist(chromosome[chromosome.size() - 1], chromosome[0]);
}
void Population::pop_fitness(const vector<vector<int>>& pop, vector<double>& fitness_pop)
{
	fitness_pop.clear();
	fitness_pop.reserve(pop.size());
	for (const auto& chromo : pop)
	{
		double fit;
		fitness(chromo, fit);
		fitness_pop.emplace_back(fit);
	}
}


void Population::Elitism(const int& n_elites, const vector<double>& fit_vec, vector<int>& indices)
{
	partial_sort(indices.begin(), indices.begin() + n_elites, indices.end(),
		[&fit_vec](int i, int j)
		{
			return fit_vec[i] < fit_vec[j];
		});

	for (int i = 0; i < n_elites; i++)
	{
		int original_idx = indices[i];

		vector<int> candidate = curr_pop[original_idx];

		TwoOpt_Candidate(candidate);

		double original_score = fit_vec[original_idx];
		double new_score;
		fitness(candidate, new_score);


		if (new_score < original_score + 1e-6)
		{
			parents.emplace_back(candidate);
		}
		else
		{
			parents.emplace_back(curr_pop[original_idx]);
			cout << " [Safety Net Triggered: 2-Opt worsened score] ";
		}
	}
}

void Population::Fortune_Wheel(const int& n_elites, const int& bad, bool& boosted, const int& number_gens)
{
	vector<double> prob, fitness, cummulative;
	double rev_fitness_sum = 0.0;

	vector<int> indices(curr_pop.size());
	iota(indices.begin(), indices.end(), 0);

	pop_fitness(curr_pop, fitness);

	parents.clear();
	int n = curr_pop.size();
	parents.reserve(n);

	Elitism(n_elites, fitness, indices);

	last_score = curr_score;
	best.assign(parents[0].begin(), parents[0].end());// best in population is the best elite
	curr_score = fitness[indices[0]];

	Verify_Stagnation(boosted, number_gens, bad);// verify stagnation between current gen and last gen

	for (int i = 0; i < indices.size(); i++) //elites can also participate in the crossover
	{
		rev_fitness_sum += (1.0 / fitness[indices[i]]);
	}

	for (int i = 0; i < indices.size(); i++)
	{
		prob.emplace_back((1.0 / fitness[indices[i]]) / rev_fitness_sum);
	}

	cummulative.reserve(prob.size());
	cummulative.emplace_back(prob[0]);
	for (int i = 1; i < prob.size(); i++)
	{
		cummulative.emplace_back(prob[i] + cummulative[i - 1]);
	}

	uniform_real_distribution<> dist(0.0, 1.0);

	while (parents.size() < n)
	{
		double p = dist(this->gen);
		int idx = lower_bound(cummulative.begin(), cummulative.end(), p) - cummulative.begin();
		parents.emplace_back(curr_pop[indices[idx]]);
	}
}

void Population::Swap(vector<int>& chromo)
{
	uniform_int_distribution<> dist(0, chromo.size() - 1);
	int i = dist(this->gen);
	int j = dist(this->gen);
	while (i == j)
		j = dist(this->gen);
	swap(chromo[i], chromo[j]);


}

void Population::Mutate(const double& prob)
{
	uniform_real_distribution<> dist(0.0, 1.0);
	for (auto& chromo : curr_pop)
	{
		double p = dist(this->gen);
		if (p < prob)
		{
			Swap(chromo);
		}
	}
}
void Population::Invert(vector<int>& chromo)
{
	uniform_int_distribution<> dist(0, chromo.size() - 1);
	int i = dist(this->gen);
	int j = dist(this->gen);
	while (i == j)
		j = dist(this->gen);

	if (i > j)
		swap(i, j);

	reverse(chromo.begin() + i, chromo.begin() + j + 1);
}

void Population::Inversion_Mutation(const double& prob, const int& n_elites)
{
	uniform_real_distribution<> dist(0.0, 1.0);
	for (int i = n_elites; i < curr_pop.size(); i++)
	{
		double p = dist(this->gen);
		if (p < prob)
		{
			Invert(curr_pop[i]);
		}
	}
}

void Population::OX(const vector<int>& parent1, const vector<int>& parent2, vector<int>& child)
{
	int size = static_cast<int>(parent1.size());

	if (static_cast<int>(child.size()) != size) child.assign(size, -1);
	else fill(child.begin(), child.end(), -1);

	uniform_int_distribution<> dist(0, size - 1);
	int id1 = dist(this->gen);
	int id2 = dist(this->gen);

	while (id1 == id2) id2 = dist(this->gen);
	if (id1 > id2) swap(id1, id2);

	vector<bool> visited(size + 1, false);

	for (int i = id1; i <= id2; i++)
	{
		int gene = parent1[i];
		child[i] = gene;
		visited[gene] = true;
	}

	int current_p2_idx = (id2 + 1) % size;
	int current_child_idx = (id2 + 1) % size;

	for (int i = 0; i < size; i++)
	{
		int gene = parent2[current_p2_idx];

		if (!visited[gene])
		{
			child[current_child_idx] = gene;
			visited[gene] = true;

			current_child_idx = (current_child_idx + 1) % size;
		}

		current_p2_idx = (current_p2_idx + 1) % size;
	}
}

void Population::TwoOpt_Candidate(vector<int>& chromosome)
{
	int size = (int)chromosome.size();
	bool improved = true;
	int passes = 0;

	vector<int> positions(size);
	for (int i = 0; i < size; i++) positions[chromosome[i]] = i;

	if (neighbor_list.empty()) return;

	while (improved && passes < 10)
	{
		improved = false;
		passes++;

		for (int i = 0; i < size; i++)
		{
			int u = chromosome[i];
			int u_next = chromosome[(i + 1) % size]; 

			for (const int& v : neighbor_list[u])
			{
				if (v == u_next || v == chromosome[(i - 1 + size) % size]) continue; 

				int j = positions[v]; 
				int v_next = chromosome[(j + 1) % size];

				double curr_dist = dist(u, u_next) + dist(v, v_next);

				double new_dist = dist(u, v) + dist(u_next, v_next);

				if (new_dist < curr_dist - 1e-6)
				{
					
					int idx_u_next = (i + 1) % size;
					int idx_v = j;

					if (idx_u_next < idx_v)
					{
						reverse(chromosome.begin() + idx_u_next, chromosome.begin() + idx_v + 1);

						for (int k = idx_u_next; k <= idx_v; k++) {
							positions[chromosome[k]] = k;
						}
					}
					else
					{
						

						int idx_v_next = (j + 1) % size;
						int idx_u = i;

						if (idx_v_next < idx_u)
						{
							reverse(chromosome.begin() + idx_v_next, chromosome.begin() + idx_u + 1);
							for (int k = idx_v_next; k <= idx_u; k++) {
								positions[chromosome[k]] = k;
							}
						}
						else
						{
							
							continue;
						}
					}

					improved = true;
					
				}
			}
		}
	}
}

void Population::Crossover(const double& prob_mut, const int& n_elites, const double& boosted_prob, const bool& boosted, const int& idx_gen, const int& n_generations)
{
	const double START_P = 0.05;
	const double END_P = 0.40;

	double ratio = (double)idx_gen / (double)n_generations;
	double p_dynamic = START_P + (END_P - START_P) * ratio;

	// If boosted, force 50% probability for 2-opt algorithm
	if (boosted)
	{
		p_dynamic = 0.50;
	}

	//2-point OX crossover 
	curr_pop.clear();

	if (parents.empty()) return;

	int size = parents[0].size();

	for (int i = 0; i < n_elites; i++) //add the elites in the population
	{
		curr_pop.emplace_back(parents[i]);
	}

	int i;
	vector<int> child1(size);
	vector<int> child2(size);
	for (i = n_elites; i + 1 < parents.size(); i += 2)
	{
		OX(parents[i], parents[i + 1], child1);
		curr_pop.emplace_back(child1);

		OX(parents[i + 1], parents[i], child2);
		curr_pop.emplace_back(child2);
	}


	if (i < parents.size())
		curr_pop.emplace_back(parents[i]);


	if (boosted)
	{
		Inversion_Mutation(boosted_prob, n_elites);
	}
	else
	{
		Inversion_Mutation(prob_mut, n_elites);
	}

	uniform_real_distribution<> real(0.0, 1.0);
	for (int i = n_elites; i < curr_pop.size(); i++)
	{
		double prob = real(this->gen);

		if (prob < p_dynamic)
		{
			TwoOpt_Candidate(curr_pop[i]);
		}
	}

}

void Population::Verify_Stagnation(bool& boosted, const int& number_gens, const int& bad)
{
	if (boosted)
	{
		gens--;


		if (gens <= 0)
		{
			boosted = false;
			eq_gen = 0;
		}
		return;
	}

	if (abs(curr_score - last_score) < 1e-6)
	{
		eq_gen++;
	}
	else
	{
		eq_gen = 0;
	}

	if (eq_gen >= bad)
	{
		boosted = true;
		gens = number_gens;
		eq_gen = 0;
	}
}
void Population::PrintTourDetails(const vector<int>& tour)
{
	double total_dist = 0;
	cout << "\n--- DETALII TUR ---\n";

	for (size_t i = 0; i < tour.size() - 1; i++)
	{
		int u = tour[i];
		int v = tour[i + 1];

		double d = dist(u, v);

		cout << "Pas " << i + 1 << ": " << u << " -> " << v
			<< " | Dist: " << d << endl;

		total_dist += d;
	}

	int last = tour.back();
	int first = tour.front();
	double d_last = dist(last, first);
	total_dist += d_last;

	cout << "Pas Final: " << last << " -> " << first
		<< " | Dist: " << d_last << endl;
	cout << "TOTAL VERIFICAT: " << total_dist << endl;
	cout << "-------------------\n";
}

void Population::Debug_Check_Elite(const string& stage_name)
{
	if (curr_pop.empty()) return;

	double real_dist = 0.0;
	fitness(curr_pop[0], real_dist);

	cout << "[DEBUG " << stage_name << "] Index 0 Real Score: " << (long long)real_dist;

	
	if (real_dist > curr_score + 1.0 && curr_score != DBL_MAX)
	{
		cout << " !!! ALERTA: ELITA A FOST STRICATA! (Expected: " << (long long)curr_score << ")";
	}
	cout << endl;
}

void Population::SimulatedAnnealing(vector<int>& tour,const double& start_temp,const double& cool_rate)
{
	double temp = start_temp;
	double current_dist;
	fitness(tour, current_dist); 

	double best_dist_found = current_dist;
	vector<int> best_tour_found = tour;

	int size = (int)tour.size();

	vector<int> positions(size);
	for (int k = 0; k < size; k++) positions[tour[k]] = k;

	uniform_real_distribution<> dist_real(0.0, 1.0);
	uniform_int_distribution<> dist_int(0, size - 1);

	while (temp > 1e-4) 
	{
		int steps_per_temp = size;

		for (int k = 0; k < steps_per_temp; k++)
		{
			int i = dist_int(this->gen);
			int u = tour[i];
			int u_next = tour[(i + 1) % size];

			if (neighbor_list[u].empty()) continue;

			uniform_int_distribution<> dist_neigh(0, (int)neighbor_list[u].size() - 1);
			int v = neighbor_list[u][dist_neigh(this->gen)];

			int j = positions[v];
			int v_next = tour[(j + 1) % size];

			if (u == v || u_next == v || v_next == u) continue;

			double delta = (dist(u, v) + dist(u_next, v_next)) - (dist(u, u_next) + dist(v, v_next));

			bool accept = false;

			if (delta < 0)
			{
				accept = true;
			}
			else
			{
				if (dist_real(this->gen) < exp(-delta / temp))
				{
					accept = true;
				}
			}

			if (accept)
			{
				int idx_u_next = (i + 1) % size;
				int idx_v = j;
				bool swapped = false;

				if (idx_u_next < idx_v)
				{
					reverse(tour.begin() + idx_u_next, tour.begin() + idx_v + 1);
					for (int k = idx_u_next; k <= idx_v; k++) positions[tour[k]] = k;
					current_dist += delta;
					swapped = true;
				}
				else
				{
					int idx_v_next = (j + 1) % size;
					int idx_u = i;
					if (idx_v_next < idx_u)
					{
						reverse(tour.begin() + idx_v_next, tour.begin() + idx_u + 1);
						for (int k = idx_v_next; k <= idx_u; k++) positions[tour[k]] = k;
						current_dist += delta;
						swapped = true;
					}
				}

				if (swapped && current_dist < best_dist_found - 1e-6)
				{
					best_dist_found = current_dist;
					best_tour_found = tour; 
				}
			}
		}

		temp *= cool_rate;
	}

	tour = best_tour_found;
}