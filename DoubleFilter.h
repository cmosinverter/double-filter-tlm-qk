#ifndef DOUBLE_FILTER_H_
#define DOUBLE_FILTER_H_
#include <systemc>
using namespace sc_core;

#include "tlm"
#include "tlm_utils/simple_target_socket.h"

#include "filter_def.h"

class DoubleFilter : public sc_module {
public:
  tlm_utils::simple_target_socket<DoubleFilter> t_skt;

  sc_fifo<unsigned char> i_r;
  sc_fifo<unsigned char> i_g;
  sc_fifo<unsigned char> i_b;
  sc_fifo<unsigned char> o_r;
  sc_fifo<unsigned char> o_g;
  sc_fifo<unsigned char> o_b;
  sc_fifo<unsigned char> i_col_check;

  SC_HAS_PROCESS(DoubleFilter);
  DoubleFilter(sc_module_name n);
  ~DoubleFilter() = default;

private:
  void do_filter();
  int val[MASK_N];

  unsigned int base_offset;
  void blocking_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay);
};
#endif
