import sys
import socket
import struct
from appJar import gui

#app object
app = None

#socket used to communicate
g_sock = None
sock_busy = 0

#tracker mode 0 - manual, 1 - auto
mode = -1

#tracker configs
configs = {}

#logs
logs = []

def log(str):
    global logs
    logs += str
    app.setTextArea('txt_area_log', str + '\n')

def connect(ip, port):
    
    global g_sock

    try:
        g_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        g_sock.settimeout(30) #30 second timeout
        g_sock.connect((ip, port))
        app.queueFunction(notify_app, 1)
        log('Connected')

    except Exception as e:
        log('Connection error: ' + str(e))
        g_sock.close()
        g_sock = None
        
def disconnect():
    
    global g_sock 
    while sock_busy:
        pass

    try:

        g_sock.shutdown(socket.SHUT_RDWR)
        g_sock.close()
        g_sock = None
        app.queueFunction(notify_app, 0)
        log('Disconnected')
    except Exception as e:
        log('Disconnect error: ' + str(e))
    


def on_connection(btn_name):

    #if not connected
    if not g_sock:
        ip = app.getEntry('ent_ip')
        port = int(app.getEntry('ent_port'))
        log('Connecting: \'' + ip  + '\': ' + str(port))
        app.thread(connect, ip, port)   
        return

    app.thread(disconnect)

def on_up(btn_name):
    packet = struct.pack('ihh', 1, 0, int(app.getEntry('ent_stride_deg')))
    app.thread(send_packet, packet)

def on_down(btn_name):

    packet = struct.pack('ihh', 1, 0, int(app.getEntry('ent_stride_deg')) * -1)
    app.thread(send_packet, packet)

def on_left(btn_name):

    packet = struct.pack('ihh', 1, int(app.getEntry('ent_stride_deg')) * -1, 0)
    app.thread(send_packet, packet)

def on_right(btn_name):

    packet = struct.pack('ihh', 1, int(app.getEntry('ent_stride_deg')), 0)
    app.thread(send_packet, packet)

def on_manual(btn_name):
    if not g_sock:
        app.warningBox('Error', 'Not connected')
        return
    
    app.showSubWindow('manual_control')
    global mode
    if not mode == 0:
        packet = struct.pack('ii', 0, 0)
        app.thread(send_packet, packet)
        mode = 0
        log('Switched to manual')

def on_config(btn_name): 
    if not g_sock:
        app.warningBox('Error', 'Not connected')
        return
    app.showSubWindow('win_config')
    

def on_update(btn_name):
    #sanity check first
    blur = int(app.getEntry('ent_blur'))

    if blur % 2 == 0:
        app.warningBox('Error', 'Blue size must be odd number')
        blur += 1
        app.setEntry('ent_blur', str(blur))

    if blur < 0:
        blur = 0
        app.setEntry('ent_blur', str(blur))

    thresh = int(app.getEntry('ent_thresh'))

    if thresh < 0:
        thresh = 0
        app.setEntry('ent_thresh', str(tresh))

    packet = struct.pack('ihh', 3, blur, thresh)
    app.thread(send_packet, packet)

def on_auto(btn_name): 
    if not g_sock:
        app.warningBox('Error', 'Not connected')
        return

    global mode

    app.hideSubWindow('manual_control')
    
    if not mode == 1:
        packet = struct.pack('ii', 0, 1)
        app.thread(send_packet, packet)
        mode = 1
        log('Switched to Auto')

def send_packet(packet):
    try:
        g_sock.send(packet)
        log('Sent: ' + str(len(packet)) + ' bytes of data')
    except Exception as e:
        log('Sending error: ' + str(e))




def notify_app(signal):
    #disconnected
    if signal == 0:
        app.setButton('btn_connect', 'Connect')
    #just connected
    elif signal == 1:
        app.setButton('btn_connect', 'Disconnect')
        
        #TODO: sync up



#main set up

app = gui('Client')
    

#set up connections widgets

app.addEntry('ent_ip', 0, 0)
app.addNumericEntry('ent_port', 0, 1)
app.setEntryDefault('ent_ip', 'IPv4')
app.setEntryDefault('ent_port', 14000)
#app.setEntryMaxLength('ent_ip', 15)
app.setEntryMaxLength('ent_port', 5)
    
app.addButton('btn_connect', on_connection, 0, 2)
app.setButton('btn_connect', 'Connect')


#stup up general buttons
app.addButton('btn_manual', on_manual, 1, 0)
app.setButton('btn_manual', 'Man')
app.addButton('btn_config', on_config, 1, 1)
app.setButton('btn_config', 'Configuration')
app.addButton('btn_auto', on_auto, 1, 2)
app.setButton('btn_auto', 'Auto')

#add log space
app.addScrolledTextArea('txt_area_log', 2, 0, 3)

#define manual control window
app.startSubWindow('manual_control', 'Manual Control')

app.addButton('btn_up', on_up, 0, 1)
app.addButton('btn_left', on_left, 1,0)
app.addButton('btn_right', on_right, 1,2)
app.addButton('btn_down', on_down, 2,1)
app.setButton('btn_up', '^')
app.setButton('btn_left', '<')
app.setButton('btn_right', '>')
app.setButton('btn_down', 'V')

app.addNumericEntry('ent_stride_deg', 1, 1)
app.setEntryDefault('ent_stride_deg', 10)
app.setEntryMaxLength('ent_stride_deg', 3)

app.stopSubWindow()

app.hideSubWindow('manual_control')

#define config window
app.startSubWindow('win_config', 'Configurations')

app.addLabel('lb_blur', 'Blur size', 0, 0)
app.addNumericEntry('ent_blur', 0, 1)
app.addLabel('lb_thresh', 'Motion Threshold', 1, 0)
app.addNumericEntry('ent_thresh', 1, 1)
app.addLabel('lb_dist', 'Distance', 2, 0)
app.addNumericEntry('ent_dist', 2, 1)

#app.addLabel('lb_blur', 'Blur size', 0, 0)
#app.addNumericEntry('ent_blur', 0, 1)


#app.addLabel('lb_blur', 'Blur size', 0, 0)
#app.addNumericEntry('ent_blur', 0, 1)

app.addButton('btn_update', on_update, 3, 0)
app.setButton('btn_update', 'Update')

app.stopSubWindow()

app.hideSubWindow('win_config')

app.go()
    
    
"""    
    packet = None

    while True:
        type_num = input('type: ')
        type_num = int(type_num)

        if type_num == 1 or type_num == 3:
            a = input('first: ')
            a = int(a)

            b = input('second: ')
            b = int(b)

            packet = struct.pack('ihh', type_num, a, b)

            print(len(packet))
        else:
            data = input('data: ')
            data = int(data)
            packet = struct.pack('ii', type_num, data)
        
        sock.send(packet)

"""
