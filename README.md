# Tema 4 Protocoale de Comunicatii
### Luca Anastasia, 322CC

Scopul temei a fost de a construi un program asemenea unui client care accepta comenzi de la tastatura si trimite in functie de comanda cereri catre server, functionand ca o interfata in linia de comanda ca o biblioteca virtuala. Limbajul in care am implementat aceasta tema: C. 
Mentiune: Am folosit partial scheletul din laboratorul 9 (fisierele requests.c, helpers.c, buffer.c)

Manipularea functiilor are loc in fisierul **client.c**, unde in dependenta de comanda citita se indeplinesc respectivele cerinte. In caz ca din stdin se citeste o comanda gresita se arunca o eroare.

In fisierul **commands.c** am implementat pentru fiecare comanda cate o functie care realizeaza request-uri mentionate in tema in functie de ceea ce face: POST, GET sau DELETE. Scurt despre utilitatea functiilor:

- **void register_user(int sockfd);**
Se inregistreaza inputul din stdin in variabilele username si password, mai apoi ca sa fie adaugate cu ajutorul functiilor din **parson.c** in obiecte JSON si inseriate intr-un string ca sa fie trimis catre server (cu asta se ocupa functiile din requests.c). De la server primim pe prima linie un cod ce descrie statusul operatiunii (2XX - Succes si 4XX - Client Error sunt cele folosite). In dependenta de status code am printat mesaje de SUCCESS sau ERROR.

- **void login_user(int sockfd);**
Analog cu register, totusi aceasta operatiune intoarce si un cookie activ pe care il extragem din raspunsul serverului din header-ul "Set-Cookie". Acest cookie retinut in variabila globala pe parcursul functiilor va folosi ca demonstratie ca un utilizator este logat. De asemenea, analizand raspunsul serverului printez mesaje de eroare sau succes.

- **void enter_library(int sockfd);**
Aceasta functie nu mai citeste nimic din stdin, ci ofera acces la o biblioteca virtuala, doar daca utilizatorul este logat. In cazul cand nu e logat (nu este prezent niciun cookie) intoarce eroare. Atunci cand utilizatorul este logat, i se ofera acces la biblioteca, iar serverul trimite in raspuns la sfarsit un token in format JSON care l-am extras. Acest token va servi ca demonstratie pentru urmatoarele operatiuni care fac modificari asupra bibliotecii.

- **void get_books(int sockfd);**
Efectuand operatiile necesare si tratand erorile mentionate, aceasta functie afiseaza in format JSON obiectele carti ce le contine parsate de server.

- **void get_book(int sockfd);**
Aceasta functie citeste de la tastatura id-ul cartii pentru a cere de la server informatii despre aceasta carte. Se efectueaza in ordine aceleasi operatii ca in toate functiile: se deschide conexiunea socket, se trimite cerere (POST | GET | DELETE), se trimite catre server cu informatiile si payload-ul necesar, se primeste raspuns de la server si se analizeaza si extrage ce e necesar. Folosind functiile din biblioteca parson pentru pretty print am afisat informatiile cartii.

- **void add_book(int sockfd);**
Se citesc de la tastatura informatiile pentru carte (title, author, genre, publisher, page number) in care se pot introduce orice caracter pana la tasta enter. Daca macar unul din campuri nu este completat, se afiseaza eroare. De asemenea, numarul de pagini este citit ca sir de caractere, astfel ca iterand prin el sa verific prin intermediul functiei isdigit, daca se gaseste un caracter care sa nu fie numar sa se intoarca eroare. Ulterior, daca parola respecta tipul necesar se converteste in integer. Informatiile despre carte se adauga intr-un obiect JSON serializat ce va fi transmis catre server pentru ca el sa adauge cartea in lista de carti din biblioteca.

- **void delete_book(int sockfd)**
Aceasta comanda foloseste DELETE request. Se indica id-ul cartii pentru care utilizatorul vrea sa o elimine din biblioteca. In caz ca nu se gaseste acest id in lista de carti, serverul intoarce un mesaj de eroare 4XX si afisez eroarea.

- **void logout_user(int sockfd);**
In momentul cand se indeplineste comanda de delogare cookie-ul si token-ul se reseteaza pentru o functionalitate corecta. Respectiv se trateaza eroarea daca incerci sa te deloghezi nefiind logat.

La baza tuturor acestor functii sta apelul functiilor din fisierul **requests.c**, luat din laborator, unde am implementat 3 functii pentru fiecare tip de request (POST | GET | DELETE). Aceste functii creeaza formatul specific pentru a-l trimite catre server (headere. cookies, token cand sunt necesare). Si de asemenea **helpers.c** pentru interactiunea cu serverul.
