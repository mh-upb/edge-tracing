#ifndef EDGEPROCESSOR_H_
#define EDGEPROCESSOR_H_

#include <vector>
#include <utility>

#include <opencv2/core.hpp>

#include "EdgeMap.h"
#include "Edges.h"

class EdgeProcessor
{
public:
	/** Constructor
	 */
	EdgeProcessor();

	/**
	 * Main function for edge tracing.
	 * @img 			Input Image.
	 */
	void traceEdges(cv::Mat &img);

	/* Print information about the input image and traced edges.
	 */
	void printEdgeInfos(cv::Mat &img);

	/** Get read-only reference to edgeIdMap.
	 */
	const EdgeMap &getEdgeIdMap() const;

	/** Get read-only reference to edge vector.
	 */
	const Edges &getEdges() const;

	/** Erase all empty edges from edge vector and update edgeIdMap appropriately.
	 */
	void cleanUpEdges();

	/** Reset all clusters and find new clusters based on the image and the edges
	 */
	void resetClusters(cv::Mat img);

	/**
	 * Remove edges which are shorter than the given number of pixels.
	 * @numberPixels		Edges shorter than this number of pixels will be deleted.
	 * @free				If true, free standing edges (not connected to a cluster) will be deleted.
	 * @dangling			If true, edges which are only connected to one cluster will be deleted.
	 * @bridged				If true, edges which are connected to two clusters will be deleted.
	 */
	bool removeEdgesShorterThan(size_t numberPixels, bool free=true, bool dangling=true, bool bridged=false);

	/**
	 * Remove edges which are longer than the given number of pixels.
	 * @numberPixels		Edges shorter than this number of pixels will be deleted.
	 * @free				If true, free standing edges (not connected to a cluster) will be deleted.
	 * @dangling			If true, edges which are only connected to one cluster will be deleted.
	 * @bridged				If true, edges which are connected to two clusters will be deleted.
	 */
	bool removeEdgesLongerThan(size_t numberPixels, bool free=true, bool dangling=true, bool bridged=false);

	/**
	 * Connects edges starting or ending in the same cluster based on a simple continuity check
	 * based on the angle of each edge in the image plane (small difference = good continuity).
	 * Take this function as an example to implement more advanced strategies.
	 * @numberPixels	Number of pixels considered to compute the angle of each edge in the plane.
	 * @thresholdAngle	Angle difference must be smaller than or equal to this threshold.
	 * @alpha			Weighting factors in the cost function C = alpha*angleDiff + beta*blockDistance.
	 * @beta			Weighting factors in the cost function C = alpha*angleDiff + beta*blockDistance.
	 */
	void connectEdgesInClusters(size_t numberPixels, double thresholdAngle, double alpha=1.0, double beta=1.0, bool connectSameEdge=true);

	/**
	 * Integrate short edges with start and end point in cluster into the cluster(s). If these are two
	 * different clusters, they will be merged to one cluster. The edge itself will be deleted.
	 */
	void threePointEdgesToClusters();

	/**
	 * Bridges gaps between closely aligned edges based on their pixel distance and angular difference.
	 * @numberPixels	Number of pixels considered to compute the angle of nearby edges in the plane.
	 * @thresholdAngle	Angle difference must be smaller than or equal to this threshold to connect two edges.
	 * @blockDistance	L1 (Manhattan) distance considered to search for nearby edges.
	 * @alpha			Weighting factors in the cost function C = alpha*angleDiff + beta*blockDistance.
	 * @beta			Weighting factors in the cost function C = alpha*angleDiff + beta*blockDistance.
	 */
	void bridgeEdgeGaps(size_t numberPixels, double thresholdAngle, int blockDistance=5, double alpha=1.0, double beta=1.0);

	/**
	 * Closes edges with the same same edge if they are connected to a cluster, which means that the start
	 * and end point of that edge are in the same cluster.
	 */
	void closeEdgesInClusters();

	/**
	 * Reverse the order of the points in all edges.
	 */
	void reverseAllEdges();

	/**
	 * Connects edges in clusters with exactly two edge connections. This function will not connect the same
	 * edge (there is an extra function for that), which would mean that the start and end point of one edge are in a cluster.
	 * @onlyIf8Neighbors				Only connect the two edges of they are 8-neighbors.
	 * @deleteClustersAfterConnect		Delete the cluster after the two edges have been connected.
	 */
	void connectEdgesInTwoEdgeClusters(bool onlyIf8Neighbors=false, bool deleteClustersAfterConnect=false);

