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

#define CHECK(status, message) if (status == -1 ) { perror(message); exit(-1); }

// definition TAILLE_PLATEAU
#define TAILLE_PLATEAU 10


typedef struct 
{ 
  char (*plateau)[][TAILLE_PLATEAU];
  char message[100];
} t_corps;


//st boite au lettre
typedef struct requete
{
  long type;
  t_corps lettre;
} t_requete;



//declaration des fonctions
void viderBuffer();
void init_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void affiche_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void placement(char (*plateau)[][TAILLE_PLATEAU]);
void jouer();
void menu();


int boite;

void viderBuffer()
{
  int c = 0;
  while (c != '\n' && c != EOF)
  {
    c = getchar();
  }
}


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

void envoieMessage(int idBoite, t_requete *message)
{
  int ret;
  ret = msgsnd(idBoite, message, sizeof(t_requete), 0);

  if (ret == -1)
  {
    perror("msgsnd");
    exit(1);
  }
}




// fonction initialisation du plateau
void init_plateau(char (*plateau)[][TAILLE_PLATEAU])
{
  int i, j;
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      (*plateau)[i][j] = '~';
    }
  }
}

// fonction affichage du plateau
void affiche_plateau(char (*plateau)[][TAILLE_PLATEAU])
{
  // on clear le terminal
  system("clear");

  int i, j;
  printf("  ");
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    if (i != 0)
    {
      // on utilise l'assci pour afficher les lettres
      printf("\033[32m");

      printf("%c ", i + 64);
      printf("\033[00m");
    }
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      if (i == 0)
      {
        printf("\033[32m");

        printf("%d ", j);
        printf("\033[00m");
      }
      else
      {
        printf("\033[34m");

        printf("%c ", (*plateau)[i][j]);
        printf("\033[00m");
      }
    }
    printf("\n");
  }
}

