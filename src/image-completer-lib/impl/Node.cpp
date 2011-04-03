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
#include "Node.h"

#define PROFILE_MEM 0

#include "tech/MathUtils.h"
#if PROFILE_MEM
#include "tech/Profile.h"
#endif

#include "ConstNodeLabels.h"
#include "EnergyCalculatorContainer.h"
#include "Label.h"
#include "LfnIcSettings.h"
#include "MaskLod.h"
#include "ScopedNodeEnergyBatch.h"

#include "tech/DbgMem.h"

// See comments for EnergyCalculator::BatchQueued::QueueCalculation.
#define ASSERT_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, index) \
	wxASSERT(handle == static_cast<uint>(index))

// Similar to ASSERT_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX, except for use with
// ScopedNodeEnergyBatchQueued objects, which can validly return an invalid
// handle if the node doesn't overlap a known region.
#define ASSERT_NODE_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, index) \
	wxASSERT(handle == static_cast<uint>(index) || handle == EnergyCalculator::BatchQueued::INVALID_HANDLE)

// If set, when the node scales up and maps from a lower resolution label to
// its higher resolution labels, a single, random one of those higher
// resolution labels will be used. Can be faster, but might throw away better
// labels to use.
#define NODE_SCALE_UP_PICK_RANDOM_MAPPED_LABEL 0

//
// Node implementation
//
LfnIc::Node::Context::Context(const Settings& settings, const LabelSet& labelSet, EnergyCalculatorContainer& energyCalculatorContainer) :
settings(settings),
labelSet(labelSet),
energyCalculatorContainer(energyCalculatorContainer)
{
}

LfnIc::Node::Node(Context& context, const MaskLod& mask, int x, int y) :
m_context(&context),
m_depth(0),
m_overlapsKnownRegion(false),
m_hasPrunedOnce(false)
{
	// Add original resolution
	m_resolutions.push_back(Resolution(x, y));

	memset(m_neighbors, 0, sizeof(m_neighbors));

	// Determine m_overlapsKnownRegion
	{
		const MaskLod::LodData& maskData = mask.GetLodData(mask.GetHighestLod());
		int left = GetLeft();
		int top = GetTop();

		// Adjust left and top to be >= 0. Must initialize col and row accordingly.
		const int leftAdjustment = std::max(-left, 0);
		const int topAdjustment = std::max(-top, 0);
		left += leftAdjustment;
		top += topAdjustment;

		const int colStart = leftAdjustment;
		const int rowStart = topAdjustment;

		// Set the number of rows and columns by the patch width and height,
		// and clamp to prevent overflow.
		const int colsNum = std::min(m_context->settings.patchWidth, maskData.width - left);
		const int rowsNum = std::min(m_context->settings.patchHeight, maskData.height - top);

		for (int row = rowStart, y = top; row < rowsNum; ++row, ++y)
		{
			const Mask::Value* maskBufferPtr = &maskData.buffer[LfnTech::GetRowMajorIndex(maskData.width, left, y)];
			for (int col = colStart; col < colsNum; ++col, ++maskBufferPtr)
			{
				if (*maskBufferPtr == Mask::KNOWN)
				{
					m_overlapsKnownRegion = true;
					break;
				}
			}
		}
	}
}

LfnIc::Node::Node(const Node& other) :
m_context(other.m_context),
m_depth(other.m_depth),
m_overlapsKnownRegion(other.m_overlapsKnownRegion),
m_hasPrunedOnce(other.m_hasPrunedOnce)
{
	m_resolutions = other.m_resolutions;
	memcpy(m_neighbors, other.m_neighbors, sizeof(m_neighbors));
}

int LfnIc::Node::GetX() const
{
	return GetCurrentResolution().x;
}

int LfnIc::Node::GetY() const
{
	return GetCurrentResolution().y;
}

