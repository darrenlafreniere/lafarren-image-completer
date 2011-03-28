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

	// Setup channel weights

	// Un-weighted
	/*
	for(unsigned int i = 0; i < static_cast<unsigned int>(LfnIc::Image::Pixel::NUM_CHANNELS); i++)
	{
	LfnIc::Image::ComponentWeights[i] = 1.0;
	}
	*/

	// Manual weights
	/*
	LfnIc::Image::ComponentWeights[0] = .1;
	LfnIc::Image::ComponentWeights[1] = .1;
	LfnIc::Image::ComponentWeights[2] = .1;
	LfnIc::Image::ComponentWeights[3] = 200.0;
	*/

	// Uniform weighting - set the weight of each channel so it has the perceived range of 255
	// If a channel already has the range 255, the weight is set to 1. If a channel has a range smaller
	// than 255, its weight will be > 1. If a channel has a weight larger than 255, its weight will be set to < 1.
	// A weight should never be negative. There is no magic to scaling to 255, it is just that usually there will be some
	// RGB type components so 255 should make several of the weights close to 1.

	std::cout << "Weights: ";
	for(unsigned int i = 0; i < static_cast<unsigned int>(LfnIc::Image::Pixel::NUM_CHANNELS); i++)
	{
    /*
		typedef itk::NthElementImageAdaptor<AppImageITKType, float> ImageAdaptorType;
		ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
		adaptor->SelectNthElement(i);
		adaptor->SetImage(m_image);
    */

    typedef itk::Image<LfnIc::Image::Pixel::PixelType, 2> ScalarImageType;

    typedef itk::VectorIndexSelectionCastImageFilter<AppImageITKType, ScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(i);
    indexSelectionFilter->SetInput(m_image);
    indexSelectionFilter->Update();

    typedef itk::MinimumMaximumImageCalculator <ScalarImageType> ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(indexSelectionFilter->GetOutput());
    imageCalculatorFilter->Compute();

    m_componentWeights[i] = 255. / (imageCalculatorFilter->GetMaximum() - imageCalculatorFilter->GetMinimum());
    std::cout << m_componentWeights[i] << " ";
	}
	std::cout << std::endl;

	return true;
}

float AppITKImage::GetComponentWeight(unsigned int component) const
{
  if(component >= static_cast<unsigned int>(Pixel::NUM_CHANNELS))
  {
      std::cerr << "Requested weight for component " << component << " and there are only " << Pixel::NUM_CHANNELS << " components!" << std::endl;
      exit(-1);
  }
  return m_componentWeights[component];
}

void AppITKImage::Save()
{
	// If the image is RGB and unsigned char, write it to the specified output file (likely png)
	typedef itk::ImageFileWriter<AppImageITKType> WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(m_image);

	if(typeid(unsigned char) == typeid(Image::Pixel::PixelType) && Image::Pixel::NUM_CHANNELS == 3)
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
	//return m_image != NULL;
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
