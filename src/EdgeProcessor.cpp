#include "EdgeProcessor.h"

#include <algorithm>
#include <iostream>
#include <cmath>

constexpr uint8_t UPPER_LEFT	= 0b11000001; // 193
constexpr uint8_t UPPER_RIGHT	= 0b01110000; // 112
constexpr uint8_t LOWER_RIGHT	= 0b00011100; // 28
constexpr uint8_t LOWER_LEFT	= 0b00000111; // 7

// Constructor
EdgeProcessor::EdgeProcessor()
{
	//std::cout << "Object created: EdgeProcessor\n";
	edgeIdCounter = 0;
}

void EdgeProcessor::traceEdges(cv::Mat &img)
{
	// Reset / Initialization
	edgeIdCounter = 0;
	edges.clear();
	edgeMap.init(img.rows, img.cols);

	// Preprocessing: Identify cluster points
	preprocessClusters(img);

	// Check each edge pixel
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			// Trace only non-cluster pixels without an edgeId (= skip tracing for pixels with edgeId or in a cluster)
			if (img.at<uchar>(y, x) > 0 && edgeMap.getNumberOfEdgeIds(x, y) == 0 && edgeMap.getClusterPoints(x, y).size() == 0)
			{
				// Main tracing function
				std::vector<cv::Point> edge;
				traceEdge(img, cv::Point(x, y), edge);
			}
		}
	}
}

void EdgeProcessor::preprocessClusters(cv::Mat &img)
{
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			if (img.at<uchar>(y, x) > 0 && edgeMap.getClusterPoints(x, y).size() == 0) // Only check edge and unclustered pixels
			{
				cv::Point point = cv::Point(x, y);
				uint8_t binaryCode = getBinaryCode(img, point);
				std::vector<cv::Point> neighbors = getDirectNeighbors(img, point);

				// True if point is a cluster point
				if (containsFourCluster(binaryCode) || neighbors.size() > 2)
				{
					std::vector<cv::Point> clusterPoints;
					clusterPoints.push_back(point); // Current point is cluster point
					int c = 0;

					// Expand clusterPoints by recursively checking neighboring points for cluster status
					while (c < (int)clusterPoints.size())
					{
						// Also called in first run, which is not necessary, but avoids additional check for first run
						std::vector<cv::Point> neighbors = getDirectNeighbors(img, clusterPoints[c]);

						for (const auto& n : neighbors)
						{
							// True if neighbor n is not (already) in clusterPoints
							if (std::find(clusterPoints.begin(), clusterPoints.end(), n) == clusterPoints.end())
							{
								uint8_t binaryCode = getBinaryCode(img, n);
								std::vector<cv::Point> neighbors = getDirectNeighbors(img, n);

								if (containsFourCluster(binaryCode) || neighbors.size() > 2)
								{
									clusterPoints.push_back(n);
								}
							}
						}

						c++;
					}

					// Save all points (coordinates) of a cluster at each point of the cluster in ambiguityMap
					for (const auto& i : clusterPoints)
					{
						edgeMap.pushBackClusterPoints(i.x, i.y, clusterPoints);
					}
				}
			}
		}
	}
}

// This function is called recursively
void EdgeProcessor::traceEdge(cv::Mat &img, cv::Point startPoint, std::vector<cv::Point> &edge)
{
	// Add startPoint to new edge
	edge.push_back(startPoint);
	edgeMap.pushBackEdgeId(startPoint.x, startPoint.y, edgeIdCounter);

	// Get direct neighbors of startPoint clockwise from top left
	std::vector<cv::Point> neighbors = getDirectNeighbors(img, startPoint);
	std::vector<cv::Point> unvisitedNeighbors;

	if (!edgeMap.isCluster(startPoint.x, startPoint.y))
	{
		for (const auto& point : neighbors)
		{
			if ((edgeMap.getNumberOfEdgeIds(point.x, point.y) == 0 || edgeMap.isCluster(point.x, point.y)))
			{
				unvisitedNeighbors.push_back(point);
			}
		}
	}

	if (unvisitedNeighbors.size() == 2)
	{
		// Start tracing in the direction of the first unvisited neighbor
		std::vector<cv::Point> edgePartOne{startPoint};
		traceEdge(img, unvisitedNeighbors[0], edgePartOne);

		// Start new edge to other neighbor if the other neighbor is unvisited, then merge
		// The other neighbor is visited in case of closed contours or if is approached from a cluster
		std::vector<cv::Point> edgePartTwo{startPoint};
		traceEdge(img, unvisitedNeighbors[1], edgePartTwo);

		mergeEdges(edgeIdCounter-2, edgeIdCounter-1);
	}
	else if (unvisitedNeighbors.size() == 1) // Follow edge
	{
		traceEdge(img, unvisitedNeighbors[0], edge);
	}
	else if (unvisitedNeighbors.size() == 0) // End edge
	{
		edges.pushBack(edge);
		edgeIdCounter++;
	}
}

