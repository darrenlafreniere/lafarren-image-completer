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

#include "ITKImage.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

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
  this->m_Image->Graft(reader->GetOutput());

  return true;
}

void ITKImage::Save()
{
  typedef itk::ImageFileWriter<ITKImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(this->m_Image);
  writer->SetFileName(m_filePath);
  writer->Update();
}

bool ITKImage::IsValid() const
{
	return m_Image;
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
