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
#include "TaucsMatrix.h"

#include "tech/DbgMem.h"

TaucsMatrix::TaucsMatrix()
	: m_matrix(NULL)
{
}

TaucsMatrix::TaucsMatrix(int m, int n, int nnz, int flags)
	: m_matrix(taucs_ccs_create(m, n, nnz, flags))
{
	wxASSERT(m_matrix);
}

TaucsMatrix::~TaucsMatrix()
{
	if (m_matrix)
	{
		taucs_ccs_free(m_matrix);
	}
}

#include "tech/UnDbgMem.h" // allows placement new
TaucsMatrix& TaucsMatrix::GetTranspose(TaucsMatrix& out) const
{
	const int n = GetN();
	const int m = GetM();
	const int flags = GetFlags();
	const int colPtrN = GetColPtr(n);

	out.~TaucsMatrix();
	new(&out) TaucsMatrix(n, m, colPtrN, flags);
	if (flags & TAUCS_SYMMETRIC)
	{
		// symmetric - just copy the matrix
		memcpy(out.GetColPtrs(), GetColPtrs(), sizeof(int) * (n + 1));
		memcpy(out.GetRowIndices(), GetRowIndices(), sizeof(int) * colPtrN);
		memcpy(out.GetValues(), GetValues(), sizeof(taucs_datatype) * colPtrN);
	}
	else
	{
		// non-symmetric matrix -> need to build data structure.
		// we'll go over the columns and build the rows
		std::vector<std::vector<int> > rows(m);
		std::vector<std::vector<taucs_datatype> > values(m);
		for (int c = 0, nc = n; c < nc; ++c)
		{
			for (int rowi = GetColPtr(c), nrows = GetColPtr(c + 1); rowi < nrows; ++rowi)
			{
				const int rowIndex = GetRowIndex(rowi);
				rows[rowIndex].push_back(c);
				values[rowIndex].push_back(GetValue(rowi));
			}
		}

		// copying the rows as columns in ret
		int cind = 0;
		for (int r = 0, nr = m; r < nr; ++r)
		{
			out.GetColPtrs()[r] = cind;

			for (int j = 0, nj = rows[r].size(); j < nj; ++j)
			{
				out.GetRowIndex(cind) = rows[r][j];
				out.GetValue(cind) = values[r][j];
				++cind;
			}
		}
		out.GetColPtr(m) = cind;

		wxASSERT(cind == colPtrN);
	}

	return out;
}
#include "tech/DbgMem.h"