bool LfnIc::Node::AddNeighbor(Node& neighbor, NeighborEdge edge)
{
#ifdef _DEBUG
	wxASSERT(!m_neighbors[edge]);
	wxASSERT(m_labelInfoSet.size() == 0);

	int edgeDirectionX = 0;
	int edgeDirectionY = 0;
	GetNeighborEdgeDirection(edge, edgeDirectionX, edgeDirectionY);
	wxASSERT(
		(neighbor.GetCurrentResolution().x - GetCurrentResolution().x) == (m_context->settings.latticeGapX * edgeDirectionX) &&
		(neighbor.GetCurrentResolution().y - GetCurrentResolution().y) == (m_context->settings.latticeGapY * edgeDirectionY));

	for (int i = 0; i < NumNeighborEdges; ++i)
	{
		wxASSERT(m_neighbors[i] != &neighbor);
	}
#endif
	m_neighbors[edge] = &neighbor;
	return true;
}

LfnIc::Node* LfnIc::Node::GetNeighbor(NeighborEdge edge) const
{
	wxASSERT(edge >= FirstNeighborEdge);
	wxASSERT(edge <= LastNeighborEdge);
	return m_neighbors[edge];
}

LfnIc::NeighborEdge LfnIc::Node::GetNeighborEdge(const Node& neighbor) const
{
	for (int i = 0; i < NumNeighborEdges; ++i)
	{
		if (m_neighbors[i] == &neighbor)
		{
			return NeighborEdge(i);
		}
	}

	wxFAIL_MSG("LfnIc::Node::GetNeighborIndex: specified node is not a neighbor!");
	return InvalidNeighborEdge;
}

