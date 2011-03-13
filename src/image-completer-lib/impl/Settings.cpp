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

#include "Pch.h"
#include "PriorityBpSettings.h"

#include "tech/MathUtils.h"
#include "tech/StrUtils.h"

#include "PriorityBpHost.h"
#include "Settings.h"

#include "tech/DbgMem.h"

//
// LfnIc::Settings implementation
//
const int LfnIc::Settings::LATTICE_GAP_MIN = 4;
const float LfnIc::Settings::PATCH_TO_LATTICE_RATIO = 2.0f;
const int LfnIc::Settings::PATCH_SIDE_MIN = LATTICE_GAP_MIN * PATCH_TO_LATTICE_RATIO;
const int64 LfnIc::Settings::PATCH_PIXELS_MAX = (IMAGE_DIMENSION_MAX * PATCH_TO_LATTICE_RATIO) * (IMAGE_DIMENSION_MAX * PATCH_TO_LATTICE_RATIO);

const LfnIc::Belief LfnIc::Settings::CONFIDENCE_BELIEF_THRESHOLD_MIN = BELIEF_MIN;
const LfnIc::Belief LfnIc::Settings::CONFIDENCE_BELIEF_THRESHOLD_MAX = BELIEF_MAX;

const LfnIc::Belief LfnIc::Settings::PRUNE_BELIEF_THRESHOLD_MIN = BELIEF_MIN;
const LfnIc::Belief LfnIc::Settings::PRUNE_BELIEF_THRESHOLD_MAX = BELIEF_MAX;

const LfnIc::Energy LfnIc::Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MIN = ENERGY_MIN;
const LfnIc::Energy LfnIc::Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MAX = ENERGY_MAX;

const int LfnIc::Settings::POST_PRUNE_LABEL_MIN = 3;
const int LfnIc::Settings::POST_PRUNE_LABEL_MAX = INT_MAX;

// Internal construct helper
static void SettingsConstructHelper(LfnIc::Settings& out, int latticeGapX, int latticeGapY)
{
#if _DEBUG
	const LfnIc::Energy maxRgbComponentEnergy = 255;
	const LfnIc::Energy maxREnergy = maxRgbComponentEnergy;
	const LfnIc::Energy maxGEnergy = maxRgbComponentEnergy;
	const LfnIc::Energy maxBEnergy = maxRgbComponentEnergy;
	const LfnIc::Energy maxPixelEnergy = (maxREnergy * maxREnergy) + (maxGEnergy * maxGEnergy) + (maxBEnergy * maxBEnergy);
	const LfnIc::Energy maxPatchPixels = LfnIc::ENERGY_MAX / maxPixelEnergy;
	wxASSERT(LfnIc::Settings::PATCH_PIXELS_MAX < maxPatchPixels);
#endif

	out.debugLowResolutionPasses = false;
	out.lowResolutionPassesMax = 0;
	out.numIterations = LfnIc::Settings::NUM_ITERATIONS_DEFAULT;

	out.latticeGapX = latticeGapX;
	out.latticeGapY = latticeGapY;
	out.patchWidth  = out.latticeGapX * LfnIc::Settings::PATCH_TO_LATTICE_RATIO;
	out.patchHeight = out.latticeGapY * LfnIc::Settings::PATCH_TO_LATTICE_RATIO;

	// Based on the patch size, 3 components (rgb), and an
	// acceptable component difference (between 0.0 and 1.0),
	// calculate an acceptable, mediocre ssd to base other
	// settings on.
	const float ssd0ComponentAcceptableDiff = 0.15f;
	const LfnIc::Energy ssd0ComponentDiff = LfnIc::Energy(ssd0ComponentAcceptableDiff * 255.0f);
	const LfnIc::Energy ssd0ComponentDiffSq = ssd0ComponentDiff * ssd0ComponentDiff;
	const LfnIc::Energy ssd0RgbDiffSq = 3 * ssd0ComponentDiffSq;
	const LfnIc::Energy ssd0 = out.patchWidth * out.patchHeight * ssd0RgbDiffSq;

	out.confidenceBeliefThreshold = -ssd0;
	out.pruneBeliefThreshold = -ssd0 * LfnIc::Energy(2);
	out.pruneEnergySimilarThreshold = ssd0 / LfnIc::Energy(2);
	out.postPruneLabelsMin = LfnIc::Settings::POST_PRUNE_LABEL_MIN;
	out.postPruneLabelsMax = LfnIc::Settings::POST_PRUNE_LABEL_MIN * 4;
	out.compositorPatchType = LfnIc::CompositorPatchTypeDefault;
	out.compositorPatchBlender = LfnIc::CompositorPatchBlenderDefault;
}

