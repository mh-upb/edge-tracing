#include "EdgeMap.h"

#include <set>

namespace { constexpr bool WRITE_EDGE_IDS_AT_ALL_CLUSTER_POINTS = false; }

EdgeMap::EdgeMap() : rows (0), cols(0)
{
}

void EdgeMap::init(int rows, int cols)
{
	// Reset
	dataEdgeIds.clear();
	dataClusters.clear();

	// Save number of image rows and cols
	EdgeMap::rows = rows;
	EdgeMap::cols = cols;

	// Initialize data structure
	dataEdgeIds.resize(rows * cols);
	dataClusters.resize(rows * cols);
}

const std::vector<int> &EdgeMap::getEdgeIds(int x, int y) const
{
	return dataEdgeIds[x + y * cols];
}

const std::vector<cv::Point> &EdgeMap::getClusterPoints(int x, int y) const
{
	// Cluster points at given position
	return dataClusters[x + y * cols];
}

int EdgeMap::getCols() const
{
	return cols;
}

int EdgeMap::getRows() const
{
	return rows;
}

int EdgeMap::getMaxEdgeId() const
{
	int maxId = 0;

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			for (int i = 0; i < EdgeMap::getNumberOfEdgeIds(x, y); i++)
			{
				if (dataEdgeIds[x + y * cols][i] > maxId)
				{
					maxId = dataEdgeIds[x + y * cols][i];
				}
			}
		}
	}

	return maxId;
}

void EdgeMap::pushBackEdgeId(int x, int y, int edgeId)
{
	if constexpr (WRITE_EDGE_IDS_AT_ALL_CLUSTER_POINTS)
	{
		if (dataClusters[x + y * cols].size() == 0)
		{
			// Push back edgeId at given position
			dataEdgeIds[x + y * cols].push_back(edgeId);
		}
		else
		{
			// Only push back edgeId if not already in cluster
			if (std::find(dataEdgeIds[x + y * cols].begin(), dataEdgeIds[x + y * cols].end(), edgeId) == dataEdgeIds[x + y * cols].end())
			{
				for (const auto& p : dataClusters[x + y * cols])
				{
					dataEdgeIds[p.x + p.y * cols].push_back(edgeId);
				}
			}
		}
	}
	else
	{
		// Only push back edgeId if not already in cluster
		if (std::find(dataEdgeIds[x + y * cols].begin(), dataEdgeIds[x + y * cols].end(), edgeId) == dataEdgeIds[x + y * cols].end())
		{
			dataEdgeIds[x + y * cols].push_back(edgeId);
		}
	}
}

void EdgeMap::pushBackClusterPoints(int x, int y, std::vector<cv::Point> clusterPoints)
{
	// Save clusterPoints at given position
	dataClusters[x + y * cols] = clusterPoints;
}

void EdgeMap::addPointToCluster(int x, int y, cv::Point point)
{
	// Here, cluster points are only stored at the given position
	dataClusters[x + y * cols].push_back(point);

	// Then, store all cluster points at every position, including the new point
	for (const auto& p : dataClusters[x + y * cols])
	{
		dataClusters[p.x + p.y * cols] = dataClusters[x + y * cols];
	}
}

void EdgeMap::eraseEdgeId(int x, int y, int edgeId)
{
	// Get position of edgeId in dataEdgeIds[x + y * cols]
	auto iterator = std::find(dataEdgeIds[x + y * cols].begin(), dataEdgeIds[x + y * cols].end(), edgeId);

	// Erase if edgeId found
	if (iterator != dataEdgeIds[x + y * cols].end())
	{
		dataEdgeIds[x + y * cols].erase(iterator);
	}
}

void EdgeMap::clearClusterPoint(int x, int y)
{
	dataClusters[x + y * cols].clear();
}

void EdgeMap::clearCluster(int x, int y)
{
	for (const auto& p : dataClusters[x + y * cols])
	{
		dataClusters[p.x + p.y * cols].clear();
	}
}

std::vector<int> EdgeMap::getClusterEdgeIds(int x, int y) const
{
	std::set<int> edgeIdsInCluster; // Set automatically avoids duplicate entries, results are ordered

	for (const auto& point : dataClusters[x + y * cols])
	{
		for (const auto& edgeId : dataEdgeIds[point.x + point.y * cols])
		{
			edgeIdsInCluster.insert(edgeId);
		}
	}

	// Convert set to vector and return
    return std::vector<int>(edgeIdsInCluster.begin(), edgeIdsInCluster.end());
}

bool EdgeMap::isCluster(int x, int y)
{
	if (dataClusters[x + y * cols].size() >= 1)
	{
		return true;
	}

	return false;
}

void EdgeMap::resetEdgeIdMap()
{
	dataEdgeIds.clear();
	dataEdgeIds.resize(rows * cols);
}

void EdgeMap::resetClusterMap()
{
	dataClusters.clear();
	dataClusters.resize(rows * cols);
}

bool EdgeMap::isPointInCluster(int x, int y, cv::Point point)
{
	for (const cv::Point& p : dataClusters[x + y * cols])
	{
		if (p == point)
		{
			return true;
		}
	}

	return false;
}





















