
const bool pfont[45][4][3] =
{
 {
    {1,1,1},
    {0,1,1},
    {1,0,1},
    {1,1,1}
 },
 {
    {1,0,0},
    {1,1,1},
    {1,0,1},
    {1,1,1}
 },
 {
    {1,1,1},
    {1,0,0},
    {1,0,0},
    {1,1,1}
 },
 {
    {0,0,1},
    {1,1,1},
    {1,0,1},
    {1,1,1}
 },
 {
    {1,1,1},
    {1,0,1},
    {1,1,0},
    {1,1,1}
 },
 {
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {1,0,0}
 },
 {
    {1,1,1},
    {1,0,1},
    {0,1,1},
    {1,1,1}
 },
 {
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
 },
 {
    {1,1,1},
    {0,1,0},
    {0,1,0},
    {1,1,1}
 },
 {
    {0,0,1},
    {0,0,1},
    {0,0,1},
    {1,1,1}
 },
 {
    {1,0,1},
    {1,1,0},
    {1,0,1},
    {1,0,1}
 },
 {
    {1,0,0},
    {1,0,0},
    {1,0,0},
    {1,1,1}
 },
 {
    {1,1,1},
    {1,1,1},
    {1,0,1},
    {1,0,1}
 },
 {
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,0,1}
 },
 {
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
 },
 {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {1,0,0}
 },
 {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {0,0,1}
 },
 {
    {1,1,1},
    {1,0,0},
    {1,0,0},
    {1,0,0}
 },
 {
    {1,1,1},
    {1,0,0},
    {0,0,1},
    {1,1,1}
 },
 {
    {1,1,1},
    {0,1,0},
    {0,1,0},
    {0,1,0}
 },
 {
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
 },
 {
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {0,1,0}
 },
 {
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {1,1,1}
 },
 {
    {1,0,1},
    {0,1,0},
    {1,0,1},
    {1,0,1}
 },
 {
    {1,0,1},
    {1,0,1},
    {0,1,0},
    {0,1,0}
 },
 {
    {1,1,1},
    {0,0,1},
    {1,0,0},
    {1,1,1}
 },

 ////Extra characters
 {//question mark
    {1,1,1},
    {0,0,1},
    {1,1,1},
    {1,0,0}
 },
  {//comma
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {1,1,0}
 },
  {//full stop
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,1,0}
 },
   {//semicolon
    {0,0,0},
    {0,1,0},
    {0,0,0},
    {1,1,0}
 },
   {//exclamation mark
    {0,1,0},
    {0,1,0},
    {0,0,0},
    {0,1,0}
 },
 {//carriage return = 31
 },
{//colon
{0,0,0},
{0,1,0},
{0,0,0},
{0,1,0}
},
{//hyphon
{0,0,0},
{0,0,0},
{1,1,1},
{0,0,0}
},
//NUMBERS
{//0
{1,1,1},
{1,0,1},
{1,0,1},
{1,1,1}
},
{//1
{0,1,0},
{0,1,0},
{0,1,0},
{0,1,0}
},
{//2
{1,1,0},
{0,0,1},
{1,0,0},
{1,1,1}
},
{//3
{1,1,1},
{0,1,1},
{0,1,1},
{1,1,1}
},
{//4
{0,0,1},
{0,1,1},
{1,1,1},
{0,0,1}
},
{//5
{1,1,1},
{1,0,0},
{0,0,1},
{1,1,0}
},
{//6
{1,0,0},
{1,1,1},
{1,0,1},
{1,1,1}
},
{//7
{1,1,1},
{0,0,1},
{0,0,1},
{0,0,1}
},
{//8
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1}
},
{//9
{1,1,1},
{1,0,1},
{1,1,1},
{0,0,1}
},//#44: apostrophe
{
{0,1,0},
{0,1,0},
{0,0,0},
{0,0,0}
}
};

/*********
* Text Routines
*
**********/

void print_straight_letter(int i)
{
  glPushMatrix();
  for (int j = 0; j < 4; j++)
    {
      glPushMatrix();
      for (int k = 0; k < 3; k++)
        {
          if (pfont[i][j][k])
            {
              glBegin(GL_POLYGON);
              glVertex2f(-0.03, 0.03);
              glVertex2f(0.03, 0.03);
              glVertex2f(0.03, -0.03);
              glVertex2f(-0.03, -0.03);
              glEnd();
            }
          glTranslatef(0.07f, 0.0f, 0.00f);
        }
      glPopMatrix();
      glTranslatef(0.0f, -0.07f, 0.00f);
    }
  glPopMatrix();
}