void LfnIc::SettingsConstruct(Settings& out)
{
	SettingsConstructHelper(out, LfnIc::Settings::LATTICE_GAP_MIN, LfnIc::Settings::LATTICE_GAP_MIN);
}

void LfnIc::SettingsConstruct(Settings& out, const Image& inputImage)
{
	// Calculate a suggested lattice gap x and y
	const int imageSizeAtGapMin = 100;
	const float widthScale = float(inputImage.GetWidth()) / float(imageSizeAtGapMin);
	const float heightScale = float(inputImage.GetHeight()) / float(imageSizeAtGapMin);
	int latticeGapX = std::max(LfnTech::Lerp(0, Settings::LATTICE_GAP_MIN, widthScale), Settings::LATTICE_GAP_MIN);
	int latticeGapY = std::max(LfnTech::Lerp(0, Settings::LATTICE_GAP_MIN, heightScale), Settings::LATTICE_GAP_MIN);

	// If calculated gaps violate the maximum gap ratio, shink one of the
	// components.
	const float gapRatioMax = 2.0f;
	const float gapRatio = float(latticeGapX) / float(latticeGapY);
	if (gapRatio > gapRatioMax)
	{
		latticeGapX = latticeGapY * gapRatioMax;
	}
	if ((1.0f / gapRatio) > gapRatioMax)
	{
		latticeGapY = latticeGapX * gapRatioMax;
	}

	wxASSERT(latticeGapX >= Settings::LATTICE_GAP_MIN);
	wxASSERT(latticeGapY >= Settings::LATTICE_GAP_MIN);
	wxASSERT(latticeGapX * latticeGapY <= Settings::PATCH_PIXELS_MAX);

	SettingsConstructHelper(out, latticeGapX, latticeGapY);
}

void LfnIc::SettingsConstruct(Settings& out, int latticeGapX, int latticeGapY)
{
	SettingsConstructHelper(out, latticeGapX, latticeGapY);
}

