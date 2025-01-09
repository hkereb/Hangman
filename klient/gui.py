import sys
from os.path import split
import resources

from ui_skeleton import Ui_MainWindow
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox, QGraphicsOpacityEffect
from PySide6.QtCore import QObject, Signal, QThread, QRegularExpression, QTime
from PySide6.QtGui import QRegularExpressionValidator, QPixmap


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

        self.image_paths = {
            0: ":images/0-lives-no-eyes.png",
            1: ":images/1-lives.png",
            2: ":images/2-lives.png",
            3: ":images/3-lives.png",
            4: ":images/4-lives.png",
            5: ":images/5-lives.png",
            6: ":images/6-lives.png",
            7: ":images/7-lives.png",
            8: ":images/8-lives.png",
            9: ":images/9-lives.png",
            10: ":images/10-lives.png"
        }

        self.ui.player0_label.setPixmap(QPixmap(self.image_paths[10]))
        self.ui.player1_label.setPixmap(QPixmap(self.image_paths[10]))
        self.ui.player2_label.setPixmap(QPixmap(self.image_paths[10]))
        self.ui.player3_label.setPixmap(QPixmap(self.image_paths[10]))
        self.ui.player4_label.setPixmap(QPixmap(self.image_paths[10]))
        self.ui.player0_label.setScaledContents(True)
        self.ui.player1_label.setScaledContents(True)
        self.ui.player2_label.setScaledContents(True)
        self.ui.player3_label.setScaledContents(True)
        self.ui.player4_label.setScaledContents(True)
        self.opacity_effect1 = QGraphicsOpacityEffect()
        self.opacity_effect1.setOpacity(0.5)
        self.opacity_effect2 = QGraphicsOpacityEffect()
        self.opacity_effect2.setOpacity(0.5)
        self.opacity_effect3 = QGraphicsOpacityEffect()
        self.opacity_effect3.setOpacity(0.5)
        self.opacity_effect4 = QGraphicsOpacityEffect()
        self.opacity_effect4.setOpacity(0.5)
        self.ui.player1_label.setGraphicsEffect(self.opacity_effect1)
        self.ui.player2_label.setGraphicsEffect(self.opacity_effect2)
        self.ui.player3_label.setGraphicsEffect(self.opacity_effect3)
        self.ui.player4_label.setGraphicsEffect(self.opacity_effect4)

        self.player_names_labels = [
            self.ui.player1_name_label,
            self.ui.player2_name_label,
            self.ui.player3_name_label,
            self.ui.player4_name_label
        ]
        self.player_labels = [
            self.ui.player1_label,
            self.ui.player2_label,
            self.ui.player3_label,
            self.ui.player4_label
        ]
        self.opacity = [
            self.opacity_effect1,
            self.opacity_effect2,
            self.opacity_effect3,
            self.opacity_effect4
        ]

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
        self.ui.letter_input.returnPressed.connect(self.submit_letter)
        self.ui.send_letter_btn.clicked.connect(self.submit_letter)

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
        if not self.ui.letter_input.text().strip():
            return
        letter = self.ui.letter_input.text()
        letter = letter.lower()

        self.sig_submit_letter.emit(letter)
        self.ui.letter_input.clear()

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
        elif message.startswith("06"):
            result = substr_msg(message)
            parts = result.split(',')

            decision = parts[0]
            letter = parts[1]

            if decision == "0":
                if self.ui.fails_label.text() == "":
                    self.ui.fails_label.setText(letter.upper())
                else:
                    old = self.ui.fails_label.text()
                    self.ui.fails_label.setText(old + ", " + letter.upper())

                lives = int(parts[2])
                self.ui.player0_label.setPixmap(QPixmap(self.image_paths[lives]))
                if lives == 0:
                    self.ui.send_letter_btn.setEnabled(False)
                    self.ui.letter_input.setReadOnly(True)
        ###
        elif message.startswith("11"):
            time = substr_msg(message)
            self.ui.time2_label_2.setText(time)
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
        ### init gry
        elif message.startswith("73"):
            msg = substr_msg(message)
            parts = msg.split(";")
            word = parts[0]
            spaced_word = " ".join(word)
            self.ui.word_label.setText(spaced_word)

            time = parts[1]
            self.ui.time2_label_2.setText(time)

            rounds = int(parts[2])
            self.ui.rounds2_label.setText(f"1/{rounds}")

            your_nick = parts[3]
            self.ui.ranking_list.addItem(f"0    {your_nick}")

            opponents = parts[4]
            nicks = opponents.split(",")

            for i, nick in enumerate(nicks):
                self.player_names_labels[i].setText(nicks[i])
                self.opacity[i].setOpacity(1.0)
                self.ui.ranking_list.addItem(f"0    {nick}")

            self.ui.stackedWidget.setCurrentWidget(self.ui.game_page)
        ###
        elif message.startswith("75"):
            word = substr_msg(message).upper()
            spaced_word = " ".join(word)
            self.ui.word_label.setText(spaced_word)
        ###
        elif message.startswith("76"):
            msg = substr_msg(message)
            info = msg.split(",")
            nick = info[0]
            lives = int(info[1])
            for i, player_name in enumerate(self.player_names_labels):
                if player_name.text() == nick:
                    self.player_labels[i].setPixmap(QPixmap(self.image_paths[lives]))
                    break
        ###
        elif message.startswith("77"):
            msg = substr_msg(message)
            info = msg.split(",")
            nick = info[0]
            points = int(info[1])

            for i in range(self.ui.ranking_list.count()):
                item = self.ui.ranking_list.item(i)
                item_info = item.text().split("    ")
                item_nick = item_info[1]

                if item_nick == nick:
                    item.setText(f"{points}    {nick}")
                    break

            self.updateRanking()
        ###
        elif message.startswith("78"):
            msg = substr_msg(message)
            info = msg.split(";")
            ranking = info[0]
            all_words = info[1]

            players = ranking.split(",")
            words = all_words.split(",")

            items = []
            for player in players:
                player_info = player.split(":")
                nick = player_info[0]
                points = player_info[1]
                items.append((points, nick))

            items.sort(reverse=True, key=lambda x: x[0])
            self.ui.ranking_list_2.clear()
            for points, nick in items:
                self.ui.ranking_list_2.addItem(f"{points}    {nick}")

            for word in words:
                self.ui.all_words_list.addItem(word.upper())

            self.ui.stackedWidget.setCurrentWidget(self.ui.end_page)
        ###
        elif message.startswith("79"):
            msg = substr_msg(message)
            info = msg.split(",")
            word = info[0].upper()
            spaced_word = " ".join(word)
            current_round = int(info[1])
            max_round = int(info[2])

            self.ui.word_label.setText(spaced_word)
            self.ui.rounds2_label.setText(f"{current_round}/{max_round}")
            self.ui.fails_label.clear()
            for i, player in enumerate(self.player_labels):
                self.player_labels[i].setPixmap(QPixmap(self.image_paths[10]))
            self.ui.player0_label.setPixmap(QPixmap(self.image_paths[10]))

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

    def updateRanking(self):
        items = []
        for i in range(self.ui.ranking_list.count()):
            item = self.ui.ranking_list.item(i)
            item_info = item.text().split("    ")

            points = int(item_info[0])
            nick = item_info[1]
            items.append((points, nick))

        items.sort(reverse=True, key=lambda x: x[0])
        self.ui.ranking_list.clear()

        for points, nick in items:
            self.ui.ranking_list.addItem(f"{points}    {nick}")
