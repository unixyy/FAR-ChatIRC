
# FAR-ChatIRC

This project aims to create a messaging application entirely in C. 
It takes place within the framework of the IG3 FAR course at Polytech Montpellier.
It is composed of a multitude of sprint aiming at progressively improving the number and quality of the functionalities.

Start the server :
- Compilation : gcc -o server server.c funcServ.c
- Execution : ./server NUMPORT

Start a client :
- Compilation : gcc -o client client.c funcCli.c
- Execution : ./client IPSERVER NUMPORT

Created by Jason, Richard and Thomas.



################################################################
################################################################

TODO :
- 2: Ajout d’un sémaphore indiquant le nombre de place restante sur le serveur pour faciliter le remplacement de client et assurer un nombre de client maximum (  Utiliser la bibliothèque sys/sem.h : exemple ici ! )
- 3: Gestion des signaux (Ctrl+C) client et serveur
- 4: Ajout d’une variable partagée pour une fermeture propre des threads lors de la déconnexion des clients et la connexion de nouveau clients
- 5: Synchronisation des threads des clients terminés (coté serveur pthread kill du thread zombie)
- 6: Nettoyage et optimisation code (mettre correctement les /n sur l'affichage du client)

    |
    |
    v

- Sémpahore pour waiting dans l'accept client
- Au Ctrl+C serveur on cloture les clients (avec messages) avec revc == 0 coté client 
- thread de netoyage coté serveur qui clear les thread zombies
- Revoir /n dans les messages envoyés


################################################################
################################################################
