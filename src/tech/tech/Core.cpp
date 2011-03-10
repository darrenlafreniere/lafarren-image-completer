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
#include "tech/Core.h"

#include "tech/DbgMem.h"

#define BITS_TO_BYTES(x) (x / 8)

wxCOMPILE_TIME_ASSERT(sizeof(int8) == BITS_TO_BYTES(8), int8_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(uint8) == BITS_TO_BYTES(8), uint8_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(byte) == 1, byte_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(int16) == BITS_TO_BYTES(16), int16_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(uint16) == BITS_TO_BYTES(16), uint16_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(int32) == BITS_TO_BYTES(32), int32_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(uint32) == BITS_TO_BYTES(32), _IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(int64) == BITS_TO_BYTES(64), int64_IsIncorrectSize);
wxCOMPILE_TIME_ASSERT(sizeof(uint64) == BITS_TO_BYTES(64), uint64_IsIncorrectSize);