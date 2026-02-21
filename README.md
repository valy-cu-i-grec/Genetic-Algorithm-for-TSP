===== Hybrid Genetic Algorithm for Large-Scale TSP
A high-performance C++ implementation designed to solve the Traveling Salesman Problem (TSP) using a Hybrid Genetic Algorithm (Memetic Algorithm). This project is optimized for handling massive datasets from TSPLIB, utilizing spatial indexing and local search heuristics to achieve near-optimal solutions efficiently.

===== Key Technical Features
Memetic Algorithm Approach: Combines global exploration (Genetic Algorithm) with local exploitation (2-Opt) to prevent premature convergence.
Spatial Optimization (KD-Trees): Uses the nanoflann library to implement a KD-Tree for candidate list generation, reducing the search space for local search to the 20 nearest neighbors.
Post-Optimization (Simulated Annealing): A final Simulated Annealing phase is applied to the best-found tour to escape local optima and refine the results.
Advanced GA Operators:
Crossover: Order Crossover (OX) for preserving relative ordering.
Mutation: Inversion mutation with dynamic probability adjustment.
Selection: Fortune Wheel (Roulette) selection combined with strict Elitism.
Concurrency: High-performance batch processing using Multithreading (std::async, std::future) to solve multiple instances in parallel.
Distance Metrics: Supports EUC_2D (Euclidean), CEIL_2D, and GEO (Geographic/Haversine) distances.

===== Performance Benchmarks
Results obtained after multiple runs on standard TSPLIB instances. The Gap (%) represents the deviation from the known optimal solution.

<img width="665" height="287" alt="image" src="https://github.com/user-attachments/assets/db3f6ed0-51bf-43b1-a8b5-bf5e43314e19" />


===== Project Structure
H3_Main.cpp: Entry point, handles multithreaded execution and batch testing.
Population.cpp/h: Core logic for GA operators (Crossover, Mutation, Selection, 2-Opt).
GeneticAlg.cpp/h: Controller class for the optimization process and SA.
KDTreeAdapter.h: Integration with nanoflann for spatial queries.
DataCities.h & Utils.h: Data structures and distance calculation formulas.

===== Requirements & Building
Compiler: C++17 compatible (GCC 7+, MSVC 2019+).
Dependencies: nanoflann.hpp (header-only, included).
