#pragma once
#include "Utils.h" 
#include <iostream>
#include <vector>
#include <random>
#include <cfloat> 
#include <algorithm>

using std::vector;
using std::pair;

struct CityNode {
	double x, y;
	int id;
};


class Population
{
private:
	std::mt19937 gen;//Marsenne twister
	void Swap(vector<int>& chromo);
	void Invert(vector<int>& chromo);
	void OX(const vector<int>& parent1, const vector<int>& parent2, vector<int>& child);

public:
	DistanceType distType = EUC_2D; 
	void SetDistanceType(DistanceType type) {
		this->distType = type;
	}
	vector<vector<int>> curr_pop; //the whole current population
	vector<vector<int>> parents; //part of the population that are chosen as parent for the next gen;
	
	double curr_score = DBL_MAX; //best fitness;
	double last_score = DBL_MAX; //best fitness for last gen
	
	vector<int> best; //candidate with the best fitness
	int eq_gen = 0; // counter for equal best_scores between gens
	int gens; //gens left for boosting

	vector<City> nodes;

	vector<vector<int>> neighbor_list; 
	//neighbor_list[i] = list of the 20 most close neighbours of city i

	Population();
	
	void Load_Data(const vector<City>& raw_cities);
	void SetNeighbours(const vector<vector<int>>& neighbors);

	void Print_pop();

	void Init_Pop(const int& pop_size); //randomly generates the whole population
	
	inline double dist(const int& i, const int& j) const
	{
		if (i<0 || i>nodes.size() || j < 0 || j >= nodes.size()) return DBL_MAX;

		switch (distType) 
		{
		case CEIL_2D: return GetDistanceCeil2D(nodes[i], nodes[j]);
		case GEO:     return GetDistanceGeo(nodes[i], nodes[j]);
		default:      return GetDistanceEuclidean(nodes[i], nodes[j]);
		}
	}
	
	void fitness(const vector<int>& chromosome, double& fit);
	void pop_fitness(const vector<vector<int>>& pop, vector<double>& fitness_pop);
	
	void Fortune_Wheel(const int& n_elites, const int& bad, bool& boosted, const int& number_gens);
	void Elitism(const int& n_elites,const vector<double>& fit, vector<int>& indices);
	
	void Mutate(const double& prob);
	void Inversion_Mutation(const double& prob, const int& n_elites);
	
	void Crossover(const double& prob_mut,const int& n_elites, const double& boosted_prob, const bool& boosted, const int& idx_gen, const int& n_generations);
	void Verify_Stagnation(bool& boosted,const int& number_gens, const int& bad);
	
	void TwoOpt_Candidate(vector<int>& chromosome);
	
	void PrintTourDetails(const vector<int>& best);

	void Debug_Check_Elite(const string& stage_name);

	void SimulatedAnnealing(vector<int>& tour, const double& start_temp, const double& cooling_rate);
};
