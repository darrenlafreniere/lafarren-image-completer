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
#include "PatchTypePoisson.h"

#include "tech/MathUtils.h"

#include "ImageFloat.h"
#include "MaskWritable.h"
#include "PoissonSolver.h"
#include "Settings.h"

#include "tech/DbgMem.h"

CompositorRoot::PatchType* PatchTypePoisson::Factory::Create(const Compositor::Input& input, ImageFloat& imageFloat) const
{
	return new PatchTypePoisson(input, imageFloat);
}

PatchTypePoisson::PatchTypePoisson(const Compositor::Input& input, ImageFloat& imageFloat)
	: PatchTypeNormal(input, imageFloat)
{
	Poisson::Complete(imageFloat, input.mask, imageFloat);

	const Settings& settings = input.settings;

	m_poissonCloner.reset(new Poisson::Cloner(imageFloat));
	m_patchMask.reset(new MaskWritable(settings.patchWidth, settings.patchHeight, Mask::UNKNOWN));
	m_patchImagePoisson.reset(new ImageFloat(settings.patchWidth, settings.patchHeight));
}

const ImageFloat& PatchTypePoisson::Get(const Patch& patch) const
{
	Poisson::Cloner& poissonCloner = *m_poissonCloner;
	const ImageFloat& patchImageNormal = PatchTypeNormal::Get(patch);
	const MaskWritable& patchMask = *m_patchMask;
	ImageFloat& patchImagePoisson = *m_patchImagePoisson;

	poissonCloner.Clone(patchImageNormal, patchMask, patch.destLeft, patch.destTop, patchImagePoisson, 0, 0);
	return patchImagePoisson;
}
