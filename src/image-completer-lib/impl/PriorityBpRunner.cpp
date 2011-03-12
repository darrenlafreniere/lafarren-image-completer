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
#include "PriorityBpRunner.h"

#include "tech/Profile.h"

#include "ConstNodeLabels.h"
#include "Label.h"
#include "NodeSet.h"
#include "PriorityBpSettings.h"

#include "tech/DbgMem.h"

#define ENABLE_TIME_PROFILING 0
#define ENABLE_MEM_PROFILING 0

#if ENABLE_TIME_PROFILING
#define PRIORITY_BP_TIME_PROFILE(__name__) TECH_TIME_PROFILE_EVERY_SAMPLE(__name__)
#else
#define PRIORITY_BP_TIME_PROFILE(__name__)
#endif

#if ENABLE_MEM_PROFILING
#define PRIORITY_BP_MEM_PROFILE(__name__) TECH_MEM_PROFILE(__name__)
#else
#define PRIORITY_BP_MEM_PROFILE(__name__)
#endif

//
// Priority-BP algorithm (as lifted from the white paper):
//
//
//	assign priorities to nodes and declare them uncommitted
//	for k = 1 to K do {K is the number of iterations}
//		execute ForwardPass and then BackwardPass
//	assign to each node p its label ^xp that maximizes bp(.)
//
//	ForwardPass:
//		for time = 1 to N do {N is the number of nodes}
//			p = "uncommitted" node of highest priority
//			apply "label pruning" to node p
//			forwardOrder[time] = p ; p?committed = true;
//			for any "uncommitted" neighbor q of node p do
//				send all messages mpq(.) from node p to node q
//				update beliefs bq(.) as well as priority of node q
//
//	BackwardPass:
//		for time = N to 1 do
//		p = forwardOrder[time]; p?committed = false;
//		for any "committed" neighbor q of node p do
//			send all messages mpq(.) from node p to node q
//			update beliefs bq(.) as well as priority of node q
//

LfnIc::PriorityBpRunner::PriorityBpRunner(const Settings& settings, NodeSet& nodeSet) :
m_settings(settings),
m_nodeSet(nodeSet),
m_forwardOrder(nodeSet.size())
{
}

void LfnIc::PriorityBpRunner::RunAndGetPatches(std::vector<Patch>& outPatches)
{
	Run();
	PopulatePatches(outPatches);
}

void LfnIc::PriorityBpRunner::Run()
{
#if _DEBUG
	// Sometimes it's useful when debugging to skip a lot of the algorithm.
	// Populating the m_forwardOrder array ensures that the patches will be
	// written without a crash if other bits are skipped.
	{
		for (int i = 0, n = m_nodeSet.size(); i < n; ++i)
		{
			m_forwardOrder[i] = &m_nodeSet[i];
		}
	}
#endif

	// Assign node priorities and declare them uncommitted
	{
		PRIORITY_BP_TIME_PROFILE("LfnIc::PriorityBpRunner::Run - initial priorities");
		PRIORITY_BP_MEM_PROFILE("LfnIc::PriorityBpRunner::Run - initial priorities");

		for (int i = 0, n = m_nodeSet.size(); i < n; ++i)
		{
			Node& node = m_nodeSet[i];
			m_nodeSet.UpdatePriority(node);
			m_nodeSet.SetCommitted(node, false);
		}

		wxASSERT(m_forwardOrder.size() == m_nodeSet.size());
	}

	wxASSERT(m_settings.numIterations >= 1);
	for (int i = 0; i < m_settings.numIterations; ++i)
	{
		PRIORITY_BP_TIME_PROFILE("LfnIc::PriorityBpRunner::Run - iteration");
		PRIORITY_BP_MEM_PROFILE(Str::Format("LfnIc::PriorityBpRunner::Run - iteration %d", i));

		ForwardPass();
		BackwardPass();
	}
}

void LfnIc::PriorityBpRunner::ForwardPass()
{
	PRIORITY_BP_TIME_PROFILE("LfnIc::PriorityBpRunner::ForwardPass");
	PRIORITY_BP_MEM_PROFILE("LfnIc::PriorityBpRunner::ForwardPass");

	for (int i = 0, n = m_nodeSet.size(); i < n; ++i)
	{
		Node* node = m_nodeSet.GetHighestPriorityUncommittedNode();
		wxASSERT(node);

		node->PruneLabels();
		m_forwardOrder[i] = node;

		m_nodeSet.SetCommitted(*node, true);
		ProcessNeighbors(*node, UncommittedNeighbors);
	}
}

void LfnIc::PriorityBpRunner::BackwardPass()
{
	PRIORITY_BP_TIME_PROFILE("LfnIc::PriorityBpRunner::BackwardPass");
	PRIORITY_BP_MEM_PROFILE("LfnIc::PriorityBpRunner::BackwardPass");

	for (int i = m_nodeSet.size(); --i >= 0; )
	{
		Node* node = m_forwardOrder[i];
		wxASSERT(node);

		m_nodeSet.SetCommitted(*node, false);
		ProcessNeighbors(*node, CommittedNeighbors);
	}
}

void LfnIc::PriorityBpRunner::ProcessNeighbors(Node& node, ProcessNeighborsType type)
{
	for (int i = 0; i < NumNeighborEdges; ++i)
	{
		NeighborEdge edge = NeighborEdge(i);
		Node* neighbor = node.GetNeighbor(edge);
		if (neighbor)
		{
			const bool desiredCommitment = (type == CommittedNeighbors);
			if (m_nodeSet.IsCommitted(*neighbor) == desiredCommitment)
			{
				node.SendMessages(*neighbor);
				m_nodeSet.UpdatePriority(*neighbor);
			}
		}
	}
}

// Sort the patches in ascending order of priority, so that the more
// confident patches are laid atop the less confidence patches.
struct SortPatchesByPriority
{
	bool operator()(const LfnIc::Patch& patchA, const LfnIc::Patch& patchB)
	{
		return patchA.priority < patchB.priority;
	};
};

void LfnIc::PriorityBpRunner::PopulatePatches(std::vector<Patch>& outPatches) const
{
	const int nodeNum = m_nodeSet.size();
	outPatches.resize(nodeNum);

	for (int i = 0; i < nodeNum; ++i)
	{
		Node* node = m_forwardOrder[i];
		wxASSERT(node);

		// Due to the algorithm's sorting, the first label is the one with the
		// highest belief for the node.
		ConstNodeLabels nodeLabels(*node);
		const Label& label = nodeLabels.GetLabel(0);

		Patch& patch = outPatches[i];
		patch.srcLeft = label.left;
		patch.srcTop  = label.top;

		patch.destLeft = node->GetLeft();
		patch.destTop  = node->GetTop();

		patch.priority = m_nodeSet.GetPriority(*node);
	}


	std::sort(outPatches.begin(), outPatches.end(), SortPatchesByPriority());
}

LfnIc::PriorityBpRunner::ForwardOrder::ForwardOrder(int nodeNum) :
Super(nodeNum)
{
}
