import os
import ujson
import aicube
from libs.PipeLine import ScopedTiming
from libs.Utils import *
from media.sensor import *
from media.display import *
from media.media import *
import nncase_runtime as nn
import image
import gc
import time
from machine import Pin, FPIOA, PWM, UART

try:
    import ulab.numpy as np
except ImportError:
    import numpy as np

# 实例化Pin2为输出
fpioa = FPIOA()

# 蜂鸣器与指示灯
fpioa.set_function(48, FPIOA.GPIO48)
fpioa.set_function(17, FPIOA.GPIO17)
pin2 = Pin(48, Pin.OUT, pull=Pin.PULL_DOWN, drive=7) # 19绿灯
pin3 = Pin(17, Pin.OUT, pull=Pin.PULL_UP, drive=7)   # 21激光


# 通信 (统一使用UART1, 3M极限波特率以兼容图传与指令)
fpioa.set_function(3, FPIOA.UART1_TXD)#8
fpioa.set_function(4, FPIOA.UART1_RXD)#10
fpioa.set_function(5, FPIOA.UART2_TXD)#11
fpioa.set_function(6, FPIOA.UART2_RXD)#13
uart1 = UART(UART.UART1, baudrate=3000000, bits=UART.EIGHTBITS, parity=UART.PARITY_NONE, stop=UART.STOPBITS_ONE)
uart2 = UART(UART.UART2, baudrate=115200, bits=UART.EIGHTBITS, parity=UART.PARITY_NONE, stop=UART.STOPBITS_ONE)

# ============ 分辨率设定 ============
DISPLAY_WIDTH = ALIGN_UP(800, 16)
DISPLAY_HEIGHT = 480

# 核心缝合点：将AI通道的尺寸统一为800x480
OUT_RGB888P_WIDTH = 800
OUT_RGB888P_HEIGH = 480

root_path = "/sdcard/mp_deployment_source/"
config_path = root_path + "deploy_config.json"
deploy_conf = {}
debug_mode = 1

def two_side_pad_param(input_size, output_size):
    ratio_w = output_size[0] / input_size[0]
    ratio_h = output_size[1] / input_size[1]
    ratio = min(ratio_w, ratio_h)
    new_w = int(ratio * input_size[0])
    new_h = int(ratio * input_size[1])
    dw = (output_size[0] - new_w) / 2
    dh = (output_size[1] - new_h) / 2
    top = int(round(dh - 0.1))
    bottom = int(round(dh + 0.1))
    left = int(round(dw - 0.1))
    right = int(round(dw - 0.1))
    return top, bottom, left, right, ratio

def read_deploy_config(config_path):
    with open(config_path, 'r') as json_file:
        try:
            config = ujson.load(json_file)
        except ValueError as e:
            print("JSON 解析错误:", e)
    return config

def send_offsets(a):
    try:
        data = f"@{int(a)}\r\n".encode()
        uart2.write(data)
        print((data))
    except Exception as e:
        print("UART发送失败:", e)

def handle_category_detection(category_id):
    if category_id == 0:  # 类别A
        send_offsets(1)
    elif category_id == 1:  # 类别B
        send_offsets(2)
    elif category_id == 2:  # 类别C
        send_offsets(3)

