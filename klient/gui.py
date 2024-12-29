import sys
from ui_skeleton import Ui_MainWindow
from PySide6.QtWidgets import QApplication, QMainWindow

class MainApp(QMainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # ustawienie strony startowej
        self.ui.stackedWidget.setCurrentWidget(self.ui.nick_page)

        # tu logika
        self.ui.nick_submit_btn.clicked.connect(self.submit_nick)

    def submit_nick(self):
        nick = self.ui.nick_field.text()
        print(f"Nick submitted: {nick}")