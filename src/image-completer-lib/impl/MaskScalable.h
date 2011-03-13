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

#ifndef MASK_SCALABLE_H
#define MASK_SCALABLE_H

#include "LfnIcImage.h"
#include "MaskLod.h"
#include "Scalable.h"

namespace LfnIc
{
	// Forward declaration. Defined in Mask.cpp.
	class MaskInternal;

	// TODO: this will be moved to the cmd line project, and made to accept a wxImage
	// instead of a LfcIc::Image. The Complete() interface must be reworked first.
	class AppWxMask : public Mask
	{
	public:
		AppWxMask(int inputImageWidth, int inputImageHeight, const Image& maskImage, int offsetX = 0, int offsetY = 0);
		virtual Value GetValue(int x, int y) const;

	private:
		std::vector<Value> m_values;
		const int m_width;
		const int m_height;
		const int m_offsetX;
		const int m_offsetY;
	};

	//
	// Implements both the Mask and Scalable interfaces and provides an in
	// place scalable mask. Initializes the mask from the highest resolution
	// mask (the input mask passed to LfnIc::Complete()).
	//
	class MaskScalable : public MaskLod, public Scalable
	{
	public:
		MaskScalable(int inputImageWidth, int inputImageHeight, const Mask& mask);
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
