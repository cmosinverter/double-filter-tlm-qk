#include <cmath>
#include <iomanip>

#include "DoubleFilter.h"

using namespace std;

DoubleFilter::DoubleFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &DoubleFilter::blocking_transport);
}


void DoubleFilter::do_filter() {

  std::vector<int> reds, greens, blues;
  int counter = 0;
  unsigned char flag;
  while (true) {

    flag = i_col_check.read();
    if (flag == 1){
      for (int i = 0; i < 8; i ++){
        flag = i_col_check.read();
      }
    } else {
      for (int i = 0; i < 2; i ++){
        flag = i_col_check.read();
      }
    }

    // Median filter
    int center_r, center_g, center_b;
    int sum_r = 0;
    int sum_g = 0;
    int sum_b = 0;

    if (flag == 1) {
      reds.clear();
      greens.clear();
      blues.clear();

      for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
          reds.push_back(i_r.read());
          greens.push_back(i_g.read());
          blues.push_back(i_b.read());
        }
      }
      counter = 0;
    } else {

      reds[0 + (counter % 3)] = i_r.read();
      reds[3 + (counter % 3)] = i_r.read();
      reds[6 + (counter % 3)] = i_r.read();
      greens[0 + (counter % 3)] = i_g.read();
      greens[3 + (counter % 3)] = i_g.read();
      greens[6 + (counter % 3)] = i_g.read();
      blues[0 + (counter % 3)] = i_b.read();
      blues[3 + (counter % 3)] = i_b.read();
      blues[6 + (counter % 3)] = i_b.read();
      counter = counter + 1;

    }

    if (counter == 0) {
      center_r = reds[reds.size() / 2];
      center_g = greens[greens.size() / 2];
      center_b = blues[blues.size() / 2];
    } else {
      if ((counter % 3) == 1) {
        center_r = reds[5];
        center_g = greens[5];
        center_b = blues[5];
      }
      else if ((counter % 3) == 2) {
        center_r = reds[3];
        center_g = greens[3];
        center_b = blues[3];
      }
      else if ((counter % 3) == 0) {
        center_r = reds[4];
        center_g = greens[4];
        center_b = blues[4];
      }
    }

    std::sort(reds.begin(), reds.end());
    std::sort(greens.begin(), greens.end());
    std::sort(blues.begin(), blues.end());
    
    // Mean filter
    for (auto x: reds) {
        sum_r += x;
    }
    for (auto x: greens) {
        sum_g += x;
    }
    for (auto x: blues) {
        sum_b += x;
    }
    o_r.write(round((sum_r - center_r + 2 * reds[reds.size() / 2]) / 10));
    o_g.write(round((sum_g - center_g + 2 * greens[greens.size() / 2]) / 10));
    o_b.write(round((sum_b - center_b + 2 * blues[blues.size() / 2]) / 10));
  }
}

void DoubleFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case DOUBLE_FILTER_RESULT_ADDR:
      buffer.uc[0] = o_r.read();
      buffer.uc[1] = o_g.read();
      buffer.uc[2] = o_b.read();
      buffer.uc[3] = 0;
      break;
    case DOUBLE_FILTER_CHECK_ADDR:
      buffer.uint = o_r.num_available();
      break;
    default:
      std::cerr << "Error! DoubleFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;

  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case DOUBLE_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      if (mask_ptr[3] == 0xff) {
        i_col_check.write(data_ptr[3]);
      }
      break;
    default:
      std::cerr << "Error! DoubleFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
