/*
Nom : navale.c
Auteur : Julien Zamit et Baptiste Blomme
Date : 05/10/2022
Description : Jeu de bataille navale deux joueurs
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CHECK(status, message) \
  if (status == -1)            \
  {                            \
    perror(message);           \
    exit(-1);                  \
  }

// definition TAILLE_PLATEAU
#define TAILLE_PLATEAU 10

typedef struct
{
  char message[100];
  int x;
  int y;
} t_corps;

// st boite au lettre
typedef struct requete
{
  long type;
  t_corps corps;
} t_requete;

// declaration des fonctions
void viderBuffer();
void init_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void affiche_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void placement(char (*plateau)[][TAILLE_PLATEAU]);
void jouer();
void menu();

int boite;
int player; // 1 pour le createur de la partie et 2 pour le joueur qui rejoint la partie

/**
 * @brief Vide le tampon d'entrée standard
 */
void viderBuffer()
{
  // Initialise la variable c à 0
  int c = 0;
  // Tant que c n'est pas un retour à la ligne ou la fin du fichier, lire un caractère depuis l'entrée standard
  while (c != '\n' && c != EOF)
  {
    c = getchar();
  }
}

/**
 * Affiche des informations sur une boîte de message dont l'identifiant est donné en paramètre.
 *
 * @param idBoite L'identifiant de la boîte de message.
 */
void infoBoite(int idBoite)
{
  struct msqid_ds boite;
  int ret;
  ret = msgctl(idBoite, IPC_STAT, &boite);
  if (ret == -1)
  {
    perror("msgctl");
    exit(1);
  }
  printf("ID de la boite : %d\n", boite.msg_perm.__key);
  printf("UID de la boite : %d\n", boite.msg_perm.uid);
  printf("GID de la boite : %d\n", boite.msg_perm.gid);
  printf("UID de la boite : %d\n", boite.msg_perm.cuid);
  printf("GID de la boite : %d\n", boite.msg_perm.cgid);
  printf("Mode de la boite : %d\n", boite.msg_perm.mode);
  printf("Nombre de messages dans la boite : %ld\n", boite.msg_qnum);
  printf("Taille maximale de la boite : %ld\n", boite.msg_qbytes);
  printf("PID du dernier processus ayant envoyé un message : %d\n", boite.msg_lspid);
  printf("PID du dernier processus ayant reçu un message : %d\n", boite.msg_lrpid);
  printf("Temps de la dernière opération sur la boite : %ld\n", boite.msg_stime);
}

/**
 * Fonction : envoieMessage
 * Cette fonction envoie un message de type t_requete dans la boite aux lettres
 * identifiee par idBoite.
 * idBoite : identifiant de la boite aux lettres dans laquelle envoyer le message
 * message : pointeur vers le message a envoyer
 * Valeur de retour : aucune
 */
void envoieMessage(int idBoite, t_requete *message)
{
  // Envoi du message dans la boite aux lettres
  int ret;
  ret = msgsnd(idBoite, message, sizeof(t_requete), 0);
  // Verification des erreurs d'envoi
  if (ret == -1)
  {
    perror("msgsnd");
    exit(1);
  }
}

/*
 * Fonction : init_plateau
 * ------------------------
 * Initialise chaque case du plateau de jeu à la valeur '~'.
 *
 * plateau: Pointeur vers le plateau de jeu à initialiser.
 */
void init_plateau(char (*plateau)[][TAILLE_PLATEAU])
{
  int i, j;

  // Pour chaque ligne du plateau
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    // Pour chaque colonne du plateau
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      // On initialise la case à '~'
      (*plateau)[i][j] = '~';
    }
  }
}

/*
 * Fonction : affiche_plateau
 * --------------------------
 * Affiche le plateau de jeu avec une légende pour les numéros de colonne et les lettres de ligne.
 * Les couleurs sont utilisées pour mettre en valeur la légende et les cases du plateau.
 *
 * plateau: Pointeur vers le plateau de jeu à afficher.
 */
