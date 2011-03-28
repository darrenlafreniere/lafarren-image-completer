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

#ifndef APP_ITK_IMAGE_H
#define APP_ITK_IMAGE_H

#ifdef USE_ITK

#include "AppData.h"

#include "itkImage.h"
#include "itkVectorImage.h"

//
// Implements LfnIc::Image, using a itk::Image to load, store, and save the data.
//
class AppITKImage : public AppData::Image
{
public:
	// AppITKImage interface
	AppITKImage();

	typedef itk::VectorImage<LfnIc::Image::Pixel::PixelType, 2> AppImageITKType;

	// LfnIc::Image interface
	bool LoadAndValidate(const std::string& imagePath);
	void Save();
	virtual bool Init(int width, int height);
	virtual bool IsValid() const;
	virtual const std::string& GetFilePath() const;
	virtual Pixel* GetData();
	virtual const Pixel* GetData() const;
	virtual int GetWidth() const;
	virtual int GetHeight() const;

private:
	// Internal data
	AppImageITKType::Pointer m_image;
    float m_channelWeights[LfnIc::Image::Pixel::NUM_CHANNELS];
};

#endif // USE_ITK
#endif
