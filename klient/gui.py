import sys
from os.path import split

from PySide6 import QtGui

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
    sig_time_ran_out = Signal(str)
    sig_leave_room = Signal(str) # todo usunąć niepotrzebne stringi z sygnałów
    sig_disconnect_me = Signal(str)

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

        self.ui.nick_field.setMaxLength(20)

        self.ui.start_btn.setVisible(False)
        self.ui.start_btn.setEnabled(False)

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

        self.setWindowIcon(QtGui.QIcon(self.image_paths[1]))

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

        # CONNECT
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
        self.ui.waitroom_back_btn.clicked.connect(self.leave_room)
        self.ui.end_back_btn.clicked.connect(self.leave_room)
        self.ui.create_join_back_btn.clicked.connect(self.back_to_nick_page)
        self.ui.leave_room_btn.clicked.connect(self.leave_room)

    def submit_nick(self):
        nick = self.ui.nick_field.text()

        self.sig_submit_nick.emit(nick)
        print(f"Nick submitted: {nick}")

    def submit_ip(self):
        if not self.ui.nick_field.text().strip():
            QMessageBox.warning(self, "Info", "Nickname field is obligatory!")
            return
        if not self.ui.ip_field.text().strip():
            QMessageBox.warning(self, "Info", "IP field is obligatory!")
            return
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

    def play_again(self):
        self.clean_waitroom_page()
        self.clean_game_page()
        self.clean_end_page()

        self.ui.stackedWidget.setCurrentWidget(self.ui.waitroom_page)

    def leave_room(self):
        self.sig_leave_room.emit("")
        print("Let me leave the room, please")

    def back_to_nick_page(self):
        msg_box = QMessageBox(self)
        msg_box.setIcon(QMessageBox.Icon.Information)
        msg_box.setWindowTitle("Information")
        msg_box.setText("This action will disconnect you from the current server.\nDo you want to continue?")
        msg_box.setStandardButtons(QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)

        reply = msg_box.exec_()

        if reply == QMessageBox.StandardButton.Yes:
            self.clean_create_or_join_page()
            self.ui.stackedWidget.setCurrentWidget(self.ui.nick_page)
            self.sig_disconnect_me.emit("")

    def handle_server_response(self, message):
        if not message.startswith("11"): # ignoruj timer
            print("serwer: " + message)
        ### decyzja o nicku
        if message.startswith("01"):
            result = substr_msg(message)
            if result == "1":  # nick zaakceptowany
                self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)
            elif result == "01":  # nick odrzucony
                QMessageBox.information(self, "Nickanme cannot be assigned", "Nickname has already been taken")
                self.ui.nick_field.clear()
            elif result == "02":  # nie znaleziono gracza
                QMessageBox.critical(self, "Nickname cannot be assigned", "Player has not been found\nTry reconnecting")
        ### decyzja o utworzeniu pokoju
        elif message.startswith("02"):
            result = substr_msg(message)
            if result == "1":  # pokój pomyślnie stworzony
                self.ui.stackedWidget.setCurrentWidget(self.ui.waitroom_page)
            elif result == "01":  # nazwa zajęta
                self.ui.create_name_field.clear()
                QMessageBox.information(self, "Room cannot be created", "The name has already been taken")
            elif result == "02":  # nie znaleziono gracza
                QMessageBox.critical(self, "Room cannot be created", "Player has not been found\nTry reconnecting")
        ### decyzja o dołączeniu do pokoju
        elif message.startswith("03"):
            result = substr_msg(message)
            if result == "1":  # pomyślnie dołączono do pokoju
                self.ui.stackedWidget.setCurrentWidget(self.ui.waitroom_page)
            elif result == "01":  # niepoprawne hasło
                QMessageBox.information(self, "Cannot join the Room", "Incorrect password")
                self.ui.join_room_name_field.clear()
            elif result == "02":  # max graczy
                QMessageBox.information(self, "Cannot join the Room", "The Room is full")
            elif result == "03":  # trwa gra
                QMessageBox.information(self, "Cannot join the Room", "There is an ongoing game in the Room")
            elif result == "04":  # pokój nie istnieje
                QMessageBox.information(self, "Cannot join the Room", "The Room with given name does not exist")
                self.ui.join_room_name_field.clear()
                self.ui.join_password_field.clear()
            elif result == "05":  # nie znaleziono gracza
                QMessageBox.critical(self, "Cannot join the Room", "Player has not been found\nTry reconnecting")
        ### odpowiedź na literę w grze
        elif message.startswith("06"):
            result = substr_msg(message)
            if result == "01":
                QMessageBox.critical(self, "Cannot guess", "The Room has not been found\nTry reconnecting")
                return
            elif result == "02":
                QMessageBox.critical(self, "Cannot guess", "Player has not been found\nTry reconnecting")
                return
            elif result == "03":
                QMessageBox.information(self, "Cannot guess", "You are out of lives for this round")
                return

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
        ### odpowiedź na usunięcie z pokoju
        elif message.startswith("09"):
            result = substr_msg(message)
            if result == "1":  # pomyślnie usunięto gracza z pokoju
                self.clean_create_or_join_page()
                self.clean_waitroom_page()
                self.clean_game_page()
                self.clean_end_page()
                self.ui.stackedWidget.setCurrentWidget(self.ui.create_or_join_page)
            elif result == "0":  # błąd
                pass
        ### aktualny timer w grze
        elif message.startswith("11"):
            time = substr_msg(message)
            self.ui.time2_label_2.setText(time)
        ### koniec czasu w grze
        elif message.startswith("12"):
            self.sig_time_ran_out.emit("")
        ### udało się połączyć pod IP
        elif message.startswith("69"):
            self.sig_has_connected.emit()
        ### lista dostępnych pokoi
        elif message.startswith("70"):
            nicks_encoded = substr_msg(message)
            nicks = nicks_encoded.split(",")

            self.ui.rooms_list.clear()

            for lobby in nicks:
                self.ui.rooms_list.addItem(lobby)

            print("Lista pokoi została zaktualizowana.")
        ### lista graczy w danym pokoju
        elif message.startswith("71"):
            nicks_encoded = substr_msg(message)
            nicks = nicks_encoded.split(",")

            self.ui.players_list.clear()

            for nick in nicks:
                self.ui.players_list.addItem(nick)

            print("Lista graczy w pokoju została zaktualizowana.")
        ### decyzja czy można rozpocząć grę
        elif message.startswith("72"):
            decision = substr_msg(message)
            self.ui.start_btn.setVisible(True)

            if decision == "1":
                self.ui.start_btn.setEnabled(True)
                print("Gra może zostać rozpoczęta.")
            else:
                self.ui.start_btn.setEnabled(False)
                print("Gra NIE może zostać rozpoczęta.")
        ### init gry
        elif message.startswith("73"):
            msg = substr_msg(message)
            parts = msg.split(";")

            decision = parts[0]
            if decision == "0":
                QMessageBox.critical(self, "The game cannot be started", "The Room has not been found\nTry reconnecting")
                return

            word = parts[1]
            spaced_word = " ".join(word)
            self.ui.word_label.setText(spaced_word)

            time = parts[2]
            self.ui.time2_label_2.setText(time)

            rounds = int(parts[3])
            self.ui.rounds2_label.setText(f"1/{rounds}")

            your_nick = parts[4]
            self.ui.ranking_list.addItem(f"0    {your_nick}")

            opponents = parts[5]
            nicks = opponents.split(",")

            for i, nick in enumerate(nicks):
                self.player_names_labels[i].setText(nicks[i])
                self.opacity[i].setOpacity(1.0)
                self.ui.ranking_list.addItem(f"0    {nick}")

            self.ui.stackedWidget.setCurrentWidget(self.ui.game_page)
        ### update hasła w grze
        elif message.startswith("75"):
            word = substr_msg(message).upper()
            spaced_word = " ".join(word)
            self.ui.word_label.setText(spaced_word)
        ### update wisielców
        elif message.startswith("76"):
            msg = substr_msg(message)
            info = msg.split(",")
            nick = info[0]
            lives = int(info[1])
            for i, player_name in enumerate(self.player_names_labels):
                if player_name.text() == nick:
                    self.player_labels[i].setPixmap(QPixmap(self.image_paths[lives]))
                    break
        ### update rankingu
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
        ### koniec gry
        elif message.startswith("78"):
            msg = substr_msg(message)
            info = msg.split(";")
            ranking = info[0]
            all_words = info[1]

            players = ranking.split(",")
            words = all_words.split(",")

            items = [(int(player.split(":")[1]), player.split(":")[0]) for player in players]
            items.sort(reverse=True, key=lambda x: x[0])

            self.ui.ranking_list_2.clear()
            self.ui.ranking_list_2.addItems([f"{points}    {nick}" for points, nick in items])

            for word in words:
                self.ui.all_words_list.addItem(word.upper())

            self.ui.stackedWidget.setCurrentWidget(self.ui.end_page)
        ### następna runda
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

            self.ui.send_letter_btn.setEnabled(True)
            self.ui.letter_input.setReadOnly(False)
        ### jeden z graczy opuścił trwającą grę
        elif message.startswith("80"):
            nick = substr_msg(message)

            for i, name_label in enumerate(self.player_names_labels):
                if name_label.text() == nick:
                    self.player_names_labels[i].setText("")
                    self.player_labels[i].setPixmap(QPixmap(self.image_paths[10]))
                    self.opacity[i].setOpacity(0.5)
                    break

            for i in range(self.ui.ranking_list.count()):
                item = self.ui.ranking_list.item(i)
                item_info = item.text().split("    ")

                list_nick = item_info[1]

                if nick == list_nick:
                    self.ui.ranking_list.takeItem(i)
                    break

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
            print("emmited 71 through changing page")
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

    def clean_upon_disconnect(self):
        self.ui.stackedWidget.setCurrentWidget(self.ui.nick_page)
        self.clean_create_or_join_page()
        self.clean_waitroom_page()
        self.clean_game_page()
        self.clean_end_page()

    def clean_nick_page(self):
        self.ui.nick_field.clear()
        self.ui.ip_field.setText("192.168.100.8")

    def clean_create_or_join_page(self):
        self.ui.create_name_field.clear()
        self.ui.create_password_field.clear()
        self.ui.join_room_name_field.clear()
        self.ui.join_password_field.clear()
        self.ui.time_edit.setTime(QTime(0, 0, 30))
        self.ui.rounds_number_spin.setValue(1)
        self.ui.level_combobox.setCurrentIndex(0)
        self.ui.rooms_list.clear()

    def clean_waitroom_page(self):
        self.ui.players_list.clear()
        self.ui.start_btn.setVisible(False)
        self.ui.start_btn.setEnabled(False)

    def clean_game_page(self):
        self.ui.letter_input.clear()
        self.ui.fails_label.clear()
        self.ui.ranking_list.clear()
        self.ui.send_letter_btn.setEnabled(True)
        self.ui.letter_input.setReadOnly(False)
        for label in self.player_labels:
            label.setPixmap(QPixmap(self.image_paths[10]))

        for effect in self.opacity:
            effect.setOpacity(0.5)

        for name_label in self.player_names_labels:
            name_label.setText("")

    def clean_end_page(self):
        self.ui.all_words_list.clear()
        self.ui.ranking_list_2.clear()