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
#include "Label.h"

#include "tech/Core.h"
#include "tech/MathUtils.h"

#include "Image.h"
#include "Mask.h"
#include "PriorityBpSettings.h"

#include "tech/DbgMem.h"

wxCOMPILE_TIME_ASSERT(sizeof(PriorityBp::Label) == sizeof(int), LabelExpectedToBeSizeofInt);
wxCOMPILE_TIME_ASSERT(SHRT_MAX <= PriorityBp::Settings::IMAGE_DIMENSION_MAX, LabelStorageTooSmallForMaxImageDimension);

// See the edge case in LabelSet::Resolution::Resolution(const Resolution&).
inline void GetCoordinatesToIncludeOddEdge(int width, int height, int& outIncludeOddEdgeAtX, int& outIncludeOddEdgeAtY)
{
	const bool widthIsOdd = (width & 1) == 1;
	const bool heightIsOdd = (height & 1) == 1;

	outIncludeOddEdgeAtX = widthIsOdd ? (width - 3) : -1;
	outIncludeOddEdgeAtY = heightIsOdd ? (height - 3) : -1;
}

PriorityBp::LabelSet::LabelSet(const Settings& settings, const Image& inputImage, const MaskLod& mask) :
m_inputImage(inputImage),
m_depth(0)
{
	// Create the original resolution label set.
	m_resolutions.push_back(new Resolution(settings, inputImage, mask));
}

PriorityBp::LabelSet::~LabelSet()
{
	for (int i = 0, n = m_resolutions.size(); i < n; ++i)
	{
		delete m_resolutions[i];
	}
}

const PriorityBp::Label& PriorityBp::LabelSet::operator[](int i) const
{
	return GetCurrentResolution().labels[i];
}

int PriorityBp::LabelSet::size() const
{
	return GetCurrentResolution().labels.size();
}

void PriorityBp::LabelSet::ScaleUp()
{
	wxASSERT(m_depth > 0);

	// We don't expect to scale back down to this resolution, so free up some
	// memory. This is checked by the assert at the bottom of
	// LabelSet::ScaleDown().
	delete m_resolutions[m_depth];
	m_resolutions[m_depth] = NULL;

	--m_depth;
}

void PriorityBp::LabelSet::ScaleDown()
{
	wxASSERT(m_depth >= 0);

	// If there's no mask for the next lower depth, create one from the current resolution.
	if (static_cast<unsigned int>(m_depth) == m_resolutions.size() - 1)
	{
		const Resolution& resolutionToScaleDown = GetCurrentResolution();

		Resolution* resolution = new Resolution(resolutionToScaleDown);

		// We expect the image to have been scaled down before the label set.
		wxASSERT(resolution->labelBitArray.GetWidth() == m_inputImage.GetWidth());
		wxASSERT(resolution->labelBitArray.GetHeight() == m_inputImage.GetHeight());

		m_resolutions.push_back(resolution);
	}

	++m_depth;
	wxASSERT(m_depth < int(m_resolutions.size()));

	// LabelSet::ScaleUp() doesn't expect us to scale back down, so it
	// deletes the lower resolution without reducing the array size. Verify
	// that we have a valid resolution here.
	wxASSERT(m_resolutions[m_depth]);
}

int PriorityBp::LabelSet::GetScaleDepth() const
{
	return m_depth;
}

void PriorityBp::LabelSet::GetLowToCurrentResolutionMapping(const Label& lowResolutionLabel, LabelSet::LowToCurrentResolutionMapping& out) const
{
	int includeOddEdgeAtX, includeOddEdgeAtY;
	GetCoordinatesToIncludeOddEdge(m_inputImage.GetWidth(), m_inputImage.GetHeight(), includeOddEdgeAtX, includeOddEdgeAtY);

	const int baseX = lowResolutionLabel.left * 2;
	const int baseY = lowResolutionLabel.top * 2;

	const int blockSizeX = (baseX == includeOddEdgeAtX) ? 3 : 2;
	const int blockSizeY = (baseY == includeOddEdgeAtY) ? 3 : 2;

	const LabelBitArray& labelBitArray = GetCurrentResolution().labelBitArray;

	out.m_size = 0;
	for (int offsetY = 0; offsetY < blockSizeY; ++offsetY)
	{
		for (int offsetX = 0; offsetX < blockSizeX; ++offsetX)
		{
			const int x = baseX + offsetX;
			const int y = baseY + offsetY;
			if (labelBitArray.IsSet(x, y))
			{
				Label& label = out.m_labels[out.m_size++];
				label.left = x;
				label.top = y;
			}
		}
	}
}

PriorityBp::LabelSet::LabelBitArray::LabelBitArray(int width, int height) :
m_data(NULL),
m_width(width),
m_height(height),
m_dataNumUints(((width * height) + 7) / 8)
{
	m_data = new uint[m_dataNumUints];
	memset(m_data, 0, sizeof(uint) * m_dataNumUints);
}

PriorityBp::LabelSet::LabelBitArray::~LabelBitArray()
{
	delete [] m_data;
}

void PriorityBp::LabelSet::LabelBitArray::Set(int x, int y)
{
	int index;
	int shift;
	GetIndexAndShift(x, y, index, shift);

	m_data[index] |= (1 << shift);
	wxASSERT(IsSet(x, y));
}

bool PriorityBp::LabelSet::LabelBitArray::IsSet(int x, int y) const
{
	int index;
	int shift;
	GetIndexAndShift(x, y, index, shift);

	return (m_data[index] & (1 << shift)) != 0;
}

