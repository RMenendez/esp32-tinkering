#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <atomic>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace {

constexpr char DEVICE_NAME[] = "ESP32S3_Motor";
constexpr char SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char CHARACTERISTIC_UUID_RX[] =
    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char CHARACTERISTIC_UUID_TX[] =
    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

constexpr uint8_t MOTOR_A_IN1_PIN = 7;
constexpr uint8_t MOTOR_A_IN2_PIN = 6;
constexpr uint8_t MOTOR_B_IN1_PIN = 4;
constexpr uint8_t MOTOR_B_IN2_PIN = 5;

constexpr uint32_t PWM_FREQUENCY_HZ = 10000;
constexpr uint8_t PWM_RESOLUTION_BITS = 8;
constexpr uint32_t MAX_DURATION_MS = 10000;
constexpr uint8_t COMMAND_TEXT_SIZE = 64;
constexpr UBaseType_t COMMAND_QUEUE_LENGTH = 8;
constexpr UBaseType_t MOTOR_QUEUE_LENGTH = 8;
constexpr UBaseType_t EVENT_QUEUE_LENGTH = 8;
constexpr TickType_t MOTOR_POLL_PERIOD = pdMS_TO_TICKS(10);
constexpr uint32_t MOTOR_TASK_NOTIFY_SAFE_STOP = 1U << 0;

// Set these true if the corresponding physical motor's forward direction is
// opposite to the logical forward direction used by this sketch.
constexpr bool MOTOR_A_INVERTED = false;
constexpr bool MOTOR_B_INVERTED = false;

enum class RequestType : uint8_t {
  Move,
  Stop,
  Status,
};

enum class Movement : uint8_t {
  Forward,
  Backward,
  Left,
  Right,
};

struct BleCommand {
  char text[COMMAND_TEXT_SIZE];
  uint8_t length;
  bool tooLong;
  bool containsNull;
};

struct MotorRequest {
  RequestType type;
  Movement movement;
  uint32_t durationMs;
  uint8_t speedPercent;
};

struct MotorEvent {
  char text[96];
};

QueueHandle_t gBleCommandQueue = nullptr;
QueueHandle_t gMotorRequestQueue = nullptr;
QueueHandle_t gMotorEventQueue = nullptr;
TaskHandle_t gMotorTaskHandle = nullptr;

BLECharacteristic* gTxCharacteristic = nullptr;
std::atomic<bool> gBleConnected{false};
std::atomic<uint32_t> gDroppedBleWrites{0};

bool gPwmAttached[4] = {false, false, false, false};
bool gMotorHardwareReady = false;

const uint8_t kMotorPins[4] = {
    MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN};

const char* movementName(Movement movement) {
  switch (movement) {
    case Movement::Forward:
      return "FWD";
    case Movement::Backward:
      return "BACK";
    case Movement::Left:
      return "LEFT";
    case Movement::Right:
      return "RIGHT";
  }
  return "UNKNOWN";
}

void writePwm(uint8_t pin, uint8_t duty) {
  for (size_t i = 0; i < 4; ++i) {
    if (kMotorPins[i] == pin) {
      if (gPwmAttached[i]) {
        ledcWrite(pin, duty);
      } else {
        digitalWrite(pin, LOW);
      }
      return;
    }
  }
  digitalWrite(pin, LOW);
}

void stopMotors() {
  writePwm(MOTOR_A_IN1_PIN, 0);
  writePwm(MOTOR_A_IN2_PIN, 0);
  writePwm(MOTOR_B_IN1_PIN, 0);
  writePwm(MOTOR_B_IN2_PIN, 0);
}

void setMotor(uint8_t in1Pin, uint8_t in2Pin, Movement direction,
              uint8_t duty, bool inverted) {
  bool forward = direction == Movement::Forward;
  if (inverted) {
    forward = !forward;
  }

  writePwm(in1Pin, 0);
  writePwm(in2Pin, 0);
  if (forward) {
    writePwm(in1Pin, duty);
  } else {
    writePwm(in2Pin, duty);
  }
}

void postMotorEvent(const char* text) {
  MotorEvent event{};
  snprintf(event.text, sizeof(event.text), "%s", text);
  if (xQueueSend(gMotorEventQueue, &event, 0) != pdTRUE) {
    Serial.println("ERROR: motor event queue full; status event was dropped");
  }
}

