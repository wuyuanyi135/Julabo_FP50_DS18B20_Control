//
// Created by wuyuanyi on 2020-09-05.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_FP50_H
#define JULABO_FP50_DS18B20_CONTROL_FP50_H
#include <circular_queue/circular_queue.h>
#include "Arduino.h"
#include "debug.h"
#include "pt/asyncpt.h"
#include "pt/pt-sem.h"
#include "pt/pt.h"

#define QUEUE_SIZE 10
#define RECV_BUFFER_SIZE 300
#define COMMAND_TIME_GAP_MS 400
#define IN_COMMAND_TIME_GAP_MS 50
#define IN_COMMAND_TIMEOUT_MS 2000
// When in command is waiting. Do not send more commands.
#define IN_COMMAND_EXCLUSIVE 1

enum SelfTuning {
  Off = 0,
  Once,
  Always,
};

enum Dynamics {
  Aperiodic = 0,
  Standard,
};

struct Resolvable {
  struct pt_sem *sem;
  String *dest;
  String identifier;
  ulong timestamp;
};

// FP50 should be put in software handshake mode
class FP50 {
 public:
  explicit FP50(Stream &serial);

 public:
  // OUT commands
  /// Use working temperature of setpointN. Used command: OUT_MODE_01
  /// \param id
  void select_setpoint(int id);

  /// Set self tuning mode
  /// Used commands: OUT_MODE_02
  /// \param opt
  void set_self_tuning(SelfTuning opt);

  /// Stop or start the device. Used commands: OUT_MODE_05
  /// \param start
  void switch_power(bool start);

  /// Set the control dyanmics. Used commands: OUT_MODE_08
  /// \param opt
  void set_dynamics(Dynamics opt);

  /// Set the setpoint
  /// Used commands:
  /// OUT_SP_00 OUT_SP_01 OUT_SP_02
  /// \param temp temperature
  /// \param id which setpoint
  void set_setpoint(double temp, uint8 id = 0);

  /// Set high temperature warning limit (OUT_SP_03)
  /// \param overtemp
  void set_overtemp(double overtemp);

  /// Set low temperature warning limit (OUT_SP_04)
  /// \param subtemp
  void set_subtemp(double subtemp);

  /// Set the pump pressure stage (OUT_SP_07)
  /// \param pressure
  void set_pump_pressure(uint8 pressure);

 public:
  // Status and version
  /// Get version (VERSION)
  /// \return
  PT_THREAD(get_version(AsyncPT &pt, String &version));

  /// Get status (STATUS)
  /// \return
  PT_THREAD(get_status(AsyncPT &pt, String &status));

 public:
  // IN commands

  /// Current bath temperature (IN_PV_00)
  /// \return
  PT_THREAD(get_bath_temperature(AsyncPT &pt, double &temp));

  /// Heating power (%) (IN_PV_01)
  /// \return
  PT_THREAD(get_heating_power(AsyncPT &pt, double &power));

  /// Get working temperature setpoint N (IN_SP_00 IN_SP_01 IN_SP_02)
  /// \param id
  /// \return
  PT_THREAD(get_setpoint(AsyncPT &pt, uint8 id, double &setpoint));

  PT_THREAD(get_overtemp(AsyncPT &pt, double &temp));
  PT_THREAD(get_subtemp(AsyncPT &pt, double &temp));

  /// IN_SP_07
  /// \return
  PT_THREAD(get_pump_stage(AsyncPT &pt, double &pump));

  PT_THREAD(check_ready(AsyncPT &pt, bool &ready));

 private:
  Stream &serial;
  bool xon = true;
  circular_queue<String> cmdQueue;
  circular_queue<Resolvable> semQueue;
  uint64 lastSentTime;
  circular_queue<char> buffer;

 public:
  bool ok = false;

 public:
  void queue_command(String command);
  void queue_command_with_response(String command, pt_sem &sem, String &recv);

 public:
  // Tasks
  PT_THREAD(daemon(struct pt *pt));
  void begin();
};

#endif  // JULABO_FP50_DS18B20_CONTROL_FP50_H