// fonction placement des bateaux
void placement(char (*plateau)[][TAILLE_PLATEAU])
{
  // tableau nom des batteaux
  char nom_bateau[5][20] = {"porte-avion", "croiseur", "contre-torpilleur", "sous-marin", "torpilleur"};
  // tableau taille des bateaux
  int taille_bateau[5] = {5, 4, 3, 3, 2};

  int i, j;
  int x;
  char y;
  char direction;
  int ok;
  int sorti;
  // string message d'erreur
  char message[100];
  //on initialise le message d'erreur
  sprintf(message, " ");
  for (i = 0; i < 5; i++)
  {
    ok = 0;
    sorti = 0;
    while (ok == 0 || sorti != 0)
    {
      sorti = 0;
      affiche_plateau(plateau);
      printf("\033[31m");
      printf("%s\n", message);
      printf("\033[00m");
      printf("placement du %s (taille %d)\n", nom_bateau[i], taille_bateau[i]);

      printf("x : ");
      scanf("%d", &x);
      viderBuffer();
      printf("y : ");
      scanf("%c", &y);
      viderBuffer();
      // on transfosrme y en int
      y = y - 64;

      printf("direction Haut, Bas, Droite ou Gauche (H,B,D,G): ");
      scanf("%c", &direction);
      viderBuffer();
      // on verifie que les données sont correctes
      if (x >= 0 && x < TAILLE_PLATEAU && y >= 0 && y < TAILLE_PLATEAU && (direction == 'H' || direction == 'B' || direction == 'D' || direction == 'G'))
      {
        ok = 1;
      }
      else
      {

        sprintf(message, "données incorrectes");
      }
      if (ok == 1)
      {
        // on verifie que bateau ne sort pas du tableau
        if (direction == 'H')
        {
          if (y - taille_bateau[i] < 0)
          {
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
            sprintf(message, "bateau sort du tableau");
            sorti = 1;
          }
          else
          {
            ok = 1;
          }
        }
        // on verifi que le bateau ne chevauche pas un autre bateau
        if (ok == 1 && sorti == 0)
        {
          if (direction == 'H')
          {
            for (j = 0; j < taille_bateau[i]; j++)
            {
              if ((*plateau)[y - j][x] != '~')
              {
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
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
        }
      }
    }





    // on suprime le message d'erreur
    sprintf(message, " ");
    // on place le bateau representer par un 'B'
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

// fonction qui génère une clé aléatoire de 5 caractères
int genererCle()
{
  int cle = 0;
  int i;
  for (i = 0; i < 5; i++)
  {
    cle = cle * 10 + rand() % 10;
  }
  // vérifier que cette clé n'est pas déjà présente dans le fichier cle.txt
  FILE *fichier = fopen("cle.txt", "r");
  int cleFichier;
  int ok = 1;
  while (fscanf(fichier, "%d", &cleFichier) != EOF)
  {
    if (cleFichier == cle)
    {
      ok = 0;
    }
  }
  fclose(fichier);
  if (ok == 0)
  {
    return genererCle();
  }
  else
  {
    return cle;
  }
}


void handler(int sig)
{
  printf("Signal %d reçu !\n", sig);
  printf("Adversaire trouvé !\n");

  //on cree la boite aux lettres
  int cle = getpid();
  boite = msgget(cle, IPC_CREAT | 0666);
  if (boite == -1)
  {
    perror("msgget");
    exit(1);
  }
  infoBoite(boite);



  printf("Appuyez sur entrée pour continuer\n");
  viderBuffer();
  system("clear");

  jouer();
}


// Création de la partie et envoie de la clé dans une boîte au lettre
void creer_partie(){
  struct sigaction newact_fils;
  printf("\nCréation de la partie ! \n\n");
  int cle = getpid();
  printf("Votre clé est : %d\n\n", cle);
  printf("-> Envoyez cette clé à votre adversaire pour qu'il rejoigne ! \n\n");

  // ecrire dans le fichier cle.txt
  FILE *fichier = NULL;
  fichier = fopen("cle.txt", "a");
  fprintf(fichier, "%d\n", cle);
  fclose(fichier);
  
  // attente de l'adversaire
  printf("--En attente de l'adversaire--\n");
  // Attendre jusqu'à la réception du signal SIGUSR1
  sigemptyset(&newact_fils.sa_mask);
  newact_fils.sa_flags = 0;
  newact_fils.sa_handler = handler;
  sigaction(SIGUSR1, &newact_fils, NULL);
  sigsuspend(&newact_fils.sa_mask);


  printf("Adversaire trouvé !\n");
  

  printf("Appuyez sur entrée pour continuer\n");
  viderBuffer();
  system("clear");

  jouer();
}

void rejoindre_partie(){
  printf("\nRejoindre une partie ! \n\n");
  int cle = 0;

  printf("Entrez la clé de la partie : ");
  scanf("%d", &cle);

  // vérifier que la clé est présente dans le fichier cle.txt
  FILE *fichier = fopen("cle.txt", "r");
  int cleFichier;
  int ok = 0;
  while (fscanf(fichier, "%d", &cleFichier) != EOF)
  {
    if (cleFichier == cle)
    {
      ok = 1;
      // sortir de la boucle
      break;
    }
  }
  fclose(fichier);

  if (ok == 1)
  {
    // envoie d'un signal de confirmation SIGALARM à la fonction creer_partie
    kill(cle, SIGUSR1);
    printf("La partie est prête !\n");
    printf("Appuyez sur entrée pour continuer\n");
    viderBuffer();
    system("clear");
    jouer();
  }
  else
  {
    printf("La partie n'existe pas !\n");
    printf("Appuyez sur entrée pour continuer\n");
    viderBuffer();
    system("clear");
    menu();
  }
}

// fonction jouer qui place les bateau et lance la partie
void jouer()
{
  // Placement des bateaux
  char plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];
  init_plateau(&plateau);
  placement(&plateau);

  //quand le bateau est placé on attend que l'adversaire place ses bateaux
  printf("En attente de l'adversaire...\n");
  




}

// fonction menu
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


int main()
{
  system("clear");  // on affiche en vert
  printf("\033[32m");
  printf("Bienvenue dans la bataille navale\n");
  printf("\033[00m");
  menu();

  return 0;
}