void postMotorEventf(const char* format, const char* movement,
                     uint32_t durationMs, uint8_t speedPercent) {
  MotorEvent event{};
  snprintf(event.text, sizeof(event.text), format, movement,
           static_cast<unsigned long>(durationMs),
           static_cast<unsigned int>(speedPercent));
  if (xQueueSend(gMotorEventQueue, &event, 0) != pdTRUE) {
    Serial.println("ERROR: motor event queue full; status event was dropped");
  }
}

void applyMovement(Movement movement, uint8_t speedPercent) {
  const uint8_t duty =
      static_cast<uint8_t>((static_cast<uint16_t>(speedPercent) * 255U) / 100U);

  // In-place turns assume motor A is the left wheel and motor B is the right.
  switch (movement) {
    case Movement::Forward:
      setMotor(MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, Movement::Forward, duty,
               MOTOR_A_INVERTED);
      setMotor(MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, Movement::Forward, duty,
               MOTOR_B_INVERTED);
      break;
    case Movement::Backward:
      setMotor(MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, Movement::Backward, duty,
               MOTOR_A_INVERTED);
      setMotor(MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, Movement::Backward, duty,
               MOTOR_B_INVERTED);
      break;
    case Movement::Left:
      setMotor(MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, Movement::Backward, duty,
               MOTOR_A_INVERTED);
      setMotor(MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, Movement::Forward, duty,
               MOTOR_B_INVERTED);
      break;
    case Movement::Right:
      setMotor(MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN, Movement::Forward, duty,
               MOTOR_A_INVERTED);
      setMotor(MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN, Movement::Backward, duty,
               MOTOR_B_INVERTED);
      break;
  }
}

void handleMotorRequest(const MotorRequest& request, bool& movementActive,
                        Movement& activeMovement, uint32_t& deadlineMs) {
  if (request.type == RequestType::Stop) {
    stopMotors();
    if (movementActive) {
      postMotorEvent("STOPPED COMMAND");
    }
    movementActive = false;
    postMotorEvent("OK STOP");
    return;
  }

  if (request.type == RequestType::Status) {
    if (movementActive) {
      const uint32_t now = millis();
      const int32_t remaining =
          static_cast<int32_t>(deadlineMs - now);
      char status[96];
      snprintf(status, sizeof(status), "STATUS MOVING %s remaining_ms=%lu",
               movementName(activeMovement),
               static_cast<unsigned long>(remaining > 0 ? remaining : 0));
      postMotorEvent(status);
    } else {
      postMotorEvent("STATUS IDLE");
    }
    return;
  }

  if (!gBleConnected.load()) {
    stopMotors();
    movementActive = false;
    return;
  }
  if (!gMotorHardwareReady) {
    postMotorEvent("ERR motor hardware unavailable");
    return;
  }

  if (movementActive) {
    char replaced[96];
    snprintf(replaced, sizeof(replaced), "STOPPED REPLACED %s",
             movementName(activeMovement));
    postMotorEvent(replaced);
  }

  applyMovement(request.movement, request.speedPercent);
  activeMovement = request.movement;
  deadlineMs = millis() + request.durationMs;
  movementActive = true;
  postMotorEventf("STARTED %s duration_ms=%lu speed=%u",
                  movementName(activeMovement),
                  static_cast<unsigned long>(request.durationMs),
                  request.speedPercent);
}

void motorControllerTask(void*) {
  bool movementActive = false;
  Movement activeMovement = Movement::Forward;
  uint32_t deadlineMs = 0;

  for (;;) {
    uint32_t notificationBits = 0;
    xTaskNotifyWait(0, UINT32_MAX, &notificationBits, MOTOR_POLL_PERIOD);

    if ((notificationBits & MOTOR_TASK_NOTIFY_SAFE_STOP) != 0U) {
      stopMotors();
      if (movementActive) {
        postMotorEvent("STOPPED DISCONNECTED");
      }
      movementActive = false;
      xQueueReset(gMotorRequestQueue);
    }

    MotorRequest request{};
    while (xQueueReceive(gMotorRequestQueue, &request, 0) == pdTRUE) {
      handleMotorRequest(request, movementActive, activeMovement, deadlineMs);
    }

    if (movementActive &&
        static_cast<int32_t>(millis() - deadlineMs) >= 0) {
      stopMotors();
      movementActive = false;
      postMotorEvent("STOPPED TIMEOUT");
    }
  }
}

