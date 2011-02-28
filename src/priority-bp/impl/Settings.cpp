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

#include "PriorityBpHost.h"
#include "Settings.h"

#include "tech/DbgMem.h"

//
// Settings implementation
//
const int Settings::LATTICE_GAP_MIN = 4;
const float Settings::PATCH_TO_LATTICE_RATIO = 2.0f;
const int Settings::PATCH_SIDE_MIN = LATTICE_GAP_MIN * PATCH_TO_LATTICE_RATIO;
const int64 Settings::PATCH_PIXELS_MAX = (IMAGE_DIMENSION_MAX * PATCH_TO_LATTICE_RATIO) * (IMAGE_DIMENSION_MAX * PATCH_TO_LATTICE_RATIO);

const Belief Settings::CONFIDENCE_BELIEF_THRESHOLD_MIN = BELIEF_MIN;
const Belief Settings::CONFIDENCE_BELIEF_THRESHOLD_MAX = BELIEF_MAX;

const Belief Settings::PRUNE_BELIEF_THRESHOLD_MIN = BELIEF_MIN;
const Belief Settings::PRUNE_BELIEF_THRESHOLD_MAX = BELIEF_MAX;

const Energy Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MIN = ENERGY_MIN;
const Energy Settings::PRUNE_ENERGY_SIMILAR_THRESHOLD_MAX = ENERGY_MAX;

const int Settings::POST_PRUNE_LABEL_MIN = 3;
const int Settings::POST_PRUNE_LABEL_MAX = INT_MAX;

static void Construct(Settings& settings, int latticeGapX, int latticeGapY)
{
#if _DEBUG
	const Energy maxRgbComponentEnergy = 255;
	const Energy maxREnergy = maxRgbComponentEnergy;
	const Energy maxGEnergy = maxRgbComponentEnergy;
	const Energy maxBEnergy = maxRgbComponentEnergy;
	const Energy maxPixelEnergy = (maxREnergy * maxREnergy) + (maxGEnergy * maxGEnergy) + (maxBEnergy * maxBEnergy);
	const Energy maxPatchPixels = ENERGY_MAX / maxPixelEnergy;
	wxASSERT(Settings::PATCH_PIXELS_MAX < maxPatchPixels);
#endif

	settings.debugLowResolutionPasses = false;
	settings.lowResolutionPassesMax = 0;
	settings.numIterations = Settings::NUM_ITERATIONS_DEFAULT;

	settings.latticeGapX = latticeGapX;
	settings.latticeGapY = latticeGapY;
	settings.patchWidth  = settings.latticeGapX * Settings::PATCH_TO_LATTICE_RATIO;
	settings.patchHeight = settings.latticeGapY * Settings::PATCH_TO_LATTICE_RATIO;

	// Based on the patch size, 3 components (rgb), and an
	// acceptable component difference (between 0.0 and 1.0),
	// calculate an acceptable, mediocre ssd to base other
	// settings on.
	const float ssd0ComponentAcceptableDiff = 0.15f;
	const Energy ssd0ComponentDiff = Energy(ssd0ComponentAcceptableDiff * 255.0f);
	const Energy ssd0ComponentDiffSq = ssd0ComponentDiff * ssd0ComponentDiff;
	const Energy ssd0RgbDiffSq = 3 * ssd0ComponentDiffSq;
	const Energy ssd0 = settings.patchWidth * settings.patchHeight * ssd0RgbDiffSq;

	settings.confidenceBeliefThreshold = -ssd0;
	settings.pruneBeliefThreshold = -ssd0 * Energy(2);
	settings.pruneEnergySimilarThreshold = ssd0 / Energy(2);
	settings.postPruneLabelsMin = Settings::POST_PRUNE_LABEL_MIN;
	settings.postPruneLabelsMax = Settings::POST_PRUNE_LABEL_MIN * 4;
	settings.compositorPatchType = CompositorPatchTypeDefault;
	settings.compositorPatchBlender = CompositorPatchBlenderDefault;
}

Settings::Settings()
{
	Construct(*this, LATTICE_GAP_MIN, LATTICE_GAP_MIN);
}

Settings::Settings(const HostImage& inputImage)
{
	// Calculate a suggested lattice gap x and y
	const int imageSizeAtGapMin = 100;
	const float widthScale = float(inputImage.GetWidth()) / float(imageSizeAtGapMin);
	const float heightScale = float(inputImage.GetHeight()) / float(imageSizeAtGapMin);
	int latticeGapX = std::max(Lerp(0, LATTICE_GAP_MIN, widthScale), LATTICE_GAP_MIN);
	int latticeGapY = std::max(Lerp(0, LATTICE_GAP_MIN, heightScale), LATTICE_GAP_MIN);

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

	wxASSERT(latticeGapX >= LATTICE_GAP_MIN);
	wxASSERT(latticeGapY >= LATTICE_GAP_MIN);
	wxASSERT(latticeGapX * latticeGapY <= PATCH_PIXELS_MAX);

	Construct(*this, latticeGapX, latticeGapY);
}

