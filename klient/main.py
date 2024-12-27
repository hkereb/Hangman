import socket
import PySide6.QtCore
import sys
import random
###################################
from gui import MainWindow
from PySide6 import QtWidgets as qtw
from PySide6 import QtCore as qtc


def connect_to_server(ip, port):
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        client_socket.connect((ip, port))
        print(f"Połączono z serwerem {ip}:{port}")

        # Wysyłanie wiadomości do serwera
        # message = "Hello, server!"
        # client_socket.sendall(message.encode('utf-8'))
        # print(f"Wysłano wiadomość: {message}")

        # Odbieranie odpowiedzi od serwera
        # response = client_socket.recv(1024)
        # print(f"Otrzymano odpowiedź od serwera: {response.decode('utf-8')}")

    except Exception as e:
        print(f"Błąd: {e}")

    # client_socket.close()
    # print("Zamknięto połączenie.")

if __name__ == "__main__":
    server_ip = "127.0.0.1"
    server_port = 1111
    #connect_to_server(server_ip, server_port)

    app = qtw.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

