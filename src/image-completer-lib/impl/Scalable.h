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

#ifndef SCALABLE_H
#define SCALABLE_H

namespace LfnIc
{
	//
	// Interface for scaling an object up and down in-place. Implementators
	// of this interface must preserve data such that:
	//
	//	dataBefore = objectThatImplementsScalable.CloneData()
	//	objectThatImplementsScalable.ScaleDown()
	//	objectThatImplementsScalable.ScaleUp() // object was not mutated between scaling calls
	//	dataAfter = objectThatImplementsScalable.CloneData()
	//	assert(dataBefore == dataAfter)
	//
	class Scalable
	{
	public:
		// Scales the instance's data up to the next higher resolution/depth.
		// Asserts that this instance isn't at its original depth.
		virtual void ScaleUp() = 0;

		// Scales the instance's data down to the next lower resolution/depth.
		virtual void ScaleDown() = 0;

		// Returns the instance's current scale depth. 0 is the original
		// resolution's depth, and 1 is the next lower resolution/depth.
		virtual int GetScaleDepth() const = 0;

	protected:
		// Cannot destroy Scalable instance through a pointer to this base.
		~Scalable() {}
	};

	//
	// Scales down each added Scalable instance, and when this goes out of
	// scope, scales them back up in the same order as they were added.
	//
	class ScopedScaleDownAndUpInOrder
	{
	public:
		inline void Add(Scalable& scalable)
		{
			scalable.ScaleDown();
			m_scalables.push_back(&scalable);
		}

		inline ~ScopedScaleDownAndUpInOrder()
		{
			for (int i = 0, n = m_scalables.size(); i < n; ++i)
			{
				m_scalables[i]->ScaleUp();
			}
		}

	private:
		std::vector<Scalable*> m_scalables;
	};
}

#endif
