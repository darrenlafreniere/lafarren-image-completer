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
#include "ImageScalable.h"

#include "tech/ImageUtils.h"
#include "MaskScalable.h"

#include "tech/DbgMem.h"

//
// Internal Image extension. Exists to give ImageScalable destruction permission.
//
namespace LfnIc
{
	class ImageConstInternal : public ImageConst
	{
	public:
		virtual ~ImageConstInternal() {}
	};
}

//
// Extends ImageConstInternal by delegating all calls to a Image instance.
//
namespace LfnIc
{
	class ImageConstDelegateToImage : public ImageConstInternal
	{
	public:
		ImageConstDelegateToImage(const Image& image);

		virtual const Pixel* GetData() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

	private:
		// Hide copy constructor.
		ImageConstDelegateToImage(const ImageConstDelegateToImage&) : m_image(*reinterpret_cast<const Image*>(0xCDCDCDCD)) {}

		const Image& m_image;
	};
}

LfnIc::ImageConstDelegateToImage::ImageConstDelegateToImage(const Image& image) :
m_image(image)
{
}

const LfnIc::Image::Pixel* LfnIc::ImageConstDelegateToImage::GetData() const
{
	return m_image.GetData();
}

int LfnIc::ImageConstDelegateToImage::GetWidth() const
{
	return m_image.GetWidth();
}

int LfnIc::ImageConstDelegateToImage::GetHeight() const
{
	return m_image.GetHeight();
}

//
// Extends ImageConstInternal. Initializes its data from halving the resolution of
// an input Image instance
//
namespace LfnIc
{
	class ImageScaledDown : public ImageConstInternal
	{
	public:
		ImageScaledDown(const ImageConst& imageToScaleDown, const Mask& imageToScaleDownMask);
		virtual ~ImageScaledDown();

		virtual const Pixel* GetData() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

	private:
		// Hide copy constructor.
		ImageScaledDown(const ImageScaledDown&) {}

		int m_width;
		int m_height;
		Pixel* m_rgb;
	};
}

LfnIc::ImageScaledDown::ImageScaledDown(const ImageConst& imageToScaleDown, const Mask& imageToScaleDownMask)
{
	// Alias to reduce wordiness.
	const int otherWidth = imageToScaleDown.GetWidth();
	const int otherHeight = imageToScaleDown.GetHeight();
	const Pixel* otherRgb = imageToScaleDown.GetData();

	// Downsample otherRgb into m_rgb by averaging 2x2 pixel blocks into 1 pixel.
	// The low resolution image is half that of the high resolution image.
	m_width = otherWidth / 2;
	m_height = otherHeight / 2;
	wxASSERT(m_width > 0 && m_height > 0);

	m_rgb = new Pixel[m_width * m_height];

	const int stride = m_width * sizeof(Pixel);
	const int otherStride = otherWidth * sizeof(Pixel);
	for (int y = 0, otherY = 0; y < m_height; ++y, otherY += 2)
	{
		wxASSERT(otherY < otherHeight);
		Pixel* rgbCurrent = LfnTech::GetRowMajorPointer(m_rgb, stride, 0, y);

		const Pixel* otherRgbCurrentUpper = LfnTech::GetRowMajorPointer(otherRgb, otherStride, 0, otherY);

		// If the bottom edge of the imageToScaleDown image has no lower row,
		// point at the upper one; it'll average to the same value
		// and avoids an extra conditional in the loop.
		const Pixel* otherRgbCurrentLower = ((otherY + 1) < otherHeight)
			? LfnTech::GetRowMajorPointer(otherRgb, otherStride, 0, otherY + 1)
			: otherRgbCurrentUpper;

		for (int x = 0, otherX = 0; x < m_width; ++x, ++rgbCurrent, otherX += 2, otherRgbCurrentUpper += 2, otherRgbCurrentLower += 2)
		{
			wxASSERT(otherX < otherWidth);
			float channel[Image::Pixel::NUM_CHANNELS] = { 0.0f };

			// numPixelsToAverage is the number of high resolution pixels we're
			// collapsing/averaging into a single low resolution pixel. At most,
			// this will be four, but may be zero if all four pixels are unknown.
			float numPixelsToAverage = 0.0f;

			// Read the right column of the 2x2 quad.
			{
				// Left-top of 2x2 quad.
				if (imageToScaleDownMask.GetValue(otherX, otherY) == Mask::KNOWN)
				{
					numPixelsToAverage += 1.0f;
					for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
					{
						channel[component] += otherRgbCurrentUpper[0].channel[component];
					}
				}

				// Left-bottom of 2x2 quad.
				if (imageToScaleDownMask.GetValue(otherX, otherY + 1) == Mask::KNOWN)
				{
					numPixelsToAverage += 1.0f;
					for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
					{
						channel[component] += otherRgbCurrentLower[0].channel[component];
					}
				}
			}

			// Read the right column of the 2x2 quad. Make sure we don't read
			// off the right-hand edge.
			if ((otherX + 1) < otherWidth)
			{
				// Right-top of 2x2 quad.
				if (imageToScaleDownMask.GetValue(otherX + 1, otherY) == Mask::KNOWN)
				{
					numPixelsToAverage += 1.0f;
					for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
					{
						channel[component] += otherRgbCurrentUpper[1].channel[component];
					}
				}

				// Right-bottom of 2x2 quad.
				if (imageToScaleDownMask.GetValue(otherX + 1, otherY + 1) == Mask::KNOWN)
				{
					numPixelsToAverage += 1.0f;
					for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
					{
						channel[component] += otherRgbCurrentLower[1].channel[component];
					}
				}
			}

			// Average the components if there's anything to average.
			if (numPixelsToAverage > 0.0f)
			{
				for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
				{
					channel[component] /= numPixelsToAverage;
					//wxASSERT(channel[component] >= 0.0f && channel[component] <= 255.0f);
				}
			}

			// Assign the averaged components to the pixel.
			for (int component = 0; component < Image::Pixel::NUM_CHANNELS; ++component)
			{
				rgbCurrent->channel[component] = channel[component];
			}
		}
	}
}

