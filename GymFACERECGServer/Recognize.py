import os
import cv2
import pandas as pd
import paho.mqtt.client as mqtt
import time

# MQTT parameters
broker = "test.mosquitto.org"
port = 1883
topic = "real_unique_topic"

# Callback function for logging
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)

# Create a client instance with a specific MQTT version
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)  # MQTT v3.1.1

# Assign the callback function
client.on_connect = on_connect

# Connect to the broker
client.connect(broker, port)

# Initialize variables to store detected student name
detected_student = None

def recognize_attendance():
    recognizer = cv2.face.LBPHFaceRecognizer_create()  # cv2.createLBPHFaceRecognizer()
    recognizer.read("./TrainingImageLabel/Trainner.yml")
    harcascadePath = "haarcascade_frontalface_default.xml"
    faceCascade = cv2.CascadeClassifier(harcascadePath)
    df = pd.read_csv("StudentDetails"+os.sep+"StudentDetails.csv")
    font = cv2.FONT_HERSHEY_SIMPLEX

    # Initialize and start realtime video capture
    cam = cv2.VideoCapture(0)
    cam.set(3, 640)  # set video width
    cam.set(4, 480)  # set video height
    # Define min window size to be recognized as a face
    minW = 0.1 * cam.get(3)
    minH = 0.1 * cam.get(4)

    while True:
        _,im = cam.read()
        gray = cv2.cvtColor(im, cv2.COLOR_BGR2GRAY)
        faces = faceCascade.detectMultiScale(gray, 1.2, 5, minSize=(int(minW), int(minH)), flags=cv2.CASCADE_SCALE_IMAGE)
        for(x, y, w, h) in faces:
            cv2.rectangle(im, (x, y), (x+w, y+h), (10, 159, 255), 2)
            Id, conf = recognizer.predict(gray[y:y+h, x:x+w])

            if conf < 100:
                aa = df.loc[df['Id'] == Id]['Name'].values
                detected_student = aa[0]
                print("Detected: {}, Confidence: {:.2f}".format(detected_student, conf))
                # Publish the detected student name to MQTT topic
                if(conf >=40):
                    while(True):
                        client.publish(topic, detected_student)
                        time.sleep(0.2)
            else:
                detected_student = 'Unknown'
                print("Detected: Unknown, Confidence: {:.2f}".format(100 - conf))

            detected_student = str(detected_student)
            cv2.putText(im, str(detected_student), (x+5, y-5), font, 1, (255, 255, 255), 2)
            cv2.putText(im, "Confidence: {:.2f}".format(100 - conf), (x + 5, y + h - 5), font, 1, (0, 255, 0), 1)

        cv2.imshow('Attendance', im)
        if cv2.waitKey(1) == ord('q'):
            break

    client.loop_forever()

    cam.release()
    cv2.destroyAllWindows()

# Call the function to start face recognition

# Start the MQTT loop