Settings::Settings(int latticeGapX, int latticeGapY)
{
	Construct(*this, latticeGapX, latticeGapY);
}

bool Settings::IsValid(InvalidMemberHandler* invalidMemberHandler) const
{
	// Implement a "null" InvalidMemberHandler, and use it if the passed in
	// handler is NULL in order to avoid checking the handler pointer for each
	// member.
	class InvalidMemberHandlerNull : public InvalidMemberHandler
	{
	public:
		virtual void OnInvalidMemberDetected(int memberOffset, const char* message) {}
	};

	InvalidMemberHandlerNull invalidMemberHandlerNull;
	InvalidMemberHandler& handler = invalidMemberHandler ? *invalidMemberHandler : invalidMemberHandlerNull;

	bool valid = true;

	// Helper macros:
#define VALIDATE_NOT_LESS_THAN(member, min) \
	if (!(member >= min)) \
	{ \
		valid = false; \
		handler.OnInvalidMemberDetected(offsetof(Settings, member), wxString::Format("(%I64d) is less than %I64d", int64(member), int64(min))); \
	}

#define VALIDATE_NOT_GREATER_THAN(member, max) \
	if (!(member <= max)) \
	{ \
		valid = false; \
		handler.OnInvalidMemberDetected(offsetof(Settings, member), wxString::Format("(%I64d) is less than %I64d", int64(member), int64(max))); \
	}

#define VALIDATE_IN_RANGE(member, min, max) \
	VALIDATE_NOT_LESS_THAN(member, min) else VALIDATE_NOT_GREATER_THAN(member, max)

	// Perform the validation:
	VALIDATE_NOT_LESS_THAN(lowResolutionPassesMax, LOW_RESOLUTION_PASSES_AUTO);
	VALIDATE_NOT_LESS_THAN(numIterations, 1);

	VALIDATE_NOT_LESS_THAN(latticeGapX, LATTICE_GAP_MIN);
	VALIDATE_NOT_LESS_THAN(latticeGapY, LATTICE_GAP_MIN);

	VALIDATE_NOT_LESS_THAN(patchWidth, PATCH_SIDE_MIN);
	VALIDATE_NOT_LESS_THAN(patchHeight, PATCH_SIDE_MIN);

	if (!(patchWidth * patchHeight <= PATCH_PIXELS_MAX))
	{
		valid = false;
		const int memberOffset = (patchWidth > patchHeight) ? offsetof(Settings, patchWidth) : offsetof(Settings, patchHeight);
		handler.OnInvalidMemberDetected(memberOffset, wxString::Format("is yielding too large of a patch (%d * %d > %I64d)", patchWidth, patchHeight, PATCH_PIXELS_MAX));
	}

	VALIDATE_IN_RANGE(confidenceBeliefThreshold, CONFIDENCE_BELIEF_THRESHOLD_MIN, CONFIDENCE_BELIEF_THRESHOLD_MAX);
	VALIDATE_IN_RANGE(pruneBeliefThreshold, PRUNE_BELIEF_THRESHOLD_MIN, PRUNE_BELIEF_THRESHOLD_MAX);
	VALIDATE_IN_RANGE(pruneEnergySimilarThreshold, PRUNE_ENERGY_SIMILAR_THRESHOLD_MIN, PRUNE_ENERGY_SIMILAR_THRESHOLD_MAX);

	VALIDATE_IN_RANGE(postPruneLabelsMin, POST_PRUNE_LABEL_MIN, POST_PRUNE_LABEL_MAX);
	VALIDATE_IN_RANGE(postPruneLabelsMax, POST_PRUNE_LABEL_MIN, POST_PRUNE_LABEL_MAX);

	if (compositorPatchType <= CompositorPatchTypeInvalid || compositorPatchType >= CompositorPatchTypeNum)
	{
		valid = false;
		handler.OnInvalidMemberDetected(offsetof(Settings, compositorPatchType), "is invalid");
	}

	if (compositorPatchBlender <= CompositorPatchBlenderInvalid || compositorPatchBlender >= CompositorPatchBlenderNum)
	{
		valid = false;
		handler.OnInvalidMemberDetected(offsetof(Settings, compositorPatchBlender), "is invalid");
	}

	return valid;
}

//
// SettingsScalable implementation
//
SettingsScalable::SettingsScalable(const Settings& settings) :
Settings(settings), // copy
m_depth(0)
{
}

void SettingsScalable::ScaleUp()
{
	// Copy the last saved resolution settings and pop up.
	wxASSERT(m_depth > 0);
	Settings& thisSettings = *this;
	thisSettings = m_resolutions[--m_depth];
	m_resolutions.pop_back();
}

void SettingsScalable::ScaleDown()
{
	// Save the current resolution settings.
	wxASSERT(m_depth == m_resolutions.size());
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

int SettingsScalable::GetScaleDepth() const
{
	return m_depth;
}
