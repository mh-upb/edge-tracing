#include "Visualizer.h"

#include <cstdio>
#include <iostream>

namespace
{
	constexpr bool MARK_START_AND_END_POINTS = true;
	constexpr bool MARK_EDGEID_AND_INDICES = false;
	constexpr bool MARK_COORDINATES = false;
	constexpr bool MARK_AMBIGUITY_POINTS = true;

	/** Generate one color for each edge.
	 *  @numberOfValues		Number of RGB values to generate.
	 */
	std::vector<cv::Scalar> generateRgbValues(int numberOfValues)
	{
		cv::RNG rng(31231); // OpenCV random number generator

		std::vector<cv::Scalar> rgbValues;
		rgbValues.reserve(numberOfValues);

		for (int i = 0; i < numberOfValues; i++)
		{
			rgbValues.emplace_back(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			// std::cout << rgbValues.at(i)->val[0] << std::endl;
		}

		// Overwrite first value with color of choice
		rgbValues[0] = cv::Scalar(170, 226, 226);

		// std::cout << "Generated " << rgbValues.size() << " random RGB values." << std::endl;
		return rgbValues;
	}

} // end namespace

void Visualizer::saveResultAsSVG(cv::Mat &img, const Edges &edges, const EdgeMap &edgeMap, bool showInput)
{
	// Generate a color for each edge
	std::vector<cv::Scalar> rgbValues = generateRgbValues(edges.size());

	// Open new SVG file
	FILE *file;
	file = fopen("./output/tracedEdges.svg", "w");

	if (!file)
	{
		std::cerr << "Failed to write tracedEdges.svg. Check folder structure." << std::endl;
		return;
	}

	// Setup SVG canvas
	fprintf(file, "<svg width=\"%d\" height=\"%d\">\n", img.cols, img.rows);
	fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"black\" />\n");

	// Draw pixels of input image (for reference, later overlaid by the traced pixels)
	if (showInput)
	{
		for (int y = 0; y < img.rows; y++)
		{
			for (int x = 0; x < img.cols; x++)
			{
				if (img.at<uchar>(y, x) > 0)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" fill=\"gray\" />\n", x, y);
				}
			}
		}
	}

	// Get all traced edges
	const std::vector<std::vector<cv::Point>> &edgesData = edges.getEdges();

	// Draw edges exclusively based on edgesData
	for (int i = 0; i < (int)edgesData.size(); i++)
	{
		for (int j = 0; j < (int)edgesData[i].size(); j++)
		{
			// Position
			int x = edgesData[i][j].x;
			int y = edgesData[i][j].y;

			// Color
			int r = (int)rgbValues.at(i).val[0];
			int g = (int)rgbValues.at(i).val[1];
			int b = (int)rgbValues.at(i).val[2];

			fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y, r, g, b);

			if constexpr (MARK_START_AND_END_POINTS)
			{
				// Mark start point
				if (j == 0)
				{
					fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.075\" stroke=\"grey\" stroke-width=\"0.05\" fill=\"none\" />\n", x + 0.5, y + 0.5);
				}

				// Mark end point
				if (j == ((int)edgesData[i].size() - 1))
				{
					fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.1\" fill=\"grey\" />\n", x + 0.5, y + 0.5);
				}
			}

			// Mark edgeId and index of point in that edge in the format [edgeId, index]
			if constexpr (MARK_EDGEID_AND_INDICES)
			{
				fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.15, i, j);
			}

			// Mark coordinates
			if constexpr (MARK_COORDINATES)
			{
				fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.95, x, y);
			}
		}
	}

	// Draw borders around cluster points and shared edges in cluster points
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			const std::vector<int> &edgeIds = edgeMap.getEdgeIds(x, y);

			if (edgeIds.size() > 1)
			{
				double scale = (double)1 / edgeIds.size();

				for (int i = 0; i < (int)edgeIds.size(); i++)
				{
					// Color
					int j = (int)edgeIds[i]; // current edgeId
					int r = (int)rgbValues.at(j).val[0];
					int g = (int)rgbValues.at(j).val[1];
					int b = (int)rgbValues.at(j).val[2];

					fprintf(file, "<rect x=\"%d\" y=\"%f\" width=\"1\" height=\"%f\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y + scale * i, scale, r, g, b);

					// Print edge number
					const auto& tempEdge = edgesData[j];
					int index = std::find(tempEdge.begin(), tempEdge.end(), cv::Point(x, y)) - tempEdge.begin();

					// Mark edgeId and index of point in that edge in the format [edgeId, index]
					if (MARK_EDGEID_AND_INDICES)
					{
						fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.15 + scale * i, j, index);
					}

					// Mark start point
					if (MARK_START_AND_END_POINTS && index == 0)
					{
						fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.075\" stroke=\"grey\" stroke-width=\"0.05\" fill=\"none\" />\n", x + 0.5, y + (0.5 + i) * scale);
					}

					// Mark end point
					if (MARK_START_AND_END_POINTS && index == (int)tempEdge.size()-1)
					{
						fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.1\" fill=\"grey\" />\n", x + 0.5, y + (0.5 + i) * scale);
					}
				}
			}

			if (MARK_AMBIGUITY_POINTS)
			{
				// Draw borders around cluster points
				int numberOfClusterPoints = edgeMap.getNumberOfClusterPoints(x, y);

				if (numberOfClusterPoints == 1)
				{ 	// You can specify different colors for single-pixel ambiguities and multi-pixel ambiguities here (the last three values = RGB)
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", x, y, 255, 0, 0);
				}
				else if (numberOfClusterPoints > 1)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", x, y, 255, 0, 0);
				}
			}
		}
	}

	// End and close SVG file
	fprintf(file, "</svg>");
	fclose(file);

	std::cout << "File tracedEdges.svg written.\n";
}

