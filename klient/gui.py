from ui_skeleton import Ui_MainWindow
from PySide6.QtWidgets import QApplication, QMainWindow

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

    #     # Dodaj swoją logikę tutaj
    #     self.ui.nick_submit_btn.clicked.connect(self.submit_nick)
    #
    # def submit_nick(self):
    #     nick = self.ui.nick_field.text()
    #     print(f"Nick submitted: {nick}")