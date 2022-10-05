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
// definition TAILLE_PLATEAU
#define TAILLE_PLATEAU 10

// plateau (matrice caré de longueur TAILLE_PLATEAU)
char plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];

void viderBuffer()
{
  int c = 0;
  while (c != '\n' && c != EOF)
  {
    c = getchar();
  }
}

// fonction initialisation du plateau
void init_plateau()
{
  int i, j;
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      plateau[i][j] = '~';
    }
  }
}

// fonction affichage du plateau
void affiche_plateau()
{
  //on clear le terminal
  system("clear");

  int i, j;
  printf("  ");
  for (i = 0; i < TAILLE_PLATEAU; i++)
  {
    if (i != 0)
    {
      // on utilise l'assci pour afficher les lettres
      printf("%c ", i + 64);
    }
    for (j = 0; j < TAILLE_PLATEAU; j++)
    {
      if (i == 0)
      {
        printf("%d ", j);
      }
      else
      {
        printf("%c ", plateau[i][j]);
      }
    }
    printf("\n");
  }
}

// fonction placement des bateaux
void placement()
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
  //string message d'erreur 
  char message[100];
  for (i = 0; i < 5; i++)
  {
    ok = 0;
    sorti = 0;
    while (ok == 0 || sorti != 0)
    {
      affiche_plateau();
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
          if (y + taille_bateau[i] -1> TAILLE_PLATEAU)
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
          if (x - taille_bateau[i] +1< 0)
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
              if (plateau[y - j][x] != '~')
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
              if (plateau[y + j][x] != '~')
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
              if (plateau[y][x + j] != '~')
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
              if (plateau[y][x - j] != '~')
              {
                sprintf(message, "bateau chevauche un autre bateau");
                ok = 0;
              }
            }
          }
        }
      }
    }
    //on suprime le message d'erreur
    sprintf(message, "");
    // on place le bateau representer par un 'B'
    if (direction == 'H')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        plateau[y - j][x] = 'B';
      }
    }
    else if (direction == 'B')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        plateau[y + j][x] = 'B';
      }
    }
    else if (direction == 'D')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        plateau[y][x + j] = 'B';
      }
    }
    else if (direction == 'G')
    {
      for (j = 0; j < taille_bateau[i]; j++)
      {
        plateau[y][x - j] = 'B';
      }
    }
  }
}

int main()
{
  init_plateau();
  placement();
  affiche_plateau();
  return 0;
}