void LfnIc::Node::SendMessages(Node& neighbor) const
{
	// At this point, this node must have its own label info set.
	wxASSERT(m_labelInfoSet.size() > 0);

	// And we expect that it has been pruned.
	wxASSERT(int(m_labelInfoSet.size()) <= m_context->settings.postPruneLabelsMax);

	// Make sure the neighbor has its own label info set to store this node's messages to it.
	neighbor.PopulateLabelInfoSetIfNeeded();
	wxASSERT(neighbor.m_labelInfoSet.size() > 0);

	// p: this node
	// q: neighbor node
	// r: this node's neighbors except q
	const NeighborEdge pEdgeInQ = neighbor.GetNeighborEdge(*this);
	const NeighborEdge qEdgeInP = this->GetNeighborEdge(neighbor);
	wxASSERT(pEdgeInQ != InvalidNeighborEdge);
	wxASSERT(qEdgeInP != InvalidNeighborEdge);

	// Figure out overlapping region (pre-compute the result of this per neighbor?)
	const int patchWidth = m_context->settings.patchWidth;
	const int patchHeight = m_context->settings.patchHeight;

	const int pLeft = GetLeft();
	const int pTop = GetTop();
	const int pRight = pLeft + patchWidth - 1;
	const int pBottom = pTop + patchHeight - 1;

	const int qLeft = neighbor.GetLeft();
	const int qTop = neighbor.GetTop();
	const int qRight = qLeft + patchWidth - 1;
	const int qBottom = qTop + patchHeight - 1;

	const int overlapLeft = std::max(pLeft, qLeft);
	const int overlapTop = std::max(pTop, qTop);
	const int overlapRight = std::min(pRight, qRight);
	const int overlapBottom = std::min(pBottom, qBottom);

	const int overlapWidth = overlapRight - overlapLeft + 1;
	const int overlapHeight = overlapBottom - overlapTop + 1;
	const int pOverlapLeftOffset = overlapLeft - pLeft;
	const int pOverlapTopOffset = overlapTop - pTop;
	const int qOverlapLeftOffset = overlapLeft - qLeft;
	const int qOverlapTopOffset = overlapTop - qTop;

	// TODO: cache this?
	const int pLabelNum = m_labelInfoSet.size();
	std::vector<Energy> pLabelEnergies(pLabelNum);
	{
		const EnergyCalculator::BatchParams energyBatchParams(pLabelNum, m_context->settings.patchWidth, m_context->settings.patchHeight, GetLeft(), GetTop(), true);
		ScopedNodeEnergyBatchQueued energyBatch(*this, m_context->energyCalculatorContainer.Get(energyBatchParams, pLabelNum), energyBatchParams);

		// Queue energy calculations
		for (int pi = 0; pi < pLabelNum; ++pi)
		{
			const Label& label = m_labelInfoSet[pi].label;
			const EnergyCalculator::BatchQueued::Handle handle = energyBatch.QueueCalculation(label.left, label.top);
			ASSERT_NODE_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, pi);
		}

		energyBatch.ProcessCalculations();

		// Get and use energy calculation results
		for (int pi = 0; pi < pLabelNum; ++pi)
		{
			pLabelEnergies[pi] = energyBatch.GetResult(EnergyCalculator::BatchQueued::Handle(pi));
		}
	}

	// Send messages for every label in the neighbor's set. Keep track of the
	// minimum message sent from p to q, to normalizing all p->q messages by
	// subtracting away that minimum.
	//
	// The more natural way of organizing this iteration is:
	//
	// for each (q label) { for each (p label) {} }
	//
	// However, because p's labels have already been pruned, the more
	// efficient way to batch the energy calculations is to swap the loop
	// order.
	const int qLabelNum = neighbor.m_labelInfoSet.size();
	std::vector<Energy> messages(qLabelNum, ENERGY_MAX);
	Energy messagesMin = ENERGY_MAX;
	// Iterate over this node's labels to determine which should supply
	// the message for each q, which will be the one that produces the
	// lowest energy.
	for (int pi = 0, pn = pLabelNum; pi < pn; ++pi)
	{
		const LabelInfo& pLabelInfo = m_labelInfoSet[pi];
		const int pOverlapLeft = pLabelInfo.label.left + pOverlapLeftOffset;
		const int pOverlapTop = pLabelInfo.label.top + pOverlapTopOffset;

		const EnergyCalculator::BatchParams energyBatchParams(qLabelNum, overlapWidth, overlapHeight, pOverlapLeft, pOverlapTop, false);
		EnergyCalculator::BatchQueued energyBatch(m_context->energyCalculatorContainer.Get(energyBatchParams, qLabelNum).BatchOpenQueued(energyBatchParams));

		// Queue energy calculations
		for (int qi = 0; qi < qLabelNum; ++qi)
		{
			const Label& qLabel = neighbor.m_labelInfoSet[qi].label;
			const int qOverlapLeft = qLabel.left + qOverlapLeftOffset;
			const int qOverlapTop = qLabel.top + qOverlapTopOffset;

			const EnergyCalculator::BatchQueued::Handle handle = energyBatch.QueueCalculation(qOverlapLeft, qOverlapTop);
			ASSERT_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, qi);
		}

		energyBatch.ProcessCalculations();

		// Get and use energy calculation results
		for (int qi = 0; qi < qLabelNum; ++qi)
		{
			Energy messageCandidate = pLabelEnergies[pi] + energyBatch.GetResult(EnergyCalculator::BatchQueued::Handle(qi));

			for (int r = 0; r < NumNeighborEdges; ++r)
			{
				if (r != qEdgeInP)
				{
					messageCandidate += pLabelInfo.messages[r];
				}
			}

			if (messageCandidate < messages[qi])
			{
				messages[qi] = messageCandidate;

				if (messageCandidate < messagesMin)
				{
					messagesMin = messageCandidate;
				}
			}
		}
	}

	// Normalize p->q messages and assign them.
	for (int qi = 0, qn = neighbor.m_labelInfoSet.size(); qi < qn; ++qi)
	{
		Energy& message = messages[qi];
		wxASSERT(message >= ENERGY_MIN && message < ENERGY_MAX);
		message -= messagesMin;
		neighbor.m_labelInfoSet[qi].messages[pEdgeInQ] = message;
	}
}

namespace LfnIc
{
	// TODO: move belief into LabelInfo?
	struct PruneInfo
	{
		int labelIndex;
		Belief belief;
	};

	// For sorting:
	bool operator <(const PruneInfo& a, const PruneInfo& b)
	{
		// Use > to sort in descending order
		return (a.belief > b.belief);
	}
}

