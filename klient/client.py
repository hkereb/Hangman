from PySide6.QtCore import QObject, Signal, QThread
import socket
import threading

class NetworkClient(QThread):
    message_received = Signal(str)
    error_occurred = Signal(str)

    def __init__(self, server_ip, server_port, time_to_wait, parent=None):
        super(NetworkClient, self).__init__(parent)
        self.time_wait = time_to_wait
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

    def send_to_server(self, command_number, body):
        if self.socket:
            try:
                command_number_bytes = command_number.encode('utf-8')
                body_bytes = body.encode('utf-8')

                message = command_number_bytes + b"\\" + body_bytes
                self.socket.sendall(message)
            except Exception as e:
                self.error_occurred.emit(str(e))
