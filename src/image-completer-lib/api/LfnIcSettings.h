//
// Copyright 2010, Darren Lafreniere
// <http://www.lafarren.com/image-completer/>
//
// This file is part of lafarren.com's Image Completer.
//
// Image Completer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Image Completer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Image Completer, named License.txt. If not, see
// <http://www.gnu.org/licenses/>.
//

#ifndef LFN_IC_SETTINGS_H
#define LFN_IC_SETTINGS_H

#include "tech/Core.h"
#include "LfnIcTypes.h"

namespace LfnIc
{
	// Forward declarations
	class Image;

	///
	/// Image completion settings; provided by the client code that is using
	/// the image completer.
	///
	struct Settings
	{
		///
		/// Constants
		///
		static const int LOW_RESOLUTION_PASSES_AUTO = -1;
		static const int NUM_ITERATIONS_DEFAULT = 6;

		static const int IMAGE_DIMENSION_MAX = 32767;
		static const int IMAGE_WIDTH_MAX = IMAGE_DIMENSION_MAX;
		static const int IMAGE_HEIGHT_MAX = IMAGE_DIMENSION_MAX;

		static const int LATTICE_GAP_MIN;
		static const float PATCH_TO_LATTICE_RATIO;
		static const int PATCH_SIDE_MIN;
		static const int64 PATCH_PIXELS_MAX;

		static const Belief CONFIDENCE_BELIEF_THRESHOLD_MIN;
		static const Belief CONFIDENCE_BELIEF_THRESHOLD_MAX;

		static const Belief PRUNE_BELIEF_THRESHOLD_MIN;
		static const Belief PRUNE_BELIEF_THRESHOLD_MAX;

		static const Energy PRUNE_ENERGY_SIMILAR_THRESHOLD_MIN;
		static const Energy PRUNE_ENERGY_SIMILAR_THRESHOLD_MAX;

		static const int POST_PRUNE_LABEL_MIN;
		static const int POST_PRUNE_LABEL_MAX;

		//
		// Public data
		//

		/// If enabled, each low resolution pass will generate a separate
		/// output image.
		bool debugLowResolutionPasses;

		/// The number of low-resolution passes to take, or
		/// LOW_RESOLUTION_PASSES_AUTO to automatically determine when to
		/// stop. The more low resolution passes, the faster the completion
		/// will be at the expense of solution accuracy.
		int lowResolutionPassesMax;

		/// The number of priority-bp iterations to run.
		int numIterations;

		/// The gap between nodes in the Markov Random Field lattice. Both
		/// values must be >= LATTICE_GAP_MIN.
		int latticeGapX;
		int latticeGapY;

		/// The dimensions of the patches used to complete the unknown region
		/// of the image. Both dimensions must be >= PATCH_SIDE_MIN, and
		/// (patchWidth * patchHeight) must be <= PATCH_PIXELS_MAX.
		int patchWidth;
		int patchHeight;

		/// The threshold applied to a node's labels' beliefs, above which labels
		/// are considered potential candidates for completion. The fewer labels
		/// with beliefs above this threshold, the more confident the node is
		/// about finding a good candidate, thus the node is given higher
		/// priority during execution.
		Belief confidenceBeliefThreshold;

		/// The threshold applied to a node's labels' beliefs, below which labels
		/// are pruned away from being candidates for that node.
		Belief pruneBeliefThreshold;

		/// The threshold applied to the energy between two labels, one of which
		/// has been spared pruning, below which the other label is considered
		/// too similiar to the first, and is thus pruned.
		Energy pruneEnergySimilarThreshold;

		/// The min and max numbers of labels a node should after have pruning.
		/// Both of these values must be >= POST_PRUNE_LABEL_MIN and
		/// <= POST_PRUNE_LABEL_MAX.
		int postPruneLabelsMin;
		int postPruneLabelsMax;

		/// Compositor settings. See these enums for more info.
		CompositorPatchType compositorPatchType;
		CompositorPatchBlender compositorPatchBlender;
	};

	/// Constructs appropriate default settings given a variety of inputs
	EXPORT void SettingsConstruct(Settings& out);
	EXPORT void SettingsConstruct(Settings& out, const Image& inputImage);
	EXPORT void SettingsConstruct(Settings& out, int latticeGapX, int latticeGapY);

	/// Validity tests; if invalid and a InvalidMemberHandler instance is
	/// supplied, OnError will be called for each invalid member.
	/// Can be implemented and passed to IsValid() to catch individual
	/// member errors.
	class SettingsInvalidMemberHandler
	{
	public:
		/// memberOffset:
		/// Offset of a member variable that was considered invalid. E.g.,
		/// if latticeGapX was invalid, then memberOffset will ==
		/// offsetof(LfnIc::Settings, latticeGapX).
		///
		/// message:
		/// Why it was considered invalid.
		virtual void OnInvalidMemberDetected(const Settings& settings, int memberOffset, const char* message) = 0;
	};

    /// Determine if settings are valid.
	EXPORT bool AreSettingsValid(const Settings& settings, SettingsInvalidMemberHandler* handlerPtr = NULL);
}

#endif
