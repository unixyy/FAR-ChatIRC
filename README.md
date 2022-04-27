
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
- 1: Gestion d’erreur coté serveur sur l’existence du destinataire
Gestion pseudo déjà existant
- 2: Amélioration du code et gestion des nouveaux clients.
Si pas déjà fait, ajout d’un mutex pour le tableau des clients. 
Ajout d’un sémaphore indiquant le nombre de place restante sur le serveur pour faciliter le remplacement de client et assurer un nombre de client maximum (  Utiliser la bibliothèque sys/sem.h : exemple ici ! )
Gestion des signaux (Ctrl+C) client et serveur
Ajout d’une variable partagée pour une fermeture propre des threads lors de la déconnexion des clients et la connexion de nouveau clients
Synchronisation des threads des clients terminés
Attention : La bibliothèque de sémaphore n’est pas la même sous MacOs, rappel la cible de l’application est un système Linux, pour corriger lien
- 3: Nettoyage  et optimisation code
Amélioration mise en page client (pseudo de celui qui envoie et si privé ou pas...)

################################################################
################################################################
