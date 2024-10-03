import socket
import matplotlib.pyplot as plt
import numpy as np


# time interval
dt = 1
# global variables
t_now = 0
x = np.array([])
y = np.array([])
z = np.array([])
t = np.array([])
HOST, PORT = "192.168.137.1", 8002

def Plot():
    plt.subplot(3,1,1)
    plt.title("x-direction")
    plt.plot(t, x)
    

    plt.subplot(3,1,2)
    plt.title("y-direction")
    plt.plot(t, y)

    plt.subplot(3,1,3)
    plt.title("z-direction")
    plt.plot(t, z)

    plt.pause(0.01)
    plt.show()

    return

def handle(conn, addr):
    # self.request is the TCP socket connected to the client
    data = bytes.decode(conn.recv(1024).strip())
    print("Received from {}:".format(addr))
    print(data)

    if data == "SM":
        print("\n***Significant motion detected.***\n")
    else:    
        global x, y, z, t, t_now

        #parsing
        xyz = data.split(" ")
        x = np.append(x, int(xyz[0]))
        y = np.append(y, int(xyz[1]))
        z = np.append(z, int(xyz[2]))
        t = np.append(t, t_now*dt)
        t_now += 1
        
        Plot()
        
        conn.sendall(b":D")

plt.ion()

# Establish the TCP server socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))

s.listen(1)
conn, addr = s.accept()
print('Connected by', addr)
while True:
    try:
        handle(conn, addr)

    except socket.error:
        print("Error Occured.")
        break
conn.close()