bool LfnIc::AreSettingsValid(const Settings& settings, SettingsInvalidMemberHandler* handlerPtr)
{
	// Implement a "null" InvalidMemberHandler, and use it if the passed in
	// handler is NULL in order to avoid checking the handler pointer for each
	// member.
	class SettingsInvalidMemberHandlerNull : public SettingsInvalidMemberHandler
	{
	public:
		virtual void OnInvalidMemberDetected(const Settings& settings, int memberOffset, const char* message) {}
	};

	SettingsInvalidMemberHandlerNull handlerNull;
	SettingsInvalidMemberHandler& handler = handlerPtr ? *handlerPtr : handlerNull;

	bool valid = true;

	// Helper macros:
#define VALIDATE_NOT_LESS_THAN(member, min) \
	if (!(settings.member >= min)) \
	{ \
		valid = false; \
		handler.OnInvalidMemberDetected(settings, offsetof(Settings, member), LfnTech::Str::Format("(%I64d) is less than %I64d", int64(settings.member), int64(min)).c_str()); \
	}

#define VALIDATE_NOT_GREATER_THAN(member, max) \
	if (!(settings.member <= max)) \
	{ \
		valid = false; \
		handler.OnInvalidMemberDetected(settings, offsetof(Settings, member), LfnTech::Str::Format("(%I64d) is less than %I64d", int64(settings.member), int64(max)).c_str()); \
	}

#define VALIDATE_IN_RANGE(member, min, max) \
	VALIDATE_NOT_LESS_THAN(member, min) else VALIDATE_NOT_GREATER_THAN(member, max)

	// Perform the validation:
	VALIDATE_NOT_LESS_THAN(lowResolutionPassesMax, Settings::LOW_RESOLUTION_PASSES_AUTO);
	VALIDATE_NOT_LESS_THAN(numIterations, 1);

	VALIDATE_NOT_LESS_THAN(latticeGapX, Settings::LATTICE_GAP_MIN);
	VALIDATE_NOT_LESS_THAN(latticeGapY, Settings::LATTICE_GAP_MIN);

	VALIDATE_NOT_LESS_THAN(patchWidth, Settings::PATCH_SIDE_MIN);
	VALIDATE_NOT_LESS_THAN(patchHeight, Settings::PATCH_SIDE_MIN);

	if (!(settings.patchWidth * settings.patchHeight <= Settings::PATCH_PIXELS_MAX))
	{
		valid = false;
		const int memberOffset = (settings.patchWidth > settings.patchHeight) ? offsetof(Settings, patchWidth) : offsetof(Settings, patchHeight);
		handler.OnInvalidMemberDetected(settings, memberOffset, LfnTech::Str::Format("is yielding too large of a patch (%d * %d > %I64d)", settings.patchWidth, settings.patchHeight, Settings::PATCH_PIXELS_MAX).c_str());
	}

	VALIDATE_IN_RANGE(confidenceBeliefThreshold, Settings::CONFIDENCE_BELIEF_THRESHOLD_MIN, Settings::CONFIDENCE_BELIEF_THRESHOLD_MAX);
	VALIDATE_IN_RANGE(pruneBeliefThreshold, Settings::PRUNE_BELIEF_THRESHOLD_MIN, Settings::PRUNE_BELIEF_THRESHOLD_MAX);
	VALIDATE_IN_RANGE(pruneEnergySimilarThreshold, Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MIN, Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MAX);

	VALIDATE_IN_RANGE(postPruneLabelsMin, Settings::POST_PRUNE_LABEL_MIN, Settings::POST_PRUNE_LABEL_MAX);
	VALIDATE_IN_RANGE(postPruneLabelsMax, Settings::POST_PRUNE_LABEL_MIN, Settings::POST_PRUNE_LABEL_MAX);

	if (settings.compositorPatchType <= CompositorPatchTypeInvalid || settings.compositorPatchType >= CompositorPatchTypeNum)
	{
		valid = false;
		handler.OnInvalidMemberDetected(settings, offsetof(Settings, compositorPatchType), "is invalid");
	}

	if (settings.compositorPatchBlender <= CompositorPatchBlenderInvalid || settings.compositorPatchBlender >= CompositorPatchBlenderNum)
	{
		valid = false;
		handler.OnInvalidMemberDetected(settings, offsetof(Settings, compositorPatchBlender), "is invalid");
	}

	return valid;
}

//
// SettingsScalable implementation
//
LfnIc::SettingsScalable::SettingsScalable(const Settings& settings) :
Settings(settings), // copy
m_depth(0)
{
}

void LfnIc::SettingsScalable::ScaleUp()
{
	// Copy the last saved resolution settings and pop up.
	wxASSERT(m_depth > 0);
	Settings& thisSettings = *this;
	thisSettings = m_resolutions[--m_depth];
	m_resolutions.pop_back();
}

void LfnIc::SettingsScalable::ScaleDown()
{
	// Save the current resolution settings.
	wxASSERT(static_cast<unsigned int>(m_depth) == m_resolutions.size());
	m_resolutions.push_back(*this);

	++m_depth;

	// Reduce the lattice gap by half.
	latticeGapX /= 2;
	latticeGapY /= 2;

	// Recompute the patch size, rather than divide by two, to avoid even/odd issues.
	patchWidth = latticeGapX * PATCH_TO_LATTICE_RATIO;
	patchHeight = latticeGapY * PATCH_TO_LATTICE_RATIO;

	// NUM_NODE_LABELS_KEPT_MULTIPLIER is used so that lower resolutions keep
	// more labels per node. This is desired because error caused by the
	// reduced data is propagated to higher resolutions. Keeping more labels
	// at lower resolutions allows for better error recovery at the next
	// higher resolution.
	const int NUM_NODE_LABELS_KEPT_MULTIPLIER = 4;
	postPruneLabelsMin *= NUM_NODE_LABELS_KEPT_MULTIPLIER;
	postPruneLabelsMax *= NUM_NODE_LABELS_KEPT_MULTIPLIER;
}

int LfnIc::SettingsScalable::GetScaleDepth() const
{
	return m_depth;
}
