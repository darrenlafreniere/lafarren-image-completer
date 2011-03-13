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
#include "MaskScalable.h"

#include "tech/MathUtils.h"

#include "ImageConst.h"

#include "tech/DbgMem.h"

//
// Internal Mask implementation.
//
// Stores LOD mask data, provides coarse-to-fine region testing, and provides
// two methods of initialization: from input image data, or from halving the
// resolution of an input Mask instance.
//

class LfnIc::MaskInternal : public MaskLod
{
public:
	MaskInternal(int width, int height, const Mask& maskToCopy);
	MaskInternal(const MaskLod& maskToScaleDown);

	virtual int GetLowestLod() const;
	virtual Value GetValue(int x, int y) const;
	virtual const LodData& GetLodData(int lod) const;
	virtual const Mask::Value* GetLodBuffer(int lod) const;
	virtual bool RegionXywhHasAny(int x, int y, int w, int h, Value value) const;
	virtual bool RegionXywhHasAll(int x, int y, int w, int h, Value value) const;

private:
	// Hide copy constructor.
	MaskInternal(const MaskInternal&) {}

	// The region is specified by an inclusive left,top and an inclusive
	// right,bottom.
	bool RegionLtrbHasAny(int left, int top, int right, int bottom, Value value) const;
	bool RegionLtrbHasAll(int left, int top, int right, int bottom, Value value) const;

	//
	// Internal definitions
	//
	typedef std::vector<LodData> LodSet;

	enum RegionSearchMode
	{
		REGION_SEARCH_ANY,
		REGION_SEARCH_ALL,
	};

	//
	// Internal methods
	//
	void CreateLowerLodsFromHighest();
	LodData& AddLod();
	bool RegionLtrbSearch(int left, int top, int right, int bottom, Value value, RegionSearchMode mode) const;

	//
	// Data
	//
	int m_width;
	int m_height;
	LodSet m_lodSet;
};

LfnIc::MaskInternal::MaskInternal(int width, int height, const Mask& maskToCopy) :
m_width(width),
m_height(height)
{
	// Create lod 0 to be the same size as the input image, copying
	// `'s values at each point.
	{
		LodData& lodData0 = AddLod();

		Value* lod0Ptr = &lodData0.buffer[0];
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x, ++lod0Ptr)
			{
				*lod0Ptr = maskToCopy.GetValue(x, y);
			}
		}
	}

	CreateLowerLodsFromHighest();
}

LfnIc::MaskInternal::MaskInternal(const MaskLod& maskToScaleDown)
{
	// Create lod 0 directly from maskToScaleDown's lod 1.
	{
		const LodData& otherLodData1 = maskToScaleDown.GetLodData(1);
		m_width = otherLodData1.width;
		m_height = otherLodData1.height;

		LodData& lodData0 = AddLod();
		const int numValues = m_width * m_height;
		Value* lod0Current = &lodData0.buffer[0];
		const Value* otherLod1Current = &otherLodData1.buffer[0];
		for (int i = 0; i < numValues; ++i, ++lod0Current, ++otherLod1Current)
		{
			// When reducing the mask, force indeterminates to unknown. Lower
			// resolution passes must err on the side of performing image
			// completion when working with reduced data.
			const Value otherLod1Value = *otherLod1Current;
			if (otherLod1Value == INDETERMINATE)
			{
				*lod0Current = UNKNOWN;
			}
			else
			{
				*lod0Current = otherLod1Value;
			}
		}
	}

	CreateLowerLodsFromHighest();
}

int LfnIc::MaskInternal::GetLowestLod() const
{
	return m_lodSet.size() - 1;
}

LfnIc::Mask::Value LfnIc::MaskInternal::GetValue(int x, int y) const
{
	Mask::Value value = KNOWN;

	if (x >= 0 && y >= 0 && x < m_width && y < m_height)
	{
		value = Value(m_lodSet[0].buffer[LfnTech::GetRowMajorIndex(m_width, x, y)]);
	}

	return value;
}

const LfnIc::MaskLod::LodData& LfnIc::MaskInternal::GetLodData(int lod) const
{
	wxASSERT(lod >= GetHighestLod());
	wxASSERT(lod <= GetLowestLod());
	return m_lodSet[lod];
}

const LfnIc::Mask::Value* LfnIc::MaskInternal::GetLodBuffer(int lod) const
{
	return &GetLodData(lod).buffer[0];
}

bool LfnIc::MaskInternal::RegionXywhHasAny(int x, int y, int w, int h, Value value) const
{
	return RegionLtrbHasAny(x, y, x + w - 1, y + h - 1, value);
}

bool LfnIc::MaskInternal::RegionXywhHasAll(int x, int y, int w, int h, Value value) const
{
	return RegionLtrbHasAll(x, y, x + w - 1, y + h - 1, value);
}

