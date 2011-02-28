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
#include "NeighborEdge.h"

#include "tech/DbgMem.h"

struct NeighborEdgeDirection
{
	int x;
	int y;
};

// Array must match size and order of NeighborEdge enum:
static const NeighborEdgeDirection g_neighborEdgeDirection[] =
{
	{-1, 0},
	{0, -1},
	{1,  0},
	{0,  1},
};

inline bool IsNeighborEdgeValid(NeighborEdge edge)
{
	return (edge >= FirstNeighborEdge && edge <= LastNeighborEdge);
}

bool PriorityBp::GetNeighborEdgeDirection(NeighborEdge edge, int& outX, int& outY)
{
	bool result = true;

	if (IsNeighborEdgeValid(edge))
	{
		outX = g_neighborEdgeDirection[edge].x;
		outY = g_neighborEdgeDirection[edge].y;
	}
	else
	{
		outX = 0;
		outY = 0;
		result = false;
	}

	return result;
}

int PriorityBp::GetNeighborEdgeDirectionX(NeighborEdge edge)
{
	return IsNeighborEdgeValid(edge) ? g_neighborEdgeDirection[edge].x : 0;
}

int PriorityBp::GetNeighborEdgeDirectionY(NeighborEdge edge)
{
	return IsNeighborEdgeValid(edge) ? g_neighborEdgeDirection[edge].y : 0;
}