void print_straight_text(const string& s)
{
  glPushMatrix();
  glScalef(0.2, 0.2, 0.2);
  int c = 0;

  int i = 0;
  glPushMatrix();                                 //preserve left coord
  glPushMatrix();                                 //start first line
  while (s[i] != 0)
    {
      if (s[i] == 42)                             //newline
        {
          c = 0;
          glPopMatrix();
          glTranslatef(0.0f, -0.07f * 6, 0);
          glPushMatrix();
        }
      else
        {
          if (chartonum(s[i]) >= 0)
            print_straight_letter(chartonum(s[i]));
          glTranslatef(0.07f * 4, 0.0f, 0.00f);
        }
      i++;
      c++;
    }
  glPopMatrix();
  glPopMatrix();
  glPopMatrix();
}

void printletter(int i)
{
  glPushMatrix();
  for (int j = 0; j < 4; j++)
    {
      glPushMatrix();
      for (int k = 0; k < 3; k++)
        {
          if (pfont[i][j][k])
            {
              glBegin(GL_QUADS);

              glVertex2f(-0.03 + (0.0025f * ((rand() % 3) - 1)), 0.03 + (0.0025f * ((rand() % 3) - 1)));
              glVertex2f(0.03 + (0.0025f * ((rand() % 3) - 1)), 0.03 + (0.0025f * ((rand() % 3) - 1)));
              glVertex2f(0.03 + (0.0025f * ((rand() % 3) - 1)), -0.03 + (0.0025f * ((rand() % 3) - 1)));
              glVertex2f(-0.03 + (0.0025f * ((rand() % 3) - 1)), -0.03 + (0.0025f * ((rand() % 3) - 1)));
              glEnd();
            }
          glTranslatef(0.07f, 0.0f, 0.00f);
        }
      glPopMatrix();
      glTranslatef(0.0f, -0.07f, 0.00f);
    }
  glPopMatrix();
}


int chartonum(char c)
{
  if ((c >= 97) && (c <= 122))
    return c - 97;
  else if ((c >= 64) && (c <= 57))
    return(26 + c - 48);
  else if (c == 63)                               //question mark;
    return 26;
  else if (c == 44)                               //comma
    return 27;
  else if (c == 46)                               //full stop
    return 28;
  else if (c == 59)                               //semi-colon
    return 29;
  else if (c == 33)                               //exclamation mark
    return 30;
  else if (c == 42)
    return 31;
  else if (c == 58)                               //colon
    return 32;
  else if (c == 45)
    return 33;                                                                            //hyphen
  else if (c >= 48 && c <= 57)
    return 34 + c - 48;
  else if (c == '\'')
    return 44;
  else return -1;
}


void print_text(const char *s)
{
  glPushMatrix();
  glColor3fv(c_text);

  glScalef(0.4f, 0.4f, 1);

  glTranslatef(-2.5 + 0.275f, 2.5 - 0.3f, 0);

  int c = 0;

  int i = 0;
  glPushMatrix();                                     //preserve left coord
  glPushMatrix();                                         //start first line
  while (s[i] != 0)
    {
      if (s[i] == 42)                                     //newline
        {
          c = 0;
          glPopMatrix();
          glTranslatef(0.0f, -0.07f * 6, 0);
          glPushMatrix();
        }
      else
        {
          if (c > 15)
            {
              c = 0;
              glPopMatrix();
              glTranslatef(0.0f, -0.07f * 6, 0);
              glPushMatrix();
            }
          if (chartonum(s[i]) >= 0)
            printletter(chartonum(s[i]));
          glTranslatef(0.07f * 4, 0.0f, 0.00f);
        }
      i++;
      c++;
    }
  glPopMatrix();
  glPopMatrix();
  glPopMatrix();
  //can't remember why I need three...doesn't work properly otherwise though :/
}


void print_text2(const char *s)
{
  glPushMatrix();
  glColor3fv(c_text);

  glScalef(0.4f, 0.4f, 1);

  int c = 0;

  int i = 0;
  glPushMatrix();                                 //preserve left coord
  glPushMatrix();                                 //start first line
  while (s[i] != 0)
    {
      if (s[i] == 42)                             //newline
        {
          c = 0;
          glPopMatrix();
          glTranslatef(0.0f, -0.07f * 6, 0);
          glPushMatrix();
        }
      else
        {
          if (c > 15)
            {
              c = 0;
              glPopMatrix();
              glTranslatef(0.0f, -0.07f * 6, 0);
              glPushMatrix();
            }
          if (chartonum(s[i]) >= 0)
            printletter(chartonum(s[i]));
          glTranslatef(0.07f * 4, 0.0f, 0.00f);
        }
      i++;
      c++;
    }

  if (i < 176)
    {
      if (c > 15)
        {
          c = 0;
          glPopMatrix();
          glTranslatef(0.0f, -0.07f * 6, 0);
          glPushMatrix();
        }
    }

  if (rand() % 5 == 0)
    {
      glBegin(GL_QUADS);
      glVertex2f(-0.02, 0.06);
      glVertex2f(0.02, 0.06);
      glVertex2f(0.02, -0.27);
      glVertex2f(-0.02, -0.27);
      glEnd();
    }

  glPopMatrix();
  glPopMatrix();
  glPopMatrix();
}
