#include <naos.h>
#include <naos/ble.h>
#include <naos/wifi.h>
#include <naos/mqtt.h>
#include <naos/bridge.h>
#include <naos/sys.h>
#include <art32/numbers.h>
#include <art32/smooth.h>
#include <art32/convert.h>
#include <driver/gpio.h>
#include <string.h>

#include "buttons.h"
#include "encoder.h"
#include "end_stop.h"
#include "l6470.h"
#include "led.h"

static naos_status_t current_status = NAOS_DISCONNECTED;

int32_t micro_steps = 128;
double gear_ratio = 0;
int32_t resolution = 0;
double max_speed = 0;
double acceleration = 0;
double deceleration = 0;
double min_target = 0;
double max_target = 0;
bool use_sensor_1 = false;
bool use_sensor_2 = false;

a32_smooth_t *sensor_smooth_1;
a32_smooth_t *sensor_smooth_2;

bool button_active = false;

static void set_status() {
  // set led accordingly
  switch (current_status) {
    case NAOS_DISCONNECTED:
      led_set(1024, 0, 0);
      break;
    case NAOS_CONNECTED:
      led_set(0, 0, 1024);
      break;
    case NAOS_NETWORKED:
      led_set(0, 1024, 0);
      break;
  }
}

static void status(naos_status_t status) {
  // set last status
  current_status = status;

  // set new status
  set_status();
}

static void ping() {
  // blink white once
  led_set(1024, 1024, 1024);
  naos_delay(300);

  // set status
  set_status();
}

static void set_micro_steps(int32_t n) {
  // make motor stop (remove power)
  l6470_hard_hiz();

  // set micro steps
  l6470_set_step_mode_int(n);

  // reset position
  l6470_reset_position();
}

static void set_max_speed(double n) {
  // set setting
  l6470_set_maximum_speed(l6470_calculate_maximum_speed(n));

  // set full step mode to two times max speed (no full stepping)
  l6470_set_full_step_speed(l6470_calculate_full_step_speed(n * 2));
}

static void set_acceleration(double n) {
  // set setting
  l6470_set_acceleration(l6470_calculate_acceleration(n));
}

static void set_deceleration(double n) {
  // set setting
  l6470_set_deceleration(l6470_calculate_deceleration(n));
}

static void forward() {
  // run forward
  l6470_run(L6470_FORWARD, l6470_calculate_speed(max_speed));
}

static void backward() {
  // run backward
  l6470_run(L6470_REVERSE, l6470_calculate_speed(max_speed));
}

static void approach(double target) {
  // calculate position factor
  double steps_per_rev = micro_steps * resolution * gear_ratio;

  // calculate minimum and maximum physical position
  double min_phys_target = L6470_I22_MIN / steps_per_rev;
  double max_phys_target = L6470_I22_MAX / steps_per_rev;

  // constrain target
  target = a32_constrain_d(target, min_phys_target, max_phys_target);
  target = a32_constrain_d(target, min_target, max_target);

  // calculate real position
  int32_t pos = (int32_t)(target * steps_per_rev);

  // constrain position
  pos = (int32_t)a32_constrain_l(pos, L6470_I22_MIN, L6470_I22_MAX);

  // approach target
  l6470_approach_target(pos);
}

static void stop() {
  // stop motor
  l6470_soft_stop();
}

static void reset() {
  // stop motor
  l6470_hard_stop();

  // set home pos
  l6470_reset_position();
}

static void loop() {
  // this loop is called at a rate of 100/s

  // get time
  uint32_t now = naos_millis();

  // check sensor 1
  if (use_sensor_1) {
    // prepare counter
    static uint32_t last_send_1 = 0;

    // read sensor
    double es1 = end_stop_read_1();

    // smooth value
    es1 = a32_smooth_update(sensor_smooth_1, es1);

    // check if 100ms passed
    if (last_send_1 + 100 < now) {
      // set time
      last_send_1 = now;

      // publish sensor value
      naos_publish_d("sensor1", es1, 0, false, NAOS_LOCAL);
    }
  }

  // check sensor 2
  if (use_sensor_2) {
    // prepare counter
    static uint32_t last_send_2 = 0;

    // read sensors
    double es2 = end_stop_read_2();

    // smooth value
    es2 = a32_smooth_update(sensor_smooth_2, es2);

    // check if 100ms passed
    if (last_send_2 + 100 < now) {
      // set time
      last_send_2 = now;

      // publish sensor value
      naos_publish_d("sensor2", es2, 0, false, NAOS_LOCAL);
    }
  }
}

static void press(buttons_type_t type, bool pressed) {
  // prepare home press counter
  static uint32_t home_press = 0;

  // turn forward if cw is released
  if (type == BUTTONS_TYPE_CW && !pressed) {
    forward();
    button_active = true;
  }

  // turn backwards if ccw is released
  if (type == BUTTONS_TYPE_CCW && !pressed) {
    backward();
    button_active = true;
  }

  // stop motor stop is pressed
  if (type == BUTTONS_TYPE_STOP && pressed) {
    stop();
  }

  // set time if pressed
  if (type == BUTTONS_TYPE_HOME && pressed) {
    home_press = naos_millis();
  }

  // handle home button release
  if (type == BUTTONS_TYPE_HOME && !pressed) {
    // get time difference
    uint32_t diff = naos_millis() - home_press;

    // approach home if buttons has been released quickly
    if (diff < 500) {
      approach(0);
      button_active = true;
      return;
    }

    // otherwise reset position
    reset();

    // flash LED
    led_set(255, 255, 255);
    naos_delay(300);
    set_status();
  }
}

static void position(double p) {}

static void end_stop(end_stop_pin_t pin, bool on) {}

