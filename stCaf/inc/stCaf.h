/*
 * stCore.h
 *
 *  Created on: 28 Apr 2012
 *      Author: benedictpaten
 */

#ifndef STCAF_H_
#define STCAF_H_

#include "sonLib.h"
#include "stPinchGraphs.h"
#include "stPinchIterator.h"
#include "stCactusGraphs.h"
#include "cactus.h"

///////////////////////////////////////////////////////////////////////////
// Setup the pinch graph from a cactus graph
///////////////////////////////////////////////////////////////////////////

/*
 * Create a pinch graph from a flower containing no blocks. Each thread is given the name of its 5' cap,
 * and stubs are represented by length 1 blocks.
 */
stPinchThreadSet *stCaf_constructEmptyPinchGraph(Flower *flower);

/*
 * Initialises the flower for filling out and creates a pinch graph.
 */
stPinchThreadSet *stCaf_setup(Flower *flower);

///////////////////////////////////////////////////////////////////////////
// Annealing fuctions -- adding alignments to pinch graph
///////////////////////////////////////////////////////////////////////////

/*
 * Add the set of alignments, represented as pinches, to the graph.
 */
void stCaf_anneal(stPinchThreadSet *threadSet, stPinchIterator *pinchIterator);

/*
 * Add the set of alignments, represented as pinches, to the graph, allowing alignments only between segments in the same component.
 */
void stCaf_annealBetweenAdjacencyComponents(stPinchThreadSet *threadSet, stPinchIterator *pinchIterator);

///////////////////////////////////////////////////////////////////////////
// Melting fuctions -- removing alignments from the pinch graph
///////////////////////////////////////////////////////////////////////////

/*
 * Removes homologies from the graph.
 */
void stCaf_melt(Flower *flower, stPinchThreadSet *threadSet, bool blockFilterfn(stPinchBlock *), int32_t blockEndTrim, int64_t minimumChainLength);

///////////////////////////////////////////////////////////////////////////
// Pinch graph to cactus graph
///////////////////////////////////////////////////////////////////////////

/*
 * Functions which converts a pinch graph into a cactus graph.
 */
stCactusGraph *stCaf_getCactusGraphForThreadSet(Flower *flower, stPinchThreadSet *threadSet, stCactusNode **startCactusNode, stList **deadEndComponent,  bool attachEndsInFlower);

///////////////////////////////////////////////////////////////////////////
// Finishing: Converting a pinch graph into the flower hierarchy
///////////////////////////////////////////////////////////////////////////

/*
 * Add the adjacencies between caps of a flower.
 * All ends must be in the flower before this function is called.
 */
void stCaf_addAdjacencies(Flower *flower);

/*
 * Takes a pinch graph for a flower and adds the alignments it contains back to the flower as a cactus.
 */
void stCaf_finish(Flower *flower, stPinchThreadSet *threadSet);

#endif /* STCAF_H_ */