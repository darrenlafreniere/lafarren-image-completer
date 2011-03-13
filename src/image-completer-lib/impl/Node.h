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

#ifndef NODE_H
#define NODE_H

#include "Label.h"
#include "NeighborEdge.h"
#include "LfnIcTypes.h"
#include "Scalable.h"

namespace LfnIc
{
	// Forward declarations
	class ConstNodeLabels;
	class EnergyCalculatorContainer;
	class MaskLod;
	struct Settings;

	//
	// A single Markov Random Field node.
	//
	class Node : public Scalable
	{
	public:
		// Consolidates the external references needed by each node.
		struct Context
		{
			Context(const Settings& settings, const LabelSet& labelSet, EnergyCalculatorContainer& energyCalculatorContainer);

			const Settings& settings;
			const LabelSet& labelSet;
			EnergyCalculatorContainer& energyCalculatorContainer;
		};

		//
		// Methods
		//
		Node(Context& context, const MaskLod& mask, int x, int y);
		Node(const Node& other);

		int GetX() const;
		int GetY() const;

		// Neighbor addition and access:
		bool AddNeighbor(Node& neighbor, NeighborEdge edge);
		Node* GetNeighbor(NeighborEdge edge) const;
		NeighborEdge GetNeighborEdge(const Node& neighbor) const;

		// Sends all beliefe propagation messages from this node to its
		// neighbor.
		void SendMessages(Node& neighbor) const;

		// Applies label pruning to this node.
		void PruneLabels();

		Priority CalculatePriority() const;

		// Fast belief calculation when the label energy is already known.
		Belief CalculateBelief(Energy labelEnergy, const Energy messages[NumNeighborEdges]) const;

		// Slow belief calculation when the label energy is unknown.
		Belief CalculateBelief(const Label& label, const Energy messages[NumNeighborEdges]) const;

		// Gets the image space left and top coordinates for any label placed
		// at this node.
		int GetLeft() const;
		int GetTop() const;

		// Returns true if this node's rectangle overlaps any of the mask's
		// known region. If false, the node will not waste any cycles
		// calculating the energy of its labels against the image.
		bool OverlapsKnownRegion() const;

		// Scalable interface
		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

	private:
		//
		// Internal definitions
		//

		friend class ConstNodeLabels;

		// During the first pass, once a node is either pruned or receives
		// messages, it store its own label set.
		//
		// The messages order matches the node's own m_neighbors member array.
		struct LabelInfo
		{
			Label label;
			Energy messages[NumNeighborEdges];

			void SetLabelAndClearMessages(const Label& label);
		};

		typedef std::vector<LabelInfo> LabelInfoSet;

		struct Resolution
		{
			inline Resolution(short x, short y) : x(x), y(y) {}
			short x;
			short y;
		};

		//
		// Internal methods
		//

		// If this node doesn't have its own label set, the global label set
		// will be copied into this node.
		void PopulateLabelInfoSetIfNeeded();

		inline Resolution& GetCurrentResolution() { return m_resolutions[m_depth]; }
		inline const Resolution& GetCurrentResolution() const { return m_resolutions[m_depth]; }

		//
		// Data
		//
		Context* m_context;

		std::vector<Resolution> m_resolutions;
		int m_depth;

		// Size and order matches the NeighborEdge enum:
		Node* m_neighbors[NumNeighborEdges];
		LabelInfoSet m_labelInfoSet;

		bool m_overlapsKnownRegion;
		bool m_hasPrunedOnce;
	};
}

#endif
