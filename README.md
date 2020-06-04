Projet PR6 
===========

### OPTIONS DE TRACES ###

Il y a 3 options possibles :

1. afficher les traces dans la console
2. écrire les traces dans un fichier texte généré par notre code et nommé par l'heure à laquelle on le crée
3. les ignorer complètement

Dans le premier cas, il faut que le dossier contienne un fichier `trace`. Pour le générer il faut exécuter la commande
	$ touch trace	

Dans le deuxième cas, le dossier doit contenir un fichier `trace2`. Pour le générer, il faut exécuter la commande
	$ touch trace2

Pour la troisième option, il n'est pas nécessaire de générer de nouveau fichier

* Ces commandes sont à exécuter avant le lancement du programme 


### COMPILATION ###

Nous avons créé un fichier Makefile, pour lancer le programme, il faut donc exécuter les commandes 
	$ make
	$ ./dazibao

Si on lance la commande `./dazibao` sans autres arguments, le protocole se connectera au serveur `jch.irif.fr`. Pour se connecter à un autre serveur, il faut l'entrer comme argument de la commande `/dazibao`.