# Contents of this Folder

Visualizations of the results will be saved in this folder.

- `tracedEdges.svg`: Overall result with the identified ambiguities and traced edges.
- `edgeIdMap.svg`: A visualization of the *edgeIdMap*, where each color corresponds to a different *edgeId*.

Visualizations can be written at any step (such as before and after postprocessing). Use an SVG editor such as [Inkscape](https://inkscape.org/) to zoom into details. Activate the flags at the top of `Visualizer.cpp` to enable writing the *edgeIds* and other information to the SVG.