static void offline() {
  // stop motor
  l6470_soft_stop();
}

static void monitor() {
  // get position
  int32_t raw_pos = l6470_get_absolute_position();
  double pos = (double)raw_pos / (micro_steps * resolution * gear_ratio);

  // get speed
  double raw_speed = l6470_get_speed();
  double speed = raw_speed / (micro_steps * resolution * gear_ratio);

  // clear button_active once motor has fully stopped
  if (button_active && speed == 0) {
    button_active = false;
  }

  // set parameters
  naos_set_d("position", pos);
  naos_set_d("speed", speed);
  naos_set_b("running", speed != 0);

  // publish messages
  naos_publish_d("position", pos, 0, false, NAOS_LOCAL);
  naos_publish_d("speed", speed, 0, false, NAOS_LOCAL);
  naos_publish_b("running", speed != 0, 0, false, NAOS_LOCAL);
}

static void online() {
  // subscribe to MQTT command topics (local scope)
  naos_subscribe("forward", 0, NAOS_LOCAL);
  naos_subscribe("backward", 0, NAOS_LOCAL);
  naos_subscribe("target", 0, NAOS_LOCAL);
  naos_subscribe("stop", 0, NAOS_LOCAL);
  naos_subscribe("reset", 0, NAOS_LOCAL);
  naos_subscribe("home", 0, NAOS_LOCAL);
}

static void message(const char *topic, const uint8_t *payload, size_t len, naos_scope_t scope) {
  // buttons override MQTT: ignore commands while button_active
  if (button_active) {
    return;
  }

  if (scope != NAOS_LOCAL) {
    return;
  }

  // make null-terminated string from payload
  char buf[64];
  size_t copy_len = len < sizeof(buf) - 1 ? len : sizeof(buf) - 1;
  memcpy(buf, payload, copy_len);
  buf[copy_len] = '\0';

  // handle "forward" command
  if (strcmp(topic, "forward") == 0) {
    forward();
    return;
  }

  // handle "backward" command
  if (strcmp(topic, "backward") == 0) {
    backward();
    return;
  }

  // handle "target" command
  if (strcmp(topic, "target") == 0) {
    double target = a32_str2d(buf);
    approach(target);
    return;
  }

  // handle "stop" command
  if (strcmp(topic, "stop") == 0) {
    stop();
    return;
  }

  // handle "reset" command
  if (strcmp(topic, "reset") == 0) {
    reset();
    return;
  }

  // handle "home" command
  if (strcmp(topic, "home") == 0) {
    approach(0);
    return;
  }
}

static naos_param_t params[] = {
    {.name = "micro-steps", .type = NAOS_LONG, .default_l = 128, .sync_l = &micro_steps, .func_l = set_micro_steps},
    {.name = "gear-ratio", .type = NAOS_DOUBLE, .default_d = 5.18, .sync_d = &gear_ratio},
    {.name = "resolution", .type = NAOS_LONG, .default_l = 200, .sync_l = &resolution},
    {.name = "max-speed", .type = NAOS_DOUBLE, .default_d = 900, .sync_d = &max_speed, .func_d = set_max_speed},
    {.name = "acceleration",
     .type = NAOS_DOUBLE,
     .default_d = 900,
     .sync_d = &acceleration,
     .func_d = set_acceleration},
    {.name = "deceleration",
     .type = NAOS_DOUBLE,
     .default_d = 900,
     .sync_d = &deceleration,
     .func_d = set_deceleration},
    {.name = "min-target", .type = NAOS_DOUBLE, .default_d = -5, .sync_d = &min_target},
    {.name = "max-target", .type = NAOS_DOUBLE, .default_d = 5, .sync_d = &max_target},
    {.name = "use-sensor-1", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_sensor_1},
    {.name = "use-sensor-2", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_sensor_2},
    {.name = "forward", .type = NAOS_ACTION, .func_a = forward},
    {.name = "backward", .type = NAOS_ACTION, .func_a = backward},
    {.name = "approach", .type = NAOS_DOUBLE, .func_d = approach, .mode = NAOS_VOLATILE},
    {.name = "stop", .type = NAOS_ACTION, .func_a = stop},
    {.name = "reset", .type = NAOS_ACTION, .func_a = reset},
    {.name = "position", .type = NAOS_DOUBLE, .mode = NAOS_VOLATILE},
    {.name = "speed", .type = NAOS_DOUBLE, .mode = NAOS_VOLATILE},
    {.name = "running", .type = NAOS_BOOL, .mode = NAOS_VOLATILE},
};

static naos_config_t config = {
    .app_name = "NetStepper2",
    .app_version = "0.6.0",
    .parameters = params,
    .num_parameters = sizeof(params) / sizeof(naos_param_t),
    .ping_callback = ping,
    .status_callback = status,
    .online_callback = online,
    .message_callback = message,
    .loop_callback = loop,
    .loop_interval = 10,
    .offline_callback = offline,
};

void app_main() {
  // install gpio interrupt service
  gpio_install_isr_service(0);

  // initialize HMI
  led_init();
  buttons_init(press);
  encoder_init(position);

  // initialize controller
  l6470_init();

  // initialize sensors
  sensor_smooth_1 = a32_smooth_new(16);
  sensor_smooth_2 = a32_smooth_new(16);
  end_stop_init(end_stop, true);

  // set initial status
  set_status();

  // initialize naos
  naos_init(&config);
  naos_ble_init((naos_ble_config_t){});
  naos_wifi_init();
  naos_mqtt_init(1);
  naos_bridge_install();
  naos_start();

  // set minimum speed
  l6470_set_minimum_speed(l6470_calculate_minimum_speed(0));

  // run monitor
  naos_repeat("monitor", 100, monitor);
}
