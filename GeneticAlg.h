#pragma once
#include "Population.h"
#include <vector>
#include <climits>
#include <string>
class GeneticAlg
{
private:
	int n_population = 200;
	int n_generations = 500;

	int bad = 10; //number of tolerated equal best scores
	int boosted_gens = 10;
	
	int number_of_elites = 4;
	
	bool boost_mutation = false;
	double p_mut = 0.001;
	double boosted_mut = 0.01;
	double overall_best = DBL_MAX;

public:
	Population pop;
	vector<int> best_route;

	void Configure(int pop_size, int gens)
	{
		n_population = pop_size;
		n_generations = gens;
	}

	void SetDistanceType(DistanceType type) 
	{
		pop.SetDistanceType(type);
	}

	GeneticAlg() = default;
	void GA_Iteration(const int& idx_gen);
	double RunGA(const string& filename);
};