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

#ifndef TAUCS_MATRIX_H
#define TAUCS_MATRIX_H

#include "TaucsGuarded.h"

//
// Wraps the taucs_ccs_matrix structure, and binds its lifetime to the
// lifetime of the TaucsMatrix instance.
//
class TaucsMatrix
{
public:
	TaucsMatrix();
	// Signature matches the taucs_ccs_create function signature.
	TaucsMatrix(int m, int n, int nnz, int flags);
	~TaucsMatrix();

	inline bool IsValid() const { return m_matrix != NULL; }

	// columns
	inline int GetN() const { return m_matrix->n; }

	// don't use if symmetric
	inline int GetM() const { return m_matrix->m; }

	inline int GetFlags() const { return m_matrix->flags; }

	// pointers to where columns begin in rowind and values.
	// 0-based. Length is (n+1).
	inline int& GetColPtr(int col) { return m_matrix->colptr[col]; }
	inline int GetColPtr(int col) const { return m_matrix->colptr[col]; }
	inline int* GetColPtrs() { return m_matrix->colptr; }
	inline const int* GetColPtrs() const { return m_matrix->colptr; }

	// row indices
	inline int& GetRowIndex(int row) { return m_matrix->rowind[row]; }
	inline int GetRowIndex(int row) const { return m_matrix->rowind[row]; }
	inline int* GetRowIndices() { return m_matrix->rowind; }
	inline const int* GetRowIndices() const { return m_matrix->rowind; }

	// values
	inline taucs_datatype& GetValue(int i) { return m_matrix->taucs_values[i]; }
	inline taucs_datatype GetValue(int i) const { return m_matrix->taucs_values[i]; }
	inline taucs_datatype* GetValues() { return m_matrix->taucs_values; }
	inline const taucs_datatype* GetValues() const { return m_matrix->taucs_values; }

	// Implicit casts to taucs_ccs_matrix.
	inline operator taucs_ccs_matrix*()
	{
		return m_matrix;
	}

	inline operator const taucs_ccs_matrix*() const
	{
		return m_matrix;
	}

	// Reconstructs the out matrix to the transpose of this. Returns a
	// reference to out.
	TaucsMatrix& GetTranspose(TaucsMatrix& out) const;

private:
	// Cannot copy or assign
	TaucsMatrix(const TaucsMatrix& other) {}
	TaucsMatrix& operator=(const TaucsMatrix& other) { return *this; }

	taucs_ccs_matrix* m_matrix;
};


#endif // TAUCS_MATRIX_H
