//
// Created by wuyuanyi on 2020-09-05.
//

#include "FP50.h"
FP50::FP50(Stream &serial)
    : serial(serial),
      cmdQueue(QUEUE_SIZE),
      semQueue(QUEUE_SIZE),
      buffer((size_t)RECV_BUFFER_SIZE) {}

void FP50::begin() {
  LOGVF("Receive buffer capacity: %d", (int)buffer.capacity());
}

void FP50::select_setpoint(int id) {
  if (id > 2 || id < 0) return;
  String command;
  command = "OUT_MODE_01 ";
  command += id;
  command += "\r";
  LOGVF("Select setpoint %d.", id);
  queue_command(command);
}
void FP50::set_self_tuning(SelfTuning opt) {
  String command;
  command = "OUT_MODE_02 ";
  command += (int)opt;
  command += "\r";
  LOGVF("Set self tuning: %d.", opt);
  queue_command(command);
}
void FP50::switch_power(bool start) {
  String command;
  command = "OUT_MODE_05 ";
  command += (int)start;
  command += "\r";
  LOGVF("Switch power: %d.", start);
  queue_command(command);
}
void FP50::set_dynamics(Dynamics opt) {
  String command;
  command = "OUT_MODE_08 ";
  command += (int)opt;
  command += "\r";
  LOGVF("Set dynamics: %d.", opt);
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
  LOGVF("Set setpoint %d: %f.", id, temp);
  queue_command(command);
}

void FP50::set_overtemp(double overtemp) {
  String command;
  char buf[7];
  snprintf(buf, 7, "%.2f", overtemp);
  command = "OUT_SP_03 ";
  command += buf;
  command += "\r";
  LOGVF("Set overtemp: %f.", overtemp);
  queue_command(command);
}

void FP50::set_subtemp(double subtemp) {
  String command;
  char buf[7];
  snprintf(buf, 7, "%.2f", subtemp);
  command = "OUT_SP_04 ";
  command += buf;
  command += "\r";
  LOGVF("Set subtemp: %f.", subtemp);
  queue_command(command);
}

void FP50::set_pump_pressure(uint8 pressure) {
  if (pressure < 1 || pressure > 4) return;
  String command;
  command = "OUT_SP_07 ";
  command += " ";
  command += pressure;
  command += "\r";
  LOGVF("Set pump pressure: %d.", pressure);
  queue_command(command);
}

char FP50::get_version(AsyncPT &pt, String &version) {
  PT_BEGIN(&pt.pt);
  LOGV("Getting version.");
  queue_command_with_response("VERSION\r", pt.sem, version);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  PT_END(&pt.pt);
}

char FP50::get_status(AsyncPT &pt, String &status) {
  PT_BEGIN(&pt.pt);
  LOGV("Getting status.");
  queue_command_with_response("STATUS\r", pt.sem, status);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  PT_END(&pt.pt);
}

char FP50::get_bath_temperature(AsyncPT &pt, double &temp) {
  static String sTemp;
  PT_BEGIN(&pt.pt);
  LOGV("Getting bath temperature.");
  queue_command_with_response("IN_PV_00\r", pt.sem, sTemp);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  temp = sTemp.toDouble();
  PT_END(&pt.pt);
}

char FP50::get_heating_power(AsyncPT &pt, double &power) {
  static String sPower;
  PT_BEGIN(&pt.pt);
  LOGV("Getting heating power.");
  queue_command_with_response("IN_PV_01\r", pt.sem, sPower);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  power = sPower.toDouble();
  PT_END(&pt.pt);
}

char FP50::get_setpoint(AsyncPT &pt, uint8 id, double &setpoint) {
  static String sPoint;
  static String req;
  PT_BEGIN(&pt.pt);
  if (id > 2 || id < 0) {
    PT_EXIT(&pt.pt);
  }
  req = "IN_SP_0";
  req += id;
  req += "\r";
  LOGV("Getting setpoint.");
  queue_command_with_response(req, pt.sem, sPoint);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  setpoint = sPoint.toDouble();
  PT_END(&pt.pt);
}

