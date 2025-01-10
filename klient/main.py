import sys
import client
from client import NetworkClient
###################################
from gui import MainApp
from PySide6 import QtWidgets as qtw
from PySide6 import QtCore as qtc

if __name__ == "__main__":
    #SERVER_IP = "192.168.100.8"
    SERVER_PORT = 1111

    client = NetworkClient(None, None, None)

    app = qtw.QApplication([])

    window = MainApp()
    window.show()

    ############# signals ##############
    def try_connect(ip):
        global client
        if not client.isConnected:
            try:
                client = NetworkClient(ip, SERVER_PORT, time_to_wait=3)
                client.connect_to_server()
                client.message_received.connect(window.handle_server_response)

            except Exception as e:
                # qtw.QMessageBox.warning(window, "Info", f"Can't connect to server")
                pass
        else:
            window.submit_nick()


    def connection_error(error_message):
        print("cant connnect")
        qtw.QMessageBox.critical(window, "Connection Error", f"Cannot connect to the server:\n{error_message}")
        client.message_received.disconnect(window.handle_server_response)
        window.sig_submit_nick.disconnect()

    def unlock_other_signals():
        window.sig_submit_nick.connect(lambda nick: client.send_to_server("01", f"{nick}"))
        window.sig_create_room.connect(lambda name, password, level, rounds, time_sec: client.send_to_server("02", f"name:{name},password:{password},difficulty:{level},rounds:{rounds},time:{time_sec}"))
        window.sig_join_room.connect(lambda name, password: client.send_to_server("03", f"name:{name},password:{password}"))
        window.sig_rooms_list.connect(lambda message: client.send_to_server("70", ""))
        window.sig_players_list.connect(lambda message: client.send_to_server("71", ""))
        window.sig_start.connect(lambda message: client.send_to_server("73", ""))
        window.sig_submit_letter.connect(lambda letter: client.send_to_server("06", f"{letter}"))
        window.sig_time_ran_out.connect(lambda message: client.send_to_server("80", ""))

        window.submit_nick()


    client.sig_cant_connect.connect(connection_error)
    window.sig_connect.connect(try_connect)
    window.sig_has_connected.connect(unlock_other_signals)
    ####################################

    # event loop
    sys.exit(app.exec())