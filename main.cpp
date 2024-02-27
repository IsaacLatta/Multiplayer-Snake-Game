#include "Game.h"

   ///**************************///
  ///   Author: Isaac Latta    ///
 ///   Date: January 16, 2024 ///
///**************************///

int main()
{
    SetConsoleOutputCP(CP_UTF8);
   
   std::unique_ptr<Game> game = std::make_unique<Game>();
   game->start_game();
    
    return 0;
}
