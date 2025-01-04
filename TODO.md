## TODO:

### Obsługa komend (strona serwera):
#### Klient -> Serwer:

- [x] 01\\[nick] - przypisanie nicku  
- [x] 02\\[nazwa,hasło,poziom,rundy,czas] - utworzenie nowego pokoju o podanej nazwie 
- [x] 03\\[nazwa,haslo] - dołączenie do pokoju o podanej nazwie
- [x] 06\\[litera] - próba zgadnięcia litery
- [x] 07\\[pokój] - restart gry
- [x] 08\\[pokój] - podanie aktualnego stanu gry
- [x] 09\\[pokój] - opuszczenie pokoju przez gracza

- [x] 70\\[] - prośba o przesłanie listy pokoi
- [x] 71\\[] - prośba o przesłanie listy nicków graczy obecnych w danym pokoju
- [x] 73\\[] - prośba o rozpoczęcie gry w pokoju gracza


#### Serwer -> Klient:
- [x] 01\\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [x] 02\\[bool] - potwierdzenie stworzenia pokoj
- [x] 03\\[bool] - potwierdzenie dołączenia do pokoju
- [x] 05\\[szyfrowane_hasło] - start gry

- [x] 70\\[nazwa1,..,nazwa99] - przesłanie listy nazw pokoi
- [x] 71\\[bool,nazwa1,..,nazwa99] - przesłanie listy nazw graczy w danym pokoju
- [x] 72\\[bool] - pozwolenie na rozpoczęcie gry (0 jeżeli gracze opuszczą waitroom jeszcze zanim zacznie się gra i jest <2)
- [x] 73\\[zaszyfrowane_hasło] - sygnał do przejścia do ekranu głównego gry

