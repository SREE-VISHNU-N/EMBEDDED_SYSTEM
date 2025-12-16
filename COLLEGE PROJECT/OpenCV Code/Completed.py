import cv2
import serial
import time
import mediapipe as mp

# ====== SETTINGS ======
COM_PORT = 'COM4'     # Change this to your Arduino port
BAUDRATE = 9600
CAM_INDEX = 0         # Webcam index
STABLE_FRAMES = 6     # number of stable frames before confirming

# ====== SERIAL SETUP ======
try:
    arduino = serial.Serial(COM_PORT, BAUDRATE, timeout=1)
    time.sleep(2)  # wait for Arduino to reset
    print("✅ Arduino connected")
except:
    arduino = None
    print("⚠️ Arduino not found, running in demo mode")

# ====== MEDIAPIPE HANDS ======
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1)
mp_draw = mp.solutions.drawing_utils    

cap = cv2.VideoCapture(1)

# Debounce variables
last_count = -1
stable_count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    finger_count = 0

    if result.multi_hand_landmarks:
        for handLms in result.multi_hand_landmarks:
            lmList = []
            for id, lm in enumerate(handLms.landmark):
                h, w, c = frame.shape
                lmList.append((int(lm.x * w), int(lm.y * h)))

            # finger check
            tips = [4, 8, 12, 16, 20]
            if lmList[tips[0]][0] < lmList[tips[0] - 1][0]:  # thumb
                finger_count += 1
            for id in range(1, 5):
                if lmList[tips[id]][1] < lmList[tips[id] - 2][1]:
                    finger_count += 1

            mp_draw.draw_landmarks(frame, handLms, mp_hands.HAND_CONNECTIONS)

    cv2.putText(frame, f"Fingers: {finger_count}", (50, 100),
                cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3)

    # ====== DEBOUNCE LOGIC ======
    if finger_count == last_count:
        stable_count += 1
    else:
        stable_count = 0
    last_count = finger_count

    if stable_count >= STABLE_FRAMES:
        if arduino:
            if finger_count == 0:
                arduino.write(b'0')  # Servo lock
            elif finger_count == 1:
                arduino.write(b'1')  # LED ON
            elif finger_count == 2:
                arduino.write(b'2')  # LED OFF
            elif finger_count == 3:
                arduino.write(b'3')  # Motor ON
            elif finger_count == 4:
                arduino.write(b'4')  # Motor OFF
            elif finger_count == 5:
                arduino.write(b'5')  # Servo unlock
        stable_count = 0  # reset after action

    cv2.imshow("Hand Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
