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
#include "CompositorRoot.h"

#include "tech/MathUtils.h"

#include "Image.h"
#include "ImageFloat.h"
#include "PriorityBpSettings.h"

#include "tech/DbgMem.h"

#define DEBUG_DRAW_MASK_INTENSITY 0

//
// CompositorRoot implementation
//
LfnIc::CompositorRoot::CompositorRoot(
	const PatchType::Factory* patchTypeFactory,
	const PatchBlender::Factory* patchBlenderFactory,
	const OutputBlender* outputBlender)
	: m_patchTypeFactory(patchTypeFactory)
	, m_patchBlenderFactory(patchBlenderFactory)
	, m_outputBlender(outputBlender)
{
	wxASSERT(m_patchTypeFactory.get());
	wxASSERT(m_patchBlenderFactory.get());
	wxASSERT(m_outputBlender.get());
}

bool LfnIc::CompositorRoot::Compose(const Input& input, Image& outputImage) const
{
	bool result = false;

	const ImageConst& inputImage = input.inputImage;
	const Settings& settings = input.settings;

	ImageFloat outputImageFloat(inputImage);
	{
		ImageFloat patchesBlended(inputImage.GetWidth(), inputImage.GetHeight(), RgbFloat(0.0f, 0.0f, 0.0f));

		const int patchesNum = input.patches.size();
		if (patchesNum > 0)
		{
			std::auto_ptr<const PatchType> patchTypeAutoPtr((*m_patchTypeFactory).Create(input, outputImageFloat));
			const PatchType& patchType = *patchTypeAutoPtr;

			std::auto_ptr<const PatchBlender> patchBlenderFactoryAutoPtr((*m_patchBlenderFactory).Create(input, outputImageFloat, patchesBlended));
			const PatchBlender& patchBlender = *patchBlenderFactoryAutoPtr;

			for (int patchIdx = 0; patchIdx < patchesNum; ++patchIdx)
			{
				const Patch& patch = input.patches[patchIdx];
				wxASSERT(patch.srcLeft >= 0 && (patch.srcLeft + settings.patchWidth - 1) < inputImage.GetWidth());
				wxASSERT(patch.srcTop >= 0 && (patch.srcTop + settings.patchHeight - 1) < inputImage.GetHeight());

				const ImageFloat& patchImage = patchType.Get(patch);
				patchBlender.Blend(patch, patchImage);
			}
		}

		m_outputBlender->Blend(input, patchesBlended, outputImageFloat);
	}

	outputImageFloat.CopyTo(outputImage);
	return result;
}