bool LfnIc::MaskInternal::RegionLtrbHasAny(int left, int top, int right, int bottom, Value value) const
{
	return RegionLtrbSearch(left, top, right, bottom, value, REGION_SEARCH_ANY);
}

bool LfnIc::MaskInternal::RegionLtrbHasAll(int left, int top, int right, int bottom, Value value) const
{
	return RegionLtrbSearch(left, top, right, bottom, value, REGION_SEARCH_ALL);
}

void LfnIc::MaskInternal::CreateLowerLodsFromHighest()
{
	// Create lower lods until reaching the first all-indeterminate lod, or
	// until reaching the 1x1 lod.
	bool finished = false;
	for (int lod = 1; !finished; ++lod)
	{
		{
			// lodPrev must be retrieved separately before and after the call to
			// AddLod, since the vector resize may invalidate this lodPrev ref.
			const LodData& lodPrev = m_lodSet[lod - 1];
			finished = (lodPrev.width == 1 && lodPrev.height == 1);
		}

		if (!finished)
		{
			LodData& lodData = AddLod();
			const LodData& lodDataPrev = m_lodSet[lod - 1];

			const int lodPrevWidth = lodDataPrev.width;
			const int lodPrevHeight = lodDataPrev.height;

			// If the previous LOD had an odd width or height, then the odd
			// edge must be included in this LOD's edge.
			const int lodPrevOddEdgeX = (lodDataPrev.width & 1) ? (lodDataPrev.width - 1) : -1;
			const int lodPrevOddEdgeY = (lodDataPrev.height & 1) ? (lodDataPrev.height - 1) : -1;

			const int lodWidth = lodData.width;
			const int lodHeight = lodData.height;

			const Value* lodPrevBuffer = &lodDataPrev.buffer[0];
			Value* lodBuffer = &lodData.buffer[0];

			bool lodIsAllIndeterminate = true;
			for (int lodY = 0; lodY < lodHeight; ++lodY)
			{
				const int lodPrevY = lodY * 2;
				const bool includeLodPrevOddEdgeY = (lodPrevY + 2 == lodPrevOddEdgeY);
				const int nv = includeLodPrevOddEdgeY ? 3 : 2;

				Value* lodRowLeft = lodBuffer + LfnTech::GetRowMajorIndex(lodWidth, 0, lodY);
				for (int lodX = 0; lodX < lodWidth; ++lodX)
				{
					// Determine value by sampling the previous lod's 2x2
					// values at this point (or, 3x2, 2x3, or 3x3, depending on
					// if we're including the previous lod's odd edge(s)).
					Value value;
					{
						const int lodPrevX = lodX * 2;
						const bool includeLodPrevOddEdgeX = (lodPrevX + 2 == lodPrevOddEdgeX);
						const int nu = includeLodPrevOddEdgeX ? 3 : 2;

						bool hasSetInitialValue = false;
						for (int v = 0; v < nv; ++v)
						{
							for (int u = 0; u < nu; ++u)
							{
								wxASSERT((lodPrevX + u) < lodPrevWidth);
								wxASSERT((lodPrevY + v) < lodPrevHeight);
								const Value prevLodValueAtUV = lodPrevBuffer[LfnTech::GetRowMajorIndex(lodPrevWidth, lodPrevX + u, lodPrevY + v)];
								if (!hasSetInitialValue)
								{
									value = prevLodValueAtUV;
									hasSetInitialValue = true;
								}
								else
								{
									wxASSERT(value != INDETERMINATE);
									if (value != prevLodValueAtUV)
									{
										value = INDETERMINATE;
									}
								}

								if (value == INDETERMINATE)
								{
									break;
								}
							}

							if (value == INDETERMINATE)
							{
								break;
							}
						}
					}

					lodRowLeft[lodX] = value;
					if (value != INDETERMINATE)
					{
						lodIsAllIndeterminate = false;
					}
				}
			}

			finished = lodIsAllIndeterminate;
		}
	}
}

LfnIc::MaskLod::LodData& LfnIc::MaskInternal::AddLod()
{
	int width;
	int height;
	const int n = m_lodSet.size();
	if (n == 0)
	{
		width = m_width;
		height = m_height;
	}
	else
	{
		const LodData& lodDataPrev = m_lodSet[n - 1];
		width = std::max(1, lodDataPrev.width >> 1);
		height = std::max(1, lodDataPrev.height >> 1);
	}

	m_lodSet.resize(n + 1);
	LodData& lodData = m_lodSet[n];
	lodData.width = width;
	lodData.height = height;
	lodData.buffer.resize(lodData.width * lodData.height, KNOWN);
	return lodData;
}

