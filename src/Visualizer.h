#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "EdgeMap.h"
#include "Edges.h"

class Visualizer
{
public:
	/** Write SVG visualization of the overall result.
	 * 	@img			Input image (used to adopt the height and width of the output).
	 * 	@edges 			Internal class which holds the traced edges.
	 * 	@edgeMap		Internal class to represent the edgeIdMap and edgeClusterMap.
	 *  @showInput		Draw pixels of input image.
	 */
	static void saveResultAsSVG(cv::Mat &img, const Edges &edges, const EdgeMap &edgeMap, bool showInput=true);

	/** Write SVG visualization based on edgeIdMap.
	 * 	@img			Input image (used to adopt the height and width of the output).
	 * 	@edgeMap		Internal class to represent the edgeIdMap and edgeClusterMap.
	 *  @showInput		Draw pixels of input image.
	 */
	static void saveEdgeIdMapAsSVG(cv::Mat &img, const EdgeMap &edgeMap, bool showInput=true);

	/** Write a binary edge image based on the passed edges.
	 * 	@img			Input image (used to adopt the height and width of the output).
	 * 	@edges 			Internal class which holds the traced edges.
	 */
	static void saveEdgesAsBinaryImage(cv::Mat &img, const Edges &edges);
};

#endif // VISUALIZER_H
