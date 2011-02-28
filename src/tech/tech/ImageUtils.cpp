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

//
// Contains inline implementations of ImageUtils.h definitions.
//
#include "Pch.h"
#include "tech/ImageUtils.h"

#include "tech/MathUtils.h"

#include "tech/DbgMem.h"

static void Clip(const wxRect& bounds, wxPoint& point, wxSize& size, wxPoint& pointSecondary)
{
	wxRect r(point, size);
	r.Intersect(bounds);

	const wxPoint translation(r.GetPosition() - point);
	point = r.GetPosition();
	pointSecondary += translation;
	size = r.GetSize();
}

bool Lafarren::Copy(
	void* destBase,
	const void* srcBase,
	const wxSize& destBounds,
	const wxSize& srcBounds,
	const wxPoint& destPoint,
	const wxRect& srcRect,
	int destStride,
	int srcStride,
	int bytesPerPixel)
{
	return Copy(
		destBase,
		srcBase,
		wxRect(wxPoint(0, 0), destBounds),
		wxRect(wxPoint(0, 0), srcBounds),
		destPoint,
		srcRect,
		destStride,
		srcStride,
		bytesPerPixel);
}

bool Lafarren::Copy(
	void* destBase,
	const void* srcBase,
	const wxRect& destBounds,
	const wxRect& srcBounds,
	const wxPoint& destPoint,
	const wxRect& srcRect,
	int destStride,
	int srcStride,
	int bytesPerPixel)
{
	bool result = false;

	if (!destBounds.IsEmpty() && !srcBounds.IsEmpty())
	{
		wxPoint destPointClipped(destPoint);
		wxPoint srcPointClipped(srcRect.GetPosition());
		wxSize sizeClipped(srcRect.GetSize());

		// Clip against src bounds, then against dest bounds, then once again
		// against src bounds in case the dest clip translation moved it significantly.
		Clip(srcBounds, srcPointClipped, sizeClipped, destPointClipped);
		Clip(destBounds, destPointClipped, sizeClipped, srcPointClipped);
		Clip(srcBounds, srcPointClipped, sizeClipped, destPointClipped);

		result = Copy(destBase, srcBase, destPointClipped, wxRect(srcPointClipped, sizeClipped), destStride, srcStride, bytesPerPixel);
	}

	return result;
}

bool Lafarren::Copy(
	void* destBase,
	const void* srcBase,
	const wxPoint& destPoint,
	const wxRect& srcRect,
	int destStride,
	int srcStride,
	int bytesPerPixel)
{
	bool result = false;

	if (destBase && srcBase && destStride > 0 && srcStride > 0 && bytesPerPixel > 0)
	{
		const int srcLeft = srcRect.GetLeft();
		const int srcBottom = srcRect.GetBottom();
		const int bytesPerRowCopy = bytesPerPixel * srcRect.GetWidth();
		if (bytesPerRowCopy == destStride && bytesPerRowCopy == srcStride)
		{
			wxASSERT(destPoint.x == 0);
			wxASSERT(srcLeft == 0);
			memcpy(
				GetRowMajorPointer(destBase, destStride, bytesPerPixel, destPoint.x, destPoint.y),
				GetRowMajorPointer(srcBase, srcStride, bytesPerPixel, srcLeft, srcRect.GetTop()),
				bytesPerRowCopy * srcRect.GetHeight());
		}
		else
		{
			for (int srcY = srcRect.GetTop(), destY = destPoint.y; srcY <= srcBottom; ++srcY, ++destY)
			{
				memcpy(
					GetRowMajorPointer(destBase, destStride, bytesPerPixel, destPoint.x, destY),
					GetRowMajorPointer(srcBase, srcStride, bytesPerPixel, srcLeft, srcY),
					bytesPerRowCopy);
			}
		}

		result = true;
	}

	return result;
}
