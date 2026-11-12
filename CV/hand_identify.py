"""
hand_identify.py
基于 MediaPipe HandLandmarker（新版 tasks API）的手势识别。

识别三种手势，并在手势稳定变化时通过蓝牙（SPP 透传串口）发送 1 个字节指令给蓝牙从机（STM32 机器狗）：
    0x21 = 张开五指
    0x22 = 握拳
    0x23 = 两根手指（V）
从机端 USART3 循环 DMA 接收后由主循环执行动作（见 源码/MyDriver/RobotControl.c 的指令分发）。

用法：
    python hand_identify.py [COM口]    例如：python hand_identify.py COM7
蓝牙串口号可在设备管理器 "端口(COM和LPT)" 中查看配对后的 "标准串行蓝牙链路" 端口。
"""

import math
import os
import sys
import time

import cv2
from mediapipe import Image, ImageFormat
from mediapipe.tasks import python as mp_python
from mediapipe.tasks.python import vision

# ============================ 配置区 ============================
BT_COM_PORT = "COM6"    # 蓝牙从机在电脑上对应的串口号（命令行参数可覆盖）；设为 None 则只识别不发送
BT_BAUD = 115200        # 与从机蓝牙模块串口波特率一致（STM32 USART3 为 115200）

DEBOUNCE_FRAMES = 5     # 手势连续保持多少帧才确认（防抖，避免抖动导致误发送）
FINGER_ANGLE_THRESHOLD = 40   # 指骨夹角小于该角度(°)判定为伸直
THUMB_X_MARGIN = 0.03   # 拇指指尖与指间关节的横向距离阈值（归一化坐标）

# 手势编码（发给从机的指令字节），与从机 RobotControl.c 的 case 对应
CMD_OPEN = 0x21   # 张开五指
CMD_FIST = 0x22   # 握拳
CMD_TWO  = 0x23   # 两根手指

GESTURE_NONE = 0
GESTURE_OPEN = 1
GESTURE_FIST = 2
GESTURE_TWO  = 3

GESTURE_NAMES = {
    GESTURE_NONE: "其他",
    GESTURE_OPEN: "张开五指 (0x21)",
    GESTURE_FIST: "握拳 (0x22)",
    GESTURE_TWO:  "两根手指 (0x23)",
}
GESTURE_CMDS = {
    GESTURE_OPEN: CMD_OPEN,
    GESTURE_FIST: CMD_FIST,
    GESTURE_TWO:  CMD_TWO,
}
# ================================================================

# 1. 初始化 MediaPipe HandLandmarker（新版 tasks API，需要 .task 模型文件）
BaseOptions = mp_python.BaseOptions
HandLandmarker = vision.HandLandmarker
HandLandmarkerOptions = vision.HandLandmarkerOptions
VisionRunningMode = vision.RunningMode

# 模型文件放在脚本同目录，用绝对路径避免运行目录不同导致找不到
MODEL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "hand_landmarker.task")

options = HandLandmarkerOptions(
    base_options=BaseOptions(model_asset_path=MODEL_PATH),
    running_mode=VisionRunningMode.VIDEO,   # 视频流模式
    num_hands=2,
    min_hand_detection_confidence=0.7,
    min_hand_presence_confidence=0.5,
    min_tracking_confidence=0.5,
)
landmarker = HandLandmarker.create_from_options(options)

# 新版没有 drawing_utils，手动定义手部骨架连接关系（21 个关键点）
HAND_CONNECTIONS = [
    (0, 1), (1, 2), (2, 3), (3, 4),            # 拇指
    (0, 5), (5, 6), (6, 7), (7, 8),            # 食指
    (5, 9), (9, 10), (10, 11), (11, 12),       # 中指
    (9, 13), (13, 14), (14, 15), (15, 16),     # 无名指
    (13, 17), (17, 18), (18, 19), (19, 20),    # 小指
    (0, 17),                                   # 掌心
]


# 2. 手势判断
def _is_finger_extended(lm, mcp_idx, pip_idx, tip_idx, angle_threshold=FINGER_ANGLE_THRESHOLD):
    """判断一根手指是否伸直：PIP 关节处近节指骨方向(pip-mcp)与远端方向(tip-pip)的夹角小于阈值即伸直。

    伸直时两段指骨同向（夹角≈0°）；弯曲 90° 时夹角≈90°；完全蜷起时方向相反（夹角≈180°）。
    基于角度而非 Y 坐标比较，手部倾斜时也适用。
    """
    a = (lm[pip_idx].x - lm[mcp_idx].x, lm[pip_idx].y - lm[mcp_idx].y)
    b = (lm[tip_idx].x - lm[pip_idx].x, lm[tip_idx].y - lm[pip_idx].y)
    na, nb = math.hypot(*a), math.hypot(*b)
    if na == 0 or nb == 0:
        return False
    cos_val = max(-1.0, min(1.0, (a[0] * b[0] + a[1] * b[1]) / (na * nb)))
    return math.degrees(math.acos(cos_val)) < angle_threshold


def _is_thumb_open(lm, is_right_hand):
    """拇指伸直判断：比较拇指指尖(4)与指间关节(3)的 X 坐标。

    画面经过了 cv2.flip 水平镜像，左右手的判断方向相反：
    拇指张开时指尖比关节更远离掌心（在镜像画面中向外侧偏移）。
    """
    if is_right_hand:
        return lm[4].x < lm[3].x - THUMB_X_MARGIN
    return lm[4].x > lm[3].x + THUMB_X_MARGIN


