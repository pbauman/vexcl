#include <vector>
#include <tuple>
#include <cassert>
#include <oclutil/oclutil.hpp>
using namespace clu;
int main() {
    std::vector<cl::Context>      context;
    std::vector<cl::CommandQueue> queue;
    std::tie(context, queue) = queue_list(
	    Filter::Vendor("NVIDIA") && Filter::DoublePrecision());

    const uint n = 1 << 20;
    clu::vector<float> x(queue, CL_MEM_WRITE_ONLY, n);

    auto program = build_sources(context[0], std::string(
		"kernel void dummy(uint size, global float *x)\n"
		"{\n"
		"    uint i = get_global_id(0);\n"
		"    if (i < size) x[i] = 4.2;\n"
		"}\n"
		));

    cl::Kernel dummy = cl::Kernel(program, "dummy");
    for(uint d = 0; d < queue.size(); d++) {
	dummy.setArg(0, (uint)x.part_size(d));
	dummy.setArg(1, x(d));

	queue[d].enqueueNDRangeKernel(
		dummy, cl::NullRange, alignup(n, 256U), 256U
		);
    }

    std::cout << sum(x) << std::endl;
}
