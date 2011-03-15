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
#include "AppWxMask.h"
#include "LfnIcSettings.h"

#include "tech/MathUtils.h"

#include "tech/DbgMem.h"

AppWxMask::AppWxMask()
	: m_width(0)
	, m_height(0)
	, m_offsetX(0)
	, m_offsetY(0)
{
}

bool AppWxMask::LoadAndValidate(const std::string& imagePath, int offsetX, int offsetY)
{
    bool result = false;
    wxMessageOutput& msgOut = *wxMessageOutput::Get();

    wxImage mask;

    if (!mask.LoadFile(imagePath))
    {
        // If LoadFile fails, it already prints an wxMessageOutput error for us.
    }
    else if (!mask.IsOk())
    {
        msgOut.Printf("The image was invalid.\n");
    }
    else if (mask.GetWidth() > LfnIc::Settings::IMAGE_WIDTH_MAX || mask.GetHeight() > LfnIc::Settings::IMAGE_HEIGHT_MAX)
    {
        msgOut.Printf("The image is too large. Max size: %dx%x.\n", LfnIc::Settings::IMAGE_WIDTH_MAX, LfnIc::Settings::IMAGE_HEIGHT_MAX);
    }
    else
    {
        result = true;
    }

    wxASSERT(mask.IsOk());
    const wxImage& maskImageGreyscale = mask.ConvertToGreyscale();
    wxASSERT(maskImageGreyscale.IsOk());

    m_width = mask.GetWidth();
    m_height = mask.GetHeight();
    m_offsetX = offsetX;
    m_offsetY = offsetY;
    m_values.resize(m_width * m_height);

    const wxImage::RGBValue* rgbPtr = reinterpret_cast<const wxImage::RGBValue*>(maskImageGreyscale.GetData());
    const int numPixels = m_width * m_height;
    for (int i = 0; i < numPixels; ++i)
    {
        wxASSERT(rgbPtr[i].red == rgbPtr[i].green);
        wxASSERT(rgbPtr[i].red == rgbPtr[i].blue);
        m_values[i] = ByteToMaskValue(rgbPtr[i].red);
    }

    return result;
}

LfnIc::Mask::Value AppWxMask::GetValue(int x, int y) const
{
	const int xMaskSpace = x - m_offsetX;
	const int yMaskSpace = y - m_offsetY;

	Value value = Mask::KNOWN;
	if (xMaskSpace >= 0 && yMaskSpace >= 0 && xMaskSpace < m_width && yMaskSpace < m_height)
	{
		value = m_values[LfnTech::GetRowMajorIndex(m_width, xMaskSpace, yMaskSpace)];
	}

	return value;
}

LfnIc::Mask::Value AppWxMask::ByteToMaskValue(unsigned char byte) const
{
	const int byteUnknown = 0;
	const int byteIgnored = 128;
	const int byteKnown   = 255;

	const int byteIgnoredDiff = abs(byte - byteIgnored);
	if ((byte - byteUnknown) <= byteIgnoredDiff)
	{
		return UNKNOWN;
	}
	else
	{
		if (byteIgnoredDiff <= (byteKnown - byte))
		{
			return IGNORED;
		}
		else
		{
			return KNOWN;
		}
	}
}
