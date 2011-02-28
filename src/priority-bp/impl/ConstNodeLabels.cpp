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
#include "ConstNodeLabels.h"

#include "Label.h"
#include "Node.h"

#include "tech/DbgMem.h"

//
// ConstNodeLabels implementation
//
ConstNodeLabels::ConstNodeLabels(const Node& node) :
m_node(node),
m_globalLabelSet((node.m_labelInfoSet.size() == 0) ? &node.m_context.labelSet : NULL)
{
}

int ConstNodeLabels::size() const
{
	return m_globalLabelSet
		? m_globalLabelSet->size()
		: m_node.m_labelInfoSet.size();
}

const Label& ConstNodeLabels::GetLabel(int index) const
{
	wxASSERT(index >= 0);
	wxASSERT(index < size());
	return m_globalLabelSet
		? (*m_globalLabelSet)[index]
		: m_node.m_labelInfoSet[index].label;
}

const Energy* ConstNodeLabels::GetMessages(int index) const
{
	wxASSERT(index >= 0);
	wxASSERT(index < size());
	static const Energy globalMessageSet[NumNeighborEdges] = {Energy(0)};

	return m_globalLabelSet
		? globalMessageSet
		: m_node.m_labelInfoSet[index].messages;
}