void affiche_plateau(char (*plateau)[][TAILLE_PLATEAU])
{
  // On efface le contenu du terminal
  system("clear");

  int i, j;

  // On affiche une légende vide pour les numéros de ligne
  printf("  ");

  // Pour chaque ligne du plateau
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    // Si ce n'est pas la première ligne (qui sert à afficher les numéros de colonne)
    if (i != 0)
    {
      // On active la couleur verte pour afficher la lettre de la ligne
      printf("\033[32m");

      // On affiche la lettre (en utilisant l'ASCII)
      printf("%c ", i + 64);

      // On réactive la couleur par défaut
      printf("\033[00m");
    }

    // Pour chaque colonne du plateau
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      // Si c'est la première ligne (qui sert à afficher les numéros de colonne)
      if (i == 0)
      {
        // On active la couleur verte pour afficher le numéro de la colonne
        printf("\033[32m");

        // On affiche le numéro de la colonne
        printf("%d ", j);

        // On réactive la couleur par défaut
        printf("\033[00m");
      }
      // Sinon (ce n'est pas la première ligne)
      else
      {
        // On active la couleur bleue pour afficher la valeur de la case
        printf("\033[34m");

        // On affiche la valeur de la case
        printf("%c ", (*plateau)[i][j]);

        // On réactive la couleur par défaut
        printf("\033[00m");
      }
    }
    // On passe à la ligne suivante
    printf("\n");
  }
}

/*
 * Fonction : placement
 * ---------------------
 * Demande à l'utilisateur de placer chaque bateau sur le plateau de jeu, en vérifiant que la position choisie est valide (ne sort pas du plateau et ne chevauche pas un autre bateau).
 *
 * plateau: Pointeur vers le plateau de jeu sur lequel placer les bateaux.
 */
