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

#ifdef USE_IK
#include "AppImageITK.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkNthElementImageAdaptor.h"
#include "itkMinimumMaximumImageCalculator.h"

float LfnIc::Image::ComponentWeights[Pixel::NUM_CHANNELS];

ITKImage::ITKImage()
{
	m_Image = NULL;
}

bool ITKImage::LoadAndValidate(const std::string& imagePath)
{
	typedef itk::ImageFileReader<ITKImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(imagePath);
	reader->Update();

	if(!m_Image)
	{
		m_Image = ITKImageType::New();
	}

	//this->m_Image->Graft(reader->GetOutput());

	m_Image->SetRegions(reader->GetOutput()->GetLargestPossibleRegion());
	m_Image->Allocate();

	itk::ImageRegionConstIterator<ITKImageType> inputIterator(reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion());
	itk::ImageRegionIterator<ITKImageType> outputIterator(m_Image, m_Image->GetLargestPossibleRegion());

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
		typedef itk::NthElementImageAdaptor<ITKImageType, float> ImageAdaptorType;
		ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
		adaptor->SelectNthElement(i);
		adaptor->SetImage(m_Image);

		typedef itk::MinimumMaximumImageCalculator <ImageAdaptorType> ImageCalculatorFilterType;
		ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
		imageCalculatorFilter->SetImage(adaptor);
		imageCalculatorFilter->Compute();

		LfnIc::Image::ComponentWeights[i] = 255. / (imageCalculatorFilter->GetMaximum() - imageCalculatorFilter->GetMinimum());
		std::cout << LfnIc::Image::ComponentWeights[i] << " ";
	}
	std::cout << std::endl;

	return true;
}

void ITKImage::Save()
{
	// If the image is RGB and unsigned char, write it to the specified output file (likely png)
	typedef itk::ImageFileWriter<ITKImageType> WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(this->m_Image);

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

bool ITKImage::IsValid() const
{
	return m_Image != NULL;
}

const std::string& ITKImage::GetFilePath() const
{
	return m_filePath;
}

bool ITKImage::Init(int width, int height)
{
	itk::Index<2> start;
	start.Fill(0);

	itk::Size<2> size;
	size[0] = width;
	size[1] = height;

	itk::ImageRegion<2> region(start,size);

	if(!m_Image)
	{
		m_Image = ITKImageType::New();
	}

	m_Image->SetRegions(region);
	m_Image->Allocate();
	m_Image->FillBuffer(itk::NumericTraits<ITKPixelType>::Zero);

	return true;
}

LfnIc::Image::Pixel* ITKImage::GetData()
{
	return reinterpret_cast<LfnIc::Image::Pixel*>(m_Image->GetBufferPointer());
}

const LfnIc::Image::Pixel* ITKImage::GetData() const
{
	return reinterpret_cast<const LfnIc::Image::Pixel*>(m_Image->GetBufferPointer());
}

int ITKImage::GetWidth() const
{
	return m_Image->GetLargestPossibleRegion().GetSize()[0];
}

int ITKImage::GetHeight() const
{
	return m_Image->GetLargestPossibleRegion().GetSize()[1];
}

#endif // USE_IK
