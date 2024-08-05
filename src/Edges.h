#ifndef EDGES_H
#define EDGES_H

#include <opencv2/core.hpp>

class Edges
{
public:

	Edges() = default;

	void clear();

	void pushBack(std::vector<cv::Point> edge);

	void insert(int edgeId, std::vector<cv::Point> edge);

	void overwrite(int edgeId, std::vector<cv::Point> edge);

	void popBack();

	bool isClosed(int edgeId);

	bool isThreePixelL(int edgeId);

	/** Erases all empty edges from the edge vector
	 */
	void eraseEmptyEdges();

	void clearEdge(int edgeId);

	void reverseAll();

	const std::vector<cv::Point> &getEdge(int index) const;

	size_t size() const;

	const std::vector<std::vector<cv::Point>> &getEdges() const;

	/** Get Edge Id of given edge.
	 */
	int getEdgeId(std::vector<cv::Point> edge);

	/** Get the start point of edge with edgeId.
	 */
	cv::Point getStartPoint(int edgeId) const;

	/** Get the start point of edge with edgeId.
	 */
	cv::Point getEndPoint(int edgeId) const;

	/** Get size (number of points) of edge with edgeId.
	 */
	size_t getEdgeSize(int edgeId) const;

	void eraseEdge(std::vector<cv::Point> edge);

	/** Get n points from the edge starting from the given start or end point.
	 * @edgeId: EdgeId of the edge which should be used.
	 * @point: The point from where the n points should be taken (point has to be start or end point of edge).
	 * @numberPixels: Specifies the maximum number of pixels to be returned. If this number exceeds the total pixels in the edge, all edge pixels will be returned.
	 */
	std::vector<cv::Point> getPointsAlongEdgeFromPoint(int edgeId, cv::Point point, size_t numberPixels);

private:
	/*  Vector with all traced edges. Each edge is a vector of points (std::vector<cv::Point>).
	 *  Position of each edge in data corresponds to edgeId.
	 */
	std::vector<std::vector<cv::Point>> data;
};

#endif // EDGES_H