void LfnIc::Node::PruneLabels()
{
#if PROFILE_MEM
	TECH_MEM_PROFILE("LfnIc::Node::PruneLabels");
#endif
	ConstNodeLabels labelSet(*this);
	const int labelNum = labelSet.size();
	std::vector<PruneInfo> pruneInfos(labelNum);

	{
		const EnergyCalculator::BatchParams energyBatchParams(labelNum, m_context->settings.patchWidth, m_context->settings.patchHeight, GetLeft(), GetTop(), true);
		ScopedNodeEnergyBatchQueued energyBatch(*this, m_context->energyCalculatorContainer.Get(energyBatchParams, labelNum), energyBatchParams);

		// Queue energy calculations
		for (int i = 0; i < labelNum; ++i)
		{
			const Label& label = labelSet.GetLabel(i);
			const EnergyCalculator::BatchQueued::Handle handle = energyBatch.QueueCalculation(label.left, label.top);
			ASSERT_NODE_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, i);
		}

		energyBatch.ProcessCalculations();

		// Get and use energy calculation results
		for (int i = 0; i < labelNum; ++i)
		{
			pruneInfos[i].labelIndex = i;
			pruneInfos[i].belief = CalculateBelief(energyBatch.GetResult(EnergyCalculator::BatchQueued::Handle(i)), labelSet.GetMessages(i));
		}
	}

	// Sort pruneInfos by belief
	sort(pruneInfos.begin(), pruneInfos.end());

	// Perform the pruning
	{
		const int patchWidth = m_context->settings.patchWidth;
		const int patchHeight = m_context->settings.patchHeight;
		const int pruneEnergySimilarThreshold = m_context->settings.pruneEnergySimilarThreshold;
		const int pruneBeliefThreshold = m_context->settings.pruneBeliefThreshold;
		const int postPruneLabelsMin = m_context->settings.postPruneLabelsMin;
		const int postPruneLabelsMax = m_context->settings.postPruneLabelsMax;
		LabelInfoSet labelInfoSetKept;

		for (int pruneInfoIdx = 0, postPruneLabelNum = 0; pruneInfoIdx < labelNum && postPruneLabelNum < postPruneLabelsMax; ++pruneInfoIdx)
		{
			const int labelIdx = pruneInfos[pruneInfoIdx].labelIndex;
			const Label& label = labelSet.GetLabel(labelIdx);

			// Attempt to keep this label if the min number of post pruned
			// labels hasn't been reached, or if the label's belief is above
			// the pruning threshold.
			bool keep = false;
			if (postPruneLabelNum < postPruneLabelsMin || pruneInfos[pruneInfoIdx].belief > pruneBeliefThreshold)
			{
				if (m_hasPrunedOnce)
				{
					// If this node's labels have already been pruned, then
					// its current labels have passed the similarity filter below.
					// It is not necessary to perform that filtering twice.
					keep = true;
				}
				else
				{
					// On the first pruning, verify that this label is
					// dissimilar enough from the labels that have been kept
					// so far.
					bool isSimilarToAlreadyKeptLabel = false;

					const int keptNum = labelInfoSetKept.size();
					if (keptNum > 0)
					{
						// Use an immediate batch - there shouldn't be too
						// many calculations, and the upper bound is unknown.
						// TODO: run some tests to verify this assumption.
						const EnergyCalculator::BatchParams energyBatchParams(keptNum, patchWidth, patchHeight, label.left, label.top, false);
						EnergyCalculator::BatchImmediate energyBatch(m_context->energyCalculatorContainer.Get(energyBatchParams, keptNum).BatchOpenImmediate(energyBatchParams));

						for (int keptIdx = 0; !isSimilarToAlreadyKeptLabel && keptIdx < keptNum; ++keptIdx)
						{
							const Label& alreadyKeptLabel = labelInfoSetKept[keptIdx].label;
							const Energy e = energyBatch.Calculate(alreadyKeptLabel.left, alreadyKeptLabel.top);
							isSimilarToAlreadyKeptLabel = (e < pruneEnergySimilarThreshold);
						}
					}

					keep = !isSimilarToAlreadyKeptLabel;
				}
			}

			if (keep)
			{
				LabelInfo labelInfo;
				labelInfo.label = label;
				memcpy(labelInfo.messages, labelSet.GetMessages(labelIdx), sizeof(labelInfo.messages));
#ifdef _DEBUG
				for (int j = 0; j < NumNeighborEdges; ++j)
				{
					const Energy message = labelInfo.messages[j];
					wxASSERT(message >= ENERGY_MIN && message <= ENERGY_MAX);
				}
#endif
				labelInfoSetKept.push_back(labelInfo);
				++postPruneLabelNum;
			}
		}

#if PROFILE_MEM
		printf("PruneLabels, before: %d, after %d\n", labelNum, labelInfoSetKept.size());
#endif

		m_labelInfoSet.swap(labelInfoSetKept);
		m_hasPrunedOnce = true;
	}
}

