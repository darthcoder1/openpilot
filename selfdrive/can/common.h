#ifndef SELFDRIVE_CAN_COMMON_H
#define SELFDRIVE_CAN_COMMON_H

#include <cstddef>
#include <cstdint>
#include <string>

#ifdef __cplusplus
#include <vector>
#endif

#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

unsigned int honda_checksum(unsigned int address, uint64_t d, int l);
unsigned int toyota_checksum(unsigned int address, uint64_t d, int l);
unsigned int pedal_checksum(unsigned int address, uint64_t d, int l);

struct SignalPackValue
{
  const char *name;
  double value;
};

struct SignalParseOptions
{
  uint32_t address;
  const char *name;
  double default_value;
};

struct MessageParseOptions
{
  uint32_t address;
  int check_frequency;
};

struct SignalValue
{
  uint32_t address;
  uint16_t ts;
  const char *name;
  double value;
};

enum SignalType
{
  DEFAULT,
  HONDA_CHECKSUM,
  HONDA_COUNTER,
  TOYOTA_CHECKSUM,
  PEDAL_CHECKSUM,
  PEDAL_COUNTER,
};

struct Signal
{
  const char *name;
  int b1, b2, bo;
  bool is_signed;
  double factor, offset;
  bool is_little_endian;
  SignalType type;
};

struct Msg
{
  const char *name;
  uint32_t address;
  unsigned int size;
  size_t num_sigs;
  const Signal *sigs;
};

struct Val
{
  const char *name;
  uint32_t address;
  const char *def_val;
  const Signal *sigs;
};

struct DBC
{
  const char *name;
  size_t num_msgs;
  const Msg *msgs;
  const Val *vals;
  size_t num_vals;
};

const DBC *dbc_lookup(const std::string &dbc_name);

void dbc_register(const DBC *dbc);

#define dbc_init(dbc)                                              \
  static void __attribute__((constructor)) do_dbc_init_##dbc(void) \
  {                                                                \
    dbc_register(&dbc);                                            \
  }

#endif

// packer.cc API
void *canpack_init(const char *dbc_name);

uint64_t canpack_pack(void *inst, uint32_t address, size_t num_vals, const SignalPackValue *vals, int counter, bool checksum);

uint64_t canpack_pack_vector(void *inst, uint32_t address, const std::vector<SignalPackValue> &signals, int counter);

// parser.cc API
void *can_init(int bus, const char *dbc_name,
               size_t num_message_options, const MessageParseOptions *message_options,
               size_t num_signal_options, const SignalParseOptions *signal_options,
               bool sendcan, const char *tcp_addr, int timeout);

#ifdef __cplusplus
void *can_init_with_vectors(int bus, const char *dbc_name,
                            std::vector<MessageParseOptions> message_options,
                            std::vector<SignalParseOptions> signal_options,
                            bool sendcan, const char *tcp_addr, int timeout);
#endif

int can_update(void *can, uint64_t sec, bool wait);

void can_update_string(void *can, const char *dat, int len);

size_t can_query_latest(void *can, bool *out_can_valid, size_t out_values_size, SignalValue *out_values);

void can_query_latest_vector(void *can, bool *out_can_valid, std::vector<SignalValue> &values);