LfnIc::ImageScaledDown::~ImageScaledDown()
{
	delete [] m_rgb;
}

const LfnIc::Image::Pixel* LfnIc::ImageScaledDown::GetData() const
{
	return m_rgb;
}

int LfnIc::ImageScaledDown::GetWidth() const
{
	return m_width;
}

int LfnIc::ImageScaledDown::GetHeight() const
{
	return m_height;
}

//
// ImageScalable implementation
//
LfnIc::ImageScalable::ImageScalable(const Image& image, const MaskScalable& maskScalable)
	: m_maskScalable(maskScalable)
	, m_depth(0)
{
	// Delegate to original resolution Image at depth 0.
	m_resolutions.push_back(new ImageConstDelegateToImage(image));
}

LfnIc::ImageScalable::~ImageScalable()
{
	for (int i = 0, n = m_resolutions.size(); i < n; ++i)
	{
		delete m_resolutions[i];
	}
}

const LfnIc::Image::Pixel* LfnIc::ImageScalable::GetData() const
{
	return GetCurrentResolution().GetData();
}

int LfnIc::ImageScalable::GetWidth() const
{
	return GetCurrentResolution().GetWidth();
}

int LfnIc::ImageScalable::GetHeight() const
{
	return GetCurrentResolution().GetHeight();
}

void LfnIc::ImageScalable::ScaleUp()
{
	wxASSERT(m_depth > 0);

	// We don't expect to scale back down to this resolution, so free up some
	// memory. This is checked by the assert at the bottom of
	// ImageScalable::ScaleDown().
	delete m_resolutions[m_depth];
	m_resolutions[m_depth] = NULL;

	--m_depth;
}

void LfnIc::ImageScalable::ScaleDown()
{
	wxASSERT(m_depth >= 0);

	// If there's no mask for the next lower depth, create one from the current resolution.
	if (static_cast<unsigned int>(m_depth) == m_resolutions.size() - 1)
	{
		// ImageScaledDown requires the mask to be at the same depth as the
		// input image, in order to exclude unknown pixels from being averaged
		// into the result. If this assert is hit, check the order of the
		// scopedScaleDownAndUpInOrder Add() calls in LfnIc.cpp.
		wxASSERT_MSG(m_depth == m_maskScalable.GetScaleDepth(), "MaskScalable is being scaled down before ImageScalable!");

		const ImageConst& imageToScaleDown = GetCurrentResolution();
		m_resolutions.push_back(new ImageScaledDown(imageToScaleDown, m_maskScalable));
	}

	++m_depth;
	wxASSERT(m_depth < int(m_resolutions.size()));

	// ImageScalable::ScaleUp() doesn't expect us to scale back down, so it
	// deletes the lower resolution without reducing the array size. Verify
	// that we have a valid resolution here.
	wxASSERT(m_resolutions[m_depth]);
}

int LfnIc::ImageScalable::GetScaleDepth() const
{
	return m_depth;
}
