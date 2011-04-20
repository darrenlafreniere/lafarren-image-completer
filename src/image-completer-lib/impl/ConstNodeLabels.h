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

#ifndef CONST_NODE_LABELS_H
#define CONST_NODE_LABELS_H

#include "LfnIcTypes.h"

namespace LfnIc
{
	// Forward declarations
	class LabelSet;
	class Node;
	struct Label;

	/// Interface to iterate over a node's collection of Labels and associated
	/// messages.
	class ConstNodeLabels
	{
	public:
		ConstNodeLabels(const Node& node);

		/// Returns the number of labels at the node.
		int size() const;

		/// Label accessor.
		const Label& GetLabel(int index) const;

		/// Returns a Energy buffer of NumNeighborEdges length.
		const Energy* GetMessages(int index) const;

	private:
		const Node& m_node;
		const LabelSet* m_globalLabelSet;
	};
}

#endif