void PriorityBp::LabelSet::LabelBitArray::GetIndexAndShift(int x, int y, int& outIndex, int& outShift) const
{
	const int rowMajorIndex = Tech::GetRowMajorIndex(m_width, x, y);
	outIndex = rowMajorIndex / 8;
	outShift = rowMajorIndex & 7;
	wxASSERT(outIndex < m_dataNumUints);
}

PriorityBp::LabelSet::Resolution::Resolution(const Settings& settings, const Image& inputImage, const MaskLod& mask) :
labelBitArray(inputImage.GetWidth(), inputImage.GetHeight())
{
	const int width = inputImage.GetWidth();
	const int height = inputImage.GetHeight();
	labels.reserve(width * height);

	// Populate the label set.
	const int patchWidth = settings.patchWidth;
	const int patchHeight = settings.patchHeight;
	const int xMax = inputImage.GetWidth() - patchWidth;
	const int yMax = inputImage.GetHeight() - patchHeight;

	for (int y = 0; y <= yMax; ++y)
	{
		for (int x = 0; x <= xMax; ++x)
		{
			if (mask.RegionXywhHasAll(x, y, patchWidth, patchHeight, Mask::KNOWN))
			{
				wxASSERT((x + settings.patchWidth) <= width);
				wxASSERT((y + settings.patchHeight) <= height);
				labelBitArray.Set(x, y);
				labels.push_back(Label(x, y));
			}
			else
			{
				wxASSERT(!labelBitArray.IsSet(x, y));
			}
		}
	}

#if _DEBUG
	VerifyIntegrity();
#endif
}

PriorityBp::LabelSet::Resolution::Resolution(const Resolution& resolutionToScaleDown) :
labelBitArray(resolutionToScaleDown.labelBitArray.GetWidth() / 2, resolutionToScaleDown.labelBitArray.GetHeight() / 2)
{
	// We're halving the resolution. Each 2x2 label quad is reduced to a
	// single label.
	const int highResolutionWidth = resolutionToScaleDown.labelBitArray.GetWidth();
	const int highResolutionHeight = resolutionToScaleDown.labelBitArray.GetHeight();

	// Edge case (literally):
	// If either of the higher resolution dimensions are odd, then the 1 pixel
	// edge will be truncated and not represented in the lower resolution.
	// Combine the odd edges into their neighbor quads, such that if they're
	// set in the higher resolution, the neighboring quad's lower resolution
	// bit is set as well. When scaling back up, the lower resolution label
	// will be exploded to include these odd edge labels.
	//
	// If an edge is odd, its highResolutionIncludeOddEdgeAt variable will
	// be set to coordinate that should include the odd edge, and the loop will
	// detect when it needs to perform the special logic.
	//
	// If an edge is even, its highResolutionIncludeOddEdgeAt variable will
	// be negative, and the loop will correctly never perform the special logic.
	int highResolutionIncludeOddEdgeAtX, highResolutionIncludeOddEdgeAtY;
	GetCoordinatesToIncludeOddEdge(highResolutionWidth, highResolutionHeight, highResolutionIncludeOddEdgeAtX, highResolutionIncludeOddEdgeAtY);

	int blockSizeY = 0;
	for (int highResolutionY = 0; highResolutionY < highResolutionHeight; highResolutionY += blockSizeY)
	{
		blockSizeY = (highResolutionY == highResolutionIncludeOddEdgeAtY) ? 3 : 2;

		int blockSizeX = 0;
		for (int highResolutionX = 0; highResolutionX < highResolutionWidth; highResolutionX += blockSizeX)
		{
			bool shouldSetBitInLowResolution = false;
			{
				blockSizeX = (highResolutionX == highResolutionIncludeOddEdgeAtX) ? 3 : 2;
				for (int offsetY = 0; offsetY < blockSizeY; ++offsetY)
				{
					for (int offsetX = 0; offsetX < blockSizeX; ++offsetX)
					{
						if (resolutionToScaleDown.labelBitArray.IsSet(highResolutionX + offsetX, highResolutionY + offsetY))
						{
							shouldSetBitInLowResolution = true;
							break;
						}
					}
				}
			}

			if (shouldSetBitInLowResolution)
			{
				const int lowResolutionX = highResolutionX / 2;
				const int lowResolutionY = highResolutionY / 2;
				labelBitArray.Set(lowResolutionX, lowResolutionY);
				labels.push_back(Label(lowResolutionX, lowResolutionY));
			}
			else
			{
				wxASSERT(!labelBitArray.IsSet(highResolutionX / 2, highResolutionY / 2));
			}
		}
	}

#if _DEBUG
	VerifyIntegrity();
#endif
}

namespace PriorityBp
{
	#if _DEBUG
	struct LabelLessThan : public std::binary_function<Label, Label, bool>
	{
		bool operator()(const Label& a, const Label& b) const
		{
			return a.top < b.top || (a.top == b.top && a.left < b.left);
		}
	};
}

void PriorityBp::LabelSet::Resolution::VerifyIntegrity()
{
	std::set<Label, LabelLessThan> labelSet;
	for (int i = 0, n = labels.size(); i < n; ++i)
	{
		labelSet.insert(labels[i]);
	}

	for (int y = 0, ny = labelBitArray.GetHeight(); y < ny; ++y)
	{
		for (int x = 0, nx = labelBitArray.GetWidth(); x < nx; ++x)
		{
			const bool isLabelValid = (labelSet.find(Label(x, y)) != labelSet.end());
			if (labelBitArray.IsSet(x, y))
			{
				wxASSERT(isLabelValid);
			}
			else
			{
				wxASSERT(!isLabelValid);
			}
		}
	}
}
#endif
