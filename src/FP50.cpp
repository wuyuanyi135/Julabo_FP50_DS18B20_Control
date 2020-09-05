//
// Created by wuyuanyi on 2020-09-05.
//

#include "FP50.h"
FP50::FP50(Stream &serial) : serial(serial), cmdQueue(QUEUE_SIZE) {}
void FP50::select_setpoint(int id) {
  if (id > 2 || id < 0) return;
  String command;
  command = "OUT_MODE_01 ";
  command += id;
  command += "\r";
  queue_command(command);
}
void FP50::set_self_tuning(SelfTuning opt) {
  String command;
  command = "OUT_MODE_02 ";
  command += (int)opt;
  command += "\r";
  queue_command(command);
}
void FP50::switch_power(bool start) {
  String command;
  command = "OUT_MODE_05 ";
  command += (int)start;
  command += "\r";
  queue_command(command);
}
void FP50::set_dynamics(Dynamics opt) {
  String command;
  command = "OUT_MODE_08 ";
  command += (int)opt;
  command += "\r";
  queue_command(command);
}
void FP50::set_setpoint(double temp, uint8 id) {
  if (id > 2 || id < 0) return;
  String command;
  char buf[7];
  snprintf(buf, 7, "%.2f", temp);
  command = "OUT_SP_0";
  command += id;
  command += " ";
  command += buf;
  command += "\r";
  queue_command(command);
}
void FP50::set_overtemp(double overtemp) {
  String command;
  char buf[7];
  snprintf(buf, 7, "%.2f", overtemp);
  command = "OUT_SP_03 ";
  command += buf;
  command += "\r";
  queue_command(command);
}
void FP50::set_subtemp(double subtemp) {
  String command;
  char buf[7];
  snprintf(buf, 7, "%.2f", subtemp);
  command = "OUT_SP_04 ";
  command += buf;
  command += "\r";
  queue_command(command);
}
void FP50::set_pump_pressure(uint8 pressure) {
  if (pressure < 1 || pressure > 4) return;
  String command;
  command = "OUT_SP_07 ";
  command += " ";
  command += pressure;
  command += "\r";
  queue_command(command);
}
char FP50::get_version(struct pt *pt, String &version) {
  PT_BEGIN(pt);
  queue_command("VERSION\r");
  PT_YIELD_UNTIL(pt, serial.findUntil("\n", ""));
  String resp = serial.readStringUntil('\n');
  resp.trim();
  version = resp;
  PT_END(pt);
}

void FP50::queue_command(String command) { cmdQueue.push(command); }

char FP50::daemon(struct pt *pt) {
  static uint64 now;
  PT_BEGIN(pt);

  while (true) {
    now = millis();
    if (now - lastSentTime > COMMAND_TIME_GAP_MS && cmdQueue.available()) {
      String cmd = cmdQueue.pop();
      serial.print(cmd);
      lastSentTime = millis();
      cmd.trim();
      Log.verbose("Message sent @ %d: %s" CR, lastSentTime, cmd.c_str());
    }
    PT_YIELD(pt);
  }
  PT_END(pt);
}