std::vector<cv::Point> EdgeProcessor::getDirectNeighbors(cv::Mat &img, cv::Point p)
{
	// All direct neighbors (as in our sense) are saved in v
	std::vector<cv::Point> v;

	// Only access neighbors inside the image
	if (p.x - 1 >= 0 && p.y - 1 >= 0) // top left
	{
		if (img.at<uchar>(p.y - 1, p.x - 1) > 0 && !(img.at<uchar>(p.y - 1, p.x) > 0 || img.at<uchar>(p.y, p.x - 1) > 0))
		{
			v.push_back(cv::Point(p.x - 1, p.y - 1));
		}
	}
	if (p.y - 1 >= 0) // top center
	{
		if (img.at<uchar>(p.y - 1, p.x) > 0)
		{
			v.push_back(cv::Point(p.x, p.y - 1));
		}
	}
	if (p.x + 1 < img.cols && p.y - 1 >= 0) // top right
	{
		if (img.at<uchar>(p.y - 1, p.x + 1) > 0 && !(img.at<uchar>(p.y - 1, p.x) > 0 || img.at<uchar>(p.y, p.x + 1) > 0))
		{
			v.push_back(cv::Point(p.x + 1, p.y - 1));
		}
	}
	if (p.x + 1 < img.cols) // middle right
	{
		if (img.at<uchar>(p.y, p.x + 1) > 0)
		{
			v.push_back(cv::Point(p.x + 1, p.y));
		}
	}
	if (p.x + 1 < img.cols && p.y + 1 < img.rows) // bottom right
	{
		if (img.at<uchar>(p.y + 1, p.x + 1) > 0 && !(img.at<uchar>(p.y, p.x + 1) > 0 || img.at<uchar>(p.y + 1, p.x) > 0))
		{
			v.push_back(cv::Point(p.x + 1, p.y + 1));
		}
	}
	if (p.y + 1 < img.rows) // bottom center
	{
		if (img.at<uchar>(p.y + 1, p.x) > 0)
		{
			v.push_back(cv::Point(p.x, p.y + 1));
		}
	}
	if (p.x - 1 >= 0 && p.y + 1 < img.rows) // bottom left
	{
		if (img.at<uchar>(p.y + 1, p.x - 1) > 0 && !(img.at<uchar>(p.y + 1, p.x) > 0 || img.at<uchar>(p.y, p.x - 1) > 0))
		{
			v.push_back(cv::Point(p.x - 1, p.y + 1));
		}
	}
	if (p.x - 1 >= 0) // middle left
	{
		if (img.at<uchar>(p.y, p.x - 1) > 0)
		{
			v.push_back(cv::Point(p.x - 1, p.y));
		}
	}

	return v;
}

const bool EdgeProcessor::containsFourCluster(uint8_t binaryCode)
{
	return ((binaryCode & UPPER_LEFT) == UPPER_LEFT ||
		    (binaryCode & UPPER_RIGHT) == UPPER_RIGHT ||
		    (binaryCode & LOWER_RIGHT) == LOWER_RIGHT ||
		    (binaryCode & LOWER_LEFT) == LOWER_LEFT);
}

void EdgeProcessor::mergeEdges(int firstId, int secondId)
{
	// Procedure: Remove edges, create new edge based on two edges, insert at firstId
	// The merged edge gets the firstId

	// firstId has to be the smaller than secondId
	if (secondId < firstId)
	{
		int tempId = secondId;
		secondId = firstId;
		firstId = tempId;
	}
	else if (secondId == firstId)
	{
		std::cout << "Warning: EdgeProcessor::mergeEdges: Cannot merge edges with same id." << std::endl;
		return;
	}

	std::cout << "Merging edge " << firstId << " and " << secondId << std::endl;

	std::vector<cv::Point> firstEdge = edges.getEdge(firstId);
	std::vector<cv::Point> secondEdge = edges.getEdge(secondId);

	edges.clearEdge(firstId);
	edges.clearEdge(secondId);

	// Assign the same edgeId to both edges
	for (const auto& point : secondEdge)
	{
		// In the edgeIdMap, replace the secondId with the firstId
		// The merging includes both Ids, but pushBackEdgeId makes sure that edgeId is not already in point
		edgeMap.eraseEdgeId(point.x, point.y, secondId);
		edgeMap.pushBackEdgeId(point.x, point.y, firstId);
	}

	// Check if the connection point is start point of both edges
	// For the regular tracing, this is the only branch which can be entered (when going into two directions).
	// The remaining checks are only required for post-processing.
	if (firstEdge.front() == secondEdge.front())
	{
		secondEdge.erase(secondEdge.begin());

		// Delete shared pixel if present at both ends. This occurs when connecting the same edge with a line or merging
		// two edges that share a pixel in a cluster. When connected outside the cluster, they form a single closed edge,
		// requiring the removal of the shared pixel to avoid duplication.
		if (firstEdge.back() == secondEdge.back())
		{
			secondEdge.pop_back(); // Remove last element
		}

		// Reverse second edge and prepend to first edge
		firstEdge.insert(firstEdge.begin(), secondEdge.rbegin(), secondEdge.rend());

		//std::cout << "EdgeProcessor::mergeEdges - Case I" << std::endl;
	}

	// Merge edges where the start point of the first edge equals the end point of the second edge
	else if (firstEdge.front() == secondEdge.back())
	{
		secondEdge.pop_back();

		// Same principle as above
		if (firstEdge.back() == secondEdge.front())
		{
			secondEdge.erase(secondEdge.begin());
		}

		// Prepend second edge to first edge
		firstEdge.insert(firstEdge.begin(), secondEdge.begin(), secondEdge.end());

		//std::cout << "EdgeProcessor::mergeEdges - Case II" << std::endl;
	}

	// Merge edges where the end point of the first edge equals the start point of the second edge
	else if (firstEdge.back() == secondEdge.front())
	{
		secondEdge.erase(secondEdge.begin());
		firstEdge.insert(firstEdge.end(), secondEdge.begin(), secondEdge.end());

		//std::cout << "EdgeProcessor::mergeEdges - Case III" << std::endl;
	}

	// Merge edges with same end point
	else if (firstEdge.back() == secondEdge.back())
	{
		secondEdge.pop_back();
		firstEdge.insert(firstEdge.end(), secondEdge.rbegin(), secondEdge.rend());

		//std::cout << "EdgeProcessor::mergeEdges - Case IV" << std::endl;
	}

	edges.overwrite(firstId, firstEdge);
}

