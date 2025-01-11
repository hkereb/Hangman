# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'MainWindowCdPcOh.ui'
##
## Created by: Qt User Interface Compiler version 6.8.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QAbstractItemView, QApplication, QComboBox, QFrame,
    QGroupBox, QLabel, QLineEdit, QListView,
    QListWidget, QListWidgetItem, QMainWindow, QPushButton,
    QSizePolicy, QSpinBox, QStackedWidget, QTimeEdit,
    QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(800, 600)
        MainWindow.setMinimumSize(QSize(800, 600))
        MainWindow.setMaximumSize(QSize(800, 600))
        MainWindow.setWindowTitle(u"Hangman")
        MainWindow.setWindowOpacity(1.0)
        MainWindow.setIconSize(QSize(24, 24))
        MainWindow.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonFollowStyle)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.stackedWidget = QStackedWidget(self.centralwidget)
        self.stackedWidget.setObjectName(u"stackedWidget")
        self.stackedWidget.setGeometry(QRect(0, 0, 800, 600))
        self.stackedWidget.setMinimumSize(QSize(800, 600))
        self.stackedWidget.setMaximumSize(QSize(800, 600))
        self.stackedWidget.setAcceptDrops(True)
        self.stackedWidget.setAutoFillBackground(False)
        self.stackedWidget.setFrameShape(QFrame.Shape.NoFrame)
        self.nick_page = QWidget()
        self.nick_page.setObjectName(u"nick_page")
        self.connect_btn = QPushButton(self.nick_page)
        self.connect_btn.setObjectName(u"nick_submit_btn")
        self.connect_btn.setGeometry(QRect(360, 360, 81, 21))
        font3 = QFont()
        font3.setPointSize(10)
        self.connect_btn.setFont(font3)
        self.nick_field = QLineEdit(self.nick_page)
        self.nick_field.setPlaceholderText("Choose your nickname")
        self.nick_field.setObjectName(u"nick_field")
        self.nick_field.setGeometry(QRect(280, 320, 241, 20))
        self.title_label = QLabel(self.nick_page)
        self.title_label.setObjectName(u"title_label")
        self.title_label.setGeometry(QRect(260, 170, 291, 71))
        font = QFont()
        font.setFamilies([u"Javanese Text"])
        font.setPointSize(45)
        font.setBold(False)
        font.setItalic(False)
        self.title_label.setFont(font)
        self.subtitle_label = QLabel(self.nick_page)
        self.subtitle_label.setObjectName(u"subtitle_label")
        self.subtitle_label.setGeometry(QRect(330, 230, 151, 41))
        font1 = QFont()
        font1.setFamilies([u"Javanese Text"])
        font1.setPointSize(13)
        font2 = QFont()
        font2.setPointSize(13)
        font2.setFamilies([u"Javanese Text"])
        self.subtitle_label.setFont(font2)
        self.ip_field = QLineEdit(self.nick_page)
        self.ip_field.setObjectName(u"ip_field")
        self.ip_field.setGeometry(QRect(280, 290, 241, 20))
        self.ip_field.setPlaceholderText("___.___.___.___")
        self.ip_label = QLabel(self.nick_page)
        self.ip_label.setObjectName(u"ip_label")
        self.ip_label.setGeometry(QRect(220, 290, 51, 20))
        self.ip_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.nick_label = QLabel(self.nick_page)
        self.nick_label.setObjectName(u"nick_label")
        self.nick_label.setGeometry(QRect(206, 320, 61, 20))
        self.nick_label.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.stackedWidget.addWidget(self.nick_page)
        self.create_or_join_page = QWidget()
        self.create_or_join_page.setObjectName(u"create_or_join_page")
        self.create_groupbox = QGroupBox(self.create_or_join_page)
        self.create_groupbox.setObjectName(u"create_groupbox")
        self.create_groupbox.setGeometry(QRect(30, 30, 441, 351))
        self.create_name_field = QLineEdit(self.create_groupbox)
        self.create_name_field.setObjectName(u"create_name_field")
        self.create_name_field.setPlaceholderText("Name your room")
        self.create_name_field.setGeometry(QRect(180, 80, 151, 20))
        self.create_name_field.setMaxLength(20)
        self.create_password_field = QLineEdit(self.create_groupbox)
        self.create_password_field.setObjectName(u"create_password_field")
        self.create_password_field.setPlaceholderText("(optional)")
        self.create_password_field.setGeometry(QRect(180, 110, 151, 20))
        self.create_password_field.setMaxLength(20)
        self.level_combobox = QComboBox(self.create_groupbox)
        self.level_combobox.setObjectName(u"level_combobox")
        self.level_combobox.setGeometry(QRect(180, 140, 151, 22))
        self.rounds_number_spin = QSpinBox(self.create_groupbox)
        self.rounds_number_spin.setObjectName(u"rounds_number_spin")
        self.rounds_number_spin.setGeometry(QRect(180, 170, 151, 22))
        self.time_edit = QTimeEdit(self.create_groupbox)
        self.time_edit.setObjectName(u"time_edit")
        self.time_edit.setGeometry(QRect(180, 200, 151, 22))
        self.create_btn = QPushButton(self.create_groupbox)
        self.create_btn.setObjectName(u"create_btn")
        self.create_btn.setGeometry(QRect(210, 240, 91, 23))
        self.create_btn.setFont(font3)
        self.name_label = QLabel(self.create_groupbox)
        self.name_label.setObjectName(u"name_label")
        self.name_label.setGeometry(QRect(100, 80, 61, 20))
        self.name_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.pass_label = QLabel(self.create_groupbox)
        self.pass_label.setObjectName(u"pass_label")
        self.pass_label.setGeometry(QRect(100, 110, 61, 20))
        self.pass_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.level_label = QLabel(self.create_groupbox)
        self.level_label.setObjectName(u"level_label")
        self.level_label.setGeometry(QRect(100, 140, 61, 20))
        self.level_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.number_label = QLabel(self.create_groupbox)
        self.number_label.setObjectName(u"number_label")
        self.number_label.setGeometry(QRect(50, 170, 111, 20))
        self.number_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.time_label = QLabel(self.create_groupbox)
        self.time_label.setObjectName(u"time_label")
        self.time_label.setGeometry(QRect(70, 200, 91, 20))
        self.time_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.join_groupbox = QGroupBox(self.create_or_join_page)
        self.join_groupbox.setObjectName(u"join_groupbox")
        self.join_groupbox.setGeometry(QRect(30, 390, 441, 171))
        self.join_password_field = QLineEdit(self.join_groupbox)
        self.join_password_field.setObjectName(u"join_password_field")
        self.join_password_field.setGeometry(QRect(180, 80, 151, 20))
        self.join_password_field.setMaxLength(20)
        self.join_password_field.setAlignment(Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft|Qt.AlignmentFlag.AlignVCenter)
        self.join_btn = QPushButton(self.join_groupbox)
        self.join_btn.setObjectName(u"join_btn")
        self.join_btn.setGeometry(QRect(220, 120, 75, 23))
        self.join_btn.setFont(font3)
        self.join_room_name_field = QLineEdit(self.join_groupbox)
        self.join_room_name_field.setObjectName(u"join_room_name_field")
        self.join_room_name_field.setGeometry(QRect(180, 50, 151, 20))
        self.join_room_name_field.setLayoutDirection(Qt.LayoutDirection.LeftToRight)
        self.join_room_name_field.setMaxLength(20)
        self.join_room_name_field.setAlignment(Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft|Qt.AlignmentFlag.AlignVCenter)
        self.name_2_label = QLabel(self.join_groupbox)
        self.name_2_label.setObjectName(u"name_2_label")
        self.name_2_label.setGeometry(QRect(110, 50, 51, 20))
        self.name_2_label.setLayoutDirection(Qt.LayoutDirection.LeftToRight)
        self.name_2_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.pass_2_label = QLabel(self.join_groupbox)
        self.pass_2_label.setObjectName(u"pass_2_label")
        self.pass_2_label.setGeometry(QRect(100, 80, 61, 20))
        self.pass_2_label.setAlignment(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTrailing | Qt.AlignmentFlag.AlignVCenter)
        self.rooms_list = QListWidget(self.create_or_join_page)
        self.rooms_list.setObjectName(u"rooms_list")
        self.rooms_list.setGeometry(QRect(500, 40, 271, 521))
        self.list_of_rooms_label = QLabel(self.create_or_join_page)
        self.list_of_rooms_label.setObjectName(u"list_of_rooms_label")
        self.list_of_rooms_label.setGeometry(QRect(600, 20, 101, 16))
        self.create_join_back_btn = QPushButton(self.create_or_join_page)
        self.create_join_back_btn.setObjectName(u"create_join_back_btn")
        self.create_join_back_btn.setGeometry(QRect(30, 570, 75, 23))
        self.stackedWidget.addWidget(self.create_or_join_page)
        self.waitroom_page = QWidget()
        self.waitroom_page.setObjectName(u"waitroom_page")
        self.waiting_label = QLabel(self.waitroom_page)
        self.waiting_label.setObjectName(u"waiting_label")
        self.waiting_label.setGeometry(QRect(210, 140, 361, 71))
        font2 = QFont()
        font2.setFamilies([u"Javanese Text"])
        font2.setPointSize(20)
        self.waiting_label.setFont(font2)
        self.players_list = QListWidget(self.waitroom_page)
        self.players_list.setObjectName(u"players_list")
        self.players_list.setGeometry(QRect(200, 200, 371, 171))
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.MinimumExpanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.players_list.sizePolicy().hasHeightForWidth())
        self.players_list.setSizePolicy(sizePolicy)
        self.players_list.setMinimumSize(QSize(0, 21))
        self.players_list.setFont(font3)
        self.start_btn = QPushButton(self.waitroom_page)
        self.start_btn.setObjectName(u"start_btn")
        self.start_btn.setGeometry(QRect(340, 390, 101, 31))
        font5 = QFont()
        font5.setPointSize(15)
        self.start_btn.setFont(font5)
        self.waitroom_back_btn = QPushButton(self.waitroom_page)
        self.waitroom_back_btn.setObjectName(u"waitroom_back_btn")
        self.waitroom_back_btn.setGeometry(QRect(30, 570, 75, 23))
        self.stackedWidget.addWidget(self.waitroom_page)
        self.game_page = QWidget()
        self.game_page.setObjectName(u"game_page")
        self.word_label = QLabel(self.game_page)
        self.word_label.setObjectName(u"word_label")
        self.word_label.setGeometry(QRect(370, 170, 381, 61))
        font3 = QFont()
        font3.setFamilies([u"Javanese Text"])
        font3.setPointSize(26)
        self.word_label.setFont(font3)
        self.word_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.rounds1_label = QLabel(self.game_page)
        self.rounds1_label.setObjectName(u"rounds1_label")
        self.rounds1_label.setGeometry(QRect(510, 50, 71, 21))
        font4 = QFont()
        font4.setPointSize(12)
        self.rounds1_label.setFont(font4)
        self.rounds1_label.setAlignment(Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft|Qt.AlignmentFlag.AlignVCenter)
        self.time1_label = QLabel(self.game_page)
        self.time1_label.setObjectName(u"time1_label")
        self.time1_label.setGeometry(QRect(510, 80, 51, 21))
        self.time1_label.setFont(font4)
        self.time1_label.setAlignment(Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft|Qt.AlignmentFlag.AlignVCenter)
        self.letter_input = QLineEdit(self.game_page)
        self.letter_input.setObjectName(u"letter_input")
        self.letter_input.setGeometry(QRect(550, 350, 31, 31))
        font5 = QFont()
        font5.setPointSize(16)
        self.letter_input.setFont(font5)
        self.letter_input.setInputMethodHints(Qt.InputMethodHint.ImhNone)
        self.letter_input.setMaxLength(1)
        self.letter_input.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.letter_input.setClearButtonEnabled(False)
        self.ranking_list = QListWidget(self.game_page)
        self.ranking_list.setObjectName(u"ranking_list")
        self.ranking_list.setGeometry(QRect(40, 40, 301, 101))
        self.rounds2_label = QLabel(self.game_page)
        self.rounds2_label.setObjectName(u"rounds2_label")
        self.rounds2_label.setGeometry(QRect(590, 42, 51, 31))
        font6 = QFont()
        font6.setPointSize(18)
        self.rounds2_label.setFont(font6)
        self.rounds2_label.setAlignment(Qt.AlignmentFlag.AlignBottom|Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft)
        self.time2_label_2 = QLabel(self.game_page)
        self.time2_label_2.setObjectName(u"time2_label_2")
        self.time2_label_2.setGeometry(QRect(590, 72, 51, 31))
        self.time2_label_2.setFont(font6)
        self.time2_label_2.setAlignment(Qt.AlignmentFlag.AlignBottom|Qt.AlignmentFlag.AlignLeading|Qt.AlignmentFlag.AlignLeft)
        self.leave_room_btn = QPushButton(self.game_page)
        self.leave_room_btn.setObjectName(u"leave_room_btn")
        self.leave_room_btn.setGeometry(QRect(680, 60, 75, 23))
        self.ranking_label = QLabel(self.game_page)
        self.ranking_label.setObjectName(u"ranking_label")
        self.ranking_label.setGeometry(QRect(160, 15, 71, 21))
        self.ranking_label.setFont(font4)
        self.ranking_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.player0_label = QLabel(self.game_page)
        self.player0_label.setObjectName(u"player0_label")
        self.player0_label.setGeometry(QRect(40, 160, 300, 260))
        self.player1_label = QLabel(self.game_page)
        self.player1_label.setObjectName(u"player1_label")
        self.player1_label.setGeometry(QRect(40, 450, 150, 130))
        self.player2_label = QLabel(self.game_page)
        self.player2_label.setObjectName(u"player2_label")
        self.player2_label.setGeometry(QRect(230, 450, 150, 130))
        self.player3_label = QLabel(self.game_page)
        self.player3_label.setObjectName(u"player3_label")
        self.player3_label.setGeometry(QRect(420, 450, 150, 130))
        self.player4_label = QLabel(self.game_page)
        self.player4_label.setObjectName(u"player4_label")
        self.player4_label.setGeometry(QRect(610, 450, 150, 130))
        self.fails_label = QLabel(self.game_page)
        self.fails_label.setObjectName(u"fails_label")
        self.fails_label.setGeometry(QRect(440, 250, 251, 91))
        self.fails_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.fails_label.setWordWrap(True)
        self.fails_label.setStyleSheet("color: red;")
        font88 = QFont()
        font88.setPointSize(13)
        self.fails_label.setFont(font88)
        self.send_letter_btn = QPushButton(self.game_page)
        self.send_letter_btn.setObjectName(u"send_letter_btn")
        self.send_letter_btn.setGeometry(QRect(590, 355, 21, 21))
        font9 = QFont()
        font9.setPointSize(7)
        self.send_letter_btn.setFont(font9)
        self.send_letter_btn.setCheckable(False)
        self.send_letter_btn.setAutoDefault(False)
        self.send_letter_btn.setFlat(False)
        font20 = QFont()
        font20.setBold(True)
        self.player1_name_label = QLabel(self.game_page)
        self.player1_name_label.setObjectName(u"player1_name_label")
        self.player1_name_label.setGeometry(QRect(40, 430, 151, 20))
        self.player1_name_label.setFont(font20)
        self.player1_name_label.setScaledContents(False)
        self.player1_name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.player2_name_label = QLabel(self.game_page)
        self.player2_name_label.setObjectName(u"player2_name_label")
        self.player2_name_label.setGeometry(QRect(230, 430, 151, 20))
        self.player2_name_label.setFont(font20)
        self.player2_name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.player3_name_label = QLabel(self.game_page)
        self.player3_name_label.setObjectName(u"player3_name_label")
        self.player3_name_label.setGeometry(QRect(420, 430, 151, 20))
        self.player3_name_label.setFont(font20)
        self.player3_name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.player4_name_label = QLabel(self.game_page)
        self.player4_name_label.setObjectName(u"player4_name_label")
        self.player4_name_label.setGeometry(QRect(610, 430, 151, 16))
        self.player4_name_label.setFont(font20)
        self.player4_name_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.stackedWidget.addWidget(self.game_page)
        self.end_page = QWidget()
        self.end_page.setObjectName(u"end_page")
        self.ranking_list_2 = QListWidget(self.end_page)
        self.ranking_list_2.setObjectName(u"ranking_list_2")
        self.ranking_list_2.setGeometry(QRect(120, 170, 271, 231))
        self.ranking_list_2.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self.ranking_list_2.setDefaultDropAction(Qt.DropAction.IgnoreAction)
        self.ranking_list_2.setProperty(u"isWrapping", False)
        self.ranking_list_2.setResizeMode(QListWidget.ResizeMode.Adjust)
        self.ranking_list_2.setSelectionRectVisible(False)
        self.final_ranking_label = QLabel(self.end_page)
        self.final_ranking_label.setObjectName(u"final_ranking_label")
        self.final_ranking_label.setGeometry(QRect(190, 140, 131, 31))
        self.final_ranking_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        font7 = QFont()
        font7.setFamilies([u"Javanese Text"])
        font7.setPointSize(14)
        self.final_ranking_label.setFont(font7)
        self.restart_btn = QPushButton(self.end_page)
        self.restart_btn.setObjectName(u"restart_btn")
        self.restart_btn.setGeometry(QRect(330, 430, 151, 23))
        self.restart_btn.setFont(font4)
        self.stackedWidget.addWidget(self.end_page)
        self.end_back_btn = QPushButton(self.end_page)
        self.end_back_btn.setObjectName(u"end_back_btn")
        self.end_back_btn.setGeometry(QRect(30, 570, 75, 24))
        self.all_words_list = QListWidget(self.end_page)
        self.all_words_list.setObjectName(u"all_words_list")
        self.all_words_list.setGeometry(QRect(420, 170, 271, 231))
        self.all_words_label = QLabel(self.end_page)
        self.all_words_label.setObjectName(u"all_words_label")
        self.all_words_label.setGeometry(QRect(480, 140, 131, 31))
        self.all_words_label.setFont(font7)
        self.all_words_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.stackedWidget.addWidget(self.end_page)
        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)

        self.stackedWidget.setCurrentIndex(1)
        self.send_letter_btn.setDefault(False)


        QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        self.connect_btn.setText(QCoreApplication.translate("MainWindow", u"CONNECT", None))
        self.title_label.setText(QCoreApplication.translate("MainWindow", u" Hangman ", None))
        self.subtitle_label.setText(QCoreApplication.translate("MainWindow", u" Multiplayer Game ", None))
        self.ip_label.setText(QCoreApplication.translate("MainWindow", u"Server IP", None))
        self.nick_label.setText(QCoreApplication.translate("MainWindow", u"Nickname", None))
        self.create_join_back_btn.setText(QCoreApplication.translate("MainWindow", u"BACK", None))
        self.create_groupbox.setTitle(QCoreApplication.translate("MainWindow", u"Create a Room ", None))
        self.create_btn.setText(QCoreApplication.translate("MainWindow", u"CREATE ", None))
        self.name_label.setText(QCoreApplication.translate("MainWindow", u" Name", None))
        self.pass_label.setText(QCoreApplication.translate("MainWindow", u" Password", None))
        self.level_label.setText(QCoreApplication.translate("MainWindow", u" Difficulty", None))
        self.number_label.setText(QCoreApplication.translate("MainWindow", u" Number of Rounds", None))
        self.time_label.setText(QCoreApplication.translate("MainWindow", u" Time per Round", None))
        self.join_groupbox.setTitle(QCoreApplication.translate("MainWindow", u"Join a Room ", None))
        self.join_password_field.setText("")
        self.join_btn.setText(QCoreApplication.translate("MainWindow", u"JOIN", None))
        self.join_room_name_field.setText("")
        self.name_2_label.setText(QCoreApplication.translate("MainWindow", u" Name", None))
        self.pass_2_label.setText(QCoreApplication.translate("MainWindow", u" Password", None))
        self.list_of_rooms_label.setText(QCoreApplication.translate("MainWindow", u" List of Rooms ", None))
        self.waiting_label.setText(QCoreApplication.translate("MainWindow", u" waiting for the game to start... ", None))
        self.start_btn.setText(QCoreApplication.translate("MainWindow", u"START", None))
        self.word_label.setText(QCoreApplication.translate("MainWindow", u"_ _ _ _ _ _ _", None))
        self.rounds1_label.setText(QCoreApplication.translate("MainWindow", u" ROUNDS", None))
        self.time1_label.setText(QCoreApplication.translate("MainWindow", u" TIME", None))
        self.letter_input.setText("")
        self.rounds2_label.setText(QCoreApplication.translate("MainWindow", u"0/0 ", None))
        self.time2_label_2.setText(QCoreApplication.translate("MainWindow", u"0:00 ", None))
        self.leave_room_btn.setText(QCoreApplication.translate("MainWindow", u"LEAVE", None))
        self.ranking_label.setText(QCoreApplication.translate("MainWindow", u" RANKING ", None))
        self.player0_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.player1_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.player2_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.player3_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.player4_label.setText(QCoreApplication.translate("MainWindow", u"TextLabel", None))
        self.send_letter_btn.setText(QCoreApplication.translate("MainWindow", u"OK", None))
        self.final_ranking_label.setText(QCoreApplication.translate("MainWindow", u" Final Ranking ", None))
        self.all_words_label.setText(QCoreApplication.translate("MainWindow", u"Words", None))
        self.restart_btn.setText(QCoreApplication.translate("MainWindow", u"PLAY AGAIN", None))
        self.end_back_btn.setText(QCoreApplication.translate("MainWindow", u"BACK", None))
        self.waitroom_back_btn.setText(QCoreApplication.translate("MainWindow", u"BACK", None))
        pass