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

#ifdef USE_WX
#include "AppWxImage.h"
#include "LfnIcSettings.h"

bool AppWxImage::LoadAndValidate(const std::string& imagePath)
{
	bool result = false;
	wxMessageOutput& msgOut = *wxMessageOutput::Get();

	if (!m_wxImage.LoadFile(imagePath))
	{
		// If LoadFile fails, it already prints an wxMessageOutput error for us.
	}
	else if (!m_wxImage.IsOk())
	{
		msgOut.Printf("The image was invalid.\n");
	}
	else if (m_wxImage.GetWidth() > LfnIc::Settings::IMAGE_WIDTH_MAX || m_wxImage.GetHeight() > LfnIc::Settings::IMAGE_HEIGHT_MAX)
	{
		msgOut.Printf("The image is too large. Max size: %dx%x.\n", LfnIc::Settings::IMAGE_WIDTH_MAX, LfnIc::Settings::IMAGE_HEIGHT_MAX);
	}
	else
	{
		result = true;

#ifdef USE_FLOAT_PIXELS
		// Copy into float pixels.
		{
			wxASSERT(LfnIc::Image::Pixel::NUM_CHANNELS == 3);
			const wxImage::RGBValue* rgbPtr = reinterpret_cast<const wxImage::RGBValue*>(m_wxImage.GetData());
			const int numPixels = GetWidth() * GetHeight();
			m_pixels.resize(numPixels);
			for (int i = 0; i < numPixels; ++i, ++rgbPtr)
			{
				LfnIc::Image::Pixel& pixel = m_pixels[i];
				const wxImage::RGBValue& rgb = *rgbPtr;
				pixel.channel[0] = rgb.red;
				pixel.channel[1] = rgb.green;
				pixel.channel[2] = rgb.blue;
			}
		}
#endif
	}

	return result;
}

void AppWxImage::Save()
{
#ifdef USE_FLOAT_PIXELS
	// Copy float pixels into m_wxImage before saving.
	{
		wxImage::RGBValue* rgbPtr = reinterpret_cast<wxImage::RGBValue*>(m_wxImage.GetData());
		for (int i = 0, n = GetWidth() * GetHeight(); i < n; ++i, ++rgbPtr)
		{
			wxImage::RGBValue& rgb = *rgbPtr;
			const LfnIc::Image::Pixel& pixel = m_pixels[i];
			rgb.red   = pixel.channel[0];
			rgb.green = pixel.channel[1];
			rgb.blue  = pixel.channel[2];
		}
	}
#endif
	m_wxImage.SaveFile(m_filePath);
}

bool AppWxImage::IsValid() const
{
	return m_wxImage.Ok();
}

const std::string& AppWxImage::GetFilePath() const
{
	return m_filePath;
}

bool AppWxImage::Init(int width, int height)
{
#ifdef USE_FLOAT_PIXELS
	m_pixels.resize(width * height);
#endif
	return m_wxImage.Create(width, height, false);
}

LfnIc::Image::Pixel* AppWxImage::GetData()
{
#ifdef USE_FLOAT_PIXELS
	return &m_pixels[0];
#else
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_wxImage.GetData());
#endif
}

const LfnIc::Image::Pixel* AppWxImage::GetData() const
{
#ifdef USE_FLOAT_PIXELS
	return &m_pixels[0];
#else
	return reinterpret_cast<const LfnIc::Image::Pixel*>(m_wxImage.GetData());
#endif
}

int AppWxImage::GetWidth() const
{
	return m_wxImage.GetWidth();
}

int AppWxImage::GetHeight() const
{
	return m_wxImage.GetHeight();
}

#endif // USE_WX
