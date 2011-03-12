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
#include "PatchTypeDebugPatchOrder.h"

#include "ImageFloat.h"
#include "Settings.h"

#include "tech/DbgMem.h"

// Given a 0.0-1.0 alpha, returns a color from red to purple.
static LfnIc::RgbFloat GetRainbowColor(float alpha)
{
	wxASSERT(alpha >= 0.0f && alpha <= 1.0f);
	static const LfnIc::RgbFloat refColors[] =
	{
		LfnIc::RgbFloat(1.00f, 0.00f, 0.00f), // red
		LfnIc::RgbFloat(1.00f, 0.65f, 0.00f), // orange
		LfnIc::RgbFloat(1.00f, 1.00f, 0.00f), // yellow
		LfnIc::RgbFloat(0.00f, 0.50f, 0.00f), // green
		LfnIc::RgbFloat(0.00f, 0.00f, 1.00f), // blue
		LfnIc::RgbFloat(0.30f, 0.00f, 0.51f), // indigo
		LfnIc::RgbFloat(0.93f, 0.51f, 0.93f), // violet
	};
	static const int numRefColors = sizeof(refColors) / sizeof(refColors[0]);
	static const int penultimateRefColorIndex = numRefColors - 2;
	static const float refAlphaStep = 1.0f / float(numRefColors - 1);

	int refIndex = 0;
	float refAlphaLow = 0.0f;
	while (refIndex < penultimateRefColorIndex)
	{
		const float refAlphaHigh = refAlphaLow + refAlphaStep;
		if (alpha < refAlphaHigh)
		{
			break;
		}
		else
		{
			++refIndex;
			refAlphaLow = refAlphaHigh;
		}
	}

	const LfnIc::RgbFloat& colorLow = refColors[refIndex];
	const LfnIc::RgbFloat& colorHigh = refColors[refIndex + 1];
	const float blendAlpha = (alpha - refAlphaLow) / refAlphaStep;
							
	return LfnIc::RgbFloat(
		colorLow.r + blendAlpha * (colorHigh.r - colorLow.r),
		colorLow.g + blendAlpha * (colorHigh.g - colorLow.g),
		colorLow.b + blendAlpha * (colorHigh.b - colorLow.b));
}

LfnIc::CompositorRoot::PatchType* LfnIc::PatchTypeDebugPatchOrder::Factory::Create(const Compositor::Input& input, ImageFloat& imageFloat) const
{
	return new PatchTypeDebugPatchOrder(input, imageFloat);
}

LfnIc::PatchTypeDebugPatchOrder::PatchTypeDebugPatchOrder(const Compositor::Input& input, ImageFloat& imageFloat)
	: m_patches(input.patches)
	, m_patchImage(input.settings.patchWidth, input.settings.patchHeight)
{
}

const LfnIc::ImageFloat& LfnIc::PatchTypeDebugPatchOrder::Get(const Patch& patch) const
{
	LfnIc::RgbFloat rgb(0.0f, 0.0f, 0.0f);
	for (int patchIdx = 0, patchesNum = m_patches.size(); patchIdx < patchesNum; ++patchIdx)
	{
		if (&m_patches[patchIdx] == &patch)
		{
			const float rainbowAlpha = float(patchIdx) / float(patchesNum - 1);
			rgb = GetRainbowColor(rainbowAlpha);
			break;
		}
	}

	for (int i = 0, n = m_patchImage.GetWidth() * m_patchImage.GetHeight(); i < n; ++i)
	{
		m_patchImage.GetRgb()[i].r = rgb.r;
		m_patchImage.GetRgb()[i].g = rgb.g;
		m_patchImage.GetRgb()[i].b = rgb.b;
	}

	return m_patchImage;
}
