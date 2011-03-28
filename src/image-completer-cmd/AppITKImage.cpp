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

#ifdef USE_ITK
#include "AppITKImage.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
//#include "itkNthElementImageAdaptor.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

// TODO: move channel weighting into AppData::Image?
#define USE_CHANNEL_WEIGHTING 1

AppITKImage::AppITKImage()
{
	m_image = NULL;
}

bool AppITKImage::LoadAndValidate(const std::string& imagePath)
{
	typedef itk::ImageFileReader<AppImageITKType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(imagePath);
	reader->Update();

	if(!m_image)
	{
		m_image = AppImageITKType::New();
	}

	// Deep copy

	m_image->SetRegions(reader->GetOutput()->GetLargestPossibleRegion());
	m_image->SetNumberOfComponentsPerPixel(LfnIc::Image::Pixel::NUM_CHANNELS);
	m_image->Allocate();

	itk::ImageRegionConstIterator<AppImageITKType> inputIterator(reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion());
	itk::ImageRegionIterator<AppImageITKType> outputIterator(m_image, m_image->GetLargestPossibleRegion());

	while(!inputIterator.IsAtEnd())
	{
		outputIterator.Set(inputIterator.Get());
		++inputIterator;
		++outputIterator;
	}

#if USE_CHANNEL_WEIGHTING
	// Setup channel weights - Uniform weighting - set the weight of each channel so it has the perceived range of 255
	// If a channel already has the range 255, the weight is set to 1. If a channel has a range smaller
	// than 255, its weight will be > 1. If a channel has a weight larger than 255, its weight will be set to < 1.
	// A weight should never be negative. There is no magic to scaling to 255, it is just that usually there will be some
	// RGB type channels so 255 should make several of the weights close to 1.
	std::cout << "Weights: ";
	for (int c = 0; c < LfnIc::Image::Pixel::NUM_CHANNELS; c++)
	{
	    typedef itk::Image<LfnIc::Image::Pixel::ChannelType, 2> ScalarImageType;

	    typedef itk::VectorIndexSelectionCastImageFilter<AppImageITKType, ScalarImageType> IndexSelectionType;
	    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
	    indexSelectionFilter->SetIndex(c);
	    indexSelectionFilter->SetInput(m_image);
	    indexSelectionFilter->Update();

	    typedef itk::MinimumMaximumImageCalculator <ScalarImageType> ImageCalculatorFilterType;
	    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
	    imageCalculatorFilter->SetImage(indexSelectionFilter->GetOutput());
	    imageCalculatorFilter->Compute();

	    m_channelWeights[c] = 255. / (imageCalculatorFilter->GetMaximum() - imageCalculatorFilter->GetMinimum());
	    std::cout << m_channelWeights[c] << " ";
	}
	std::cout << std::endl;

	// Now that the weights have been calculated, apply them directly to the image data.
	LfnIc::Image::Pixel* pixelPtr = AppITKImage::GetData();
	for (int i = 0, n = GetWidth() * GetHeight(); i < n; ++i, ++pixelPtr)
	{
		LfnIc::Image::Pixel& pixel = *pixelPtr;
		for (int c = 0; c < LfnIc::Image::Pixel::NUM_CHANNELS; ++c)
		{
			pixel.channel[c] *= m_channelWeights[c];
		}
	}
#endif // USE_CHANNEL_WEIGHTING

	return true;
}

void AppITKImage::Save()
{
  std::cout << "Save()" << std::endl;
#if USE_CHANNEL_WEIGHTING
std::cout << "Output weights: ";
    for (int c = 0; c < LfnIc::Image::Pixel::NUM_CHANNELS; c++)
    {
        std::cout << m_channelWeights[c] << " ";
    }
    std::cout << std::endl;

    LfnIc::Image::Pixel* pixelPtr = AppITKImage::GetData();
    for (int i = 0, n = GetWidth() * GetHeight(); i < n; ++i, ++pixelPtr)
    {
        LfnIc::Image::Pixel& pixel = *pixelPtr;
        for (int c = 0; c < LfnIc::Image::Pixel::NUM_CHANNELS; ++c)
        {
            pixel.channel[c] /= m_channelWeights[c];
        }
    }
#endif

	// If the image is RGB and unsigned char, write it to the specified output file (likely png)
	typedef itk::ImageFileWriter<AppImageITKType> WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(m_image);

	if (LfnIc::Image::PixelInfo::IS_24_BIT_RGB)
	{
		writer->SetFileName(m_filePath);
	}
	else
	{
		// If the image is not 3 channel and unsigned char, append ".mhd" to the end of the filename so it can be written
		std::stringstream ss;
		ss << m_filePath << ".mhd";
		writer->SetFileName(ss.str());
	}

	writer->Update();
}

bool AppITKImage::IsValid() const
{
	return m_image;
}

const std::string& AppITKImage::GetFilePath() const
{
	return m_filePath;
}

bool AppITKImage::Init(int width, int height)
{
	itk::Index<2> start;
	start.Fill(0);

	itk::Size<2> size;
	size[0] = width;
	size[1] = height;

	itk::ImageRegion<2> region(start,size);

	if(!m_image)
	{
		m_image = AppImageITKType::New();
	}

	m_image->SetRegions(region);
    m_image->SetNumberOfComponentsPerPixel(LfnIc::Image::Pixel::NUM_CHANNELS);
	m_image->Allocate();
//	m_image->FillBuffer(itk::NumericTraits<ITKPixelType>::Zero);

	return true;
}

LfnIc::Image::Pixel* AppITKImage::GetData()
{
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_image->GetBufferPointer());
}

const LfnIc::Image::Pixel* AppITKImage::GetData() const
{
	return reinterpret_cast<const LfnIc::Image::Pixel*>(m_image->GetBufferPointer());
}

int AppITKImage::GetWidth() const
{
	return m_image->GetLargestPossibleRegion().GetSize()[0];
}

int AppITKImage::GetHeight() const
{
	return m_image->GetLargestPossibleRegion().GetSize()[1];
}

#endif // USE_ITK
