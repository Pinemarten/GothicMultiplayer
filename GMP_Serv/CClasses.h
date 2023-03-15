
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CCLASSES_HPP_INCLUDED
#define CCLASSES_HPP_INCLUDED

#define STR 0
#define DEX 1
#define MP 2
#define HP 3
#define WEP1H 4
#define WEP2H 5
#define BOW 6
#define XBOW 7
#define MLVL 8
#define SNEAK 9
#define LOCKPICKING 10
#define ACROBATICS 11
#define PICKPOCKET 12

#include <vector>

struct SClass {
public:
  unsigned char class_id;
  char *name;
  char *description;
  char *team_name;
  char *armor;
  char *prim_wep;
  char *sec_wep;
  short abilities[13];
};

class CClassManager {
private:
  unsigned char num_of_classes;

public:
  SClass *class_array = nullptr;
  CClassManager(void);
  ~CClassManager(void);
};

#endif  // CCLASSES_HPP_INCLUDED
