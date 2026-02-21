#pragma once
#include "nanoflann.hpp"
#include "DataCities.h" 
#include <vector>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;
using namespace nanoflann;

struct Point2D {
	double x, y;
	int original_index;
};

struct Point3D {
	double x, y, z;
	int original_index;
};

struct PointCloud2D {
	vector<Point2D> points;

	inline size_t kdtree_get_point_count() const { return points.size(); }

	inline double kdtree_get_pt(const size_t idx, const size_t dim) const {
		if (dim == 0) return points[idx].x;
		return points[idx].y;
	}

	template <class BBOX>
	bool kdtree_get_bbox(BBOX& ) const { return false; }
};

struct PointCloud3D {
	vector<Point3D> points;

	inline size_t kdtree_get_point_count() const { return points.size(); }

	inline double kdtree_get_pt(const size_t idx, const size_t dim) const {
		if (dim == 0) return points[idx].x;
		if (dim == 1) return points[idx].y;
		return points[idx].z;
	}

	template <class BBOX>
	bool kdtree_get_bbox(BBOX& ) const { return false; }
};


inline Point3D GeoToCartesian(double lat, double lon) 
{
	double lat_rad = lat * M_PI / 180.0;
	double lon_rad = lon * M_PI / 180.0;

	Point3D p;
	p.x = std::cos(lat_rad) * std::cos(lon_rad);
	p.y = std::cos(lat_rad) * std::sin(lon_rad);
	p.z = std::sin(lat_rad);
	return p;
}

class CandidateGenerator {
public:
	static vector<vector<int>> BuildCandidateLists(
		const vector<City>& raw_cities,
		int k_neighbors = 20,
		bool isGeo = false)
	{


		if (isGeo) {
			PointCloud3D cloud;
			cloud.points.resize(raw_cities.size());

			for (size_t i = 0; i < raw_cities.size(); i++) {
				Point3D p = GeoToCartesian(raw_cities[i].lat, raw_cities[i].lon);
				p.original_index = (int)i;
				cloud.points[i] = p;
			}

			typedef KDTreeSingleIndexAdaptor<
				L2_Simple_Adaptor<double, PointCloud3D>,
				PointCloud3D,
				3
			> my_kd_tree_3d_t;

			my_kd_tree_3d_t index(3, cloud, KDTreeSingleIndexAdaptorParams(10));
			index.buildIndex();

			vector<vector<int>> all_neighbors(raw_cities.size());
			int search_count = k_neighbors + 1;
			vector<uint32_t> ret_index(search_count);
			vector<double> out_dist_sqr(search_count);

			for (size_t i = 0; i < cloud.points.size(); i++) {
				double query_pt[3] = { cloud.points[i].x, cloud.points[i].y, cloud.points[i].z };
				index.knnSearch(query_pt, search_count, &ret_index[0], &out_dist_sqr[0]);

				all_neighbors[i].reserve(k_neighbors);
				for (int j = 1; j < search_count; j++) { 
					if (j < ret_index.size()) {
						all_neighbors[i].push_back(ret_index[j]);
					}
				}
			}
			return all_neighbors;
		}
		else
		{
			PointCloud2D cloud;
			cloud.points.resize(raw_cities.size());

			for (size_t i = 0; i < raw_cities.size(); i++) {
				cloud.points[i].x = raw_cities[i].lat;  
				cloud.points[i].y = raw_cities[i].lon; 
				cloud.points[i].original_index = static_cast<int>(i);
			}

			typedef KDTreeSingleIndexAdaptor<
				L2_Simple_Adaptor<double, PointCloud2D>,
				PointCloud2D,
				2 
			> my_kd_tree_t;

			my_kd_tree_t index(2, cloud, KDTreeSingleIndexAdaptorParams(10));
			index.buildIndex();

			vector<vector<int>> all_neighbors(raw_cities.size());

			int search_count = k_neighbors + 1;

			vector<uint32_t> ret_index(search_count);
			vector<double> out_dist_sqr(search_count);

			for (size_t i = 0; i < cloud.points.size(); i++) {
				double query_pt[2] = { cloud.points[i].x, cloud.points[i].y };

				index.knnSearch(query_pt, search_count, &ret_index[0], &out_dist_sqr[0]);

				all_neighbors[i].reserve(k_neighbors);
				for (int j = 1; j < search_count; j++) {
					if (j < ret_index.size()) {
						all_neighbors[i].push_back(ret_index[j]);
					}
				}
			}

			return all_neighbors;
		}

		
	}
};