LfnIc::Priority LfnIc::Node::CalculatePriority() const
{
	Priority priority = PRIORITY_MIN;

	ConstNodeLabels labelSet(*this);
	const int labelNum = labelSet.size();
	std::vector<Belief> beliefs(labelNum);
	Belief beliefMax = BELIEF_MIN;

	const EnergyCalculator::BatchParams energyBatchParams(labelNum, m_context->settings.patchWidth, m_context->settings.patchHeight, GetLeft(), GetTop(), true);
	ScopedNodeEnergyBatchQueued energyBatch(*this, m_context->energyCalculatorContainer.Get(energyBatchParams, labelNum), energyBatchParams);

	// Queue energy calculations
	for (int i = 0; i < labelNum; ++i)
	{
		const Label& label = labelSet.GetLabel(i);
		const EnergyCalculator::BatchQueued::Handle handle = energyBatch.QueueCalculation(label.left, label.top);
		ASSERT_NODE_ENERGY_BATCH_QUEUED_HANDLE_IS_INDEX(handle, i);
	}

	energyBatch.ProcessCalculations();

	// Get and use energy calculation results
	for (int i = 0; i < labelNum; ++i)
	{
		beliefs[i] = CalculateBelief(energyBatch.GetResult(EnergyCalculator::BatchQueued::Handle(i)), labelSet.GetMessages(i));
		if (beliefs[i] > beliefMax)
		{
			beliefMax = beliefs[i];
		}
	}

	const Belief beliefConf = Belief(m_context->settings.confidenceBeliefThreshold);
	int confusionSetNum = 0;
	for (int i = 0; i < labelNum; ++i)
	{
		const Belief beliefRel = beliefs[i] - beliefMax;
		if (beliefRel > beliefConf)
		{
			++confusionSetNum;
		}
	}

	if (confusionSetNum > 0)
	{
		priority = Priority(1) / Priority(confusionSetNum);
	}

	wxASSERT(PRIORITY_MIN <= priority && priority <= PRIORITY_MAX);
	return priority;
}

LfnIc::Belief LfnIc::Node::CalculateBelief(Energy labelEnergy, const Energy messages[NumNeighborEdges]) const
{
	Belief belief= Belief(-labelEnergy);
	for (int i = 0; i < NumNeighborEdges; ++i)
	{
		belief -= Belief(messages[i]);
	}

	wxASSERT(belief >= BELIEF_MIN && belief <= BELIEF_MAX);
	return belief;
}

LfnIc::Belief LfnIc::Node::CalculateBelief(const Label& label, const Energy messages[NumNeighborEdges]) const
{
	Energy e;
	if (OverlapsKnownRegion())
	{
		// Single energy calculation; use an immediate batch.
		const EnergyCalculator::BatchParams energyBatchParams(1, m_context->settings.patchWidth, m_context->settings.patchHeight, GetLeft(), GetTop(), true);
		ScopedNodeEnergyBatchImmediate energyBatch(*this, m_context->energyCalculatorContainer.Get(energyBatchParams, 1), energyBatchParams);

		e = energyBatch.Calculate(label.left, label.top);
	}
	else
	{
		e = ENERGY_MIN;
	}

	return CalculateBelief(e, messages);
}