def detection():
    global DISPLAY_HEIGHT, DISPLAY_WIDTH, tolerance
    print("det_infer start")

    detection_counter = {}
    last_detected_id = None
    REQUIRED_DETECTIONS = 15

    deploy_conf = read_deploy_config(config_path)
    kmodel_name = deploy_conf["kmodel_path"]
    labels = deploy_conf["categories"]
    confidence_threshold = deploy_conf["confidence_threshold"]
    nms_threshold = deploy_conf["nms_threshold"]
    img_size = deploy_conf["img_size"]
    num_classes = deploy_conf["num_classes"]
    color_four = get_colors(num_classes)
    nms_option = deploy_conf["nms_option"]
    model_type = deploy_conf["model_type"]
    if model_type == "AnchorBaseDet":
        anchors = deploy_conf["anchors"][0] + deploy_conf["anchors"][1] + deploy_conf["anchors"][2]

    kmodel_frame_size = img_size
    frame_size = [OUT_RGB888P_WIDTH, OUT_RGB888P_HEIGH]
    strides = [8, 16, 32]

    top, bottom, left, right, ratio = two_side_pad_param(frame_size, kmodel_frame_size)

    kpu = nn.kpu()
    kpu.load_kmodel(root_path + kmodel_name)

    ai2d = nn.ai2d()
    ai2d.set_dtype(nn.ai2d_format.NCHW_FMT, nn.ai2d_format.NCHW_FMT, np.uint8, np.uint8)
    ai2d.set_pad_param(True, [0, 0, 0, 0, top, bottom, left, right], 0, [114, 114, 114])
    ai2d.set_resize_param(True, nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
    ai2d_builder = ai2d.build([1, 3, OUT_RGB888P_HEIGH, OUT_RGB888P_WIDTH], [1, 3, kmodel_frame_size[1], kmodel_frame_size[0]])

    sensor = Sensor()
    sensor.reset()
    sensor.set_hmirror(False)
    sensor.set_vflip(False)

    sensor.set_framesize(width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT)
    sensor.set_pixformat(PIXEL_FORMAT_YUV_SEMIPLANAR_420)

    # 通道2给到AI和图传做算法处理，格式为RGB888
    sensor.set_framesize(width=OUT_RGB888P_WIDTH, height=OUT_RGB888P_HEIGH, chn=CAM_CHN_ID_2)
    sensor.set_pixformat(PIXEL_FORMAT_RGB_888_PLANAR, chn=CAM_CHN_ID_2)

    sensor_bind_info = sensor.bind_info(x=0, y=0, chn=CAM_CHN_ID_0)
    Display.bind_layer(**sensor_bind_info, layer=Display.LAYER_VIDEO1)

    # 强行开启 IDE 虚拟显示
    Display.init(Display.VIRT, width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT, fps=30, to_ide=True)

    osd_img = image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, image.ARGB8888)
    MediaManager.init()
    sensor.run()

    data = np.ones((1, 3, kmodel_frame_size[1], kmodel_frame_size[0]), dtype=np.uint8)
    ai2d_output_tensor = nn.from_numpy(data)

    class SystemState:
        IDLE = 0
        SCANNING = 1
        TARGET_LOCKED = 2

    current_state = SystemState.IDLE

    a = 0
    flag = 0
    CENTER_RECT_SIZE = 50
    pin2.value(1)#灯关闭
    data2 = None
    data1 = None

    frame_count = 0
    # ============ 新增：用于记录上一帧AI识别的框坐标和目标ID ============
    latest_box = None
    latest_target_id = 0 # 0代表未检测到，1/2/3代表不同目标

    while True:
        os.exitpoint()

        # 抓取原版 800x480 RGB图像 (共用)
        rgb888p_img = sensor.snapshot(chn=CAM_CHN_ID_2)

        if not rgb888p_img:
            print("摄像头无画面，请检查连接...")
            continue

        frame_count += 1
        if frame_count % 30 == 0:
            pin2.value(1)#灭灯
            #print(f"[{frame_count}] 画面正常抓取中...")

        # 处理串口数据接收

        if uart1.any():
            data2 = uart1.read()
            if data2 is not None:
                if b'KKKKK' in data2 and flag==1:
                     # 融合点一：图传触发逻辑 (收到 'KKKKK' 发图)
                    t_start = time.ticks_ms()
                    ai2d_input_np = rgb888p_img.to_numpy_ref()
                    img_chw = ai2d_input_np.reshape((3, 480, 800))

                    r_crop = img_chw[0][::4, ::4]
                    g_crop = img_chw[1][::4, ::4]
                    b_crop = img_chw[2][::4, ::4]

                    r16 = np.array(r_crop, dtype=np.uint16)
                    g16 = np.array(g_crop, dtype=np.uint16)
                    b16 = np.array(b_crop, dtype=np.uint16)

                    # ============ 在待发送的矩阵数组里画红框 ============
                    if latest_box is not None:
                        bx, by, bw, bh = latest_box
                        # 限制边界，防止框画出200x120的屏幕导致数组越界报错
                        bx = max(0, min(bx, 197))
                        by = max(0, min(by, 117))
                        bw = max(2, min(bw, 199 - bx))
                        bh = max(2, min(bh, 119 - by))

                        # 利用 numpy 切片，直接把矩形的四条边（线宽2像素）改成纯红色 (R=255, G=0, B=0)
                        r16[by:by+2, bx:bx+bw] = 255
                        g16[by:by+2, bx:bx+bw] = 0
                        b16[by:by+2, bx:bx+bw] = 0

                        r16[by+bh-2:by+bh, bx:bx+bw] = 255
                        g16[by+bh-2:by+bh, bx:bx+bw] = 0
                        b16[by+bh-2:by+bh, bx:bx+bw] = 0

                        r16[by:by+bh, bx:bx+2] = 255
                        g16[by:by+bh, bx:bx+2] = 0
                        b16[by:by+bh, bx:bx+2] = 0

                        r16[by:by+bh, bx+bw-2:bx+bw] = 255
                        g16[by:by+bh, bx+bw-2:bx+bw] = 0
                        b16[by:by+bh, bx+bw-2:bx+bw] = 0
                    # ====================================================

                    r_part = (r16 // 8) * 2048
                    g_part = (g16 // 4) * 32
                    b_part = b16 // 8

                    img_16bit = r_part + g_part + b_part

                    high_byte = img_16bit // 256
                    low_byte = img_16bit - (high_byte * 256)
                    img_16bit_swapped = low_byte * 256 + high_byte

                    final_array = np.array(img_16bit_swapped, dtype=np.uint16)
                    data_bytes = final_array.tobytes()

                    # 1. 先发送 48000 字节的图像数据
                    uart1.write(data_bytes)

                    # 2. ============ 新增：发送 1 个字节的目标 ID ============
                    uart1.write(bytes([latest_target_id]))
                    # =========================================================

                    fps = 1000.0 / time.ticks_diff(time.ticks_ms(), t_start)
                    del r_crop, g_crop, b_crop
                    del r16, g16, b16
                    del r_part, g_part, b_part
                    del img_16bit, high_byte, low_byte, img_16bit_swapped, final_array, data_bytes
                    gc.collect()

        # 融合点三：AI识别与画框逻辑
        if current_state == SystemState.IDLE:
            if data1 is None:
                data1 = uart2.read()  # 尝试读取数据
                if data1 is not None:
                    # 将字节数据解码为字符串
                    decoded_data2 = data1.decode('utf-8')  # 假设数据是UTF-8编码
                    print("Received:", decoded_data2)
                    # 进行状态转换
                    if decoded_data2 == "@222\r\n":
                        current_state = SystemState.SCANNING
                        flag = 1#开始
                        print("开始打开系统")

                        data1 = None
                else:
                    data1 = None
            else:
                data1 = None
        elif current_state == SystemState.SCANNING:
            if data1 is None:
                data1 = uart2.read()  # 尝试读取数据
                if data1 is not None:
                    # 将字节数据解码为字符串
                    decoded_data2 = data1.decode('utf-8')  # 假设数据是UTF-8编码
                    print("Received:", decoded_data2)
                    # 进行状态转换
                    if decoded_data2 == "@222\r\n":
                        current_state = SystemState.IDLE

                        print("结束打卡系统")

                        data1 = None
                else:
                    data1 = None
            else:
                data1 = None

            with ScopedTiming("total", debug_mode > 0):
                if rgb888p_img.format() == image.RGBP888:
                    ai2d_input = rgb888p_img.to_numpy_ref()
                    ai2d_input_tensor = nn.from_numpy(ai2d_input)
                    ai2d_builder.run(ai2d_input_tensor, ai2d_output_tensor)
                    kpu.set_input_tensor(0, ai2d_output_tensor)
                    kpu.run()

                    results = []
                    for i in range(kpu.outputs_size()):
                        out_data = kpu.get_output_tensor(i)
                        result = out_data.to_numpy()
                        result = result.reshape((result.shape[0]*result.shape[1]*result.shape[2]*result.shape[3]))
                        del out_data
                        results.append(result)

                    det_boxes = aicube.anchorbasedet_post_process(results[0], results[1], results[2], kmodel_frame_size, frame_size, strides, num_classes, confidence_threshold, nms_threshold, anchors, nms_option)
                    osd_img.clear()

                    # ============ 核心改动：每帧AI识别前先清空旧坐标和旧ID ============
                    latest_box = None
                    latest_target_id = 0 # 0代表没检测到目标

                    if det_boxes:
                        best_det = max(det_boxes, key=lambda x: x[1])
                        category_id = best_det[0]
                        confidence = best_det[1]

                        if category_id == last_detected_id:
                            detection_counter[category_id] = detection_counter.get(category_id, 0) + 1
                            if detection_counter[category_id] == REQUIRED_DETECTIONS:
                                handle_category_detection(category_id)
                                pin2.value(0)#亮灯
                                detection_counter.clear()
                        else:
                            pin2.value(1)#灭灯
                            detection_counter.clear()
                            detection_counter[category_id] = 1
                            last_detected_id = category_id

                        x1, y1, x2, y2 = best_det[2], best_det[3], best_det[4], best_det[5]
                        x = int(x1 * 800 // OUT_RGB888P_WIDTH)
                        y = int(y1 * 480 // OUT_RGB888P_HEIGH)
                        w = int((x2 - x1) * 800 // OUT_RGB888P_WIDTH)
                        h = int((y2 - y1) * 480 // OUT_RGB888P_HEIGH)

                        # 画 IDE / HDMI 上的 OSD
                        osd_img.draw_rectangle(x, y, w, h, color=color_four[best_det[0]][1:])
                        text = labels[best_det[0]] + " "
                        osd_img.draw_string_advanced(x, y-40, 32, text, color=color_four[best_det[0]][1:])

                        # ============ 核心改动：按比例换算框坐标，并赋值目标ID ============
                        latest_box = (x // 4, y // 4, w // 4, h // 4)

                        # 类别ID原本是0, 1, 2，加上1后变成1(一号目标), 2(二号目标), 3(三号目标)
                        latest_target_id = category_id + 1
                        # ====================================================================

                    Display.show_image(osd_img, 0, 0, Display.LAYER_OSD3)
                    gc.collect()

        elif current_state == SystemState.TARGET_LOCKED:
            pass

    del ai2d_output_tensor
    sensor.stop()
    Display.deinit()
    uart1.deinit()
    MediaManager.deinit()
    gc.collect()
    time.sleep(1)
    nn.shrink_memory_pool()
    print("det_infer end")
    return 0

if __name__=="__main__":
    detection()