const EdgeMap &EdgeProcessor::getEdgeIdMap() const // Return value is read-only
{
	return edgeMap;
}

const Edges &EdgeProcessor::getEdges() const // Return value is read-only
{
	return edges;
}

void EdgeProcessor::printEdgeInfos(cv::Mat &img)
{
	std::cout << "Input image: " << img.rows << " rows x " << img.cols << " cols = " << img.total() << " px\n";

	// Count number of edge pixels
	int cnt = 0;
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			if (img.at<uchar>(y, x) > 0)
			{
				cnt++;
			}
		}
	}

	// Print information
	std::cout << "Edge pixels in input image: " << cnt << " px\n";
	std::cout << "Number of traced edges: " << edges.size() << "\n";
}

uint8_t EdgeProcessor::getBinaryCode(cv::Mat &img, cv::Point p)
{

	/*
	Returns occupancy of all neighbors of p as binary code, 7 = most significant bit
	7 6 5
	0 p 4
	1 2 3
	*/

	uint8_t bin = 0;

	// Only access neighbors inside the image
	if (p.x - 1 >= 0 && p.y - 1 >= 0) // top left
	{
		if (img.at<uchar>(p.y - 1, p.x - 1) > 0)
		{
			bin = bin + 128;
		}
	}
	if (p.y - 1 >= 0) // top center
	{
		if (img.at<uchar>(p.y - 1, p.x) > 0)
		{
			bin = bin + 64;
		}
	}
	if (p.x + 1 < img.cols && p.y - 1 >= 0) // top right
	{
		if (img.at<uchar>(p.y - 1, p.x + 1) > 0)
		{
			bin = bin + 32;
		}
	}
	if (p.x + 1 < img.cols) // middle right
	{
		if (img.at<uchar>(p.y, p.x + 1) > 0)
		{
			bin = bin + 16;
		}
	}
	if (p.x + 1 < img.cols && p.y + 1 < img.rows) // bottom right
	{
		if (img.at<uchar>(p.y + 1, p.x + 1) > 0)
		{
			bin = bin + 8;
		}
	}
	if (p.y + 1 < img.rows) // bottom center
	{
		if (img.at<uchar>(p.y + 1, p.x) > 0)
		{
			bin = bin + 4;
		}
	}
	if (p.x - 1 >= 0 && p.y + 1 < img.rows) // bottom left
	{
		if (img.at<uchar>(p.y + 1, p.x - 1) > 0)
		{
			bin = bin + 2;
		}
	}
	if (p.x - 1 >= 0) // middle left
	{
		if (img.at<uchar>(p.y, p.x - 1) > 0)
		{
			bin = bin + 1;
		}
	}

	return bin;
}

void EdgeProcessor::cleanUpEdges()
{
	edges.eraseEmptyEdges(); // Erase all empty positions in edges
	edgeMap.resetEdgeIdMap(); // Recreate edgeIdMap from scratch

	for (const auto& edge : edges.getEdges())
	{
		int edgeId = edges.getEdgeId(edge);

		// Write edgeId at all points of current edge
		for (const auto& point : edge)
		{
			edgeMap.pushBackEdgeId(point.x, point.y, edgeId);
		}
	}
}

void EdgeProcessor::resetClusters(cv::Mat img)
{
	edgeMap.resetClusterMap();
	cv::Mat imgCopy = img.clone();

	// Draw all points of all edges in the image
	for (const auto& edge : edges.getEdges())
	{
		for (const auto& point : edge)
		{
			imgCopy.at<uchar>(point) = 255;
		}
	}

	preprocessClusters(imgCopy);
}

void EdgeProcessor::threePointEdgesToClusters()
{
	// Essentially, there are two scenarios:
	// 1: Start and end point are in the same cluster - action: remove the edge and the middle pixel.
	// 2: Start and end point are in different clusters - action: remove the edge, incorporate the middle pixel into the cluster, and then merge the clusters.
	// Note: Only the middle pixel is considered for removal or addition.

	// Iterate through all found edges and find edges with a size of 3 (candidate edges)
	for (const auto& edge : edges.getEdges())
	{
		if (edge.size() == 3)
		{
			int edgeId = edges.getEdgeId(edge);

			cv::Point startPoint = edge.front();
			cv::Point endPoint = edge.back();

			// Check if start and end point of the edge both start in a cluster (differentiate between SPA and MPA)
			bool startPointIsCluster = edgeMap.isCluster(startPoint.x, startPoint.y);
			bool endPointIsCluster =  edgeMap.isCluster(endPoint.x, endPoint.y);

			// If both start and end point are in a cluster they have to be combined
			if (startPointIsCluster && endPointIsCluster)
			{
				// Add middle pixel to the cluster where startPoint is connection point
				edgeMap.addPointToCluster(startPoint.x, startPoint.y, edge[1]);

				// Check if start and end point are NOT in the same MPA. If so, add all points from the second cluster (endPoint is connection point) to the first other
				if (!(edgeMap.getClusterEdgeIds(startPoint.x, startPoint.y) == edgeMap.getClusterEdgeIds(endPoint.x, endPoint.y)))
				{
					for (const auto& point : edgeMap.getClusterPoints(endPoint.x, endPoint.y))
					{
						edgeMap.addPointToCluster(startPoint.x, startPoint.y, point);
					}
				}

				// Delete the short edge from edgeMap and edges
				for (const auto& point : edge)
				{
					edgeMap.eraseEdgeId(point.x, point.y, edgeId);
				}

				edges.clearEdge(edgeId);
			}
		}
	}
}

