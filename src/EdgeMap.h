#ifndef EDGEIDMAP_H
#define EDGEIDMAP_H

#include <vector>

#include <opencv2/core.hpp>

// Note: int x, int y could be replaced by cv::Point
class EdgeMap
{
public:
	/** Constructor.
	 */
	EdgeMap();

	/** Initialize the data structures maintained by this class.
	 */
	void init(int rows, int cols);

	/**	Push back edgeId at given position.
	 */
	void pushBackEdgeId(int x, int y, int edgeId);

	/**	Push back cluster point at given position.
	 */
	void pushBackClusterPoints(int x, int y, std::vector<cv::Point> cluster);

	/** Adds one point to a cluster.
	 */
	void addPointToCluster(int x, int y, cv::Point point);

	/** Erase edgeId at given position.
	 */
	void eraseEdgeId(int x, int y, int edgeId);

	/** Clear EdgeId at given position.
	 */
	void clearClusterPoint(int x, int y);

	/** Clear the complete cluster at every position of the cluster.
	 */
	void clearCluster(int x, int y);

	/** Number of indices at given position.
	 */
	int getNumberOfEdgeIds(int x, int y) const;

	/** Number of cluster points at given position.
	 */
	int getNumberOfClusterPoints(int x, int y) const;

	/** Number of image columns.
	 */
	int getCols() const;

	/** Number of image rows.
	 */
	int getRows() const;

	/** Get read-only reference to edge identifiers (edgeIds).
	 */
	const std::vector<int> &getEdgeIds(int x, int y) const;

	/** Get read-only reference to cluster points.
	 */
	const std::vector<cv::Point> &getClusterPoints(int x, int y) const;

	/** Number of different edges in edgeIdMap.
	 */
	int getMaxEdgeId() const;

	/** Get all edgeIds belonging to the cluster.
	 */
	std::vector<int> getClusterEdgeIds(int x, int y) const;

	/** Checks if the point belongs to a cluster.
	 */
	bool isCluster(int x, int y);

	/** Clears dataEdgeIds, which represents the edgeIdMap.
	 */
	void resetEdgeIdMap();

	/** Clears dataClusters, which represents the eambiguityMap.
	 */
	void resetClusterMap();

	/** Checks if the specified point is part of the cluster located at position (x, y).
	 *  Useful to check if an edge has a start or end point in that cluster (point should then be a start or end point).
	 */
	bool isPointInCluster(int x, int y, cv::Point point);

private:
	std::vector<std::vector<int>> dataEdgeIds;			//!< 1D data structure representing the 2D edgeIdMap.
	std::vector<std::vector<cv::Point>> dataClusters;	//!< 1D data structure representing the 2D ambiguityMap.

	int rows; //!< Number of input image rows.
	int cols; //!< Number of input image columns.
};

inline int EdgeMap::getNumberOfEdgeIds(int x, int y) const
{
	// Number of edgeIds at given position
	return dataEdgeIds[x + y * cols].size();
}

inline int EdgeMap::getNumberOfClusterPoints(int x, int y) const
{
	// Number of edgeIds at given position
	return dataClusters[x + y * cols].size();
}

#endif // EDGEIDMAP_H
