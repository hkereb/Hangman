import sys
import client
from client import NetworkClient
###################################
from gui import MainApp
from PySide6 import QtWidgets as qtw
from PySide6 import QtCore as qtc

if __name__ == "__main__":
    SERVER_IP = "127.0.0.1"
    SERVER_PORT = 1111

    client = NetworkClient(SERVER_IP, SERVER_PORT, time_to_wait=5)
    client.connect_to_server()

    app = qtw.QApplication([])

    window = MainApp()
    window.show()

    ############# signals ##############
    window.nick_submitted.connect(lambda nick: client.send_to_server("01", f"{nick}"))
    client.message_received.connect(window.handle_server_response)
    window.update_rooms.connect(lambda message: client.send_to_server("10", ""))
    ####################################

    # event loop
    sys.exit(app.exec())