void notifyClient(const char* message) {
  if (gBleConnected.load() && gTxCharacteristic != nullptr) {
    gTxCharacteristic->setValue(message);
    gTxCharacteristic->notify();
  }
  Serial.println(message);
}

void processMotorEvents() {
  MotorEvent event{};
  while (xQueueReceive(gMotorEventQueue, &event, 0) == pdTRUE) {
    notifyClient(event.text);
  }
}

void processDroppedBleWrites() {
  const uint32_t dropped = gDroppedBleWrites.exchange(0);
  if (dropped > 0) {
    Serial.printf("ERROR: BLE command queue full; dropped %lu write(s)\n",
                  static_cast<unsigned long>(dropped));
    notifyClient("ERR command queue full");
  }
}

bool parseUnsigned(const char* text, uint32_t& result) {
  if (text == nullptr || *text == '\0') {
    return false;
  }
  for (const char* character = text; *character != '\0'; ++character) {
    if (!isdigit(static_cast<unsigned char>(*character))) {
      return false;
    }
  }

  errno = 0;
  char* end = nullptr;
  const unsigned long parsed = strtoul(text, &end, 10);
  if (errno == ERANGE || end == text || *end != '\0' ||
      parsed > UINT32_MAX) {
    return false;
  }
  result = static_cast<uint32_t>(parsed);
  return true;
}

bool enqueueMotorRequest(const MotorRequest& request) {
  if (!gMotorHardwareReady && request.type == RequestType::Move) {
    notifyClient("ERR motor hardware unavailable");
    return false;
  }
  if (xQueueSend(gMotorRequestQueue, &request, 0) != pdTRUE) {
    notifyClient("ERR motor request queue full");
    if (request.type == RequestType::Stop && gMotorTaskHandle != nullptr) {
      xTaskNotify(gMotorTaskHandle, MOTOR_TASK_NOTIFY_SAFE_STOP, eSetBits);
    }
    return false;
  }
  return true;
}

void handleBleCommand(BleCommand& command) {
  if (command.tooLong) {
    notifyClient("ERR command too long");
    return;
  }
  if (command.containsNull) {
    notifyClient("ERR command contains invalid byte");
    return;
  }

  command.text[command.length] = '\0';
  for (uint8_t i = 0; i < command.length; ++i) {
    command.text[i] =
        static_cast<char>(toupper(static_cast<unsigned char>(command.text[i])));
  }

  char* tokens[4] = {nullptr, nullptr, nullptr, nullptr};
  size_t tokenCount = 0;
  char* savePointer = nullptr;
  char* token = strtok_r(command.text, " \t\r\n", &savePointer);
  while (token != nullptr) {
    if (tokenCount == sizeof(tokens) / sizeof(tokens[0])) {
      notifyClient("ERR too many command parameters");
      return;
    }
    tokens[tokenCount++] = token;
    token = strtok_r(nullptr, " \t\r\n", &savePointer);
  }

  if (tokenCount == 0) {
    notifyClient("ERR empty command");
    return;
  }

  MotorRequest request{};
  if (strcmp(tokens[0], "STOP") == 0) {
    if (tokenCount != 1) {
      notifyClient("ERR STOP takes no parameters");
      return;
    }
    request.type = RequestType::Stop;
    enqueueMotorRequest(request);
    return;
  }

  if (strcmp(tokens[0], "STATUS") == 0) {
    if (tokenCount != 1) {
      notifyClient("ERR STATUS takes no parameters");
      return;
    }
    request.type = RequestType::Status;
    enqueueMotorRequest(request);
    return;
  }

  if (tokenCount != 3) {
    notifyClient("ERR movement format: COMMAND duration_ms speed_percent");
    return;
  }

  if (strcmp(tokens[0], "FWD") == 0) {
    request.movement = Movement::Forward;
  } else if (strcmp(tokens[0], "BACK") == 0) {
    request.movement = Movement::Backward;
  } else if (strcmp(tokens[0], "LEFT") == 0) {
    request.movement = Movement::Left;
  } else if (strcmp(tokens[0], "RIGHT") == 0) {
    request.movement = Movement::Right;
  } else {
    notifyClient("ERR unknown command");
    return;
  }

  uint32_t durationMs = 0;
  uint32_t speedPercent = 0;
  if (!parseUnsigned(tokens[1], durationMs) ||
      !parseUnsigned(tokens[2], speedPercent)) {
    notifyClient("ERR duration and speed must be unsigned integers");
    return;
  }
  if (durationMs == 0 || durationMs > MAX_DURATION_MS) {
    notifyClient("ERR duration must be 1..10000 ms");
    return;
  }
  if (speedPercent < 1 || speedPercent > 100) {
    notifyClient("ERR speed must be 1..100 percent");
    return;
  }

  request.type = RequestType::Move;
  request.durationMs = durationMs;
  request.speedPercent = static_cast<uint8_t>(speedPercent);
  enqueueMotorRequest(request);
}

