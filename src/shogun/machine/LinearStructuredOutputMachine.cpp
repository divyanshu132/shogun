/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Fernando Iglesias, Shell Hu, Thoralf Klein, Sergey Lisitsyn, 
 *          Bjoern Esser, Soeren Sonnenburg
 */

#include <shogun/machine/LinearStructuredOutputMachine.h>
#include <shogun/features/Features.h>

using namespace shogun;

CLinearStructuredOutputMachine::CLinearStructuredOutputMachine()
: CStructuredOutputMachine()
{
	register_parameters();
}

CLinearStructuredOutputMachine::CLinearStructuredOutputMachine(
		CStructuredModel*  model,
		CStructuredLabels* labs)
: CStructuredOutputMachine(model, labs)
{
	register_parameters();
}

CLinearStructuredOutputMachine::~CLinearStructuredOutputMachine()
{
}

void CLinearStructuredOutputMachine::set_w(SGVector< float64_t > w)
{
	m_w = w;
}

SGVector< float64_t > CLinearStructuredOutputMachine::get_w() const
{
	return m_w;
}

CStructuredLabels* CLinearStructuredOutputMachine::apply_structured(CFeatures* data)
{
	if (data)
	{
		set_features(data);
	}

	CFeatures* model_features = this->get_features();
	if (!model_features)
	{
		return m_model->structured_labels_factory();
	}

	int num_input_vectors = model_features->get_num_vectors();
	CStructuredLabels* out;
	out = m_model->structured_labels_factory(num_input_vectors);

	for ( int32_t i = 0 ; i < num_input_vectors ; ++i )
	{
		CResultSet* result = m_model->argmax(m_w, i, false);
		out->add_label(result->argmax);

		SG_UNREF(result);
	}
	SG_UNREF(model_features);
	return out;
}

void CLinearStructuredOutputMachine::register_parameters()
{
	SG_ADD(&m_w, "m_w", "Weight vector", ParameterProperties::MODEL);
}

void CLinearStructuredOutputMachine::store_model_features()
{
}
