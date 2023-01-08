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
  char message[100];
  int x;
  int y;
} t_corps;


//st boite au lettre
typedef struct requete
{
  long type;
  t_corps corps;
} t_requete;



//declaration des fonctions
void viderBuffer();
void init_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void affiche_plateau(char (*plateau)[][TAILLE_PLATEAU]);
void placement(char (*plateau)[][TAILLE_PLATEAU]);
void jouer();
void menu();


int boite;
int player;//1 pour le createur de la partie et 2 pour le joueur qui rejoint la partie


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
    char nom_bateau[2][20] = {"porte-avion", "croiseur"};

  //char nom_bateau[5][20] = {"porte-avion", "croiseur", "contre-torpilleur", "sous-marin", "torpilleur"};
  // tableau taille des bateaux
    int taille_bateau[2] = {5, 4};

  //int taille_bateau[5] = {5, 4, 3, 3, 2};

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
    //for (i = 0; i < 5; i++)
    for (i = 0; i < 1; i++)

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
  printf("Adversaire trouvé !\n");

  //on cree la boite aux lettres
  int cle = getpid();
  boite = msgget(cle, IPC_CREAT | 0666);
  if (boite == -1)
  {
    perror("msgget");
    exit(1);
  }



  
  system("clear");

  jouer();
}


// Création de la partie et envoie de la clé dans une boîte au lettre
void creer_partie(){
  player = 1;
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
  player = 2;
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
    // on cree la boite aux lettres
    boite = msgget(cle, IPC_CREAT | 0666);
    if (boite == -1)
    {
      perror("msgget");
      exit(1);
    }

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


// Créer une fonction de vérification de victoire. Elle renvoie 1 si le joueur a gagné, 0 sinon.
int detection_victoire(char (*plateau)[][TAILLE_PLATEAU]){
  int i, j;
  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
    {
      if ((*plateau)[i][j] == 'B')
      {
        return 0;
      }
    }
  }
  return 1;
}
int letter_to_number(char c) {
  // Vérifiez que c est une lettre majuscule de A à I
  if (c >= 'A' && c <= 'I') {
    // Transformez la lettre en un chiffre en la soustrayant à 'A'
    return c - 'A';
  } else {
    // Si c n'est pas une lettre majuscule de A à I, renvoyez -1
    return -1;
  }
}

