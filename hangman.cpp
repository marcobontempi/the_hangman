//==============================================================================
//
// hangman.cpp
//
// version   : 1.0
// topic     : hanged game
// class     : C++ main
// author    : Marco Bontempi
// created   : 02-Jan-2023
//
// abstract:
//  Hangman game project implementation
//
//------------------------------------------------------------------------------
// 2021-2024 by Marco Bontempi <marco.bontempi@gmail.com>.
//
// Redistribution  and   use  in   source  and  binary  forms,  with  or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source  code  must  retain the  above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form  must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in  the documentation
//    and/or other materials provided with the distribution.
// 3. Redistribution in any form  must include the references associated to this
//    code  in  the  documentation  and/or  other materials  provided  with  the
//    distribution.
//
// THIS SOFTWARE  IS PROVIDED BY THE AUTHOR AND  CONTRIBUTORS ``AS IS''  AND ANY
// EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING, BUT  NOT  LIMITED TO, THE IMPLIED
// WARRANTIES  OF  MERCHANTABILITY AND  FITNESS  FOR  A  PARTICULAR PURPOSE  ARE
// DISCLAIMED. IN NO  EVENT  SHALL THE AUTHOR OR CONTRIBUTORS  BE LIABLE FOR ANY
// DIRECT,  INDIRECT,  INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION)  HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY,  OR  TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//==============================================================================

#include "hangman.hpp"
#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <time.h>
#include <cctype>
#include <algorithm>
#include <sstream>
#include "vt100.hpp"

// make a string to lower case
//------------------------------------------------------------------------------
std::string to_lower(std::string str)
{
    std::for_each(str.begin(), str.end(), [](char& ch) -> void { ch = std::tolower(ch); });
    return str;
}

// make a string to upper case
//------------------------------------------------------------------------------
std::string to_upper(std::string str)
{
    std::for_each(str.begin(), str.end(), [](char& ch) -> void { ch = std::toupper(ch); });
    return str;
}

// make a string to upper case
//------------------------------------------------------------------------------
std::string to_large(std::string str)
{
  std::stringstream ans;
  size_t i, len = str.size();
  for(i = 0; i < len; ++i)
  {
    ans << (char)std::toupper(str[i]) << " ";
  }
  return ans.str();
}

// check if the letter is present in the string
//------------------------------------------------------------------------------
bool check_pattern(const char ch, const std::string& str)
{
  size_t i, len = str.size();
  for(i = 0; i < len; ++i)
  {
    if(str[i] == ch)
      return true;
  }
  return false;
}

// check if the letter is present in the string
//------------------------------------------------------------------------------
bool check_solution(const char ch, const std::string& str, std::string& pattern)
{
  bool found = false;
  size_t i, len = str.size();
  for(i = 0; i < len; ++i)
  {
    if(str[i] ==  ch)
    {
      pattern[i] = ch;
      found = true;
    }
  }
  return found;
}

