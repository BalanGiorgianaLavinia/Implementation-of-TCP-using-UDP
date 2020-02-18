----------------Balan Giorgiana-Lavinia--------------------
-----------------------321 CB------------------------------
---------------------Tema 1 PC-----------------------------


        In implementarea acestei teme m-am folosit de strctura de lista simplu 
inlantuita pentru a stoca in Sender mesajele pe care le trimite catre Receiver. 
Am ales aceasta structura pentru a avea posibilitatea stergerii din lista a 
mesajelor pentru care s-a primit ACK, putand alege apoi mai simplu ce mesaje 
trebuie trimise. Astfel am redus timpul de executie al buclei respective, 
fara sa mai iterez si sa fac verificari pentru toate mesajele.

        De asemenea, am creat o noua structura, framework, care va reprezenta, 
de fapt, payload-ul mesajului, msg, pentru a putea stoca mai usor toate detaliile 
pentru un mesaj. Astfel, noua structura, framework, are campuri pentru indexul 
mesajului, lungimea lui, payload(pentru care am recalculat lungimea, 
PAYLOAD_FRAME_SIZE) si campurile crc_length, crc_index si crc_payload pe care 
le calculez initial pentru fiecare mesaj, urmand ca mai apoi in recv sa verific 
daca mesajul a fost corupt sau nu.


-----------struct.h----------


        Am declarat structura List, noua structura framework, functiile utile 
pentru lista, precum si functiile de calculare si verificare a checksum-ului 
pentru fiecare camp din payload-ul mesajului, intr-un fisier nou, struct.h. 


        Functiile implementate in struct.h sunt:
    -> pushList care primeste o lista, o informatie si un index si realizeaza 
inserarea unei noi celule (mesaj) cu informatia si indexul date in lista.

    -> removeList care primeste o lista si un index si sterge din lista elementul 
(celula) care are indexul egal cu cel dat.

    -> getFromList care primeste o lista si un index si intoarce de aceasta data 
elementul (celula) care are campul index egal cu cel cautat sau NULL in cazul 
in care nu se gaseste in lista.

    -> crc care primeste o informatie si o lungime si returneaza codul de 
verficare pentru informatie, realizand un xor pe octetii informatiei.

    -> check_crc care primeste o structura de tip framework, recalculeaza 
checksum-ul pentru fiecare camp (length, index si payload) si daca cel putin 
unul din ele nu se potiveste cu campul calculat initial corespunzator, inseamna 
ca mesajul a fost corupt.




-------send.c------------


        Fisierul send.c contine implementarea pentru Sender astfel:

    In primul rand, am calculat toate valorile necesare, precum, 
dimensiunea fisierului cu ajutorul lui lseek, numarul total de pachete/mesaje 
la care am mai adaugat 2 (pentru mesajul cu numele fisierului si un alt mesaj 
cu numarul de mesaje ce trebuiesc primite in total de receiver) si fereastra 
(formulele din laborator).

    In al doilea rand, am initializat o lista simplu inlantuita in care urmeaza 
sa introduc mesajele pe masura ce le preiau din fisier. Primul mesaj este cel 
cu numele fisierului, iar urmatorul cu numarul de mesaje total. Odata preluate 
si completate campurile corespunzatoare lor, trimit aceste mesaje, care acum 
se afla in lista de mesaje pentru care se asteapta ACK. Apoi, pentru 
restul mesajelor, verific daca fereastra este plina. In cazul in care este plina, 
astept un ACK un anumit TIMEOUT. Daca a sosit un ACK, preiau din el indexul 
corespunzator mesajului care a fost primit cu succes si sterg acest mesaj din 
lista de mesaje in asteptare. Daca nu primesc un ACK in timp util(adica TIMEOUT), 
inseamna ca un mesaj a fost pierdut. Asadar, fie ca am prmit ACK pentru un mesaj, 
fie ca nu (adica s-a pierdut unul), preiau un alt mesaj, ii setez campurile si 
il trimit. Fac acest lucru pana cand am trimis pe retea toate cele COUNT mesaje.

    La finalul acestei operatiuni, inca nu am siguranta ca toate mesajele 
trimise au fot livrate cu succes, de aceea va trebui sa verific lista de 
mesaje in asteptare. Cat timp mai sunt mesaje in ea, astept un ACK, sterg mesajul 
corespunzator si il trimit pe urmatorul, pana cand lista devine vida, adica 
am primit ACK pentru toate mesajele trimise.

    La final, voi astepta inca un mesaj de finalizare de la recv pentru a stii 
cand sa ies din program, astefel inca si recv sa aiba timp sa isi scrie 
mesajele in fisier.


----------recv.c------------

        In recv.c, am alocat doi vectori, unul pentru a retine in el mesajele 
primite cu succes ( adica fara sa fie corupte ) de la sender in ordinea buna 
( adica la indecsii corespunzatori pe care ii preiau din payload-ul mesajului ) 
si unul pe care il completez cu 0 sau 1, 0 insemnand ca mesajul de pe indexul 
respectiv nu a ajuns (sau cel putin nu a ajuns corect/necorupt), iar 1 
insemnand ca a fost primit cu succes.

        Apoi urmeaza o bucla while in care voi astepta mesaje de la sender.
Cum nu stiu de la inceput cate mesaje urmeaza sa primesc de la sender, 
am initializat o variabila loop cu o valoare foarte mare si un numar curent 
de mesaje primite cu 0. Astfel, cat timp nu am primit toate mesajele, astept. 
        In momentul in care primesc un mesaj, il verific daca este corect/
necorupt. Daca este corupt, astept in continuare, iar daca nu este corupt si
este prima data cand primesc mesajul cu acest index, il marchez ca primit in 
vectorul de primite ( received ) si cresc numarul curent de mesaje primite. 
De asemenea, il stochez in vectorul de mesaje ce urmeaza la final sa le scriu 
in fisierul de iesire si trimit un mesaj de ACK la sender cu indexul sau. 
        Pe parcursul primirii mesajelor, verific daca a ajuns numele fisierului, 
stiind acest lucru dupa indexul 0 al mesajului sau daca a ajuns numarul de mesaje 
pe care trebuie sa le primeasc recv in total. Atunci cand primesc count-ul, 
initializez variabila loop cu aceasta noua valoare pentru a stii cand sa ies 
din bucla.

        La finalul recv-ului, scriu mesajele in fisier, golesc memoria alocata, 
inchid fisierul deschis pentru scriere si trimit mesajul de final ( cu length -8)
pentru a stii sender-ul sa incheie transmisia.

------------------------------------------------------------------------------

        Mentionez, ca pe masina locala primesc constant 85 de puncte, toate 
testele trec cu succes.

-------------------------------------------------------------------------------