// fonction jouer qui place les bateau et lance la partie
void jouer()
{
  // Placement des bateaux
  char plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];
  init_plateau(&plateau);
  placement(&plateau);
  affiche_plateau(&plateau);

  //quand le bateau est placé on attend que l'adversaire place ses bateaux
  printf("En attente de l'adversaire...\n");
  //on envoie un message pour dire que le joueur a fini de placer ses bateaux
  t_corps corps;
  strcpy(corps.message, "placement");
  t_requete requete;
  requete.type = player;
  requete.corps = corps;
  msgsnd(boite, &requete, sizeof(t_corps), 0);

  //on attend que l'adversaire ait fini de placer ses bateaux
  t_requete requeteRecue;
  //on veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
  int type = (player == 1) ? 2 : 1;
  msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
  printf("L'adversaire a fini de placer ses bateaux !\n");

  system("clear");
  printf("La partie commence !\n");

  int partie = 1;
  int tonTour = player;
  //plateau de l'adversaire
  char plateauAdversaire[TAILLE_PLATEAU][TAILLE_PLATEAU];
  init_plateau(&plateauAdversaire);

  while (partie)
  {
    if(tonTour == 1){
      //on affiche le plateau de l'adversaire
      printf("Plateau de l'adversaire\n");
      affiche_plateau(&plateauAdversaire);
      //on demande au joueur de tirer
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

      //A = 0 et Z = 9
      int y = letter_to_number(y_c);

      //on regarde si le tire est valide 
      if(x < 0 || x > 9 || y < 0 || y > 9){
        printf("Tir invalide !\n");
        printf("Appuyez sur entrée pour continuer\n");
        viderBuffer();
        system("clear");
        continue;
      }
      //on regarde si le tire est déjà effectué
      if(plateauAdversaire[y+1][x] == 'X' || plateauAdversaire[y][x+1] == 'O'){
        printf("Tir déjà effectué !\n");
        printf("Appuyez sur entrée pour continuer\n");
        viderBuffer();
        system("clear");
        continue;
      }
      //on envoie le tire à l'adversaire
      t_corps corps;

      strcpy(corps.message, "tir");
      corps.x = x;
      corps.y = y;
            
      t_requete requete;
      requete.type = player;
      requete.corps = corps;
      msgsnd(boite, &requete, sizeof(t_corps), 0);
      t_requete requeteRecue;
      //on veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
      int type = (player == 1) ? 2 : 1;
      msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
      //on regarde si le tire est un touché ou un coulé
      if(strcmp(requeteRecue.corps.message, "touche") == 0){
        printf("Touché !\n");
        strcpy(meg, "Touché !");
        //on met un X sur le plateau de l'adversaire
        plateauAdversaire[y+1][x] = 'X';
      }
      else if (strcmp(requeteRecue.corps.message, "rate") == 0){
        printf("Raté !\n");
        strcpy(meg, "Raté !");
        //on met un O sur le plateau de l'adversaire
        plateauAdversaire[y+1][x] = 'O';
      }
      else {
        printf("Partie terminée vous avez gagné!\n");
        partie = 0;
        exit(0);
      }
      //on passe le tour à l'adversaire
      tonTour = 2;
      affiche_plateau(&plateauAdversaire);
      printf("%s\n", meg);
      printf("Appuyez sur entrée pour continuer\n");
      viderBuffer();
      
    }
    else {  
      affiche_plateau(&plateau);
      printf("En attente du tir de l'adversaire...\n");
      //on est la partie adverse
      //on attend le tire de l'adversaire

      t_requete requeteRecue;
      //on veux que le type de la requette soit 1 si player = 2 et 2 si player = 1
      int type = (player == 1) ? 2 : 1;
      msgrcv(boite, &requeteRecue, sizeof(t_corps), type, 0);
      char meg[100];

      //on regarde si le tire est un touché ou un coulé
      if(strcmp(requeteRecue.corps.message, "tir") == 0){
               
        //on affiche le tire de l'adversaire
        printf("Tir de l'adversaire : %d %d\n", requeteRecue.corps.y+1, requeteRecue.corps.x);
        
        if(plateau[requeteRecue.corps.y+1][requeteRecue.corps.x] == 'B'){
          printf("Touché !\n");
          strcpy(meg, "Touché !");
          //on verifi si il reste des bateaux
          
          //on met un X sur le plateau de l'adversaire
          plateau[requeteRecue.corps.y+1][requeteRecue.corps.x] = 'X';

          int result =  detection_victoire(&plateau);
          if(result == 1){
            
            printf("Partie terminée vous avez perdu!\n");
            t_corps corps;
            strcpy(corps.message, "finito");
            t_requete requete;
            requete.type = player;
            requete.corps = corps;
            msgsnd(boite, &requete, sizeof(t_corps), 0);
            partie = 0;
            exit(0);
          }

          //on envoie un message touché à l'adversaire
          t_corps corps;
          strcpy(corps.message, "touche");
          corps.x = requeteRecue.corps.x;
          corps.y = requeteRecue.corps.y;
          t_requete requete;
          requete.type = player;
          requete.corps = corps;
          msgsnd(boite, &requete, sizeof(t_corps), 0);
        }
        
        else if(plateau[requeteRecue.corps.y+1][requeteRecue.corps.x] == '~'){
          printf("Raté !\n");
          strcpy(meg, "Raté !");
          //on met un O sur le plateau de l'adversaire
          plateau[requeteRecue.corps.y+1][requeteRecue.corps.x] = 'O';
          //on envoie un message raté à l'adversaire
          t_corps corps;
          strcpy(corps.message, "rate");
          corps.x = requeteRecue.corps.x;
          corps.y = requeteRecue.corps.y;
          t_requete requete;
          requete.type = player;
          requete.corps = corps;
          msgsnd(boite, &requete, sizeof (t_corps), 0);
        }
        tonTour = 1;
        //on attend la touche entrée pour continuer
        affiche_plateau(&plateau);
        printf("%s\n", meg);
        printf("Appuyez sur entrée pour continuer\n");
        viderBuffer();

      } 
      
    }
  }







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