char FP50::get_overtemp(AsyncPT &pt, double &temp) {
  static String tmp;
  PT_BEGIN(&pt.pt);
  LOGV("Getting overtemp.");
  queue_command_with_response("IN_SP_03\r", pt.sem, tmp);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  temp = tmp.toDouble();
  PT_END(&pt.pt);
}

char FP50::get_subtemp(AsyncPT &pt, double &temp) {
  static String tmp;
  PT_BEGIN(&pt.pt);
  LOGV("Getting subtemp.");
  queue_command_with_response("IN_SP_04\r", pt.sem, tmp);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  temp = tmp.toDouble();
  PT_END(&pt.pt);
}

char FP50::get_pump_stage(AsyncPT &pt, double &pump) {
  static String tmp;
  PT_BEGIN(&pt.pt);
  LOGV("Getting pump stage.");
  queue_command_with_response("IN_SP_07\r", pt.sem, tmp);
  PT_SEM_WAIT(&pt.pt, &pt.sem);
  pump = tmp.toDouble();
  PT_END(&pt.pt);
}

void FP50::queue_command(String command) { cmdQueue.push(command); }

char FP50::daemon(struct pt *pt) {
  static int recv;
  static char response[128];
  PT_BEGIN(pt);

  while (true) {
    // Clear up timeout in command
    {
      if (semQueue.available()) {
        const Resolvable &resolvable = semQueue.peek();
        if (millis() - resolvable.timestamp > IN_COMMAND_TIMEOUT_MS) {
          String ident = resolvable.identifier;
          ident.trim();
          LOGEF("Command [%s] timed out!", ident.c_str());
          PT_SEM_SIGNAL(pt, resolvable.sem);
          semQueue.pop();
          buffer.flush();
        }
      }
    }

    // Receive
    while (true) {
      recv = serial.read();
      if (recv != -1) {
        // Is it flow control char?
        if ((char)recv == 0x13) {
          xon = false;
          LOGV("XOFF");
          continue;
        } else if ((char)recv == 0x11) {
          xon = true;
          LOGV("XON");
          continue;
        }

        // Is it end of transaction?
        if ((char)recv == '\n') {
          const size_t size = buffer.available();
          LOGVF("CR received. Size=%zu", size);

          buffer.pop_n(response, size);
          response[size + 1] = '\0'; // overwrite LF
          if (semQueue.available()) {
            const Resolvable &p = semQueue.pop();

//            LOGVF("[%s] responds in %lu", response, millis() - p.timestamp);
            *p.dest = String(response);
            p.dest->trim();
            lastSentTime = millis() + IN_COMMAND_TIME_GAP_MS;
            PT_SEM_SIGNAL(pt, p.sem);
          } else {
            // TODO: HARD ERROR
            LOGEF("No pending task but [%s] has been received",
                  response);
            buffer.flush();
          }
          continue;
        }

        // Otherwise, store this character for later.
        if (!buffer.push(recv)) {
          LOGE("Recv buffer is full.");
        }
      } else {
        // This read round is end. Will see if more come later.
        break;
      }
    }

    // Send data
    {
      if ((millis() - lastSentTime > COMMAND_TIME_GAP_MS) &&
          (cmdQueue.available()) && (xon) &&
          !(IN_COMMAND_EXCLUSIVE && semQueue.available() > 1)) {
        String cmd = cmdQueue.pop();
        if (!cmd) {
          LOGE("Command is not valid");
        } else {
          serial.print(cmd);
          lastSentTime = millis();
          cmd.trim();
          LOGVF("Message sent @ %llu: \"%s\". ", lastSentTime, cmd.c_str());
        }
      }
    }
    PT_YIELD(pt);
  }
  PT_END(pt);
}
void FP50::queue_command_with_response(String command, pt_sem &sem,
                                       String &recv) {
  cmdQueue.push(command);
  Resolvable r;
  r.sem = &sem;
  r.dest = &recv;
  r.identifier = command;
  r.timestamp = millis();
  semQueue.push(r);
}