bool LfnIc::MaskInternal::RegionLtrbSearch(int left, int top, int right, int bottom, Value value, RegionSearchMode mode) const
{
	wxASSERT(left <= right);
	wxASSERT(top <= bottom);

	const bool earlyReturnComparison = (mode == REGION_SEARCH_ANY) ? true : false;
	const bool earlyReturnValue = (mode == REGION_SEARCH_ANY) ? true : false;
	const bool finalReturnValue = (mode == REGION_SEARCH_ANY) ? false : true;

	int lod;
	lod = LfnTech::LogBase2(std::max(right - left, bottom - top) + 1);
	lod = std::min(int(m_lodSet.size() - 1), lod);
	wxASSERT(lod >= 0);
	for (; lod >= 0; --lod)
	{
		const LodData& lodData = m_lodSet[lod];
		const int lodWidth = lodData.width;
		const int lodHeight = lodData.height;
		const Value* lodBuffer = &lodData.buffer[0];

		const int leftAtLod   = left   >> lod;
		const int topAtLod    = top    >> lod;
		const int rightAtLod  = right  >> lod;
		const int bottomAtLod = bottom >> lod;

		bool foundIndeterminates = false;
		for (int y = topAtLod; y <= bottomAtLod; ++y)
		{
			const Value* lodRow = lodBuffer + LfnTech::GetRowMajorIndex(lodWidth, 0, y);
			for (int x = leftAtLod; x <= rightAtLod; ++x)
			{
				const Value blockValue = (x >= 0 && y >= 0 && x < lodWidth && y < lodHeight) ? lodRow[x] : KNOWN;

				if (blockValue == INDETERMINATE)
				{
					foundIndeterminates = true;
				}
				else if ((blockValue == value) == earlyReturnComparison)
				{
					return earlyReturnValue;
				}
			}
		}

		if (!foundIndeterminates)
		{
			break;
		}
	}

	return finalReturnValue;
}

//
// MaskScalable implementation
//
LfnIc::MaskScalable::MaskScalable(int inputImageWidth, int inputImageHeight, const Mask& mask) :
m_depth(0)
{
	// Create original resolution mask.
	m_resolutions.push_back(new MaskInternal(inputImageWidth, inputImageHeight, mask));
}

LfnIc::MaskScalable::~MaskScalable()
{
	for (int i = 0, n = m_resolutions.size(); i < n; ++i)
	{
		delete m_resolutions[i];
	}
}

int LfnIc::MaskScalable::GetLowestLod() const
{
	return GetCurrentResolution().GetLowestLod();
}

LfnIc::Mask::Value LfnIc::MaskScalable::GetValue(int x, int y) const
{
	return GetCurrentResolution().GetValue(x, y);
}

const LfnIc::MaskLod::LodData& LfnIc::MaskScalable::GetLodData(int lod) const
{
	return GetCurrentResolution().GetLodData(lod);
}

const LfnIc::Mask::Value* LfnIc::MaskScalable::GetLodBuffer(int lod) const
{
	return GetCurrentResolution().GetLodBuffer(lod);
}

bool LfnIc::MaskScalable::RegionXywhHasAny(int x, int y, int w, int h, Value value) const
{
	return GetCurrentResolution().RegionXywhHasAny(x, y, w, h, value);
}

bool LfnIc::MaskScalable::RegionXywhHasAll(int x, int y, int w, int h, Value value) const
{
	return GetCurrentResolution().RegionXywhHasAll(x, y, w, h, value);
}

void LfnIc::MaskScalable::ScaleUp()
{
	wxASSERT(m_depth > 0);

	// We don't expect to scale back down to this resolution, so free up some
	// memory. This is checked by the assert at the bottom of
	// MaskScalable::ScaleDown().
	delete m_resolutions[m_depth];
	m_resolutions[m_depth] = NULL;

	--m_depth;
}

void LfnIc::MaskScalable::ScaleDown()
{
	wxASSERT(m_depth >= 0);

	// If there's no mask for the next lower depth, create one from the current resolution.
	if (static_cast<unsigned int>(m_depth) == m_resolutions.size() - 1)
	{
		const MaskLod& maskToScaleDown = GetCurrentResolution();
		m_resolutions.push_back(new MaskInternal(maskToScaleDown));
	}

	++m_depth;
	wxASSERT(m_depth < int(m_resolutions.size()));

	// MaskScalable::ScaleUp() doesn't expect us to scale back down, so it
	// deletes the lower resolution without reducing the array size. Verify
	// that we have a valid resolution here.
	wxASSERT(m_resolutions[m_depth]);
}

int LfnIc::MaskScalable::GetScaleDepth() const
{
	return m_depth;
}
