#电脑连接测试*******************************************************************************************************************#/
import time
import network
from umqttsimple import MQTTClient
from machine import UART, Pin
uart2 = UART(2, baudrate=115200, tx=Pin(17), rx=Pin(16))

temp=0


def uart2_take():
    if uart2.any():
       cmd = uart2.readline().decode('utf-8').strip()
       if cmd.upper() == '555':
           print(f'9')
           #uart2.write("LED 已开启\n")
       elif cmd.upper() == 'OFF':
           print(f'8')
           #uart2.write("LED 已关闭\n")

def do_connect():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect('xxx', 'xxxxxxxxxx')
        i = 1
        while not wlan.isconnected():
            print("正在链接...{}".format(i))
            i += 1
            time.sleep(1)
    print('network config:', wlan.ifconfig())
def send_data():
    c.publish("aa","%s" % temp)

def sub_cb(topic, msg): # 回调函数，收到服务器消息后会调用这个函数
    print(topic, msg)
    if topic.decode("utf-8") == "ledctl" and msg.decode("utf-8") == "on":
        # 发送数据
        uart2.write('g\n')
        print(f'5')
    elif topic.decode("utf-8") == "ledctl" and msg.decode("utf-8") == "off":
        print(f'6')
        # 发送数据
        uart2.write('h\n')

# 1. 联网
do_connect()
# 2. 创建mqt
c = MQTTClient("umqtt_client", "159.75.161.237")  # 建立一个MQTT客户端
c.set_callback(sub_cb)  # 设置回调函数
c.connect()  # 建立连接
c.subscribe(b"ledctl")  # 监控ledctl这个通道，接收控制命令
i = 0
while True:
    c.check_msg()
    #uart2_take()
    time.sleep(0.5)
    send_data()
    i+=1
    if temp >=50:
        temp =0
    temp+=0.55
    if i>=30:
        i=0
        c.ping()



#联网看时间测试*******************************************************************************************************************#/
# import urequests
# import time
# from machine import UART
# from machine import RTC
# import ntptime
# # 定义 UART 控制对象
# uart = UART(2, 115200)
# 
# # 从 NTP 服务器获取时间
# ntptime.settime()
# rtc = RTC()
# date_time = rtc.datetime()
# print(f'显示北京时间：{date_time[0]}-{date_time[1]}-{date_time[2]} {date_time[4] + 8}:{date_time[5]}:{date_time[6]}\n')
# 


# # 定义请求参数字典
# request_params = {'city': '上海', 'key': 'ace02910c91120510ce8451a05d232ea'}
# response = urequests.post(f'http://apis.juhe.cn/simpleWeather/query?key={request_params ["key"]}&city={request_params ["city"]}')
# #curl -k -i "http://apis.juhe.cn/simpleWeather/query?key=key&city=苏州"
# print(response.text)
# # print(type(response.json()))
# 
# # 获取数据
# realtime = response.json()['result']['realtime']
# temp = realtime['temperature']
# info = realtime['info']
# aqi = realtime['aqi']
# 
# print(f'温度：{temp}°C\n天气：{info}\n空气指数：{aqi}')
