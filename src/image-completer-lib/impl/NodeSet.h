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

#ifndef NODE_SET_H
#define NODE_SET_H

#include "Node.h"
#include "Scalable.h"

namespace LfnIc
{
	// Forward declarations
	class EnergyCalculatorContainer;
	class Image;
	class LabelSet;
	class MaskLod;
	class Node;
	struct Settings;

	//
	// Contains the Markov Random Field lattice nodes that intersect with the
	// unknown region.
	//
	class NodeSet : private std::vector<Node>, public Scalable
	{
	public:
		NodeSet(
			const Settings& settings,
			const Image& inputImage,
			const MaskLod& mask,
			const LabelSet& labelSet,
			EnergyCalculatorContainer& energyCalculatorContainer);

		// Elevate select vector methods to public access:
		typedef std::vector<Node> Super;
		using Super::size;
		using Super::operator[];

		// Calculates the node's priority, and updates the node set's priority
		// order.
		void UpdatePriority(const Node& node);

		// Returns the stored priority for the node.
		Priority GetPriority(const Node& node) const;

		// Declares the node "committed" or "uncommitted".
		void SetCommitted(const Node& node, bool committed);

		// Returns true if the node has been declared "committed".
		bool IsCommitted(const Node& node) const;

		// Returns the uncommitted node of the highest priority, or NULL if
		// none remain.
		Node* GetHighestPriorityUncommittedNode() const;

		// Scalable interface
		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

	private:
		//
		// Definitions
		//
		struct NodeInfo
		{
			NodeInfo();

			Priority priority;
			bool isCommitted;
		};
		typedef std::vector<NodeInfo> NodeSetInfo;

		Node::Context m_nodeContext;
		NodeSetInfo m_nodeSetInfo;

		int m_depth;
	};
}

#endif
