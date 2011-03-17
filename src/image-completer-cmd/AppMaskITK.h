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

#ifndef ITK_MASK_H
#define ITK_MASK_H

#ifdef USE_IK

#include <vector>

#include "LfnIcMask.h"

#include "itkImage.h"

//
// Implements LfnIc::Mask, using a itk::Image to load and convert a mask image.
//
class AppMaskITK : public LfnIc::Mask
{
public:
	AppMaskITK();

	typedef itk::Image<unsigned char, 2> MaskImageType;

	// The mask image can be smaller than the input image, and translated to a
	// specific offset. This isn't yet supported via the command line arguments.
	virtual Value GetValue(int x, int y) const;

	virtual bool LoadAndValidate(const std::string& imagePath, int offsetX = 0, int offsetY = 0);

private:
	Value ByteToMaskValue(unsigned char byte) const;

	MaskImageType::Pointer m_mask;

	int m_offsetX;
	int m_offsetY;
};

#endif // USE_IK
#endif