void Visualizer::saveEdgeIdMapAsSVG(cv::Mat &img, const EdgeMap &edgeMap, bool showInput)
{
	int rows = edgeMap.getRows();
	int cols = edgeMap.getCols();

	// Generate a color for each edge
	std::vector<cv::Scalar> rgbValues = generateRgbValues(edgeMap.getMaxEdgeId() + 1);

	// Open new SVG file
	FILE *file;
	file = fopen("./output/edgeIdMap.svg", "w");

	if (!file)
	{
		std::cerr << "Failed to write edgeIdMap.svg. Check folder structure." << std::endl;
		return;
	}

	// Setup SVG canvas
	fprintf(file, "<svg width=\"%d\" height=\"%d\">\n", cols, rows);
	fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"black\" />\n");

	// Draw pixels of input image (for reference, later overlaid by the traced pixels)
	if (showInput)
	{
		for (int y = 0; y < img.rows; y++)
		{
			for (int x = 0; x < img.cols; x++)
			{
				if (img.at<uchar>(y, x) > 0)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" fill=\"gray\" />\n", x, y);
				}
			}
		}
	}

	// Draw edgeIds
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			const std::vector<int> &edgeIds = edgeMap.getEdgeIds(x, y);

			// At most positions edgeIds.size() is 0 (no edge), loop is skipped then
			for (int i = 0; i < (int)edgeIds.size(); i++)
			{
				double scale = 1.0 / edgeIds.size();
				// Color
				int r = (int)rgbValues.at(edgeIds[i]).val[0];
				int g = (int)rgbValues.at(edgeIds[i]).val[1];
				int b = (int)rgbValues.at(edgeIds[i]).val[2];

				fprintf(file, "<rect x=\"%d\" y=\"%f\" width=\"1\" height=\"%f\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y + scale * i, scale, r, g, b);
			}

			// Mark coordinates of each edge pixel
			if (MARK_COORDINATES && edgeIds.size() > 0)
			{
				fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.95, x, y);
			}
		}
	}

	// Draw additional markers for ambiguous and cluster points
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			const auto& clusterPoints = edgeMap.getClusterPoints(x, y);

			if (clusterPoints.size() == 1)
			{
				fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", x, y, 255, 0, 0);
			}
			else if (clusterPoints.size() > 1)
			{
				for (const auto& p : clusterPoints)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", p.x, p.y, 255, 0, 0);
				}
			}
		}
	}

	// End and close SVG file
	fprintf(file, "</svg>");
	fclose(file);

	std::cout << "File edgeIdMap.svg written.\n";
}

void Visualizer::saveEdgesAsBinaryImage(cv::Mat &img, const Edges &edges)
{
	cv::Mat blank_image = cv::Mat::zeros(img.size(), CV_8UC1);

	// Get all traced edges
	const std::vector<std::vector<cv::Point>> &edgesData = edges.getEdges();

	// Draw edges exclusively based on edgesData
	for (int i = 0; i < (int)edgesData.size(); i++)
	{
		for (int j = 0; j < (int)edgesData[i].size(); j++)
		{
			// Position
			int x = edgesData[i][j].x;
			int y = edgesData[i][j].y;

			blank_image.at<uchar>(y, x) = 255;
		}
	}

	// Write the output image
	bool writeSuccess = cv::imwrite("./output/binary_edges.png", blank_image);

	if (writeSuccess)
	{
		std::cout << "File binary_edges.png written.\n";
	}
	else
	{
		std::cerr << "Failed to write binary_edges.png." << std::endl;
	}
}
