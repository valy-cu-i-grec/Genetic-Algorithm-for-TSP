#include "Population.h"
#include "GeneticAlg.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <fstream>
#include <future>
#include <iomanip>
#include <filesystem> 
#include <algorithm> 
#include <random>    
#include <map>
#include <cmath>

using namespace std;
namespace fs = std::filesystem;

std::mutex print_mutex;
std::mutex file_mutex;

struct InstanceConfig {
    string filename;
    int runs;
    int population;
    int generations;
    DistanceType distType;
};

struct ResultData
{
    string filename;
    double score;
    long long duration_ms;
    int num_cities;
};


struct Stats {
    double best;
    double worst;
    double mean;
    double median;
    double stdev;
};

Stats CalculateStats(vector<double>& values) {
    if (values.empty()) return { 0,0,0,0,0 };

    std::sort(values.begin(), values.end());

    Stats s;
    s.best = values.front();
    s.worst = values.back();

    double sum = 0.0;
    for (double v : values) sum += v;
    s.mean = sum / values.size();

    if (values.size() % 2 == 0) {
        s.median = (values[values.size() / 2 - 1] + values[values.size() / 2]) / 2.0;
    }
    else {
        s.median = values[values.size() / 2];
    }

    double sq_sum = 0.0;
    for (double v : values) {
        sq_sum += (v - s.mean) * (v - s.mean);
    }
    s.stdev = std::sqrt(sq_sum / values.size());

    return s;
}

ResultData SolveInstance(InstanceConfig config)
{
    GeneticAlg GA;
    GA.Configure(config.population, config.generations);
    GA.SetDistanceType(config.distType);

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        cout << "[Thread " << std::this_thread::get_id() << "] Start processing: " << config.filename << "\n";
    }

    auto start = chrono::high_resolution_clock::now();

    double best_score = GA.RunGA(config.filename);

    auto end = chrono::high_resolution_clock::now();
    long long duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    {
        string tour_filename = "Results/Tour_" + fs::path(config.filename).stem().string() +
            "_" + to_string((long long)best_score) + ".txt";
        ofstream fout(tour_filename);
        if (fout.is_open()) {
            fout << best_score << "\n";
            for (int city_idx : GA.pop.best)
            {
                fout << city_idx << " ";
            }
            fout.close();
        }
    }

    ResultData res;
    res.filename = config.filename;
    res.score = best_score;
    res.duration_ms = duration;
    res.num_cities = (int)GA.pop.nodes.size();

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        cout << "[Thread " << std::this_thread::get_id() << "] Finished " << config.filename << " | Score: " << best_score << "\n";
    }

    return res;
}

int main()
{
    if (!fs::exists("Results"))
    {
        fs::create_directory("Results");
    }

    vector<InstanceConfig> configs =
    {
        { "Input_d493.txt",      6, 100, 200, EUC_2D },
        { "Input_Uruguay.txt",   8, 100, 200, EUC_2D },

        { "Input_u1817.txt",     9, 100, 500, EUC_2D },
        { "Input_u2319.txt",     9, 100, 500, EUC_2D },

        { "Input_brd14051.txt",  5, 200, 700, EUC_2D },
        { "Input_d18512.txt",    5, 200, 700, EUC_2D }, 
        { "Input_SW24978.txt",   5, 200, 700, EUC_2D},

        { "Input_pla33810.txt",  5, 100, 500, CEIL_2D },
        { "Input_pla85900.txt",  5, 100, 500, CEIL_2D},

        { "Input_usa13509.txt",  5, 200, 500, EUC_2D },
        { "Input_dsj1000.txt",   5, 200, 1000, CEIL_2D },
    };

    vector<InstanceConfig> full_run_queue;

    int total_runs = 0;
    for (const auto& cfg : configs) total_runs += cfg.runs;
    full_run_queue.reserve(total_runs);

    for (const auto& cfg : configs) {
        for (int i = 0; i < cfg.runs; i++) {
            full_run_queue.push_back(cfg);
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(full_run_queue.begin(), full_run_queue.end(), g);

    ofstream summary_file("Results/summary.csv", ios::app);
    if (!summary_file.is_open())
    {
        cerr << "Nu pot deschide summary.csv\n";
        return -1;
    }
    summary_file.seekp(0, ios::end);
    if (summary_file.tellp() == 0)
    {
        summary_file << "Filename,Cities,Best_Score,Duration_ms\n";
    }


    std::map<string, vector<double>> instance_results;

    int max_concurrent_threads = 4;

    vector<future<ResultData>> futures;
    int current_run_idx = 0;

    while (current_run_idx < full_run_queue.size() || !futures.empty())
    {
        while (futures.size() < max_concurrent_threads && current_run_idx < full_run_queue.size()) {

            InstanceConfig current_job = full_run_queue[current_run_idx];

            futures.push_back(std::async(std::launch::async, SolveInstance, current_job));

            current_run_idx++;
        }

        auto it = futures.begin();
        while (it != futures.end())
        {
            if (it->wait_for(chrono::milliseconds(10)) == future_status::ready)
            {
                ResultData res = it->get();
                instance_results[res.filename].push_back(res.score);

                {
                    std::lock_guard<std::mutex> lock(file_mutex);
                    summary_file << res.filename << ","
                        << res.num_cities << ","
                        << fixed << setprecision(2) << res.score << ","
                        << res.duration_ms << "\n";
                    summary_file.flush();
                }

                it = futures.erase(it);
            }
            else
            {
                ++it;
            }
        }

        std::this_thread::sleep_for(chrono::milliseconds(100));
    }

    summary_file.close();

    cout << "Toate instantele au fost procesate.\n";
    return 0;
}