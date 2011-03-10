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

#ifndef PRIORITY_BP_RUNNER_H
#define PRIORITY_BP_RUNNER_H

#include "Patch.h"

namespace PriorityBp
{
	class Node;
	class NodeSet;
	class Settings;

	//
	// Encapsulates the state and execution of the Priority-BP (believe
	// propagation) algorithm over the Markov Random Field.
	//
	class PriorityBpRunner
	{
	public:
		//
		// Methods
		//

		PriorityBpRunner(const Settings& settings, NodeSet& nodeSet);

		// Executes the Priority-BP to completion, and populates the outPatches
		// object based on the solution. The patches are sorted by a
		// recommended order in which they should be applied to the output
		// image.
		void RunAndGetPatches(std::vector<Patch>& outPatches);

		// Runs Priority-BP to completion, without populating any patches.
		// This can be used during lower resolution solves in order to prune
		// the label set from each node.
		void Run();

	private:
		//
		// Internal definitions
		//
		class ForwardOrder : private std::vector<Node*>
		{
		public:
			ForwardOrder(int nodeNum);

			// Elevate select vector methods to public access:
			typedef std::vector<Node*> Super;
			using Super::size;
			using Super::operator[];
		};

		enum ProcessNeighborsType
		{
			UncommittedNeighbors,
			CommittedNeighbors,
		};

		//
		// Internal methods
		//
		void ForwardPass();
		void BackwardPass();
		void ProcessNeighbors(Node& node, ProcessNeighborsType type);
		void PopulatePatches(std::vector<Patch>& outPatches) const;

		//
		// Internal data
		//

		const Settings& m_settings;
		NodeSet& m_nodeSet;
		ForwardOrder m_forwardOrder;
	};
};

#endif
