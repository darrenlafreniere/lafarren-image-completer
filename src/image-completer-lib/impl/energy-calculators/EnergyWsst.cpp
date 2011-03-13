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
#include "EnergyWsst.h"

#if ENABLE_ENERGY_CALCULATOR_FFT

#include "tech/Core.h"
#include "tech/MathUtils.h"
#include "tech/Profile.h"

#include "EnergyCalculatorFftUtils.h"
#include "Image.h"
#include "Mask.h"

#include "tech/DbgMem.h"

#define PROFILE_WSST 0

//
// EnergyWsst implementation
//
LfnIc::EnergyWsst::EnergyWsst(const ImageConst& inputImage, int blockWidth, int blockHeight) :
m_blockWidth(blockWidth),
m_blockHeight(blockHeight),
m_tableWidth(blockWidth + inputImage.GetWidth()),
m_tableHeight(blockHeight + inputImage.GetHeight()),
m_table(NULL)
{
	Construct(inputImage, NULL);
}

LfnIc::EnergyWsst::EnergyWsst(const ImageConst& inputImage, const MaskLod& mask, int blockWidth, int blockHeight) :
m_blockWidth(blockWidth),
m_blockHeight(blockHeight),
m_tableWidth(blockWidth + inputImage.GetWidth()),
m_tableHeight(blockHeight + inputImage.GetHeight()),
m_table(NULL)
{
	Construct(inputImage, &mask);
}

LfnIc::EnergyWsst::~EnergyWsst()
{
	delete [] m_table;
}

int LfnIc::EnergyWsst::GetBlockWidth() const
{
	return m_blockWidth;
}

int LfnIc::EnergyWsst::GetBlockHeight() const
{
	return m_blockHeight;
}

LfnIc::Energy LfnIc::EnergyWsst::Calculate(int left, int top, int width, int height) const
{
	wxASSERT(width % m_blockWidth == 0);
	wxASSERT(height % m_blockHeight == 0);
	if (width > 0 && height > 0 && width % m_blockWidth == 0 && height % m_blockHeight == 0)
	{
		Energy e = Energy(0);

		// m_table is padded to the top and left by m_blockWidth and
		// m_blockHeight, respectively. left and top are in image space,
		// and so much be transformed to table space.
		const int tableLeft = left + m_blockWidth;
		const int tableTop = top + m_blockHeight;

		const int numBlockCols = width / m_blockWidth;
		const int numBlockRows = height / m_blockHeight;

		for (int j = 0, tableY = tableTop; j < numBlockRows; ++j, tableY += m_blockHeight)
		{
			for (int i = 0, tableX = tableLeft; i < numBlockCols; ++i, tableX += m_blockWidth)
			{
				if (tableX >= 0 && tableY >= 0 && tableX < m_tableWidth && tableY < m_tableHeight)
				{
					e += m_table[LfnTech::GetRowMajorIndex(m_tableWidth, tableX, tableY)];
				}
			}
		}

		return e;
	}
	else
	{
		wxFAIL_MSG("LfnIc::EnergyWsst::Calculate - width and/or height was invalid!");
		return ENERGY_MIN;
	}
}