bool EdgeProcessor::removeEdgesShorterThan(size_t numberofPixels, bool free, bool dangling, bool bridged)
{
	bool changes = false;
	for (size_t edgeId = 0; edgeId < edges.size(); edgeId++)
	{
		std::vector<cv::Point> edge = edges.getEdge(edgeId);

		if (edge.size() > 0 && edge.size() < numberofPixels)
		{
			cv::Point startPosition = edge.front();
			cv::Point endPosition = edge.back();

			bool startIsCluster = edgeMap.isCluster(startPosition.x, startPosition.y);
			bool endIsCluster = edgeMap.isCluster(endPosition.x, endPosition.y);

			if (free)
			{
				if (!startIsCluster && !endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId out of the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}

			if (dangling) // Dangling edges which are only connected to one cluster should be deleted
			{
				// One or the other, but not both
				if (startIsCluster != endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId from the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}

			if (bridged)
			{
				// Both points are in a cluster
				if (startIsCluster && endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId from the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}
		}
	}

	// Removing edges in clusters can eliminate ambiguity, allowing direct connections between remaining edges.
	// Removing edges can lead to clusters with none or only one edgeId.
	if (changes)
	{
		connectEdgesInTwoEdgeClusters(false, true);
		removeZeroAndOneEdgeClusters();
	}

	return changes;
}

bool EdgeProcessor::removeEdgesLongerThan(size_t numberofPixels, bool free, bool dangling, bool bridged)
{
	bool changes = false;
	for (size_t edgeId = 0; edgeId < edges.size(); edgeId++)
	{
		std::vector<cv::Point> edge = edges.getEdge(edgeId);

		if (edge.size() > numberofPixels)
		{
			cv::Point startPosition = edge.front();
			cv::Point endPosition = edge.back();

			bool startIsCluster = edgeMap.isCluster(startPosition.x, startPosition.y);
			bool endIsCluster = edgeMap.isCluster(endPosition.x, endPosition.y);

			if (free)
			{
				if (!startIsCluster && !endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId out of the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}

			if (dangling) // Dangling edges which are only connected to one cluster should be deleted
			{
				// One or the other, but not both
				if (startIsCluster != endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId from the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}

			if (bridged)
			{
				// Both points are in a cluster
				if (startIsCluster && endIsCluster)
				{
					// Iterate through all points of the edge and delete the edgeId from the edgeMap
					for (const auto& edgePixel : edge)
					{
						edgeMap.eraseEdgeId(edgePixel.x, edgePixel.y, edgeId);
					}

					edges.clearEdge(edgeId); // Remove edge from edges
					changes = true;
				}
			}
		}
	}

	// Removing edges in clusters can eliminate ambiguity, allowing direct connections between remaining edges.
	// Removing edges can lead to clusters with none or only one edgeId.
	if (changes)
	{
		connectEdgesInTwoEdgeClusters(false, true);
		removeZeroAndOneEdgeClusters();
	}

	return changes;
}

void EdgeProcessor::connectEdgesInClusters(size_t numberPixels, double thresholdAngle, double alpha, double beta, bool connectSameEdge)
{
	// Function summary:
	// 1. Search for an ambiguity point.
	// 2. Collect all edgeIds in that ambiguity (clusterEdgeIds).
	// 3. For the first edgeId, search for start or end points in the ambiguity.
	// 4. If found, iterate through that edgeId and the remaining edgeIds to search for start or end points in the ambiguity.
	// 5. Merge the pair with the smallest cost if it is below the thresholdAngle.
	// 6. Repeat steps 2-5 until no more changes occur.

	// Note: This approach is clear and straightforward but not the most efficient,
	// as the edgeIds are retrieved after each merge. Instead, collect connection options
	// (points and edgeIds) in a list (e.g., a vector of std::pairs) and remove edgeIds that have been merged.

	// Find all cluster (ambiguity) points and check which edgeIds are in that cluster
	for (int y = 0; y < edgeMap.getRows(); y++)
	{
		for (int x = 0; x < edgeMap.getCols(); x++)
		{
			// Check if point (x, y) is in cluster
			if (edgeMap.isCluster(x, y))
			{
				bool changes = true;
				while (changes)
				{
					changes = false;

					std::vector<int> clusterEdgeIds = edgeMap.getClusterEdgeIds(x, y); // Retrieve cluster edgeIds (ordered)
					double smallestCosts = std::numeric_limits<double>::max();

					// Initialize variables to store the optimal match between two edges
					int firstEdgeIdForMerge = -1;
					int secondEdgeIdForMerge = -1;

					cv::Point connectionPointFirstEdgeForMerge;
					cv::Point connectionPointSecondEdgeForMerge;

					// Cluster is not modified within the for-loop, therefore clusterEdgeIds.size() = constant
					for (size_t i = 0; i < clusterEdgeIds.size(); i++)
					{
						int firstEdgeId = clusterEdgeIds[i];
						//std::cout << "firstEdgeId  = " << firstEdgeId << std::endl;

						// There is no meaningful connection point to a closed edge
						if (edges.isClosed(firstEdgeId))
						{
							continue; // Skip the remaining part of the loop
						}

						// An edge can connect to a cluster at zero points (passes through), one point (start or end), or two points (start and end),
						// and the loop goes through the connection points
						std::vector<cv::Point> connectionPointsFirstEdge = findConnectionPointsInCluster(x, y, firstEdgeId);
						for (const cv::Point& connectionPointFirstEdge : connectionPointsFirstEdge)
						{
							// Calculate the angle of the first edge
							double firstAngle = getEdgeAngleWithLSM(edges.getPointsAlongEdgeFromPoint(firstEdgeId, connectionPointFirstEdge, numberPixels));
							//std::cout << "firstAngle = " << firstAngle << std::endl;

							// Go through all edgeIds in the cluster which are candidates for merging
							for (size_t j = i; j < clusterEdgeIds.size(); j++)
							{
								int secondEdgeId = clusterEdgeIds[j];

								// There is no meaningful connection point to a closed edge
								if (edges.isClosed(secondEdgeId))
								{
									continue; // Skip the remaining part of the loop
								}

								// Connecting 3px L-edges with themselves is not meaningful
								if ((firstEdgeId == secondEdgeId) && edges.isThreePixelL(firstEdgeId))
								{
									continue;
								}

								// Connect edges if they are different, or if they are the same and connectSameEdge is true
								if ((firstEdgeId != secondEdgeId) || connectSameEdge)
								{
									// Same as for-loop above (connectionPointFirstEdge)
									std::vector<cv::Point> connectionPointsSecondEdge = findConnectionPointsInCluster(x, y, secondEdgeId);
									for (const cv::Point& connectionPointSecondEdge : connectionPointsSecondEdge)
									{
										// Do not connect the same point of the same edge (required due to iterating through all connection points)
										if ((connectionPointFirstEdge == connectionPointSecondEdge) && (firstEdgeId == secondEdgeId))
										{
											continue;
										}

										// Calculate the angle of the second edge
										double secondAngle = getEdgeAngleWithLSM(edges.getPointsAlongEdgeFromPoint(secondEdgeId, connectionPointSecondEdge, numberPixels));

										// Calculate the difference between the two edge angles
										double currentAngleDiff = std::abs(firstAngle - secondAngle);
										//std::cout << "secondAngle = " << secondAngle << std::endl;

										currentAngleDiff = std::abs(currentAngleDiff - 180.0); // Best match is at 180 (edges point towards each other)
										//std::cout << "currentAngleDiff " << currentAngleDiff << " id1 = " << firstEdgeId << " id2 = " << secondEdgeId << std::endl;

										double distance = std::sqrt(std::pow((connectionPointFirstEdge.x-connectionPointSecondEdge.x), 2) + std::pow((connectionPointFirstEdge.y-connectionPointSecondEdge.y), 2));
										double currentCosts = alpha*currentAngleDiff + beta*distance;

										if ((currentAngleDiff < thresholdAngle) && (currentCosts < smallestCosts))
										{
											smallestCosts = currentCosts;

											firstEdgeIdForMerge = firstEdgeId;
											secondEdgeIdForMerge = secondEdgeId;

											connectionPointFirstEdgeForMerge = connectionPointFirstEdge;
											connectionPointSecondEdgeForMerge = connectionPointSecondEdge;

											changes = true;
										}
									}
								}
							}
						}
					}

					// Merge edges if their angle difference is below thresholdAngle (determined by the preceding if-check)
					if (changes)
					{
						// Compute the line segment connecting the two points within the cluster
						std::vector<cv::Point> tmpEdge = getLinePoints(connectionPointFirstEdgeForMerge, connectionPointSecondEdgeForMerge);
						//std::cout << tmpEdge << std::endl;
						edges.pushBack(tmpEdge);
						int tmpEdgeId = edges.size()-1; // Id of tmpEdge, which has just been pushed back

						// Merge the line segment with existing edge
						mergeEdges(firstEdgeIdForMerge, tmpEdgeId);

						// If it is the same edge, the connection line is already part of that edge after the first merge
						if (firstEdgeIdForMerge != secondEdgeIdForMerge)
						{
							mergeEdges(firstEdgeIdForMerge, secondEdgeIdForMerge);
						}
					}
				}
			}
		}
	}
}

void EdgeProcessor::closeEdgesInClusters()
{
	// This function iterates through all cluster points.
	// Alternative implementation: iterate through all edges and check if their start and end point are in the same cluster
	// (probably more efficient).

	// Find all cluster points and check which edgeIds are in that cluster
	for (int y = 0; y < edgeMap.getRows(); y++)
	{
		for (int x = 0; x < edgeMap.getCols(); x++)
		{
			// Check if point (x, y) is in cluster
			if (edgeMap.isCluster(x, y))
			{
				std::vector<int> clusterEdgeIds = edgeMap.getClusterEdgeIds(x, y);

				// Check for each edge in cluster if start and end point are in the cluster
				for (const auto& edgeId : clusterEdgeIds)
				{
					// Skip empty edges in edgeVector and edges which are too short (< 5) to have a start and end point in cluster
					if (edges.getEdgeSize(edgeId) >= 5)
					{
						cv::Point startPoint = edges.getStartPoint(edgeId);
						cv::Point endPoint = edges.getEndPoint(edgeId);

						// Add connecting edge if the points are not 8-neighbors (otherwise it is already closed)
						bool startAndEndPointInCluster = (edgeMap.isPointInCluster(x, y, startPoint)) && (edgeMap.isPointInCluster(x, y, endPoint));
						if (startAndEndPointInCluster && !edges.isClosed(edgeId))
						{
							std::vector<cv::Point> tmpEdge = getLinePoints(startPoint, endPoint);
							std::cout << tmpEdge << std::endl;

							edges.pushBack(tmpEdge);
							int tmpEdgeId = edges.size()-1; // Id of tmpEdge, which has just been pushed back
							mergeEdges(edgeId, tmpEdgeId);
						}
					}
				}
			}
		}
	}
}

bool EdgeProcessor::findStartOrEndPointInCluster(int x, int y, int edgeId, cv::Point& connectionPoint)
{
	// Only start point in cluster, end point not
    if (edgeMap.isPointInCluster(x, y, edges.getStartPoint(edgeId)) && !edgeMap.isPointInCluster(x, y, edges.getEndPoint(edgeId)))
    {
        connectionPoint = edges.getStartPoint(edgeId);
        return true;
    }
    // Only end point in cluster, start point not
    else if (edgeMap.isPointInCluster(x, y, edges.getEndPoint(edgeId)) && !edgeMap.isPointInCluster(x, y, edges.getStartPoint(edgeId)))
    {
        connectionPoint = edges.getEndPoint(edgeId);
        return true;
    }

    return false; // Neither exclusively start or end point found in cluster
}

std::vector<cv::Point> EdgeProcessor::findConnectionPointsInCluster(int x, int y, int edgeId)
{
	std::vector<cv::Point> connectionPoints;

	if (edgeMap.isPointInCluster(x, y, edges.getStartPoint(edgeId)))
	{
		connectionPoints.push_back(edges.getStartPoint(edgeId));
	}
	if (edgeMap.isPointInCluster(x, y, edges.getEndPoint(edgeId)))
	{
		connectionPoints.push_back(edges.getEndPoint(edgeId));
	}

	return connectionPoints;
}

void EdgeProcessor::bridgeEdgeGaps(size_t numberPixels, double thresholdAngle, int blockDistance, double alpha, double beta)
{
	size_t numberEdgeIds = edges.size();
	for (size_t edgeId = 0; edgeId < numberEdgeIds; edgeId++)
	{
		bool changes = true;
		while (changes)
		{
			changes = false;

			// Get Edge with current edgeId
			std::vector<cv::Point> edge = edges.getEdge(edgeId);

			// Two reasons why edge.size() > 1: There can be empty edges after merging; Do not start from isolated pixels
			if (edge.size() > 1)
			{
				// Iterate through the start and end point of current (reference) edge
				for (int i = 0; i <= 1; ++i)
				{
					if (edges.isClosed(edgeId))
					{
						continue;
					}

					cv::Point referencePoint = (i == 0) ? edges.getStartPoint(edgeId) : edges.getEndPoint(edgeId);
					double referenceAngle = getEdgeAngleWithLSM(edges.getPointsAlongEdgeFromPoint(edgeId, referencePoint, numberPixels));

					// Get all candidate edgeIds and their connectionPoints
					std::vector<std::pair<int, cv::Point>> edgesInSearchArea = getEdgesInSearchArea(referencePoint, blockDistance, thresholdAngle, referenceAngle);

					double smallestCosts = std::numeric_limits<double>::max();
					int indexForMerge = -1;

					// Find the best matching between the reference edge and all found candidate edges
					int loopIndex = 0;
					for (const auto& edgeIdAndConnectionPoint : edgesInSearchArea)
					{
						int candidateEdgeId = edgeIdAndConnectionPoint.first;

						if (edges.isClosed(candidateEdgeId))
						{
							loopIndex++;
							continue;
						}

						cv::Point candidateConnectionPoint = edgeIdAndConnectionPoint.second;

						// Calculate the difference between the angles of the two edges
						double neighborAngle = getEdgeAngleWithLSM(edges.getPointsAlongEdgeFromPoint(candidateEdgeId, candidateConnectionPoint, numberPixels));
						double currentAngleDiff = std::abs(referenceAngle - neighborAngle);
						currentAngleDiff = std::abs(currentAngleDiff - 180.0); // Best match is at 180 (edges point towards each other)

						if (edges.getEdge(candidateEdgeId).size() == 1)
						{
							double neighborPointAngle = getAngleBetweenPoints(referencePoint, candidateConnectionPoint);
							currentAngleDiff = std::abs(referenceAngle - neighborPointAngle);
						}

						double distance = std::sqrt((referencePoint.x-candidateConnectionPoint.x)*(referencePoint.x-candidateConnectionPoint.x) + (referencePoint.y-candidateConnectionPoint.y)*(referencePoint.y-candidateConnectionPoint.y));
						double currentCosts = alpha*currentAngleDiff + beta*distance;

						if ((currentAngleDiff < thresholdAngle) && (currentCosts < smallestCosts))
						{
							smallestCosts = currentCosts;
							indexForMerge = loopIndex; // Not the edgeId, but the position of the edge in edgesInSearchArea

							changes = true;
						}

						loopIndex++;
					}

					// Merge edge with the best found candidate edge if one was found
					if (changes)
					{
						std::pair<int, cv::Point> edgeIdAndConnectionPoint = edgesInSearchArea[indexForMerge];

						// Create a temporal edge which bridges the two points
						std::vector<cv::Point> tmpEdge = getLinePoints(referencePoint, edgeIdAndConnectionPoint.second);
						edges.pushBack(tmpEdge);
						int tmpEdgeId = edges.size()-1;

						// First merge one edge with the temporal edge and then with the other edge
						mergeEdges(edgeId, tmpEdgeId);
						mergeEdges(edgeId, edgeIdAndConnectionPoint.first);

						// Single pixel edges have been skipped before, as they do not provide a referenceAngle.
						// Later, they can be merged, but can have a smaller edgeId than the current edgeId, so that the merging result gets the smaller edgeId.
						// In that case, the merged edge can not be processed further, therefore set back the edgeId of the for-loop.
						edgeId =  std::min(static_cast<int>(edgeId), edgeIdAndConnectionPoint.first);

						// If start point was merged break the for-loop and start again with updated edge
						break;
					}
				}
			}
		}
	}
}

void EdgeProcessor::connectEdgesInTwoEdgeClusters(bool onlyIf8Neighbors, bool deleteClustersAfterConnect)
{
	// Find all cluster points and check which edgeIds are in that cluster
	for (int y = 0; y < edgeMap.getRows(); y++)
	{
		for (int x = 0; x < edgeMap.getCols(); x++)
		{
			std::vector<int> clusterEdgeIds = edgeMap.getClusterEdgeIds(x, y);

			// Restrict processing to clusters of at least two points.
			// It can not be the same edge, as we have two different edgeIds.
			// A single branch on a closed contour also leads to a cluster with two edgeIds.
			// Since the contour is already closed, meaningful merging is not possible (therefore exclude closed contours).
			if (clusterEdgeIds.size() == 2 && edgeMap.isCluster(x, y) && !edges.isClosed(clusterEdgeIds[0]) && !edges.isClosed(clusterEdgeIds[1]))
			{
				cv::Point connectionPoint1;
				cv::Point connectionPoint2;

				bool connectionPoint1Found = findStartOrEndPointInCluster(x, y, clusterEdgeIds[0], connectionPoint1);
				bool connectionPoint2Found = findStartOrEndPointInCluster(x, y, clusterEdgeIds[1], connectionPoint2);

				if (connectionPoint1Found && connectionPoint2Found)
				{
					bool are8Neighbors = (cv::norm(connectionPoint1 - connectionPoint2) < 1.5); // 1.5 replaces sqrt(2), as next closest distance would be 2
					if (onlyIf8Neighbors && are8Neighbors)
					{
						// Create a temporal edge which bridges the two points
						std::vector<cv::Point> tmpEdge = {connectionPoint1, connectionPoint2};
						edges.pushBack(tmpEdge);
						int tmpEdgeId = edges.size()-1;

						// First merge one edge with the temporal edge and then with the other edge
						mergeEdges(clusterEdgeIds[0], tmpEdgeId);
						mergeEdges(clusterEdgeIds[0], clusterEdgeIds[1]);

						if (deleteClustersAfterConnect)
						{
							edgeMap.clearCluster(x, y);
						}
					}
					//
					else if (!onlyIf8Neighbors)
					{
						// Create a temporal edge which bridges the two points
						std::vector<cv::Point> tmpEdge = getLinePoints(connectionPoint1, connectionPoint2);
						edges.pushBack(tmpEdge);
						int tmpEdgeId = edges.size()-1;

						// First merge one edge with the temporal edge and then with the other edge
						mergeEdges(clusterEdgeIds[0], tmpEdgeId);
						mergeEdges(clusterEdgeIds[0], clusterEdgeIds[1]);

						if (deleteClustersAfterConnect)
						{
							edgeMap.clearCluster(x, y);
						}
					}

					// Removing one of two (remaining) branches from an edge can lead to a closed contour after merging,
					// where the start or end point is not in the remaining cluster. This can be corrected.
					int edgeIdMerged = std::min(clusterEdgeIds[0], clusterEdgeIds[0]); // edgeId if the two edges have been merged
					if (edges.isClosed(edgeIdMerged))
					{
						cv::Point startPoint = edges.getStartPoint(edgeIdMerged);
						cv::Point endPoint = edges.getEndPoint(edgeIdMerged);

						if (!edgeMap.isCluster(startPoint.x, startPoint.y) && !edgeMap.isCluster(endPoint.x, endPoint.y))
						{
							// Rearrange vector so that the start point is in the cluster
							// (not necessarily required, but helpful for consistency and further processing)
							std::vector<cv::Point> edge = edges.getEdge(edgeIdMerged);
							for (auto it = edge.begin(); it != edge.end(); ++it)
							{
							    if (edgeMap.isCluster(it->x, it->y))
							    {
							    	// Rotate the vector so that the current element comes to the beginning
							    	std::rotate(edge.begin(), it, edge.end());
							        edges.overwrite(edgeIdMerged, edge);
							        break;
							    }
							}
						}
					}
				}
			}
		}
	}
}

void EdgeProcessor::removeZeroAndOneEdgeClusters()
{
	for (int y = 0; y < edgeMap.getRows(); y++)
	{
		for (int x = 0; x < edgeMap.getCols(); x++)
		{
			if (edgeMap.isCluster(x, y) && (edgeMap.getClusterEdgeIds(x, y).size() <= 1))
			{
				edgeMap.clearCluster(x, y);
			}
		}
	}
}

void EdgeProcessor::reverseAllEdges()
{
	edges.reverseAll();
}

std::vector<std::pair<int, cv::Point>> EdgeProcessor::getEdgesInSearchArea(cv::Point p, int blockDistance, double thresholdAngle, double referenceAngle)
{
	std::vector<std::pair<int, cv::Point>> edgesInSearchArea; // Saves all found edgeIds and their connection point

	for (int dy = -blockDistance; dy <= blockDistance; ++dy)
	{
		for (int dx = -blockDistance; dx <= blockDistance; ++dx)
		{
			cv::Point neighbor(p.x + dx, p.y + dy);

			// Check if the neighbor is inside the search area (based on angle)
			double neighborPointAngle = getAngleBetweenPoints(p, neighbor);
			if (std::abs(referenceAngle - neighborPointAngle) >= thresholdAngle)
			{
				continue;
			}

			// Exclude pixels outside the image
			bool isNeighborInsideImage = (neighbor.x >= 0 && neighbor.y >= 0 && neighbor.x < edgeMap.getCols() && neighbor.y < edgeMap.getRows());

			// Check if neighbor contains an edge, do not connect to clusters
			if (isNeighborInsideImage && edgeMap.getNumberOfEdgeIds(neighbor.x, neighbor.y) == 1 && !(edgeMap.isCluster(neighbor.x, neighbor.y)))
			{
				int neighborEdgeId = edgeMap.getEdgeIds(neighbor.x, neighbor.y).front(); // Contains only one element

				// Do not connect to edge itself, make sure that connection point is start or end point
				if ((neighbor == edges.getStartPoint(neighborEdgeId) || neighbor == edges.getEndPoint(neighborEdgeId)))
				{
					edgesInSearchArea.push_back(std::make_pair(neighborEdgeId, neighbor));
				}
			}
		}
	}

	return edgesInSearchArea;
}

double EdgeProcessor::getLSMError(std::vector<cv::Point> points, double& a, double& b)
{
	double x_average = 0.0;
	double y_average = 0.0;
	double x_squared_average = 0.0;
	double xy_average = 0.0;

	for (const cv::Point& point : points)
	{
		x_average += point.x;
		y_average += point.y;
		x_squared_average += point.x * point.x;
		xy_average += point.x * point.y;
	}

	x_average /= points.size();
	y_average /= points.size();
	x_squared_average /= points.size();
	xy_average /= points.size();

	if (std::abs(x_squared_average - (x_average*x_average)) < 1e-9)
	{
		return std::numeric_limits<double>::max();
	}

	a = (xy_average-x_average*y_average) / (x_squared_average-x_average*x_average);
	//b = y_average - a * x_average;
	b = points.front().y - a * points.front().x; // Compute b relative to connection point

	double approximationError = 0.0;

	for (const cv::Point& point : points)
	{
		double y_approx = a * point.x + b;
		approximationError += (y_approx - point.y) * (y_approx - point.y);
	}

	return approximationError;
}

double EdgeProcessor::getEdgeAngleWithLSM(std::vector<cv::Point> points)
{
	double a = 0.0;
	double b = 0.0;

	// Compute a, b, and approximation error with given points
	double error = getLSMError(points, a, b);

	// Compute the corresponding angle
	double dx = points.front().x - points.back().x;
	double dy = (a * points.front().x + b) - (a * points.back().x + b);
	double angle = atan2(dx, dy);

	// Swap x and y to check if it gives a better fit
	for (cv::Point& point : points)
	{
		std::swap(point.x, point.y);
	}

	// Check if the error is smaller after swapping
	double newError = getLSMError(points, a, b);
	if (newError < error)
	//if (true)
	{
		// Recompute the corresponding angle
		dx = points.front().x - points.back().x;
		dy = (a * points.front().x + b) - (a * points.back().x + b);
		angle = atan2(dy, dx);
	}

	// Normalize angle from [-180, 180] to [0, 360] degrees
	angle = angle * (180 / M_PI);

	if (angle < 0)
	{
	    angle += 360.0;
	}

	return angle;
}

double EdgeProcessor::getAngleBetweenPoints(cv::Point startPoint, cv::Point endPoint)
{
	double dx = endPoint.x - startPoint.x;
	double dy = endPoint.y - startPoint.y;

	double angle = atan2(dx, dy);
	angle = angle * (180 / M_PI);

	// Normalize angle from [-180, 180] to [0, 360] degrees
	if (angle < 0)
	{
	    angle += 360.0;
	}

	return angle;
}

std::vector<cv::Point> EdgeProcessor::getLinePoints(cv::Point startPoint, cv::Point endPoint)
{
    // Initialize the vector to store the points of the line
    std::vector<cv::Point> linePoints;

    // Starting point coordinates
    int x0 = startPoint.x;
    int y0 = startPoint.y;

    // Ending point coordinates
    int x1 = endPoint.x;
    int y1 = endPoint.y;

    // Check if the line is steep. If it is, swap x and y to decrease the slope (make it less steep)
    bool isSteep = abs(y1 - y0) > abs(x1 - x0);
    if (isSteep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    // Ensure that we are drawing from left to right
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // Delta x and delta y
    int dx = x1 - x0;
    int dy = abs(y1 - y0);

    // Initialize the error term (accumulated difference to ideal y-coordinate) to compensate for the non-integer slope
    int error = dx / 2;

    // Determine the direction of the y-step: If y0 < y1 then ystep = 1; otherwise ystep = -1
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    // Iterate over the x coordinates from start to end
    for (int x = x0; x <= x1; x++)
    {
        // Choose the point to add to the line. If the line is steep, swap x and y back to their original order
        cv::Point point = isSteep ? cv::Point(y, x) : cv::Point(x, y);
        linePoints.push_back(point);

        // Update the error term and y-coordinate as necessary
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }

    return linePoints;
}
