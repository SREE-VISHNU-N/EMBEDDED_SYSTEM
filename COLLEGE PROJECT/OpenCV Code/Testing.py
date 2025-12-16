import cv2
import serial
import time
import mediapipe as mp

# ====== SETTINGS ======
COM_PORT = 'COM4'     # change to your Arduino port
BAUDRATE = 9600
CAM_INDEX = 1         # change to 0 or 1 depending on camera
STABLE_FRAMES = 6     # frames the same count must persist before sending
# ======================

# --- serial setup (safe) ---
try:
    arduino = serial.Serial(COM_PORT, BAUDRATE, timeout=1)
    time.sleep(2)  # allow Arduino reset
    print(f"✅ Serial connected on {COM_PORT}")
except Exception as e:
    arduino = None
    print(f"⚠️ Could not open serial port {COM_PORT}: {e}\nYou can still test camera/gesture without Arduino.")

# --- mediapipe setup ---
mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7, min_tracking_confidence=0.7)

# --- camera ---
cap = cv2.VideoCapture(1)
if not cap.isOpened():
    print("❌ Camera not found. Change CAM_INDEX.")
    exit()

tip_ids = [4, 8, 12, 16, 20]

print("Show gestures: 1 finger = LED ON, 2 = LED OFF, 3 = FAN ON, 4 = FAN OFF. Press 'q' to quit.")

last_count = None
stable_count = 0
last_sent = None

while True:
    ok, frame = cap.read()
    if not ok:
        print("Camera frame not available.")
        break

    frame = cv2.flip(frame, 1)   # mirror for natural gestures
    h, w = frame.shape[:2]
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    total_fingers = 0

    if result.multi_hand_landmarks:
        for hand_landmarks in result.multi_hand_landmarks:
            lm = [(int(p.x * w), int(p.y * h)) for p in hand_landmarks.landmark]

            fingers = []
            # Thumb (mirrored webcam: thumb open if x of tip > x of previous joint)
            fingers.append(1 if lm[tip_ids[0]][0] > lm[tip_ids[0] - 1][0] else 0)

            # Other fingers: tip is above pip-joint?
            for i in range(1, 5):
                fingers.append(1 if lm[tip_ids[i]][1] < lm[tip_ids[i] - 2][1] else 0)

            total_fingers = sum(fingers)

            mp_draw.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
    else:
        # no hand detected
        total_fingers = 0

    # show
    cv2.putText(frame, f'Fingers: {total_fingers}', (40, 100),
                cv2.FONT_HERSHEY_SIMPLEX, 1.6, (0, 255, 0), 3)
    cv2.imshow("Finger Counter (press q to quit)", frame)

    # stability / debounce
    if total_fingers == last_count:
        stable_count += 1
    else:
        stable_count = 1
        last_count = total_fingers

    if stable_count >= STABLE_FRAMES and total_fingers != last_sent:
    # map counts 1..5 to commands
     if total_fingers in (1, 2, 3, 4, 5):
        cmd = str(total_fingers).encode()   # b'1', b'2', ... b'5'
        if arduino:
            try:
                arduino.write(cmd)
            except Exception as e:
                print("Serial write error:", e)
        print(f"→ Sent command: {total_fingers}")
        last_sent = total_fingers
    else:
        # counts 0 ignored
        print(f"Detected {total_fingers} (ignored)")


    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
if arduino:
    arduino.close()