def classify_gesture(hand_landmarks, handedness):
    """对单只手分类，返回 GESTURE_* 之一。"""
    lm = hand_landmarks
    is_right_hand = handedness[0].category_name == 'Right'

    thumb = _is_thumb_open(lm, is_right_hand)
    index  = _is_finger_extended(lm, 5, 6, 8)    # 食指
    middle = _is_finger_extended(lm, 9, 10, 12)  # 中指
    ring   = _is_finger_extended(lm, 13, 14, 16) # 无名指
    pinky  = _is_finger_extended(lm, 17, 18, 20) # 小指

    if thumb and index and middle and ring and pinky:
        return GESTURE_OPEN          # 5 根手指全部伸直
    if index and middle and not ring and not pinky:
        return GESTURE_TWO           # 食指+中指伸直、无名指+小指蜷起（拇指状态不影响）
    if not (index or middle or ring or pinky):
        return GESTURE_FIST          # 4 指全部蜷起（拇指卷在拳上或伸出都算握拳）
    return GESTURE_NONE


# 3. 蓝牙发送
class BluetoothSender:
    """通过蓝牙 SPP 虚拟串口向从机发送指令字节。打开失败时降级为仅本地识别，不影响程序运行。"""

    def __init__(self, port, baud):
        self.ser = None
        if not port:
            print("[蓝牙] 未配置串口号，跳过蓝牙发送（仅本地识别）")
            return
        try:
            import serial
        except ImportError:
            print("[蓝牙] 未安装 pyserial，跳过蓝牙发送（pip install pyserial）")
            return
        try:
            self.ser = serial.Serial(port, baud, timeout=0.2)
            print(f"[蓝牙] 已连接从机：{port} @ {baud}")
        except Exception as e:
            print(f"[蓝牙] 打开 {port} 失败：{e}")
            print("[蓝牙] 仅本地识别。请检查：1) 从机已上电并完成配对 2) 串口号正确 "
                  "3) 可用命令行参数指定串口：python hand_identify.py COM7")

    def send(self, code):
        if self.ser is None:
            return
        try:
            self.ser.write(bytes([code]))
            print(f"[蓝牙] 发送指令 0x{code:02X}")
        except Exception as e:
            print(f"[蓝牙] 发送失败：{e}")

    def close(self):
        if self.ser is not None:
            self.ser.close()


def draw_hand(image, hand_landmarks, handedness, gesture):
    """在画面上绘制骨架、关键点与手势文字。"""
    h, w, _ = image.shape
    for a, b in HAND_CONNECTIONS:
        ax, ay = int(hand_landmarks[a].x * w), int(hand_landmarks[a].y * h)
        bx, by = int(hand_landmarks[b].x * w), int(hand_landmarks[b].y * h)
        cv2.line(image, (ax, ay), (bx, by), (0, 255, 0), 2)
    for lm in hand_landmarks:
        cx, cy = int(lm.x * w), int(lm.y * h)
        cv2.circle(image, (cx, cy), 4, (0, 255, 0), -1)

    label = f"{handedness[0].category_name}: {GESTURE_NAMES[gesture]}"
    tx, ty = int(hand_landmarks[0].x * w), int(hand_landmarks[0].y * h) + 30
    cv2.putText(image, label, (tx, ty), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)


def main():
    # 命令行参数可覆盖蓝牙串口号：python hand_identify.py COM7
    port = sys.argv[1] if len(sys.argv) > 1 else BT_COM_PORT
    bt = BluetoothSender(port, BT_BAUD)

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("无法打开摄像头")
        bt.close()
        landmarker.close()
        return

    start_time = time.monotonic()
    current_gesture = GESTURE_NONE   # 当前帧分类结果
    stable_counter = 0               # 连续同手势帧数（防抖）
    last_sent = GESTURE_NONE         # 上次已发送的手势（沿变化发送，避免重复触发从机）

    try:
        while cap.isOpened():
            success, image = cap.read()
            if not success:
                print("无法获取摄像头画面")
                break

            # 水平翻转呈现镜像效果，并将 BGR 转换为 RGB 供 MediaPipe 处理
            image = cv2.flip(image, 1)
            rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            mp_image = Image(image_format=ImageFormat.SRGB, data=rgb_image)

            # 视频模式要求严格递增的时间戳（毫秒），这里取真实流逝时间
            timestamp_ms = int((time.monotonic() - start_time) * 1000)
            results = landmarker.detect_for_video(mp_image, timestamp_ms)

            if results.hand_landmarks:
                # 取第一只手（画面中先检测到的手）作为指令来源，其余手仅绘制
                for i, (hand_landmarks, handedness) in enumerate(
                        zip(results.hand_landmarks, results.handedness)):
                    gesture = classify_gesture(hand_landmarks, handedness)
                    draw_hand(image, hand_landmarks, handedness, gesture)

                    if i == 0:
                        if gesture == current_gesture:
                            stable_counter += 1
                        else:
                            current_gesture = gesture
                            stable_counter = 1
            else:
                # 没有检测到手：重置防抖与发送记录，手重新出现时同手势可再次触发
                current_gesture = GESTURE_NONE
                stable_counter = 0
                last_sent = GESTURE_NONE

            # 手势连续稳定 DEBOUNCE_FRAMES 帧且与上次发送的不同 → 通过蓝牙发送指令
            if (stable_counter >= DEBOUNCE_FRAMES
                    and current_gesture in GESTURE_CMDS
                    and current_gesture != last_sent):
                bt.send(GESTURE_CMDS[current_gesture])
                last_sent = current_gesture

            cv2.imshow('Hand Gesture Recognition', image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()
        landmarker.close()
        bt.close()


if __name__ == "__main__":
    main()
