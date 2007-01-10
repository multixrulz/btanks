#include "network.h"
#include "mrt/exception.h"
#include "./exception.h"

using namespace fann;

Network::Network(const Type type, const int layers_num, const unsigned int *layers, const float connection_rate) : network(NULL) {
	switch(type) {
		case Standard:
			network = fann_create_standard_array(layers_num, const_cast<unsigned int *>(layers));
		case Sparse:
			network = fann_create_sparse_array(connection_rate, layers_num, const_cast<unsigned int *>(layers));
		case Shortcut:
			network = fann_create_shortcut_array(layers_num, const_cast<unsigned int *>(layers));
	}
	if (network == NULL)
		throw_fnet(("fann_create_XXX_array(%d, %p, %g) (type %d)", layers_num, (void*)layers, connection_rate, type));
}

void Network::randomize_weights(const fann_type min, const fann_type max) {
	fann_randomize_weights(network, min, max);
}

fann_type *Network::run(fann_type * input) {
	fann_type * r = fann_run(network, input);
	if (r == NULL)
		throw_fnet(("run"));
	return r;
}

void Network::printConnections() {
	fann_print_connections(network);
}

void Network::printParameters() {
	fann_print_parameters(network);
}

const unsigned int Network::getNumInput() {
	return fann_get_num_input(network);
}
const unsigned int Network::getNumOutput() {
	return fann_get_num_output(network);
}
const unsigned int Network::getTotalNeurons() {
	return fann_get_total_neurons(network);
}
const unsigned int Network::getTotalConnections() {
	return fann_get_total_connections(network);
}


Network::~Network() {
	if (network == NULL)
		return;
	fann_destroy(network);
	network = NULL;
}