void LfnIc::Node::PopulateLabelInfoSetIfNeeded()
{
#if PROFILE_MEM
	TECH_MEM_PROFILE("LfnIc::Node::PopulateLabelInfoSetIfNeeded");
#endif
	if (m_labelInfoSet.size() == 0)
	{
		const int labelNum = m_context->labelSet.size();

		// We'll have exactly with many labels. Resize now and fill in data.
		m_labelInfoSet.resize(labelNum);

		for (int i = 0; i < labelNum; ++i)
		{
			m_labelInfoSet[i].SetLabelAndClearMessages(m_context->labelSet[i]);
		}
	}
}

int LfnIc::Node::GetLeft() const
{
	return GetCurrentResolution().x - (m_context->settings.patchWidth / 2);
}

int LfnIc::Node::GetTop() const
{
	return GetCurrentResolution().y - (m_context->settings.patchHeight / 2);
}

bool LfnIc::Node::OverlapsKnownRegion() const
{
	return m_overlapsKnownRegion;
}

void LfnIc::Node::ScaleUp()
{
	wxASSERT(m_depth > 0);

	m_resolutions.erase(m_resolutions.begin() + m_resolutions.size() - 1);

	--m_depth;
	wxASSERT(m_depth == int(m_resolutions.size()) - 1);

	// Scale up the label info set.
	{
		const LabelSet& labelSet = m_context->labelSet;
		LabelSet::LowToCurrentResolutionMapping labelMapping;

#if NODE_SCALE_UP_PICK_RANDOM_MAPPED_LABEL
		// Pick one of the corresponding high resolution labels at random.
		for (int labelInfoIdx = 0, labelInfoNum = m_labelInfoSet.size(); labelInfoIdx < labelInfoNum; ++labelInfoIdx)
		{
			LabelInfo& labelInfo = m_labelInfoSet[labelInfoIdx];

			labelSet.GetLowToCurrentResolutionMapping(labelInfo.label, labelMapping);
			labelInfo.label = labelMapping[rand() % labelMapping.size()];
		}
#else
		// On average, each one lower resolution label expands to a 2x2 quad
		// of labels, so multiply by 4 for the new set.
		LabelInfoSet newLabelInfoSet;
		newLabelInfoSet.reserve(m_labelInfoSet.size() * 4);

		for (int labelInfoIdx = 0, labelInfoNum = m_labelInfoSet.size(); labelInfoIdx < labelInfoNum; ++labelInfoIdx)
		{
			const LabelInfo& labelInfo = m_labelInfoSet[labelInfoIdx];
			labelSet.GetLowToCurrentResolutionMapping(labelInfo.label, labelMapping);

			for (int i = 0, n = labelMapping.size(); i < n; ++i)
			{
				newLabelInfoSet.resize(newLabelInfoSet.size() + 1);
				LabelInfo& newLabelInfo = newLabelInfoSet.back();

				newLabelInfo.label = labelMapping[i];
				memcpy(newLabelInfo.messages, labelInfo.messages, sizeof(labelInfo.messages));
			}
		}

		m_labelInfoSet.swap(newLabelInfoSet);
#endif
	}
}

void LfnIc::Node::ScaleDown()
{
	wxASSERT(m_depth >= 0);

	const Resolution& resolutionToScaleDown = GetCurrentResolution();
	m_resolutions.push_back(Resolution(resolutionToScaleDown.x / 2, resolutionToScaleDown.y / 2));

	++m_depth;
	wxASSERT(m_depth == int(m_resolutions.size()) - 1);

	// We don't expect the label set to be populated until running priority-bp
	// on the most-scaled-down resolution.
	wxASSERT(m_labelInfoSet.size() == 0);
}

int LfnIc::Node::GetScaleDepth() const
{
	return m_depth;
}

void LfnIc::Node::LabelInfo::SetLabelAndClearMessages(const Label& label)
{
	this->label = label;
	memset(messages, 0, sizeof(messages));

#ifdef _DEBUG
	for (int i = 0; i < NumNeighborEdges; ++i)
	{
		wxASSERT(messages[i] == Energy(0));
	}
#endif
}
