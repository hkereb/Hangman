import sys
from ui_skeleton import Ui_MainWindow
from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import QObject, Signal, QThread, QRegularExpression
from PySide6.QtGui import QRegularExpressionValidator

class MainApp(QMainWindow):
    nick_submitted = Signal(str)  # Sygnał do przesyłania nicku

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # ustawienie strony startowej
        self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)

        # walidator adresu IP
        # ip_regex = QRegularExpression(r'^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$')
        # validator = QRegularExpressionValidator(ip_regex)
        # self.ui.create_IP_field.setValidator(validator)

        # tu logika
        self.ui.nick_submit_btn.clicked.connect(self.submit_nick)

    def submit_nick(self):
        nick = self.ui.nick_field.text()
        if nick:
            self.nick_submitted.emit(nick)
            print(f"Nick submitted: {nick}")

    def handle_server_response(self, message):
        if message.startswith("01"):
            result = message[2:]
            if result == "1":  # nick zaakceptowany
                self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)
            elif result == "0":  # nick odrzucony
                self.ui.check_label.setText("\U0000274C")
                self.ui.check_label.setStyleSheet("color: red;")