void placement(char (*plateau)[][TAILLE_PLATEAU])
{
  // Tableau contenant les noms des bateaux
  char nom_bateau[5][20] = {"porte-avion", "croiseur", "contre-torpilleur", "sous-marin", "torpilleur"};

  // Tableau contenant les tailles des bateaux
  int taille_bateau[5] = {5, 4, 3, 3, 2};

  int i, j;
  int x;
  char y;
  char direction;
  int ok;
  int sorti;

  // Message d'erreur à afficher en cas de données incorrectes
  char message[100];
  // On initialise le message d'erreur
  sprintf(message, " ");

  // Pour chaque bateau
  for (i = 0; i < 5; i++)
  {
    // On initialise les variables de contrôle
    ok = 0;
    sorti = 0;

    // Tant que les données saisies par l'utilisateur sont incorrectes ou que le bateau sort du plateau
    while (ok == 0 || sorti != 0)
    {
      sorti = 0;

      // On affiche le plateau de jeu
      affiche_plateau(plateau);

      // On affiche le message d'erreur en rouge
      printf("\033[31m");
      printf("%s\n", message);
      printf("\033[00m");

      // On demande à l'utilisateur de saisir les coordonnées et la direction du bateau
      printf("placement du %s (taille %d)\n", nom_bateau[i], taille_bateau[i]);
      printf("x : ");
      scanf("%d", &x);
      viderBuffer();
      printf("y : ");
      scanf("%c", &y);
      viderBuffer();
      // On transforme y en int
      y = y - 64;
      printf("direction Haut, Bas, Droite ou Gauche (H,B,D,G): ");
      scanf("%c", &direction);
      viderBuffer();
      // Si les données saisies par l'utilisateur sont correctes
      if (x >= 0 && x < TAILLE_PLATEAU && y >= 0 && y < TAILLE_PLATEAU && (direction == 'H' || direction == 'B' || direction == 'D' || direction == 'G'))
      {
        ok = 1;
      }
      // Si les données sont incorrectes
      else
      {
        // On affiche un message d'erreur
        sprintf(message, "données incorrectes");
      }

      // Si les données sont correctes
      if (ok == 1)
      {
        // On vérifie que le bateau ne sort pas du plateau
        if (direction == 'H')
        {
          if (y - taille_bateau[i] < 0)
          {
            // On affiche un message d'erreur
            sprintf(message, "bateau sort du tableau");
            sorti = 1;
          }
          else
          {
            ok = 1;
          }
        }
        else if (direction == 'B')
        {
          if (y + taille_bateau[i] - 1 > TAILLE_PLATEAU)
          {
            // On affiche un message d'erreur
            sprintf(message, "bateau sort du tableau");
            sorti = 1;
          }
          else
          {
            ok = 1;
          }
        }
        else if (direction == 'D')
        {
          if (x + taille_bateau[i] > TAILLE_PLATEAU)
          {
            // On affiche un message d'erreur
            sprintf(message, "bateau sort du tableau");
            sorti = 1;
          }
          else
          {
            ok = 1;
          }
        }
        else if (direction == 'G')
        {
          if (x - taille_bateau[i] + 1 < 0)
          {
            // On affiche un message d'erreur
            sprintf(message, "bateau sort du tableau");
            sorti = 1;
          }
          else
          {
            ok = 1;
          }
        }

        // Si les données sont correctes et que le bateau ne sort pas du plateau
        if (ok == 1 && sorti == 0)
        {
          // On vérifie que le bateau ne chevauche pas un autre bateau
          if (direction == 'H')
          {
            for (j = 0; j < taille_bateau[i]; j++)
            {
              if ((*plateau)[y - j][x] != '~')
              {
                // On affiche un message d'erreur
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
          else if (direction == 'B')
          {
            for (j = 0; j < taille_bateau[i]; j++)
            {
              if ((*plateau)[y + j][x] != '~')
              {
                // On affiche un message d'erreur
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
          else if (direction == 'D')
          {
            for (j = 0; j < taille_bateau[i]; j++)
            {
              if ((*plateau)[y][x + j] != '~')
              {
                // On affiche un message d'erreur
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
          else if (direction == 'G')
          {
            for (j = 0; j < taille_bateau[i]; j++)
            {
              if ((*plateau)[y][x - j] != '~')
              {
                // On affiche un message d'erreur
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
        }
      }
    }
  }

  // Si les données sont correctes et que le bateau ne sort pas du plateau et ne chevauche pas un autre bateau
  if (ok == 1 && sorti == 0)
  {
    // On place le bateau sur le plateau
    if (direction == 'H')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        (*plateau)[y - j][x] = 'B';
      }
    }
    else if (direction == 'B')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        (*plateau)[y + j][x] = 'B';
      }
    }
    else if (direction == 'D')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        (*plateau)[y][x + j] = 'B';
      }
    }
    else if (direction == 'G')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        (*plateau)[y][x - j] = 'B';
      }
    }
  }
}

/**
 * genererCle - Génère une clé aléatoire de 5 chiffres et vérifie qu'elle n'est pas déjà présente dans le fichier cle.txt
 *
 * @return : La clé générée
 */
int genererCle()
{
  // On initialise la variable qui contiendra la clé générée
  int cle = 0;
  // On génère la clé en concaténant 5 nombres aléatoires compris entre 0 et 9
  int i;
  for (i = 0; i < 5; i++)
  {
    cle = cle * 10 + rand() % 10;
  }

  // On ouvre le fichier qui contient les clés existantes en mode lecture
  FILE *fichier = fopen("cle.txt", "r");

  // On déclare une variable pour stocker chaque clé lue dans le fichier
  int cleFichier;

  // On déclare une variable pour indiquer si la clé générée est déjà présente dans le fichier ou non
  int ok = 1;

  // On lit chaque clé du fichier jusqu'à la fin du fichier
  while (fscanf(fichier, "%d", &cleFichier) != EOF)
  {
    // Si la clé générée est déjà présente dans le fichier
    if (cleFichier == cle)
    {
      // On indique que la clé générée est déjà présente dans le fichier
      ok = 0;
    }
  }

  // On ferme le fichier
  fclose(fichier);

  // Si la clé générée est déjà présente dans le fichier
  if (ok == 0)
  {
    // On génère une nouvelle clé
    return genererCle();
  }
  // Si la clé générée n'est pas déjà présente dans le fichier
  else
  {
    // On renvoie la clé générée
    return cle;
  }
}

/**
 * handler - Fonction exécutée lorsque le signal SIGUSR1 est reçu
 *
 * @sig: Le numéro du signal reçu
 */
void handler(int sig)
{
  printf("Adversaire trouvé !\n");

  // On crée la boîte aux lettres en utilisant le PID du processus comme clé
  int cle = getpid();
  boite = msgget(cle, IPC_CREAT | 0666);
  // Si la création de la boîte aux lettres échoue
  if (boite == -1)
  {
    // On affiche un message d'erreur
    perror("msgget");
    // On quitte le programme
    exit(1);
  }

  // On efface le contenu de la console
  system("clear");

  // On lance la fonction de jeu
  jouer();
}

/**
 * creer_partie - Crée une nouvelle partie de bataille navale et attend l'adversaire
 */
void creer_partie()
{
  // On définit le joueur comme étant le joueur 1
  player = 1;

  // On déclare une structure sigaction pour la gestion des signaux
  struct sigaction newact_fils;

  printf("\nCréation de la partie ! \n\n");

  // On récupère le PID du processus courant
  int cle = getpid();
  printf("Votre clé est : %d\n\n", cle);
  printf("-> Envoyez cette clé à votre adversaire pour qu'il rejoigne ! \n\n");

  // On ouvre le fichier qui contient les clés des parties en mode ajout
  FILE *fichier = NULL;
  fichier = fopen("cle.txt", "a");

  // On ajoute la clé de la partie dans le fichier
  fprintf(fichier, "%d\n", cle);

  // On ferme le fichier
  fclose(fichier);

  // On affiche un message indiquant qu'on attend l'adversaire
  printf("--En attente de l'adversaire--\n");

  // On définit la fonction à exécuter lors de la réception du signal SIGUSR1
  sigemptyset(&newact_fils.sa_mask);
  newact_fils.sa_flags = 0;
  newact_fils.sa_handler = handler;
  sigaction(SIGUSR1, &newact_fils, NULL);

  // On suspend le processus jusqu'à la réception du signal SIGUSR1
  sigsuspend(&newact_fils.sa_mask);

  // On affiche un message indiquant que l'adversaire a été trouvé
  printf("Adversaire trouvé !\n");

  // On demande à l'utilisateur d'appuyer sur entrée pour continuer
  printf("Appuyez sur entrée pour continuer\n");
  viderBuffer();

  // On efface le contenu de la console
  system("clear");

  // On lance la fonction de jeu
  jouer();
}

/**
 * rejoindre_partie - Permet à l'utilisateur de rejoindre une partie de bataille navale en entrant la clé de cette partie
 */
void rejoindre_partie()
{
  // On définit le joueur comme étant le joueur 2
  player = 2;

  printf("\nRejoindre une partie ! \n\n");

  // On déclare une variable pour stocker la clé de la partie
  int cle = 0;

  // On demande à l'utilisateur de saisir la clé de la partie
  printf("Entrez la clé de la partie : ");
  scanf("%d", &cle);

  // On ouvre le fichier qui contient les clés des parties en mode lecture
  FILE *fichier = fopen("cle.txt", "r");

  // On déclare une variable pour stocker les clés du fichier
  int cleFichier;

  // On déclare une variable pour savoir si la clé saisie par l'utilisateur est présente dans le fichier
  int ok = 0;

  // On parcourt le fichier ligne par ligne
  while (fscanf(fichier, "%d", &cleFichier) != EOF)
  {
    // Si la clé du fichier correspond à la clé saisie par l'utilisateur, on met la variable ok à 1 et on sort de la boucle
    if (cleFichier == cle)
    {
      ok = 1;
      break;
    }
  }

  // On ferme le fichier
  fclose(fichier);

  if (ok == 1)
  {
    // Si la clé saisie par l'utilisateur est présente dans le fichier, on envoie un signal de confirmation (SIGUSR1) au processus ayant la clé de la partie
    kill(cle, SIGUSR1);

    // On crée la boîte aux lettres associée à la clé de la partie
    boite = msgget(cle, IPC_CREAT | 0666);
    if (boite == -1)
    {
      perror("msgget");
      exit(1);
    }

    // On affiche un message indiquant que la partie est prête
    printf("La partie est prête !\n");

    // On demande à l'utilisateur d'appuyer sur entrée pour continuer
    printf("Appuyez sur entrée pour continuer\n");

    // On vide le buffer d'entrée
    viderBuffer();

    // On efface l'écran
    system("clear");

    // On appelle la fonction jouer()
    jouer();
  }
  else
  {
    // Si la clé saisie par l'utilisateur n'est pas présente dans le fichier, on affiche un message d'erreur
    printf("La partie n'existe pas !\n");

    // On demande à l'utilisateur d'appuyer sur entrée pour continuer
    printf("Appuyez sur entrée pour continuer\n");

    // On vide le buffer d'entrée
    viderBuffer();

    // On efface l'écran
    system("clear");

    // On appelle la fonction menu()
    menu();
  }
}

/**
 * detection_victoire() vérifie s'il reste encore des bateaux ennemis sur le plateau de jeu.
 *
 * @param plateau: le plateau de jeu en cours
 * @return: 0 si il reste des bateaux ennemis sur le plateau, 1 sinon
 */
int detection_victoire(char (*plateau)[][TAILLE_PLATEAU])
{
  // On parcourt le plateau de jeu ligne par ligne
  int i, j;
  for (i = 0; i < 10; i++)
  {
    // Pour chaque ligne, on parcourt les colonnes
    for (j = 0; j < 10; j++)
    {
      // Si on trouve un bateau ennemi (représenté par 'B') sur le plateau, la fonction renvoie 0
      if ((*plateau)[i][j] == 'B')
      {
        return 0;
      }
    }
  }
  // Si on a parcouru tout le plateau sans trouver de bateau ennemi, la fonction renvoie 1
  return 1;
}

/**
 * letter_to_number() convertit une lettre majuscule de A à I en son équivalent numérique.
 *
 * @param c: la lettre à convertir
 * @return: le chiffre correspondant à la lettre (de 0 à 8) si la lettre est valide, -1 sinon
 */
int letter_to_number(char c)
{
  // Vérifiez que c est une lettre majuscule de A à I
  if (c >= 'A' && c <= 'I')
  {
    // Transformez la lettre en un chiffre en la soustrayant à 'A'
    return c - 'A';
  }
  else
  {
    // Si c n'est pas une lettre majuscule de A à I, renvoyez -1
    return -1;
  }
}

/**
 * @brief Fonction qui permet de vider le buffer
 * Cette fonction gère la partie de bataille navale d'un joueur.
 * Elle commence par initialiser le plateau du joueur et afficher ce plateau vide.
 * Ensuite, le joueur doit placer ses bateaux sur son plateau et cette fonction affiche de nouveau le plateau une fois que les bateaux ont été placés.
 * Ensuite, le joueur envoie un message à l'adversaire indiquant qu'il a fini de placer ses bateaux.
 * Ensuite, le joueur attend que l'adversaire ait fini de placer ses bateaux.
 * Une fois que l'adversaire a fini de placer ses bateaux, la partie commence véritablement et le joueur peut commencer à jouer.
 * Le joueur doit entrer les coordonnées de l'emplacement où il souhaite tirer, et cette fonction vérifie si le tir est valide (c'est-à-dire s'il est dans les limites du plateau et s'il n'a pas déjà été effectué).
 * Si le tir est valide, le joueur envoie un message à l'adversaire avec les coordonnées du tir.
 * Ensuite, le joueur attend une réponse de l'adversaire qui indique si le tir a touché ou manqué un bateau de l'adversaire.
 * Si le tir a touché un bateau, cette fonction met à jour le plateau de l'adversaire en mettant un "X" sur l'emplacement du tir.
 * Si le tir a manqué un bateau, cette fonction met à jour le plateau de l'adversaire en mettant un "O" sur l'emplacement du tir.
 * La partie continue jusqu'à ce qu'un joueur ait coulé tous les bateaux de l'autre joueur.
 * Alors la fonction affiche un message indiquant qui a gagné et la partie se termine.
 * Résumé : jouer() permet de jouer au jeu de la bataille navale en mode multijoueur. Elle gère les échanges de messages entre les joueurs pour envoyer les tirs et les résultats de ces tirs, ainsi que la fin de la partie.
 * @param boite: la boîte de messages utilisée pour les échanges entre les joueurs
 * @param player: l'identifiant du joueur courant (1 ou 2)
 */
void jouer()
{
  // Initialisation du plateau du joueur
  char plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];
  init_plateau(&plateau);
  // Affichage du plateau vide
  affiche_plateau(&plateau);
  // Placement des bateaux du joueur sur le plateau
  placement(&plateau);
  // Affichage du plateau une fois que les bateaux ont été placés
  affiche_plateau(&plateau);
  // Envoi d'un message à l'adversaire indiquant que le joueur a fini de placer ses bateaux
  t_corps corps;
  strcpy(corps.message, "placement");
  t_requete requete;
  requete.type = player;
  requete.corps = corps;
  msgsnd(boite, &requete, sizeof(t_t_corps), 0);
  // Attente de la fin du placement des bateaux de l'adversaire
  t_requete requeteRecue;
  // On veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
  int type = (player == 1) ? 2 : 1;
  msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
  printf("L'adversaire a fini de placer ses bateaux !\n");

  // Nettoyage de l'écran et affichage du message indiquant le début de la partie
  system("clear");
  printf("La partie commence !\n");

  int partie = 1;
  int tonTour = player;
  // Initialisation du plateau de l'adversaire
  char plateauAdversaire[TAILLE_PLATEAU][TAILLE_PLATEAU];
  init_plateau(&plateauAdversaire);

  while (partie)
  {
    // Si c'est au tour du joueur
    if (tonTour == 1)
    {
      // Affichage du plateau de l'adversaire
      printf("Plateau de l'adversaire\n");
      affiche_plateau(&plateauAdversaire);
      // Demande au joueur de saisir les coordonnées du tir qu'il souhaite effectuer
      printf("Où voulez-vous tirer ?\n");
      int x;
      char y_c;
      printf("x : ");
      scanf("%d", &x);
      viderBuffer();
      printf("y : ");
      scanf("%c", &y_c);
      viderBuffer();
      char meg[100];
      // Conversion de la lettre en chiffre (A = 0 et Z = 9)
      int y = letter_to_number(y_c);

      // Vérification de la validité du tir
      if (x < 0 || x > 9 || y < 0 || y > 9)
      {
        printf("Tir invalide !\n");
        printf("Appuyez sur entrée pour continuer\n");
        viderBuffer();
        system("clear");
        continue;
      }
      // Vérification que le tir n'a pas déjà été effectué
      if (plateauAdversaire[y + 1][x] == 'X' || plateauAdversaire[y][x + 1] == 'O')
      {
        printf("Tir déjà effectué !\n");
        printf("Appuyez sur entrée pour continuer\n");
        viderBuffer();
        system("clear");
        continue;
      }
      // Envoi du tir à l'adversaire
      t_corps corps;
      strcpy(corps.message, "tir");
      corps.x = x;
      corps.y = y;
      t_requete requete;
      requete.type = player;
      requete.corps = corps;
      msgsnd(boite, &requete, sizeof(t_corps), 0);
      // Attente de la réponse de l'adversaire sur le résultat du tir
      t_requete requeteRecue;
      // On veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
      int type = (player == 1) ? 2 : 1;
      msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
      // Si le tir a touché un bateau de l'adversaire
      if (strcmp(requeteRecue.corps.message, "touché") == 0)
      {
        // Mise à jour du plateau de l'adversaire avec un "X" sur l'emplacement du tir
        plateauAdversaire[y + 1][x] = 'X';
        printf("Touché !\n");
      }
      // Si le tir a manqué un bateau de l'adversaire
      else if (strcmp(requeteRecue.corps.message, "manqué") == 0)
      {
        // Mise à jour du plateau de l'adversaire avec un "O" sur l'emplacement du tir
        plateauAdversaire[y + 1][x] = 'O';
        printf("Manqué !\n");
      }
      // Si le tir a coulé un bateau de l'adversaire
      else if (strcmp(requeteRecue.corps.message, "coulé") == 0)
      {
        // Mise à jour du plateau de l'adversaire avec un "X" sur l'emplacement du tir
        plateauAdversaire[y + 1][x] = 'X';
        printf("Bateau coulé !\n");
      }
      // Si la partie est finie
      else if (strcmp(requeteRecue.corps.message, "fin") == 0)
      {
        // Affichage du message de fin de partie et arrêt de la boucle
        printf("Partie terminée ! Vous avez %s.\n", (requeteRecue.corps.gagnant == player) ? "gagné" : "perdu");
        partie = 0;
      }
      // Nettoyage de l'écran et demande au joueur d'appuyer sur entrée pour continuer
      printf("Appuyez sur entrée pour continuer\n");
      viderBuffer();
      system("clear");
      // Passage au tour de l'adversaire
      tonTour = 2;
    }
    // Si c'est au tour de l'adversaire
    else if (tonTour == 2)
    {
      // Attente d'un tir de l'adversaire
      t_requete requeteRecue;
      // On veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
      int type = (player == 1) ? 2 : 1;
      msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
      // Si l'adversaire a envoyé un message de tir
      if (strcmp(requeteRecue.corps.message, "tir") == 0)
      {
        // Récupération des coordonnées du tir de l'adversaire
        int x = requeteRecue.corps.x;
        int y = requeteRecue.corps.y;
        // Vérification de la validité du tir de l'adversaire (dans les limites du plateau et non déjà effectué)
        if (x < 0 || x > 9 || y < 0 || y > 9 || plateau[y + 1][x] == 'X' || plateau[y][x + 1] == 'O')
        {
          printf("Tir de l'adversaire invalide !\n");
          // Envoi d'un message à l'adversaire indiquant que le tir est invalide
          t_corps corps;
          strcpy(corps.message, "invalide");
          t_requete requete;
          requete.type = player;
          requete.corps = corps;
          msgsnd(boite, &requete, sizeof(t_corps), 0);
          continue;
        }
        // Si le tir de l'adversaire touche un bateau du joueur
        if (plateau[y + 1][x] == 'B')
        {
          // Mise à jour du plateau avec un "X" sur l'emplacement du tir
          plateau[y + 1][x] = 'X';
          // Envoi d'un message à l'adversaire indiquant que le tir a touché un bateau
          t_corps corps;
          strcpy(corps.message, "touché");
          t_requete requete;
          requete.type = player;
          requete.corps = corps;
          msgsnd(boite, &requete, sizeof(t_corps), 0);
        }
        // Si le tir de l'adversaire manque un bateau du joueur
        else
        {
          // Mise à jour du plateau avec un "O" sur l'emplacement du tir
          plateau[y + 1][x] = 'O';
          // Envoi d'un message à l'adversaire indiquant que le tir a manqué un bateau
          t_corps corps;
          strcpy(corps.message, "manqué");
          t_requete requete;
          requete.type = player;
          requete.corps = corps;
          msgsnd(boite, &requete, sizeof(t_corps), 0);
        }
      }
      // Si l'adversaire a envoyé un message de fin de partie
      else if (strcmp(requeteRecue.corps.message, "fin") == 0)
      {
        // Affichage du message de fin de partie et arrêt de la boucle
        printf("Partie terminée ! Vous avez %s.\n", (requeteRecue.corps.gagnant == player) ? "gagné" : "perdu");
        partie = 0;
      }
      // Passage au tour du joueur
      tonTour = 1;
    }
  }
}

/**
 * menu() affiche le menu principal du jeu de la bataille navale en mode multijoueur et gère les choix de l'utilisateur.
 */
void menu()
{

  printf("1 - Créer une partie\n");
  printf("2 - Rejoindre\n");
  printf("3 - Quitter\n");

  printf("Votre choix : ");

  int choix;
  scanf("%d", &choix);

  viderBuffer();

  switch (choix)
  {
  case 1:
    creer_partie();
    break;

  case 2:
    rejoindre_partie();
    break;

  case 3:

    printf("Au revoir\n");
    break;

  default:
    system("clear");
    printf("\033[31m");
    printf("Choix incorrect\n");

    printf("\033[00m");
    menu();
    break;
  }
}

/**
 * Fonction main du programme
 */
int main()
{
  system("clear"); // on affiche en vert
  printf("\033[32m");
  printf("Bienvenue dans la bataille navale\n");
  printf("\033[00m");
  menu();

  return 0;
}