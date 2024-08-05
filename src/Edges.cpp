#include "Edges.h"
#include <iostream>

void Edges::clear()
{
	data.clear();
}

void Edges::pushBack(std::vector<cv::Point> edge)
{
	data.push_back(edge);
}

void Edges::insert(int edgeId, std::vector<cv::Point> edge)
{
	data.insert(data.begin() + edgeId, edge);
}

void Edges::overwrite(int edgeId, std::vector<cv::Point> edge)
{
	data[edgeId] = edge;
}

void Edges::popBack()
{
	data.pop_back();
}

size_t Edges::size() const
{
	return data.size();
}

void Edges::eraseEmptyEdges()
{
	std::vector<cv::Point> emptyEdge;
	data.erase(std::remove(data.begin(), data.end(), emptyEdge), data.end());
}

void Edges::clearEdge(int edgeId)
{
	data[edgeId].clear();
}

const std::vector<cv::Point> &Edges::getEdge(int index) const
{
	return data[index];
}

const std::vector<std::vector<cv::Point>> &Edges::getEdges() const
{
	return data;
}

int Edges::getEdgeId(std::vector<cv::Point> edge)
{
	int edgeId = 0;

	for (const auto& edgeVector : data)
	{
		if (edgeVector == edge)
		{
			break;
		}

		edgeId++;
	}

	return edgeId;
}

cv::Point Edges::getStartPoint(int edgeId) const
{
	return data[edgeId].front();
}

cv::Point Edges::getEndPoint(int edgeId) const
{
	return data[edgeId].back();
}

size_t Edges::getEdgeSize(int edgeId) const
{
	return data[edgeId].size();
}

void Edges::eraseEdge(std::vector<cv::Point> edge)
{
	data.erase(std::find(data.begin(), data.end(), edge));
}

std::vector<cv::Point> Edges::getPointsAlongEdgeFromPoint(int edgeId, cv::Point point, size_t numberPixels)
{
	std::vector<cv::Point> edge = data[edgeId];
	std::vector<cv::Point> nPoints;

	if (edge.front() == point)
	{
		for (size_t i=0; i < std::min(numberPixels, edge.size()); i++)
		{
			nPoints.push_back(edge[i]);
		}
	}
	else if (edge.back() == point)
	{
		for (int i=edge.size()-1; i >= std::max(static_cast<int>(edge.size()-numberPixels), 0); i--)
		{
			nPoints.push_back(edge[i]);
		}
	}

	return nPoints;
}

bool Edges::isClosed(int edgeId)
{
	// 1.5 replaces sqrt(2), every non 8-neighbor has a distance > sqrt(2)
	if ((cv::norm(getStartPoint(edgeId) - getEndPoint(edgeId)) < 1.5) && getEdgeSize(edgeId) >= 4)
	{
		return true;
	}

	return false;
}

bool Edges::isThreePixelL(int edgeId)
{
	// 1.5 replaces sqrt(2), every non 8-neighbor has a distance > sqrt(2)
	if ((cv::norm(getStartPoint(edgeId) - getEndPoint(edgeId)) < 1.5) && getEdgeSize(edgeId) == 3)
	{
		return true;
	}

	return false;
}

void Edges::reverseAll()
{
    for (auto& edge : data)
    {
        std::reverse(edge.begin(), edge.end());
    }
}
