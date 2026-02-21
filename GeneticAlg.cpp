#include "GeneticAlg.h"
#include "nanoflann.hpp"
#include "KDTreeAdapter.h"
#include <iostream>
#include <numeric>
#include <chrono>
using namespace std;

void GeneticAlg::GA_Iteration(const int& idx_gen)
{
	vector<double> pop_fitness_vals;

	pop.pop_fitness(pop.curr_pop, pop_fitness_vals);
	pop.Fortune_Wheel(number_of_elites, bad, boost_mutation, boosted_gens);
	pop.Crossover(p_mut, number_of_elites, boosted_mut, boost_mutation, idx_gen, n_generations);

	pop.Debug_Check_Elite("End Iteration / Post Crossover");
}

double GeneticAlg::RunGA(const string& filename)
{
	
	vector<City> loaded_cities = Read_cities(filename);

	if (loaded_cities.empty()) {
		cerr << "Eroare: Nu s-au incarcat orase din fisierul " << filename << endl;
		return -1.0;
	}


	pop.Load_Data(loaded_cities);
	pop.Init_Pop(n_population);


	bool isGeo = (pop.distType == GEO);
	vector<vector<int>> neighbors = CandidateGenerator::BuildCandidateLists(loaded_cities, 20, isGeo);
	pop.SetNeighbours(neighbors);

	for (int i = 0; i < n_generations; i++)
	{
		auto start_time = chrono::high_resolution_clock::now();

		GA_Iteration(i);

		auto end_time = chrono::high_resolution_clock::now();

		auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

		best_route.assign(pop.best.begin(), pop.best.end());
		
		if (pop.curr_score < overall_best)
			overall_best = pop.curr_score;
	}

	pop.SimulatedAnnealing(pop.best, 100.0, 0.999);

	double final_score;
	pop.fitness(pop.best, final_score);
	cout << "Scor final dupa SA: " << final_score << endl;


	if (final_score < overall_best)
		return final_score;

	return overall_best;
}