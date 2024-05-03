
# Gym Assistant System
<!-- If applicable, add a logo or icon for visual appeal -->

<!-- Add a license badge, replace link_to_license with your license link -->

## Description
The Gym Assistant System is a comprehensive solution designed to enhance gym user experience and workout efficiency. It consists of three main components:

1. **GymFACERECGServer**: Utilizes OpenCV LBPHFaceRecognizer to create user profiles based on facial recognition. It stores user details, including name and weight, facilitating personalized gym experiences.
2. **WioTerminalClient**: Acts as the middleware component, facilitating communication between the GymFACERECGServer and ArduinoBLE Sense 33. It receives user data from the server and interacts with the user through the Wio Terminal interface.
3. **ArduinoBLE Sense 33**: Runs a TensorFlow Lite model to analyze IMU data and predict push-up repetitions. It communicates with the Wio Terminal to display workout statistics, such as the number of correct and incorrect push-ups, and calculates calories burned based on user weight.

## Table of Contents
- Installation
- Usage
- Features
- Contributing
- License

## Installation

### Prerequisites
- OpenCV
- MQTT Server
- TensorFlow Lite
- Arduino IDE

### Installation Steps
Clone the repository:
```bash
$ git clone https://github.com/your_username/your_project.git
$ cd your_project
```
Install dependencies:
```bash
$ pip install -r requirements.txt
```
<!-- Add installation steps for ArduinoBLE Sense 33 -->
Set up and configure MQTT server.
Upload the ArduinoBLE Sense 33 code to the device using the Arduino IDE.

## Usage
1. Run the GymFACERECGServer component to start the facial recognition server.
2. Ensure the WioTerminalClient is connected to the MQTT server.
3. Perform facial recognition login using the GymFACERECGServer.
4. Upon successful login, the Wio Terminal will display a welcome message and prompt the user to start working out.
5. Start exercising, and the ArduinoBLE Sense 33 will track push-up repetitions and send data to the Wio Terminal.
6. The Wio Terminal will display workout statistics, including the number of correct and incorrect push-ups and calories burned.

## Features
- Facial recognition-based user authentication and profile creation.
- Real-time communication between components using MQTT protocol.
- TensorFlow Lite model for push-up detection using IMU data.
- Personalized workout statistics and calorie calculation.

## Contributing
Contributions are welcome! Please follow these steps to contribute:
1. Fork the project.
2. Create your feature branch (git checkout -b feature/YourFeature).
3. Commit your changes (git commit -am 'Add some feature').
4. Push to the branch (git push origin feature/YourFeature).
5. Create a new Pull Request.

## License
This project is licensed under the MIT License - see the LICENSE.md file for details.
