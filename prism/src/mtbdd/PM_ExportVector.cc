//==============================================================================
//	
//	Copyright (c) 2002-2006, Dave Parker
//	
//	This file is part of PRISM.
//	
//	PRISM is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//	
//	PRISM is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with PRISM; if not, write to the Free Software Foundation,
//	Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//	
//==============================================================================

// includes
#include "PrismMTBDD.h"
#include <util.h>
#include <cudd.h>
#include <dd.h>
#include <odd.h>
#include "PrismMTBDDGlob.h"

//------------------------------------------------------------------------------

// local function prototypes
static void export_rec(DdNode *dd, DdNode **vars, int num_vars, int level, ODDNode *odd, long index);

// globals
static const char *export_name;

//------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_mtbdd_PrismMTBDD_PM_1ExportVector
(
JNIEnv *env,
jclass cls,
jint ve,		// vector
jstring na,		// vector name
jint va,		// vars
jint num_vars,
jint od,		// odd
jint et,		// export type
jstring fn		// filename
)
{
	DdNode *vector = (DdNode *)ve;	// vector
	DdNode **vars = (DdNode **)va; // vars
	ODDNode *odd = (ODDNode *)od;
	
	// store export info
	if (!store_export_info(et, fn, env)) return -1;
	export_name = na ? env->GetStringUTFChars(na, 0) : "v";
	
	// print file header
	switch (export_type) {
	case EXPORT_PLAIN: export_string("%d %.0f\n", odd->eoff+odd->toff, DD_GetNumMinterms(ddman, vector, num_vars)); break;
	case EXPORT_MATLAB: export_string("%s = sparse(%d,1);\n", export_name, odd->eoff+odd->toff, odd->eoff+odd->toff); break;
	}
	
	// print main part of file
	export_rec(vector, vars, num_vars, 0, odd, 0);
	
	// close file, etc.
	if (export_file) fclose(export_file);
	env->ReleaseStringUTFChars(na, export_name);
	
	return 0;
}

//------------------------------------------------------------------------------

void export_rec(DdNode *dd, DdNode **vars, int num_vars, int level, ODDNode *odd, long index)
{
	DdNode *e, *t;
	
	// base case - zero terminal
	if (dd == Cudd_ReadZero(ddman)) return;
	
	// base case - non zero terminal
	if (level == num_vars) {
		switch (export_type) {
		case EXPORT_PLAIN: export_string("%d %.12g\n", index, Cudd_V(dd)); break;
		case EXPORT_MATLAB: export_string("%s(%d)=%.12g;\n", export_name, index+1, Cudd_V(dd)); break;
		case EXPORT_MRMC: export_string("%d %.12g\n", index+1, Cudd_V(dd)); break;
		}
		return;
	}
	
	// recurse
	if (dd->index > vars[level]->index) {
		e = t = dd;
	}
	else {
		e = Cudd_E(dd);
		t = Cudd_T(dd);
	}
	export_rec(e, vars, num_vars, level+1, odd->e, index);
	export_rec(t, vars, num_vars, level+1, odd->t, index+odd->eoff);
}

//------------------------------------------------------------------------------
