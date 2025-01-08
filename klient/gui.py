import sys
from ui_skeleton import Ui_MainWindow
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox
from PySide6.QtCore import QObject, Signal, QThread, QRegularExpression, QTime
from PySide6.QtGui import QRegularExpressionValidator

def substr_msg(msg):
    return msg[3:]

class MainApp(QMainWindow):
    sig_submit_nick = Signal(str)  # sygnał do przesyłania nicku
    sig_rooms_list = Signal(str) # sygnał informujący serwer że ma wysłać aktualną listę pokoi
    sig_create_room = Signal(str, str, int, int, int) # sygnał informujący serwer że użytkownik uzupełnił dane nowego pokoju
    sig_players_list = Signal(str) # sygnał informujący serwer że ma wysłać aktualną listę graczy w pokoju
    sig_join_room = Signal(str, str) # sygnał informujący serwer że użytkownik uzupełnił dane pokoju
    sig_start = Signal(str)
    sig_connect = Signal(str)
    sig_has_connected = Signal()
    sig_submit_letter = Signal(str)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # strona startowa
        self.ui.stackedWidget.setCurrentWidget(self.ui.nick_page)
        #self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)

        # walidator adresu IP
        ip_regex = QRegularExpression(r'^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$')
        ip_validator = QRegularExpressionValidator(ip_regex)
        self.ui.ip_field.setValidator(ip_validator)
        # walidator litery
        letter_regex = QRegularExpression("^[a-zA-Z]$")
        letter_validator = QRegularExpressionValidator(letter_regex)
        self.ui.letter_input.setValidator(letter_validator)

        self.ui.level_combobox.addItems(["easy", "medium", "hard"])

        self.ui.start_btn.setVisible(False)

        self.ui.ip_field.setText("192.168.100.8")

        self.ui.time_edit.setDisplayFormat("mm:ss")
        self.ui.time_edit.setTime(QTime(0,0,30))
        self.ui.time_edit.setMinimumTime(QTime(0,0,5))
        self.ui.time_edit.setMaximumTime(QTime(0,5,0))

        self.ui.letter_input.setMaxLength(1)

        self.ui.rounds_number_spin.setMinimum(1)

        # connect
        self.ui.connect_btn.clicked.connect(self.submit_ip)
        self.ui.ip_field.textChanged.connect(self.on_ip_changed)
        self.ui.stackedWidget.currentChanged.connect(self.is_at_create_or_join_page)
        self.ui.stackedWidget.currentChanged.connect(self.is_at_waitroom_page)
        self.ui.rooms_list.itemSelectionChanged.connect(self.on_list_item_selected)
        self.ui.create_btn.clicked.connect(self.submit_create_room)
        self.ui.join_btn.clicked.connect(self.submit_join_room)
        self.ui.start_btn.clicked.connect(self.submit_start)
        self.ui.letter_input.textChanged.connect(self.on_letter_changed)

    def submit_nick(self):
        if not self.ui.nick_field.text().strip():
            QMessageBox.warning(self, "Info", "Nickname field is obligatory!")
            return

        nick = self.ui.nick_field.text()

        self.sig_submit_nick.emit(nick)
        print(f"Nick submitted: {nick}")

    def submit_ip(self):
        ip = self.ui.ip_field.text()

        self.sig_connect.emit(ip)

    def submit_create_room(self):
        if not self.ui.create_name_field.text().strip():
            QMessageBox.warning(self, "Error", "Name field is obligatory!")
            return

        name = self.ui.create_name_field.text()
        password = self.ui.create_password_field.text()
        level = self.ui.level_combobox.currentIndex() + 1
        rounds = self.ui.rounds_number_spin.value()
        time = self.ui.time_edit.time()
        time_sec = time.hour() * 3600 + time.minute() * 60 + time.second()

        self.sig_create_room.emit(name, password, level, rounds, time_sec)
        print(f"Create room submitted: {name}, {password}, {level}, {rounds}, {time_sec}")

    def submit_join_room(self):
        name = self.ui.join_room_name_field.text()
        password = self.ui.join_password_field.text()

        self.sig_join_room.emit(name, password)
        print(f"Join room submitted: {name}, {password}")

    def submit_start(self):
        self.sig_start.emit("")
        print("START!")

    def submit_letter(self):
        letter = self.ui.letter_input.text()

        self.sig_submit_letter.emit(letter)

    def handle_server_response(self, message):
        print("serwer: " + message)
        if message.startswith("01"):
            result = substr_msg(message)
            if result == "1":  # nick zaakceptowany
                self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)
            elif result == "0":  # nick odrzucony
                QMessageBox.warning(self, "Info", "Nickname has already been taken!")
                self.ui.nick_field.setStyleSheet("color: red;")
        ###
        elif message.startswith("02"):
            result = substr_msg(message)
            if result == "1":  # pokój pomyślnie stworzony
                self.ui.stackedWidget.setCurrentWidget(self.ui.waitroom_page)
            elif result == "0":  # błąd
                self.ui.create_name_field.setStyleSheet("color: red;")
        ###
        elif message.startswith("03"):
            result = substr_msg(message)
            if result == "1":  # pomyślnie dołączono do pokoju
                self.ui.stackedWidget.setCurrentWidget(self.ui.waitroom_page)
            elif result == "0":  # błąd
                self.ui.join_room_name_field.setStyleSheet("color: red;")
        ###
        elif message.startswith("69"):
            self.sig_has_connected.emit()
        ###
        elif message.startswith("70"):
            nicks_encoded = substr_msg(message)
            nicks = nicks_encoded.split(",")

            self.ui.rooms_list.clear()

            for lobby in nicks:
                self.ui.rooms_list.addItem(lobby)

            print("Lista pokoi została zaktualizowana.")
        ###
        elif message.startswith("71"):
            nicks_encoded = substr_msg(message)
            nicks = nicks_encoded.split(",")

            self.ui.players_list.clear()

            for nick in nicks:
                self.ui.players_list.addItem(nick)

            print("Lista graczy w pokoju została zaktualizowana.")
        ###
        elif message.startswith("72"):
            decision = substr_msg(message)

            if decision == "1":
                self.ui.start_btn.setVisible(True)
                print("Gra może zostać rozpoczęta.")
            else:
                self.ui.start_btn.setVisible(False)
                print("Gra NIE może zostać rozpoczęta.")
        ###
        elif message.startswith("73"):
            word = substr_msg(message)
            spaced_word = " ".join(word)
            self.ui.player0_label.setText("all lives!")
            self.ui.player1_label.setText("all lives!")
            self.ui.player2_label.setText("all lives!")
            self.ui.player3_label.setText("all lives!")
            self.ui.player4_label.setText("all lives!")
            self.ui.word_label.setText(spaced_word)
            self.ui.stackedWidget.setCurrentWidget(self.ui.game_page)

    def on_ip_changed(self):
        if self.ui.ip_field.hasAcceptableInput():
            self.ui.ip_field.setStyleSheet("color: green;")
        else:
            self.ui.ip_field.setStyleSheet("color: black;")

    def on_letter_changed(self):
        letter = self.ui.letter_input.text()
        if letter and letter.islower():
            self.ui.letter_input.setText(letter.upper())

    def is_at_create_or_join_page(self, index):
        if self.ui.stackedWidget.widget(index) == self.ui.create_or_join_page:
            self.sig_rooms_list.emit("")

    def is_at_waitroom_page(self, index):
        if self.ui.stackedWidget.widget(index) == self.ui.waitroom_page:
            self.sig_players_list.emit("")

    def on_list_item_selected(self):
        selected_item = self.ui.rooms_list.currentItem()
        if selected_item:
            self.ui.join_room_name_field.setText(selected_item.text())