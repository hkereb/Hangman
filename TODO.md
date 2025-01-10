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
- [x] 80\\[] - prośba o nową rundę (minął czas)


#### Serwer -> Klient:
- [x] 01\\[bool] - potwierdzenie ustawienia nicku (pętla while z podawaniem nicku do skutku powinna być po stronie klienta, więc potrzebna jest flaga), bool zapisany jako 0 lub 1
- [x] 02\\[bool] - potwierdzenie stworzenia pokoj
- [x] 03\\[bool] - potwierdzenie dołączenia do pokoju
- [x] 06\\[bool,litera,<życia>] - odpowiedź na próbę zgadnięcia litery (życia jeżeli bool = 0)
- [x] 11\\[] - update pozostałego czasu rundy
- [x] 12\\[] - wyczerpał się czas przewidziany na rundę 
- [x] 69\\[hello!] - potwierdzenie nawiązania połączenia
- [x] 70\\[nazwa1,..,nazwa99] - przesłanie listy nazw pokoi
- [x] 71\\[nazwa1,..,nazwa99] - przesłanie listy nazw graczy w danym pokoju
- [x] 72\\[bool] - pozwolenie na rozpoczęcie gry (0 jeżeli gracze opuszczą waitroom jeszcze zanim zacznie się gra i jest <2)
- [x] 73\\[zaszyfrowane_hasło,czas,rundy,twój_nick,lista_przeciwników] - sygnał do rozpoczęcia nowej rundy (gdy pierwszy raz to przejście do game page)
- [x] 74\\[nick] - klient o podanym nicku opuścił rozgrywkę w jej trakcie
- [x] 75\\[hasło] - update info o haśle
- [x] 76\\[nick:życia] - update wisielca przeciwnika
- [x] 77\\[nick:punkty] - update rankingu
- [x] 78\\[nick:punkty,nick:punkty;słowo1,słowo2] - koniec gry
- [x] 79\\[zaszyfrowane_hasło,runda,runda_max] - nowa runda
