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

#ifndef LFN_MASK_H
#define LFN_MASK_H

#include "LfnIcImage.h"
#include "Scalable.h"

namespace LfnIc
{
	//
	// Encapsulates the image mask's data, and provides methods for querying
	// mask data on a single pixel or a rectangular region at various levels
	// of detail. No assumptions are made about values outside of the valid
	// mask, and are therefore considered KNOWN.
	//
	class Mask
	{
	public:
		// Use typedef and consts rather than enum to enforce single byte storage:
		typedef char Value;

		// Indeterminate is used for lower levels of detail (lod > 0),
		// where the region in question contains both unknown and known
		// pixels. Examination at a higher level of detail may be required.
		static const Value INDETERMINATE;

		// Unknown pixels should be filled by the image completer.
		static const Value UNKNOWN;

		// Ignored pixels should not be filled by the image completer,
		// nor may they be used as input for the image completion
		// process, nor should they be used in energy calculations.
		static const Value IGNORED;

		// Known pixels should not be filled by the image completer,
		// but may be used as input for the image completion process.
		static const Value KNOWN;

		// Samples a single value.
		virtual Value GetValue(int x, int y) const = 0;

	protected:
		// Cannot destroy Mask instance through a pointer to this base.
		~Mask() {}
	};

	// A mask that stores its data in multiple levels of detail, and is
	// therefore able to perform efficient tests over an entire region.
	class MaskLod : public Mask
	{
	public:
		typedef std::vector<Value> LodBuffer;
		struct LodData
		{
			int width;
			int height;
			LodBuffer buffer;
		};

		// Returns the level of detail range that this mask supports. 0
		// is always the highest level of detail, where each region is 1x1
		// pixel and thus contains no indeterminates. Lower levels of detail
		// are identified by successive > 0 values. A region's max size at
		// any given lod is (2^lod)x(2^lod).
		inline int GetHighestLod() const { return 0; }
		virtual int GetLowestLod() const = 0;

		// Returns a const reference to the specified lod's data. Does not
		// verify that lod is >= GetHighestLod() and <= GetLowestLod().
		virtual const LodData& GetLodData(int lod) const = 0;

		// Convenience function for getting a pointer to an lod data's buffer.
		virtual const Value* GetLodBuffer(int lod) const = 0;

		// The region is specified by an inclusive upper left x,y, and by an
		// exclusive width and height. This method properly handles regions
		// outside of the mask boundaries, and treats that area as KNOWN.
		virtual bool RegionXywhHasAny(int x, int y, int w, int h, Value value) const = 0;
		virtual bool RegionXywhHasAll(int x, int y, int w, int h, Value value) const = 0;

	protected:
		// Cannot destroy MaskLod instance through a pointer to this base.
		~MaskLod() {}
	};

	// Forward declaration. Defined in Mask.cpp.
	class MaskInternal;

	//
	// Implements both the Mask and Scalable interfaces and provides an in
	// place scalable mask. Initializes the mask from the input image data.
	//
	class MaskScalable : public MaskLod, public Scalable
	{
	public:
		MaskScalable(int inputImageWidth, int inputImageHeight, const Image& maskImage, int maskImageOffsetX = 0, int maskImageOffsetY = 0);
		~MaskScalable();

		virtual int GetLowestLod() const;
		virtual Value GetValue(int x, int y) const;
		virtual const LodData& GetLodData(int lod) const;
		virtual const Value* GetLodBuffer(int lod) const;
		virtual bool RegionXywhHasAny(int x, int y, int w, int h, Value value) const;
		virtual bool RegionXywhHasAll(int x, int y, int w, int h, Value value) const;

		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

	private:
		inline MaskInternal& GetCurrentResolution() const { return *m_resolutions[m_depth]; }

		std::vector<MaskInternal*> m_resolutions;
		int m_depth;
	};
}

#endif
