/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2009 Soeren Sonnenburg
 * Copyright (C) 1999-2009 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include "lib/common.h"
#include "lib/io.h"
#include "kernel/PolyMatchWordStringKernel.h"
#include "kernel/SqrtDiagKernelNormalizer.h"
#include "features/Features.h"
#include "features/StringFeatures.h"

CPolyMatchWordStringKernel::CPolyMatchWordStringKernel(int32_t size, int32_t d, bool i)
: CStringKernel<uint16_t>(size),degree(d),inhomogene(i)
{
	set_normalizer(new CSqrtDiagKernelNormalizer());
}

CPolyMatchWordStringKernel::CPolyMatchWordStringKernel(
	CStringFeatures<uint16_t>* l, CStringFeatures<uint16_t>* r, int32_t d, bool i)
: CStringKernel<uint16_t>(10),degree(d),inhomogene(i)
{
	set_normalizer(new CSqrtDiagKernelNormalizer());
	init(l, r);
}

CPolyMatchWordStringKernel::~CPolyMatchWordStringKernel()
{
	cleanup();
}

bool CPolyMatchWordStringKernel::init(CFeatures* l, CFeatures* r)
{
	CStringKernel<uint16_t>::init(l,r);
	return init_normalizer();
}

void CPolyMatchWordStringKernel::cleanup()
{
	CKernel::cleanup();
}

bool CPolyMatchWordStringKernel::load_init(FILE* src)
{
	return false;
}

bool CPolyMatchWordStringKernel::save_init(FILE* dest)
{
	return false;
}

float64_t CPolyMatchWordStringKernel::compute(int32_t idx_a, int32_t idx_b)
{
	int32_t alen, blen;

	uint16_t* avec=((CStringFeatures<uint16_t>*) lhs)->get_feature_vector(idx_a, alen);
	uint16_t* bvec=((CStringFeatures<uint16_t>*) rhs)->get_feature_vector(idx_b, blen);

	ASSERT(alen==blen);

	int32_t sum=0;

	for (int32_t i=0; i<alen; i++)
		sum+= (avec[i]==bvec[i]) ? 1 : 0;

	if (inhomogene)
		sum+=1;

	float64_t result=sum;

	for (int32_t j=1; j<degree; j++)
		result*=sum;

	return result;
}
