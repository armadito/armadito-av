<!--- process with 'markdown TODO-version-linux.md' to produce HTML, if you want -->

TODO version Linux
==================

Objectif: avoir une version Linux fonctionnelle pour début septembre 2015, pour installation à la GN en site pilote mi-septembre 2015


Fonctionnalités
---------------

Cette version permettra:

- le scan à la demande, depuis l'interface utilisateur ou depuis nautilus
- le scan, automatique avec ou sans confirmation de l'utilisateur, de médias amovibles lors du montage

Le mode 'temps réel', c.à.d. scan sur accès et blocage de l'ouverture d'un fichier, n'est pas à priori prévu.

L'interface utilisateur sera minimaliste et accessible uniquement via le systray (ou via nautilus pour l'analyse d'un fichier ou d'un répertoire):

- scan à la demande
- accès aux analyses passées
- accès aux dates de mise à jour des bases antivirales
- information sur l'état de l'antivirus (agents d'analyse, de surveillance, d'alerte, de mise à jour...)
- information sur l'état du système (historique de détection, etc)


Architecture
------------

La version est architecturée autour:

- de modules d'analyse chargés dynamiquement
- d'une bibliothèque de gestion des modules d'analyse
- d'applications (ligne de commande ou GUI) implémentées avec la bibliothèque

L'interface graphique est construire avec Qt (versions 4 ou 5).


TODO
----

### TODO général ###

- merge des dépôts git pour avoir un seul dépôt
- passage à gitlab teclib


### TODO bibliothèque ###

#### configuration ####

Actuellement, fichiers de config à la .ini windows, chargés par le parseur de la glib. Inconvénient: on ne peut pas avoir plusieurs fois la même variable de configuration, ce qui pose problème pour la déclaration des associations mime-type -> module d'analyse.

TODO:

- voir si on peut utiliser tel quel le parseur glib, sinon en refaire un très simple en utilisant l'analyseur lexical de la glib
- changer l'interface de configuration des modules: une fonction par variable de configuration, une table exportée par la structure module
- implémenter la gestion des associations mime-type -> module d'analyse dans la configuration

Priorité: haute


#### libmagic ####

Utilisation de la libmagic pour déterminer le mime-type d'un fichier. Problème: la libmagic n'est pas thread-safe, ne fonctionne peut être pas sur windows

TODO:

- utiliser les fonctions de la glib pour déterminer le mime-type (g_content_type_guess). Problème: devine mal de type mime d'un binaire exécutable

Priorité: moyenne


#### remontée d'information sur les bases antivirales et mise à jour ####

- mettre en place dans les modules d'analyse des fonctions de retour d'information: liste des bases, date de mise à jour, nombre de signatures etc
- ajouter les IPC pour récupérer les informations

Priorité: haute


#### comptage des fichiers ####

- déplacer le comptage des fichiers dans scan.c, parcourir une seule fois l'arborescence pour le compte de fichiers, mémoriser les chemins dans un tableau (mémoire?), ne pas reparcourir l'arborescence pour le scan??? à voir

Priorité: haute


### TODO modules ###

#### TODO module clamav ####

- maj configuration
- ajout de la remontée d'information sur les bases antivirales

Priorité: haute


#### TODO module 5.2 ####

- maj module 5.2 % changements Ulrich
- maj configuration
- ajout de la remontée d'information sur les bases antivirales

Priorité: haute


#### TODO module PDF ####

- à écrire à partir de la bibliothèque d'analyse

Priorité: moyenne


### TODO GUI ###

Préambule: reprendre la version Qt et faire les mises à jour minimales pour que ça compile et marche


#### internationalisation ####

- refaire le boulot d'i18n de la version GTK...


#### interface ####

- ajouter notification
- ajouter tooltip avec version et dernière date de mise à jour (idem C#)
- màj dialogue de scan: à voir
- fixer problème d'affichage de l'icône systray


#### extension nautilus ####

- ajouter une extension nautilus qui envoie un message D-bus à l'interface graphique pour scanner un dossier ou un fichier
