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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "PriorityBpSettings.h"
#include "Scalable.h"

namespace PriorityBp
{
	//
	// Extends the Settings class to provide in-place scalable settings.
	// Copies in input settings data.
	//
	class SettingsScalable : public Settings, public Scalable
	{
	public:
		SettingsScalable(const Settings& settings);

		virtual void ScaleUp();
		virtual void ScaleDown();
		virtual int GetScaleDepth() const;

	private:
		std::vector<Settings> m_resolutions;
		int m_depth;
	};
}

#endif