	/**
	 * Remove all clusters with only one edgeId (such clusters can appear when removing edges).
	 */
	void removeZeroAndOneEdgeClusters();

private:
	int edgeIdCounter;	//!< Identifier of each edge incremented with each traced edge.

	Edges edges; 		//!< Traced edges (see class Edges for details, basically a Vector with all traced edges).

	EdgeMap edgeMap; 	//!< Represents the edgeIdMap and ambiguityMap (see class EdgeMap for details).

	/**
	 * Get direct neighbors (as in our sense) of point p clockwise from top left.
	 * Diagonal neighbors are only returned if they do not have any orthogonal neighbors.
	 * @img				Input Image.
	 * @p				Point of interest.
	 */
	std::vector<cv::Point> getDirectNeighbors(cv::Mat &img, cv::Point p);

	/**
	 * Returns occupancy of all neighbors of p as binary code.
	 * @img				Input Image.
	 * @p				Point of interest.
	 */
	uint8_t getBinaryCode(cv::Mat &img, cv::Point p);

	/**
	 * Check if 3x3 region contains at least one four-cluster based on its binary code (four-clusters are always located in corners).
	 * @binaryCode		Binary code of the occupancy of all neighbors (represented as integer).
	 */
	const bool containsFourCluster(uint8_t binaryCode);

	/**
	 * Preprocessing to identify all cluster points (creates the ambiguityMap)
	 * @img 			Input Image.
	 */
	void preprocessClusters(cv::Mat &img);

	/**
	 * Tracing function which is called recursively.
	 * @img			Input Image.
	 * @startPoint	Current point that is traced.
	 * @edge		Current edge to which the point belongs.
	 */
	void traceEdge(cv::Mat &img, cv::Point startPoint, std::vector<cv::Point> &edge);

	/**
	 * Function to merge (connect) two edges.
	 * @firstId				Identifier of the first edge to be merged.
	 * @secondId			Identifier of the second edge to be merged.
	 */
	void mergeEdges(int firstId, int secondId);

	/**
	 * Computes the angle between the given points in the image plane.
	 * @returns			Angle in deg.
	 */
	double getAngleBetweenPoints(cv::Point startPoint, cv::Point endPoint);

	/**
	 * Uses the Least Squares Method to approximate a straight line through the given points and writes the parameters to a, b.
	 * @points			Points to be used to approximate the straight line.
	 * @a				Parameter (slope) of the straight line in the form y = ax + b.
	 * @b				Parameter (y-intercept) of the straight line in the form y = ax + b.
	 * @returns			Approximation error.
	 */
	double getLSMError(std::vector<cv::Point> points, double& a, double& b);

	/**
	 * Computes the angle of the straight line approximated with the LSM method.
	 * @points			Points to be used to approximate the straight line.
	 * @returns			Angle in deg.
	 */
	double getEdgeAngleWithLSM(std::vector<cv::Point> points);

	/** Checks if the specified edge with the edgeId has a start or end point (but not both) in the cluster located at position (x, y).
	 *  @edgeId				Identifier of the edge.
	 *  @connectionPoint	The possible start or end point will be stored in this variable.
	 *  @return				True if start or end point has been found, otherwise False.
	 */
	bool findStartOrEndPointInCluster(int x, int y, int edgeId, cv::Point& connectionPoint);

	/** Finds connection points (start and end point) in the cluster located at position (x, y).
	 *  @edgeId				Identifier of the edge.
	 *  @return				Vector with the points, will contain a maximum of two points (start and end point).
	 */
	std::vector<cv::Point> findConnectionPointsInCluster(int x, int y, int edgeId);

	/**
	 * Computes the discrete points of a straight line between the given points using Bresenham's line algorithm.
	 * @startPoint		Point where the line should start (point is included).
	 * @endPoint		Point where the line should end (point is included).
	 * @return			List (vector) with the line points.
	 */
	std::vector<cv::Point> getLinePoints(cv::Point startPoint, cv::Point endPoint);

	/**
	 * Finds all start and end points of edges in the search area and returns them together with their corresponding edgeId.
	 * @p				Reference point.
	 * @edgeId			The edgeId to which the reference point belongs.
	 * @blockDistance	L1 (Manhattan) distance considered to search for nearby edges.
	 * @thresholdAngle	Angle difference must be smaller than or equal to this threshold.
	 * @referenceAngle	Angle taken as reference, returned edges points must be closer than thresholdAngle to this angle.
	 */
	std::vector<std::pair<int, cv::Point>> getEdgesInSearchArea(cv::Point p, int blockDistance, double thresholdAngle, double referenceAngle);
};

#endif /* EDGEPROCESSOR_H_ */

