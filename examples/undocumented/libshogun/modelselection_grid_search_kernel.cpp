/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2011 Heiko Strathmann
 * Copyright (C) 2011 Berlin Institute of Technology and Max-Planck-Society
 */

#include <shogun/base/init.h>
#include <shogun/evaluation/CrossValidation.h>
#include <shogun/evaluation/ContingencyTableEvaluation.h>
#include <shogun/evaluation/StratifiedCrossValidationSplitting.h>
#include <shogun/modelselection/GridSearchModelSelection.h>
#include <shogun/modelselection/ModelSelectionParameters.h>
#include <shogun/modelselection/ParameterCombination.h>
#include <shogun/features/Labels.h>
#include <shogun/features/SimpleFeatures.h>
#include <shogun/classifier/svm/LibSVM.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/kernel/PowerKernel.h>
#include <shogun/distance/MinkowskiMetric.h>


using namespace shogun;

void print_message(FILE* target, const char* str)
{
	fprintf(target, "%s", str);
}

CModelSelectionParameters* create_param_tree()
{
	CModelSelectionParameters* root=new CModelSelectionParameters();

	CModelSelectionParameters* c1=new CModelSelectionParameters("C1");
	root->append_child(c1);
	c1->build_values(-5, 5, R_EXP);

	CModelSelectionParameters* c2=new CModelSelectionParameters("C2");
	root->append_child(c2);
	c2->build_values(-5, 5, R_EXP);

	CGaussianKernel* gaussian_kernel=new CGaussianKernel();
	CModelSelectionParameters* param_gaussian_kernel=
			new CModelSelectionParameters("kernel", gaussian_kernel);
	CModelSelectionParameters* gaussian_kernel_width=
			new CModelSelectionParameters("width");
	gaussian_kernel_width->build_values(-5, 5, R_EXP, 1, 2);
	param_gaussian_kernel->append_child(gaussian_kernel_width);
	root->append_child(param_gaussian_kernel);

	CPowerKernel* power_kernel=new CPowerKernel();
	CModelSelectionParameters* param_power_kernel=
	new CModelSelectionParameters("kernel", power_kernel);

	root->append_child(param_power_kernel);

	CModelSelectionParameters* param_power_kernel_degree=
			new CModelSelectionParameters("degree");
	param_power_kernel_degree->build_values(1, 1, R_EXP);
	param_power_kernel->append_child(param_power_kernel_degree);

	CMinkowskiMetric* m_metric=new CMinkowskiMetric(10);
	CModelSelectionParameters* param_power_kernel_metric1=
			new CModelSelectionParameters("distance", m_metric);

	param_power_kernel->append_child(param_power_kernel_metric1);

	CModelSelectionParameters* param_power_kernel_metric1_k=
			new CModelSelectionParameters("k");
	param_power_kernel_metric1_k->build_values(1, 12, R_LINEAR);
	param_power_kernel_metric1->append_child(param_power_kernel_metric1_k);

	return root;
}

int main(int argc, char **argv)
{
	init_shogun(&print_message, &print_message, &print_message);

	int32_t num_subsets=3;
	int32_t num_vectors=20;
	int32_t dim_vectors=3;

	/* create some data and labels */
	float64_t* matrix=new float64_t[num_vectors*dim_vectors];
	CLabels* labels=new CLabels(num_vectors);
	for (int32_t i=0; i<num_vectors*dim_vectors; i++)
		matrix[i]=CMath::randn_double();

	/* create num_feautres 2-dimensional vectors */
	CSimpleFeatures<float64_t>* features=new CSimpleFeatures<float64_t> ();
	features->set_feature_matrix(matrix, dim_vectors, num_vectors);

	/* create labels, two classes */
	for (index_t i=0; i<num_vectors; ++i)
		labels->set_label(i, i%2==0 ? 1 : -1);

	/* create linear classifier (use -s 2 option to avoid warnings) */
	CLibSVM* classifier=new CLibSVM();
	classifier->set_store_sv_features(true);

	/* splitting strategy */
	CStratifiedCrossValidationSplitting* splitting_strategy=
			new CStratifiedCrossValidationSplitting(labels, num_subsets);

	/* accuracy evaluation */
	CContingencyTableEvaluation* evaluation_criterium=
			new CContingencyTableEvaluation(ACCURACY);

	/* cross validation class for evaluation in model selection, 3 repetitions */
	CCrossValidation* cross=new CCrossValidation(classifier, features, labels,
			splitting_strategy, evaluation_criterium);
	cross->set_num_runs(3);

	/* model parameter selection, deletion is handled by modsel class (SG_UNREF) */
	CModelSelectionParameters* param_tree=create_param_tree();
	param_tree->print_tree();

	/* this is on the stack and handles all of the above structures in memory */
	CGridSearchModelSelection grid_search(param_tree, cross);

	CParameterCombination* best_combination=grid_search.select_model();
	SG_SPRINT("best parameter(s):\n");
	best_combination->print_tree();

	best_combination->apply_to_machine(classifier);

	/* larger number of runs to have tighter confidence intervals */
	cross->set_num_runs(100);
	cross->set_conf_int_alpha(0.01);
	CrossValidationResult result=cross->evaluate();
	SG_SPRINT("result: ");
	result.print_result();

	/* clean up destroy result parameter */
	SG_UNREF(best_combination);

	SG_SPRINT("\nEND\n");
	exit_shogun();

	return 0;
}

