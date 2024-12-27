from PySide6.QtCore import QObject, Signal
import socket
import threading

class NetworkClient(QObject):
    message_received = Signal(str)
    error_occurred = Signal(str)

    def __init__(self, server_ip, server_port):
        super().__init__()
        self.server_ip = server_ip
        self.server_port = server_port
        self.socket = None

    def connect_to_server(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.server_ip, self.server_port))
            threading.Thread(target=self.listen_to_server, daemon=True).start()
        except Exception as e:
            self.error_occurred.emit(str(e))

    def listen_to_server(self):
        try:
            while True:
                message = self.socket.recv(1024).decode('utf-8')
                if message:
                    self.message_received.emit(message)
        except Exception as e:
            self.error_occurred.emit(str(e))

    def send_message(self, message):
        if self.socket:
            self.socket.sendall(message.encode('utf-8'))