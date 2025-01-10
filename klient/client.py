from PySide6.QtCore import QObject, Signal, QThread
import socket
import threading

class NetworkClient(QThread):
    message_received = Signal(str)
    error_occurred = Signal(str)
    sig_cant_connect = Signal(str)

    def __init__(self, server_ip, server_port, parent=None):
        super(NetworkClient, self).__init__(parent)
        self.time_wait = 2
        self.server_ip = server_ip
        self.server_port = server_port
        self.socket = None
        self.isConnected = False

    def run(self):
        self.connect_to_server()

    def connect_to_server(self):
        print(f"I will try to connect to: {self.server_ip}")
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(self.time_wait)
            self.socket.connect((self.server_ip, self.server_port))
            # todo być może zmienić
            self.socket.settimeout(None)
            self.isConnected = True
            threading.Thread(target=self.listen_to_server, daemon=True).start()
        except Exception as e:
            print(f"FAILED to connect to: {self.server_ip}")
            self.sig_cant_connect.emit(str(e))

    def listen_to_server(self):
        try:
            buffer = ""
            while True:
                data = self.socket.recv(1024).decode('utf-8')
                if data:
                    buffer += data # dodanie nowych danych do bufora

                    # rozdzielenie wiadomości na podstawie znaku końca wiadomości '\n'
                    messages = buffer.split("\n")

                    for message in messages[:-1]:  # obsługa pełnych wiadomości
                        if message:
                            self.message_received.emit(message)

                    # częściowa wiadomość
                    buffer = messages[-1]
        except Exception as e:
            self.error_occurred.emit(str(e))

    def send_to_server(self, command_number, body):
        if self.socket:
            try:
                command_number_bytes = command_number.encode('utf-8')
                body_bytes = body.encode('utf-8')

                message = command_number_bytes + b"\\" + body_bytes + b"\n"
                self.socket.sendall(message)
            except Exception as e:
                self.error_occurred.emit(str(e))