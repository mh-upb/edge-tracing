#include <iostream>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

// Classes for tracing and result visualization
#include "EdgeProcessor.h"
#include "Visualizer.h"

int main(int argc, const char *argv[])
{
	cv::Mat img;

	if (argc > 1)
	{
		std::cout << "Read Image..." << argv[1] << std::endl;
		img = cv::imread(argv[1], 0);

		if (!img.data)
		{
			std::cout << "Could not find or open image. Quit." << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << argc << argv[0] << "Invalid number of arguments. Quit." << std::endl;
		return -1;
	}

	// Identify ambiguities and trace edges
	EdgeProcessor edgeProcessor;
	edgeProcessor.traceEdges(img);

	// Print status information
	edgeProcessor.printEdgeInfos(img);

	// === POSTPROCESSING
	// Example: frogfly.png - Uncomment the following lines - See /docs/examples-with-code.md for further examples
	/*
	edgeProcessor.threePointEdgesToClusters();
	edgeProcessor.connectEdgesInClusters(5, 40.0);
	*/
	// ===

	edgeProcessor.cleanUpEdges(); // Remove empty edges from vector and adjust edgeIdMap for continuous edgeIds (optional)

	// Get read-only references to internal edges and edgeIdMap
	const Edges &edges = edgeProcessor.getEdges();
	const EdgeMap &edgeMap = edgeProcessor.getEdgeIdMap();

	// Visualization of the overall result and edgeIdMap
	// Add these lines after each step to view intermediate results
	Visualizer::saveResultAsSVG(img, edges, edgeMap);
	Visualizer::saveEdgeIdMapAsSVG(img, edgeMap);
	//Visualizer::saveEdgesAsBinaryImage(img, edges);

	std::cout << "Finished." << std::endl;
	return 0;
}
