import sys
import client
from client import NetworkClient
###################################
from gui import MainApp
from PySide6 import QtWidgets as qtw
from PySide6 import QtCore as qtc

if __name__ == "__main__":
    SERVER_IP = "192.168.100.8"
    SERVER_PORT = 1111

    client = NetworkClient(SERVER_IP, SERVER_PORT, time_to_wait=5)
    client.connect_to_server()

    app = qtw.QApplication([])

    window = MainApp()
    window.show()

    ############# signals ##############
    client.message_received.connect(window.handle_server_response)
    window.sig_submit_nick.connect(lambda nick: client.send_to_server("01", f"{nick}"))
    window.sig_create_room.connect(lambda name: client.send_to_server("02", f"{name}"))
    window.sig_join_room.connect(lambda name: client.send_to_server("03", f"{name}"))
    window.sig_rooms_list.connect(lambda message: client.send_to_server("10", ""))
    window.sig_players_list.connect(lambda message: client.send_to_server("11", ""))
    window.sig_start.connect(lambda message: client.send_to_server("12", ""))
    ####################################

    # event loop
    sys.exit(app.exec())

