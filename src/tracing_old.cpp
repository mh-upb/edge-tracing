#include <chrono>
#include <ctime>
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

	// Trace edges
	EdgeProcessor edgeProcessor;

	auto t1 = std::clock();
	auto ct1 = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < 1; ++i)
	{
		edgeProcessor.traceEdges(img);
	}

	auto ct2 = std::chrono::high_resolution_clock::now();
	auto t2 = std::clock();

	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(ct2 - ct1);
	std::cout << "It took me " << time_span.count() << " seconds. " << std::chrono::high_resolution_clock::is_steady << "\n";
	std::cout << "traceEdges took: " << double(t2-t1) / CLOCKS_PER_SEC << " s" << std::endl;

	edgeProcessor.printEdgeInfos(img);

	// Pass an image via the command line via ./build/tracing <input image>

	// Examples
	// ===========================================

	/*
	bool changes = true;
	while (changes)
	{
		changes = edgeProcessor.removeEdgesShorterThan(30);
	}
	*/

	//edgeProcessor.removeEdgesShorterThan(6, true, true, true);
	//edgeProcessor.removeEdgesShorterThan(30);

	//edgeProcessor.threePointEdgesToClusters();
	//edgeProcessor.removeEdgesShorterThan(10);
	//edgeProcessor.connectEdgesInClusters(5, 40.0, 1.0, 0);

	//edgeProcessor.removeEdgesShorterThan(30);
	//edgeProcessor.removeEdgesShorterThan(3, true, true, true);

	//
	//edgeProcessor.threePointEdgesToClusters();
	//edgeProcessor.connectEdgesInClusters(5, 40.0, 1.0, 0);
	//edgeProcessor.closeEdgesInClusters();
	//

	//edgeProcessor.threePointEdgesToClusters();
	edgeProcessor.connectEdgesInClusters(3, 60.0);
	//edgeProcessor.connectEdgesInClusters(7, 179.0);
	//edgeProcessor.reverseAllEdges();
	//edgeProcessor.closeEdgesInClusters();
	//edgeProcessor.connectEdgesInClusters(5, 80.0, 1.0, 15.0);
	//edgeProcessor.bridgeEdgeGaps(5, 40.0, 25, 1.0);
	edgeProcessor.cleanUpEdges(); // Remove empty edges from vector and adjust edgeIdMap for continuous edgeIds
	edgeProcessor.printEdgeInfos(img);
	//edgeProcessor.resetClusters(img); // Re-identify clusters from traced edges after merges to detect new clusters
	//edgeProcessor.closeEdgesInClusters();
	//edgeProcessor.connectEdgesInTwoEdgeClusters(false, true);
	//edgeProcessor.removeEdgesLongerThan(1, false, false, true);

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
