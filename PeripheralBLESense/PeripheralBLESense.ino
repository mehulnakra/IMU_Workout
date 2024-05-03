/*
  BLE_Peripheral.ino

  This program uses the ArduinoBLE library to set-up an Arduino Nano 33 BLE 
  as a peripheral device and specifies a service and a characteristic. Depending 
  of the value of the specified characteristic, an on-board LED gets on. 

  The circuit:
  - Arduino Nano 33 BLE. 

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

//########################################### TF DECLAR  ########################


#include <TensorFlowLite.h>

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "magic_wand_model_data.h"
#include "rasterize_stroke.h"
#include "imu_provider.h"

namespace {

  const int VERSION = 0x00000000;

  // Constants for image rasterization
  constexpr int raster_width = 32;
  constexpr int raster_height = 32;
  constexpr int raster_channels = 3;
  constexpr int raster_byte_count = raster_height * raster_width * raster_channels;
  int8_t raster_buffer[raster_byte_count];

  // String to calculate the local and device name
  String name;
  
  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  constexpr int kTensorArenaSize = 30 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
  
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  
  // -------------------------------------------------------------------------------- //
  // UPDATE THESE VARIABLES TO MATCH THE NUMBER AND LIST OF GESTURES IN YOUR DATASET  //
  // -------------------------------------------------------------------------------- //
  constexpr int label_count = 2;
  const char* labels[label_count] = {"0", "1"};

}  // namespace


//########################################### TF DECLAR END ########################

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

static uint8_t Data1=2;
static BLEDevice server;


BLEService gestureService(deviceServiceUuid); 
BLEByteCharacteristic gestureCharacteristic(deviceServiceCharacteristicUuid, BLEWrite);


void setup() {
  //Serial.begin(9600);
 // while (!Serial);  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  if (!BLE.begin()) {
    //Serial.println("- Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  if (!IMU.begin()) {
    //Serial.println("Failed to initialized IMU!");
    while (1);
  }
  SetupIMU();
  

  BLE.setLocalName("Arduino Nano 33 BLE (Peripheral)");
  BLE.setAdvertisedService(gestureService);
  gestureService.addCharacteristic(gestureCharacteristic);
  BLE.addService(gestureService);
  gestureCharacteristic.writeValue(true);
  BLE.advertise();

  //Serial.println("Nano 33 BLE (Peripheral Device)");
  //Serial.println(" ");

    // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  static tflite::MicroErrorReporter micro_error_reporter;  // NOLINT
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_magic_wand_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  static tflite::MicroMutableOpResolver<4> micro_op_resolver;  // NOLINT
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMean();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddSoftmax();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  interpreter->AllocateTensors();

  // Set model input settings
  TfLiteTensor* model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != raster_height) ||
      (model_input->dims->data[2] != raster_width) ||
      (model_input->dims->data[3] != raster_channels) ||
      (model_input->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad input tensor parameters in model");
    return;
  }

  // Set model output settings
  TfLiteTensor* model_output = interpreter->output(0);
  if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) ||
      (model_output->dims->data[1] != label_count) ||
      (model_output->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad output tensor parameters in model");
    return;
  }

  //Serial.println("- SETUP Discovering Central device...");

  if(!server.connected()){
  do
  {
    BLE.scanForUuid(deviceServiceUuid);
    server = BLE.available();
  } while (!server);
  }

  while(!server.connected()){
   // Serial.println("SETUP Connecting to the server !!!!!!!");
   // Serial.println(server.connect());
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  server.connect();
   delay(1000);
  }


}




void loop() {

  loop2();

}

void loop2() {

    // make sure IMU data is available then read in data
  const bool data_available = IMU.accelerationAvailable() || IMU.gyroscopeAvailable();
  if (!data_available) {
    return;
  }
  int accelerometer_samples_read;
  int gyroscope_samples_read;
  ReadAccelerometerAndGyroscope(&accelerometer_samples_read, &gyroscope_samples_read);

  // Parse and process IMU data
  bool done_just_triggered = false;
  if (gyroscope_samples_read > 0) {
    EstimateGyroscopeDrift(current_gyroscope_drift);
    UpdateOrientation(gyroscope_samples_read, current_gravity, current_gyroscope_drift);
    UpdateStroke(gyroscope_samples_read, &done_just_triggered);
  }
  if (accelerometer_samples_read > 0) {
    EstimateGravityDirection(current_gravity);
    UpdateVelocity(accelerometer_samples_read, current_gravity);
  }

  // Wait for a gesture to be done
  if (done_just_triggered) {
    // Rasterize the gesture
    RasterizeStroke(stroke_points, *stroke_transmit_length, 0.6f, 0.6f, raster_width, raster_height, raster_buffer);

    // Pass to the model and run the interpreter
    TfLiteTensor* model_input = interpreter->input(0);
    for (int i = 0; i < raster_byte_count; ++i) {
      model_input->data.int8[i] = raster_buffer[i];
    }
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
      return;
    }
    TfLiteTensor* output = interpreter->output(0);

    // Parse the model output
    int8_t max_score;
    int max_index;
    for (int i = 0; i < label_count; ++i) {
      const int8_t score = output->data.int8[i];
      if ((i == 0) || (score > max_score)) {
        max_score = score;
        max_index = i;
      }
    }
  
  Data1 = max_index;
    //TF_LITE_REPORT_ERROR(error_reporter, "Found %s (%d)", labels[max_index], max_score); //.equals("0")?"Straight Line":"Swiggle"

    receiveData1();
  }


}




void receiveData1() {

  if(!server.connected()){
  do
  {
    //Serial.println("- Discovering Central device...");
    BLE.scanForUuid(deviceServiceUuid);

    server = BLE.available();
  } while (!server);
  }




  while(!server.connected()){
   // Serial.println("Connecting to the server !!!!!!!");
  // digitalWrite(LEDR, LOW);
  // digitalWrite(LEDG, HIGH);
  // digitalWrite(LEDB, HIGH);
   // Serial.println(server.connect());
   delay(200);
   server.connect();
  }

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);




  if (server) {
    // Serial.println("* server device found!");
    // Serial.print("* Device MAC address: ");
    // Serial.println(server.address());
    // Serial.print("* Device name: ");
    // Serial.println(server.localName());
    // // Serial.print("* Advertised service UUID: ");
    // Serial.println(server.advertisedServiceUuid());
    // Serial.println(" ");
    BLE.stopScan();

//    controlPeripheral(server);
  }


BLECharacteristic gestureCharacteristic = server.characteristic(deviceServiceCharacteristicUuid);
server.discoverAttributes();
gestureCharacteristic.writeValue(Data1);

  // if (server.connected()) {
  //   Serial.println("* Connected to central device!");
  //   Serial.print("* Device MAC address: ");
  //   Serial.println(server.address());
  //   Serial.println(" ");
  // }
}
     