class MotorServerCallbacks final : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer*) override {
    gBleConnected.store(true);
  }

  void onDisconnect(BLEServer* server) override {
    gBleConnected.store(false);
    if (gMotorTaskHandle != nullptr) {
      xTaskNotify(gMotorTaskHandle, MOTOR_TASK_NOTIFY_SAFE_STOP, eSetBits);
    }
    server->startAdvertising();
  }
};

class MotorWriteCallbacks final : public BLECharacteristicCallbacks {
 public:
  void onWrite(BLECharacteristic* characteristic) override {
    const String value = characteristic->getValue();
    const size_t valueLength = value.length();
    BleCommand command{};
    if (valueLength >= sizeof(command.text)) {
      command.tooLong = true;
    } else {
      command.length = static_cast<uint8_t>(valueLength);
      for (size_t i = 0; i < valueLength; ++i) {
        if (value[i] == '\0') {
          command.containsNull = true;
        }
        command.text[i] = value[i];
      }
    }

    if (xQueueSend(gBleCommandQueue, &command, 0) != pdTRUE) {
      gDroppedBleWrites.fetch_add(1);
    }
  }
};

void stopOnStartupFailure(const char* message) {
  Serial.println(message);
  stopMotors();
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

  for (uint8_t pin : kMotorPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  for (size_t i = 0; i < 4; ++i) {
    gPwmAttached[i] = ledcAttach(kMotorPins[i], PWM_FREQUENCY_HZ,
                                 PWM_RESOLUTION_BITS);
  }
  gMotorHardwareReady = gPwmAttached[0] && gPwmAttached[1] &&
                        gPwmAttached[2] && gPwmAttached[3];
  stopMotors();
  if (!gMotorHardwareReady) {
    Serial.println("ERROR: one or more motor PWM channels failed to attach");
  }

  gBleCommandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(BleCommand));
  gMotorRequestQueue = xQueueCreate(MOTOR_QUEUE_LENGTH, sizeof(MotorRequest));
  gMotorEventQueue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(MotorEvent));
  if (gBleCommandQueue == nullptr || gMotorRequestQueue == nullptr ||
      gMotorEventQueue == nullptr) {
    stopOnStartupFailure("FATAL: failed to create one or more FreeRTOS queues");
  }

  if (xTaskCreate(motorControllerTask, "MotorController", 4096, nullptr, 2,
                  &gMotorTaskHandle) != pdPASS) {
    stopOnStartupFailure("FATAL: failed to create motor controller task");
  }

  BLEDevice::init(DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  if (server == nullptr) {
    stopOnStartupFailure("FATAL: failed to create BLE server");
  }
  server->setCallbacks(new MotorServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);
  if (service == nullptr) {
    stopOnStartupFailure("FATAL: failed to create BLE service");
  }

  BLECharacteristic* rxCharacteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxCharacteristic->setCallbacks(new MotorWriteCallbacks());

  gTxCharacteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  gTxCharacteristic->addDescriptor(new BLE2902());

  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE motor controller ready; advertise name: ESP32S3_Motor");
}

void loop() {
  processDroppedBleWrites();

  BleCommand command{};
  if (xQueueReceive(gBleCommandQueue, &command, 0) == pdTRUE) {
    handleBleCommand(command);
  }

  processMotorEvents();
  vTaskDelay(pdMS_TO_TICKS(1));
}