// hangman figure
//------------------------------------------------------------------------------
#define HANGMAN_FRAMES  7
#define HANGMAN_ROWS   14
#define HANGMAN_COLS   14
const char the_hangman[HANGMAN_FRAMES][HANGMAN_ROWS][HANGMAN_COLS] = {
{"             ",   // [0][][]
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             " },
{"   /-----\\   ",    // [1][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             ",    
 "             " },
{"   /-----\\   ",    // [2][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "      |      ",
 "      |      ",
 "      |      ",
 "      |      ",
 "      |      ",
 "             ",
 "             ",
 "             ",
 "             " },
{"   /-----\\   ",    // [3][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "      |      ",
 "     /|      ",
 "    / |      ",
 "   /  |      ",
 "      |      ",
 "             ",
 "             ",
 "             ",
 "             " },
{"   /-----\\   ",    // [4][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "      |      ",
 "     /|\\     ",
 "    / | \\    ",
 "   /  |  \\   ",
 "      |      ",
 "             ",
 "             ",
 "             ",
 "             " },
{"   /-----\\   ",    // [5][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "      |      ",
 "     /|\\     ",
 "    / | \\    ",
 "   /  |  \\   ",
 "      |      ",
 "     /       ",
 "    /        ",
 "   /         ",
 "  /          " },
{"   /-----\\   ",    // [6][][]
 "   | . . |   ",    
 "   |  o  |   ",
 "   | >-< |   ",
 "   \\-----/   ",
 "      |      ",
 "     /|\\     ",
 "    / | \\    ",
 "   /  |  \\   ",
 "      |      ",
 "     / \\     ",
 "    /   \\    ",
 "   /     \\   ",
 "  /       \\  " }};

 // letter list
 char letter_list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// initialization
//------------------------------------------------------------------------------
hangman::hangman()
: _error_condition(0), _players(0), _hang_status(0), _solution(), _pattern(), _version("1.1")
{;}

// gui
//------------------------------------------------------------------------------
void hangman::Welcome() const 
{
  if(_error_condition == 0)
  {
    std::cout << '\n';  
    std::cout << "    * * * * * * * * * * * * * * * * * * * *\n";  
    std::cout << "    *                                     *\n";  
    std::cout << "    *    T H E  H A N G M A N  G A M E    *\n";
    std::cout << "    *                                     *\n";  
    std::cout << "    * * * * * * * * * * * * * * * * * * * *\n";  
    std::cout << '\n';
    std::cout << "    vesion " << _version << '\n';
    std::cout << '\n';
    std::cout << "    by Marco & Samuele Bontempi     \n";
    std::cout << std::endl;
  }
}

// set/get
//------------------------------------------------------------------------------
void hangman::AskPlayers()
{
    if(_error_condition == 0)
    {
      std::cout << "\n\n";
      std::cout << "    Enter the number of players: ";
      std::cin >> _players;
    }
}

//------------------------------------------------------------------------------
void hangman::InitWord()
{
  if(_error_condition == 0)
  {
    // read the list of words
    std::ifstream fi("dictionary.txt");
    if(!fi) 
    {
      std::cerr << "Unable to open dictionary" << std::endl;
      _error_condition = -1;
      return;
    }
    std::vector<std::string> pool;
    std::string word;
    while(!fi.eof())
    {
      fi >> word;
      pool.push_back(word);
    };
    fi.close();
    // choose a word from the list
    srand(time(NULL));
    size_t N = pool.size();
    int idx = rand() % N;
    _solution = to_upper(pool[idx]);
    _pattern = std::string(_solution.size(), '_');
  }
}

// play game
//------------------------------------------------------------------------------
void hangman::Play()
{
  if(_error_condition == 0)
  {
    int current_player = 0;
    // print the first board screen
    _UpdateBoard();
    // start the loop of players
    while(1)
    {
      // ask for a new letter
      std::string letter = _AskLetter(current_player + 1);
      int  found = 0;
      // if the input was a char
      if(letter.size() == 1)
      {
        if(check_pattern( letter[0],            _pattern)) found = -1; // check of the letter was previously set
        if(check_solution(letter[0], _solution, _pattern)) found = +1; // check the solution
      }
      else
      {
        if(letter == _solution)
        {
          // update the pattern string
          _pattern = letter;
          // redraw the game board
          _UpdateBoard();
          // show 
          _Win(current_player);
          return;
        }
        else
        {
          found = 0;
        }
      }
      if(found == 0) _hang_status++;
      // redraw the game board
      _UpdateBoard();
      // check if the current player wins or looses
      if(_pattern == _solution && _hang_status < 6)
      {
        _Win(current_player);
        break;
      }
      else if(_hang_status >= 6)
      {
        _Loose(current_player);
        break;
      }
      // update the current player
      if(found >= 0)
      {
        current_player++;
        current_player %= _players;
      }
    }
  }
}

// auxiliary functions
// ask for a new letter
//------------------------------------------------------------------------------
std::string hangman::_AskLetter(int player_number)
{
      std::string letter;
      std::cout << "   " << to_large(letter_list) << "\n";
      std::cout << "\n";
      std::cout << "player " << player_number << " -> insert a letter: ";
      std::cin >> letter;
      // make letter upper case
      letter = to_upper(letter);
      // check if it is a single character
      if(letter.size() == 1)
      {
        // update the letter list
        int idx = letter[0] - 'A';
        letter_list[idx] = '.';
      }
      return letter;
}

//------------------------------------------------------------------------------
void hangman::_UpdateBoard() const
{
  if(_error_condition == 0)
  {
    int i;

    std::cout << clear_screen;
    for(i = 0; i < HANGMAN_ROWS; ++i)
    {
      std::cout << "    " << the_hangman[_hang_status][i];
      if(i == HANGMAN_FRAMES / 2)
      {
        std::cout << "               " << to_large(_pattern) << "    " << "(" << _solution.size() << ")";
      }
      std::cout << '\n';
    }
    std::cout << '\n';
    //std::cout << "Solution = \"" << to_large(_solution) << "\"\n";
    std::cout << std::endl;
    //...
  }
}

//------------------------------------------------------------------------------
void hangman::_Win(int player_number) const
{
  if(_error_condition == 0)
  {
    std::cout << "Solution = " << _solution << "\n";
    std::cout << "Player " << player_number + 1 << " wins!\n" << std::endl;
  }
}

//------------------------------------------------------------------------------
void hangman::_Loose(int player_number) const
{
  if(_error_condition == 0)
  {
    std::cout << "Solution = " << _solution << "\n";
    std::cout << "Player " << player_number + 1 << " looses!\n" << std::endl;
  }
}