void LfnIc::EnergyWsst::Construct(const ImageConst& inputImage, const MaskLod* mask)
{
	const int imageWidth = inputImage.GetWidth();
	const int imageHeight = inputImage.GetHeight();
	const Image::Rgb* imageRgb = inputImage.GetRgb();
	const Mask::Value* maskBuffer = mask ? mask->GetLodBuffer(mask->GetHighestLod()) : NULL;

	m_table = new Energy[m_tableWidth * m_tableHeight];
	Energy* sstTable = new Energy[m_tableWidth * m_tableHeight];
	std::auto_ptr<Energy> sstTableAutoPtr(sstTable);

	// Helper structure for accessing image and table data as energy while
	// safely handling out-of-bounds coordinates.
	class Data2d
	{
	public:
		Data2d(int tableWidth, int tableHeight, int imageWidth, int imageHeight) :
		m_tableWidth(tableWidth),
		m_tableHeight(tableHeight),
		m_imageWidth(imageWidth),
		m_imageHeight(imageHeight)
		{
		}

		// x and y are in table space
		FORCE_INLINE Energy& Get(Energy* table, int x, int y) const
		{
			// NOTE: assumes negative values will never be passed in
			//if (x >= 0 && y >= 0 && x < m_tableWidth && y < m_tableHeight)
			if (x < m_tableWidth && y < m_tableHeight)
			{
				return table[LfnTech::GetRowMajorIndex(m_tableWidth, x, y)];
			}

			static Energy e;
			return e;
		}

		// x and y are in image space
		FORCE_INLINE Energy Get(const Image::Rgb* imageRgb, const Mask::Value* maskBuffer, int x, int y) const
		{
			if (x >= 0 && y >= 0 && x < m_imageWidth && y < m_imageHeight)
			{
				const int imageIdx = LfnTech::GetRowMajorIndex(m_imageWidth, x, y);
				if (!maskBuffer || maskBuffer[imageIdx] == Mask::KNOWN)
				{
					const Image::Rgb& rgb = imageRgb[imageIdx];
					return Energy((rgb.red * rgb.red) + (rgb.green * rgb.green) + (rgb.blue * rgb.blue));
				}
			}

			static Energy e(0);
			return e;
		}

	private:
		const int m_tableWidth;
		const int m_tableHeight;
		const int m_imageWidth;
		const int m_imageHeight;
	};

	const Data2d data2d(m_tableWidth, m_tableHeight, imageWidth, imageHeight);

	{
#if PROFILE_WSST
		TECH_TIME_PROFILE_EVERY_SAMPLE("LfnIc::EnergyWsst::Construct");
#endif
		// Divide the table into blocks of size m_blockWidth * m_blockHeight and
		// compute a sum squared table for each. Start at -m_blockWidth,-m_blockHeight.
		{
#if PROFILE_WSST
			TECH_TIME_PROFILE_EVERY_SAMPLE("LfnIc::EnergyWsst::Construct - 1");
#endif
			for (int imageTop = -m_blockHeight; imageTop < imageHeight; imageTop += m_blockHeight)
			{
				const int blockHeight = std::min(m_blockHeight, imageHeight - imageTop);
				const int blockBottom = blockHeight - 1;

				for (int imageLeft = -m_blockWidth; imageLeft < imageWidth; imageLeft += m_blockWidth)
				{
					const int blockWidth = std::min(m_blockWidth, imageWidth - imageLeft);
					const int blockRight = blockWidth - 1;

					for (int j = blockBottom; j >= 0; --j)
					{
						const int imageY = imageTop + j;
						for (int i = blockRight; i >= 0; --i)
						{
							const int imageX = imageLeft + i;
							const int tableX = imageX + m_blockWidth;
							const int tableY = imageY + m_blockHeight;

							Energy& e = data2d.Get(sstTable, tableX, tableY);
							e = data2d.Get(imageRgb, maskBuffer, imageX, imageY);

							if (i < blockRight)
							{
								e += data2d.Get(sstTable, tableX + 1, tableY);
							}

							if (j < blockBottom)
							{
								e += data2d.Get(sstTable, tableX, tableY + 1);

								if (i < blockRight)
								{
									e -= data2d.Get(sstTable, tableX + 1, tableY + 1);
								}
							}
						}
					}
				}
			}
		}

		// Now combine the individual sst's to create the final windowed sst.
		{
#if PROFILE_WSST
			TECH_TIME_PROFILE_EVERY_SAMPLE("LfnIc::EnergyWsst::Construct - 2");
#endif
			for (int y = 0, i = 0; y < m_tableHeight; ++y)
			{
				const int yNeighbor0 = ((y / m_blockHeight) + 1) * m_blockHeight;
				const int yNeighbor1 = y + m_blockHeight;
				for (int x = 0; x < m_tableWidth; ++x, ++i)
				{
					const int xNeighbor0 = ((x / m_blockWidth) + 1) * m_blockWidth;
					const int xNeighbor1 = x + m_blockWidth;

					Energy& e = m_table[i];
					e = sstTable[i];

					if (xNeighbor0 < m_tableWidth && xNeighbor0 != xNeighbor1)
					{
						const Energy ex0 = data2d.Get(sstTable, xNeighbor0, y);
						const Energy ex1 = data2d.Get(sstTable, xNeighbor1, y);
						e += (ex0 - ex1);
					}

					if (yNeighbor0 < m_tableHeight && yNeighbor0 != yNeighbor1)
					{
						const Energy ey0 = data2d.Get(sstTable, x, yNeighbor0);
						const Energy ey1 = data2d.Get(sstTable, x, yNeighbor1);
						e += (ey0 - ey1);

						if (xNeighbor0 < m_tableWidth  && xNeighbor0 != xNeighbor1)
						{
							const Energy ex0y0 = data2d.Get(sstTable, xNeighbor0, yNeighbor0);
							const Energy ex0y1 = data2d.Get(sstTable, xNeighbor0, yNeighbor1);
							const Energy ex1y0 = data2d.Get(sstTable, xNeighbor1, yNeighbor0);
							const Energy ex1y1 = data2d.Get(sstTable, xNeighbor1, yNeighbor1);
							e += (ex0y0 + ex1y1 - ex1y0 - ex0y1);
						}
					}
				}
			}
		}
	}

#if FFT_VALIDATION_ENABLED
	{
		for (int y = 0; y < m_tableHeight; ++y)
		{
			const int blockHeight = std::min(m_blockHeight, m_tableHeight - y);
			const int imageY = y - m_blockHeight;
			for (int x = 0; x < m_tableWidth; ++x)
			{
				const int blockWidth = std::min(m_blockWidth, m_tableWidth - x);
				const int imageX = x - m_blockWidth;

				const Energy e = m_table[LfnTech::GetRowMajorIndex(m_tableWidth, x, y)];
				const Energy eBruteForce = EnergyCalculatorFftUtils::BruteForceCalculate1stTerm(inputImage, blockWidth, blockHeight, imageX, imageY, mask);
				wxASSERT(eBruteForce == e);
			}
		}
	}
#endif
}
#endif // ENABLE_ENERGY_CALCULATOR_FFT
