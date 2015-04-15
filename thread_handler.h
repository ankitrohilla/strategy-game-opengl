//#include "variables.h"

int i = 0;

void* threadMyHero( void* )
{
    i++;
    std::cout << "Value of i is - " << i << "\n\n";fflush(stdout);
    while( true )
    {
  //      std::cout<<"opponent temple health : "<<teams[1].currentHealth<<"\n";

       // if( !isServer )
  //      std::cout<<"am i dead : "<<heroes[myHeroId]->isDead<<"\n";fflush(stdout);

        if( (myTeamId == 0 &&
             heroes[myHeroId]->row >= 15 &&
             heroes[myHeroId]->col <= 2) ||
            (myTeamId == 1 &&
             heroes[myHeroId]->row <= 2  &&
             heroes[myHeroId]->col >= 29))
        {
            // I am at my spawn point
            heroes[myHeroId]->healthLock = true;
        }

        // Is our/their temple destroyed
        if( teams[myTeamId].currentHealth <=5 )
        {
            youLost = true;
        }
        else if( teams[enemyTeamId].currentHealth <=5 )
        {
            youWon = true;
        }

        // Am I dead
        if( heroes[myHeroId]->current_health <= 0 && heroes[myHeroId] && (isServer || whichGameMode == SINGLE_PLAYER) ) // /////////
        {
            heroes[myHeroId]->isDead = true;
            heroes[myHeroId]->whichState = DEAD;
            sleep( 10 );
           // delete( heroes[myHeroId] );
            //heroes[myHeroId] = NULL;
            std::cout << "Respawning";fflush(stdout);
            switch(myHeroId)
            {
                case 0:
                    heroes[0]->respawn( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
                    break;
                case 1:
                    heroes[1]->respawn( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
                    break;
                case 2:
                    heroes[2]->respawn( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
                    break;
                case 3:
                    heroes[3]->respawn( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);
                    break;
            }
            std::cout << "new hero created";fflush(stdout);
        }

        // whether I am server or client, I won't update my healths
        // unlock healths after interval
        // and if I am at spawn point, do not unlock it
        if( heroes[myHeroId]->healthLock )
        {
            usleep( LOCK_TIME );

            // check if I am at my spawn point
            if( (myTeamId == 0 &&
                 heroes[myHeroId]->row >= 15 &&
                 heroes[myHeroId]->col <= 2) ||
                (myTeamId == 1 &&
                 heroes[myHeroId]->row <= 2  &&
                 heroes[myHeroId]->col >= 29))
            {
                heroes[myHeroId]->healthLock = true;
                if( heroes[myHeroId]->current_health < heroes[myHeroId]->max_health )
                {
                    heroes[myHeroId]->current_health += heroes[myHeroId]->max_health/15;
                }
                else
                {
                    heroes[myHeroId]->current_health = heroes[myHeroId]->max_health;
                }

            }
            else
            {
                heroes[myHeroId]->healthLock = false;
            }
        }
        if( teams[myTeamId].healthLock ) { usleep( LOCK_TIME ); teams[myTeamId].healthLock = false; }

        // Am I slowed
        if( heroes[myHeroId]->isSlowed )
        {
            // If I have used speed item and someone reduced my speed revert back to normal speed
            // Else reduce the speed

           // while(heroes[myHeroId]->path.size());

            if( heroes[myHeroId]->speed > heroes[myHeroId]->normalSpeed )
                heroes[myHeroId]->speed = heroes[myHeroId]->normalSpeed;
            else
                heroes[myHeroId]->speed = heroes[myHeroId]->normalSpeed * SLOW_MAGIC_MULTIPLE;
        }
        else
        {
            // If I have used speed item and noone reduced my speed do nothing
            // Else my speed is normal speed
            if( heroes[myHeroId]->speed > heroes[myHeroId]->normalSpeed )
                ; // do nothing
            else
                heroes[myHeroId]->speed = heroes[myHeroId]->normalSpeed;
        }

        // Am I Disabled will be implemented in the mouse left click code

        // Am I Blackouted will be implemented in the display code

        // Am I vandalized
        if( heroes[myHeroId]->isVandalized )
        {
            displayVandalism = true;
            heroes[myHeroId]->isVandalized = false;

            std::cout << "is vandalized " << heroes[myHeroId]->vandalizedTileId << "\n";

            // if my friend invoked vandalism or I am invincible, don't reduce my health
            if( heroes[friendHeroId]->whichMagicPower != VANDALIZE && !heroes[myHeroId]->isInvincible )
            {std::cout << "should come here \n";
                if( abs( heroes[myHeroId]->row - getRowFromId(heroes[myHeroId]->vandalizedTileId)) < 4 )
                {
                    if( abs( heroes[myHeroId]->col - getColFromId(heroes[myHeroId]->vandalizedTileId)) < 4 )
                    {
                        heroes[myHeroId]->healthLock = true;
                        heroes[myHeroId]->current_health -= VANDALISM_DAMAGE;
                    }
                }
            }
        }

        // revive myself after an interval
        if( heroes[myHeroId]->whichState == STUNNED )
        {
            sleep(STUN_TIME);
            heroes[myHeroId]->whichState = STANDING;
            heroes[myHeroId]->stateLock = true;
            usleep(LOCK_TIME);
            heroes[myHeroId]->stateLock = false;
        }

        // Acquiring the items is independent of STATE apart from DEAD
        // Can acquire maximum of 4 items
        if( heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isItemPresent && heroes[myHeroId]->itemBag.size() < 4 )
        {
            heroes[myHeroId]->itemBag.push_back(heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].whichItem);

            // element is removed from the hero's map
            heroes[myHeroId]     ->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isItemPresent = false;
            heroes[friendHeroId] ->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isItemPresent = false;

            for( itemMap::iterator it = teams[myTeamId].itemSet.begin(); it != teams[myTeamId].itemSet.end(); )
            {
                if( it->second->row == heroes[myHeroId]->row && it->second->col == heroes[myHeroId]->col )
                    teams[myTeamId].itemSet.erase(it++);
                else
                    ++it;
            }

        }

        switch( heroes[myHeroId]->whichState )
        {
            case MOVING_TO_ATTACK_ENEMY_1:

                if( heroes[myHeroId]->tilesToMove == heroes[myHeroId]->path.size()/2 && heroes[myHeroId]->tilesToMove > 1 )
                {
                    heroes[myHeroId]->target_x = floor(0.5+heroes[enemyHero1Id]->x);
                    heroes[myHeroId]->target_y = floor(0.5+heroes[enemyHero1Id]->y);
                    heroes[myHeroId]->breadthFirstSearch( heroes[myHeroId]->x, heroes[myHeroId]->y, heroes[myHeroId]->target_x, heroes[myHeroId]->target_y);
                }
                if( heroes[myHeroId]->tilesToMove == 0 )
                {
                    heroes[myHeroId]->whichState = ATTACKING_ENEMY_1;
                }
                break;
            case MOVING_TO_ATTACK_ENEMY_2:
                if( heroes[myHeroId]->tilesToMove == heroes[myHeroId]->path.size()/2 && heroes[myHeroId]->tilesToMove > 1 )
                {
                    heroes[myHeroId]->target_x = floor(0.5+heroes[enemyHero2Id]->x);
                    heroes[myHeroId]->target_y = floor(0.5+heroes[enemyHero2Id]->y);
                    heroes[myHeroId]->breadthFirstSearch( heroes[myHeroId]->x, heroes[myHeroId]->y, heroes[myHeroId]->target_x, heroes[myHeroId]->target_y);
                }
                if( heroes[myHeroId]->tilesToMove == 0 )
                {
                    heroes[myHeroId]->whichState = ATTACKING_ENEMY_2;
                }
                break;
            case MOVING_TO_ATTACK_TEMPLE:
                if( heroes[myHeroId]->tilesToMove == 0 && !heroes[myHeroId]->mapLock )
                {
                    heroes[myHeroId]->whichState = ATTACKING_TEMPLE;
                }
                break;
            case ATTACKING_TEMPLE:
                //if(isServer || whichGameMode == SINGLE_PLAYER)
                 teams[enemyTeamId].currentHealth -= heroes[myHeroId]->attack;
     //           std::cout << teams[enemyTeamId].currentHealth << "\n";
                break;
            case ATTACKING_ENEMY_1:

                if( heroes[ enemyHero1Id ]->isDead ) { heroes[myHeroId]->whichState = STANDING; break; }
                if( !heroes[myHeroId]->map[ heroes[enemyHero1Id]->row ][ heroes[enemyHero1Id]->col ].isMovable ) { heroes[myHeroId]->whichState = STANDING; break; }
                if( abs( heroes[myHeroId]->row - heroes[enemyHero1Id]->row ) > 1 || abs( heroes[myHeroId]->col - heroes[enemyHero1Id]->col ) > 1 )
                {
                    heroes[myHeroId]->whichState = MOVING_TO_ATTACK_ENEMY_1;
                    heroes[myHeroId]->target_x = heroes[enemyHero1Id]->x;
                    heroes[myHeroId]->target_y = heroes[enemyHero1Id]->y;
//                    std::cout << "myx myy targtx targty   " << heroes[myHeroId]->x << " " << heroes[myHeroId]->y << " " << heroes[myHeroId]->target_x << " " << heroes[myHeroId]->target_y << "\n";
//                    std::cout << "myrow mycol enrow encol " << heroes[myHeroId]->row << " " << heroes[myHeroId]->col << " " << heroes[enemyHero1Id]->row << " " << heroes[enemyHero1Id]->col << "\n";
                    heroes[myHeroId]->breadthFirstSearch( heroes[myHeroId]->x, heroes[myHeroId]->y, heroes[myHeroId]->target_x, heroes[myHeroId]->target_y);
                }
                if( !heroes[enemyHero1Id]->isInvincible ) heroes[enemyHero1Id]->current_health -= heroes[myHeroId]->attack;
    //            std::cout << heroes[enemyHero1Id]->current_health << "\n";
                break;
            case ATTACKING_ENEMY_2:
                if( heroes[ enemyHero2Id ]->isDead ) { heroes[myHeroId]->whichState = STANDING; break; }
                if( !heroes[myHeroId]->map[ heroes[enemyHero2Id]->row ][ heroes[enemyHero2Id]->col ].isMovable ) { heroes[myHeroId]->whichState = STANDING; break; }
                if( abs( heroes[myHeroId]->row - heroes[enemyHero2Id]->row ) > 1 || abs( heroes[myHeroId]->col - heroes[enemyHero2Id]->col ) > 1 )
                {
                    heroes[myHeroId]->whichState = MOVING_TO_ATTACK_ENEMY_2;
                    heroes[myHeroId]->target_x = heroes[enemyHero2Id]->x;
                    heroes[myHeroId]->target_y = heroes[enemyHero2Id]->y;
    //                    std::cout << "myx myy targtx targty   " << heroes[myHeroId]->x << " " << heroes[myHeroId]->y << " " << heroes[myHeroId]->target_x << " " << heroes[myHeroId]->target_y << "\n";
    //                    std::cout << "myrow mycol enrow encol " << heroes[myHeroId]->row << " " << heroes[myHeroId]->col << " " << heroes[enemyHero2Id]->row << " " << heroes[enemyHero2Id]->col << "\n";
                    heroes[myHeroId]->breadthFirstSearch( heroes[myHeroId]->x, heroes[myHeroId]->y, heroes[myHeroId]->target_x, heroes[myHeroId]->target_y);
                }
                if( !heroes[enemyHero2Id]->isInvincible ) heroes[enemyHero2Id]->current_health -= heroes[myHeroId]->attack;
//                std::cout << heroes[enemyHero2Id]->current_health << "\n";
                break;
        }

        usleep( 100000 );
    }
    return NULL;
}

void* threadBotManager( int* _botId )
{
    int botId = *_botId;
    int botTeamId = botId / 2;
    int botEnemyTeamId = (botTeamId+1)%2;

    int botFriendId = 2*botTeamId      + (botId+1)%2;
    int botEnemy1Id = 2*botEnemyTeamId;
    int botEnemy2Id = 2*botEnemyTeamId + (botEnemy1Id+1)%2;

    // placeholders to define temple's location used by bots
    int botEnemyTempleRow, botEnemyTempleCol;
    int botTempleRow, botTempleCol;

    int botRow, botCol, botX, botY;

    int botFriendRow, botFriendCol, botFriendX, botFriendY;
    int botEnemy1Row, botEnemy1Col, botEnemy1X, botEnemy1Y;
    int botEnemy2Row, botEnemy2Col, botEnemy2X, botEnemy2Y;

    // to tell bots the placeholders of temple's locations
    if( botTeamId == 0 )
    {
        botTempleRow = 5;
        botTempleCol = 11;
        botEnemyTempleRow = 9;
        botEnemyTempleCol = 19;
    }
    else
    {
        botTempleRow = 9;
        botTempleCol = 19;
        botEnemyTempleRow = 5;
        botEnemyTempleCol = 11;
    }

    while( true )
    {

        // dead
        if( heroes[botId]->current_health <= 0 )
        {
            heroes[botId]->isDead = true;
            heroes[botId]->whichState = DEAD;
            usleep( 10*1000000 );
            std::cout << "Bot " << botId << " died\n";
            switch(botId)
            {
                case 0:
                    heroes[0]->respawn( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
                    break;
                case 1:
                    heroes[1]->respawn( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
                    break;
                case 2:
                    heroes[2]->respawn( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
                    break;
                case 3:
                    heroes[3]->respawn( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);
                    break;
            }
            std::cout << "Bot " << botId << " respawned\n";
        }

        botRow = heroes[botId]->row;
        botCol = heroes[botId]->col;
        botX = heroes[botId]->x;
        botY = heroes[botId]->y;

        botFriendX = heroes[botFriendId]->x;
        botEnemy1X = heroes[botEnemy1Id]->x;
        botEnemy2X = heroes[botEnemy2Id]->x;

        botFriendY = heroes[botFriendId]->y;
        botEnemy1Y = heroes[botEnemy1Id]->y;
        botEnemy2Y = heroes[botEnemy2Id]->y;

        botFriendRow = coordinateToRow(botFriendX, botFriendY);
        botFriendCol = coordinateToCol(botFriendX, botFriendY);

        botEnemy1Row = coordinateToRow(botEnemy1X, botEnemy1Y);
        botEnemy1Col = coordinateToCol(botEnemy1X, botEnemy1Y);

        botEnemy2Row = coordinateToRow(botEnemy2X, botEnemy2Y);
        botEnemy2Col = coordinateToCol(botEnemy2X, botEnemy2Y);

        // revive myself after an interval
        if( heroes[botId]->whichState == STUNNED )
        {
            std::cout << "Bot " << botId << " stunned\n";
            usleep(1000000*STUN_TIME);
            heroes[botId]->whichState = STANDING;
            heroes[botId]->stateLock = true;
            usleep(LOCK_TIME);
            heroes[botId]->stateLock = false;
            std::cout << "Bot " << botId << " stun over\n";
        }

        // unlock healths after interval/revive the person if stunned
        if( isServer )
        {
            if( heroes[botId]->healthLock ) { usleep( LOCK_TIME ); heroes[botId]->healthLock = false; }
            if( teams[botTeamId].healthLock ) { usleep( LOCK_TIME ); teams[botTeamId].healthLock = false; }
            if( heroes[botId]->whichState == STUNNED )
            {
                usleep(1000000*STUN_TIME);
                heroes[botId]->whichState = STANDING;
                heroes[botId]->stateLock = true;
                usleep(LOCK_TIME);
                heroes[botId]->stateLock = false;
            }
        }

        // Am I slowed
        if( heroes[botId]->isSlowed )
        {
            std::cout << "Bot " << botId << " slowed\n";
            // If I have used speed item and someone reduced my speed revert back to normal speed
            // Else reduce the speed

           // while(heroes[botId]->path.size());

            if( heroes[botId]->speed > heroes[botId]->normalSpeed )
                heroes[botId]->speed = heroes[botId]->normalSpeed;
            else
                heroes[botId]->speed = heroes[botId]->normalSpeed * SLOW_MAGIC_MULTIPLE;
            /*
            heroes[botId]->bfsQueue.clear();
            heroes[botId]->visitedSet.clear();
            heroes[botId]->tilesToMove = 0;
            heroes[botId]->path.clear();
            heroes[botId]->path.resize(0, 0);
            heroes[botId]->stepsMoved = 0;
            heroes[botId]->x = floor(0.5+heroes[botId]->x);
            heroes[botId]->y = floor(0.5+heroes[botId]->y);
            heroes[botId]->target_x = heroes[botId]->x;
            heroes[botId]->target_y = heroes[botId]->y;
            heroes[botId]->next_x = heroes[botId]->x;
            heroes[botId]->next_y = heroes[botId]->y;
            heroes[botId]->mapLock = false; */

        }
        else
        {
            // If I have used speed item and noone reduced my speed do nothing
            // Else my speed is normal speed
            if( heroes[botId]->speed > heroes[botId]->normalSpeed )
                ; // do nothing
            else
                heroes[botId]->speed = heroes[botId]->normalSpeed;
        }

        // Am I vandalized
        if( heroes[botId]->isVandalized )
        {
            heroes[botId]->isVandalized = false;

            // if my friend invoked vandalism or I am invincible, don't reduce my health
            if( heroes[botFriendId]->whichMagicPower != VANDALIZE && !heroes[botId]->isInvincible )
            {
                if( abs( heroes[botId]->row - getRowFromId(heroes[botId]->vandalizedTileId)) < 4 )
                {
                    if( abs( heroes[botId]->col - getColFromId(heroes[botId]->vandalizedTileId)) < 4 )
                    {
                        heroes[botId]->current_health -= VANDALISM_DAMAGE;
                    }
                }
            }
        }

        if( (botTeamId == 0 &&
             heroes[botId]->row >= 15 &&
             heroes[botId]->col <= 2) ||
            (botTeamId == 1 &&
             heroes[botId]->row <= 2  &&
             heroes[botId]->col >= 29))
        {
            // I am at my spawn point
            if( heroes[botId]->current_health < heroes[botId]->max_health )
            {
                heroes[botId]->whichState = STANDING;
                heroes[botId]->current_health += heroes[botId]->max_health/75;
                usleep(100000);
                continue;
            }
            else
            {
                heroes[botId]->current_health = heroes[botId]->max_health;
            }
        }


        switch( whichGameMode )
        {

            // implementation of bots
            case SINGLE_PLAYER:
                pthread_t replenishManager;
                // set targetx targety then call bfs is the way to move bots,
                // change their state to MOVING_TO_ATTACK_-----
                // They will automatically start attacking

                // decide what to do

                // implement magical powers here
                if( !heroes[botId]->isDisabled )
                {
                    pthread_t magicManager;

                    if( heroes[botId]->isReplenished )
                    {
                        // if blackout is my power, implementing is straight forward
                        if( heroes[botId]->whichMagicPower == BLACKOUT )
                        {
                            heroes[botId]->isBlackouting    = true;
                            heroes[botEnemy1Id]->isBlackouted = true;
                            heroes[botEnemy2Id]->isBlackouted = true;

                            pthread_cancel ( magicManager );
                            pthread_create ( &magicManager, NULL, (DurationManager)blackoutMagicDurationManager, &botId);

                            heroes[botId]->isReplenished = false;
                            heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;

                            std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);


                            std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                            pthread_cancel ( replenishManager );
                            std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);


                            pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                            goto AFTER_MAGICAL_POWERS;
                        }

                        // atleast one of botEnemy is in his range for vandalism
                        if( heroes[botId]->whichMagicPower == VANDALIZE         &&
                          ((abs(heroes[botId]->x - heroes[botEnemy1Id]->x) < 4  &&
                            abs(heroes[botId]->y - heroes[botEnemy1Id]->y) < 4) ||
                           (abs(heroes[botId]->x - heroes[botEnemy2Id]->x) < 4  &&
                            abs(heroes[botId]->y - heroes[botEnemy2Id]->y) < 4)))
                        {
                            displayVandalism = true;

                            heroes[botId]        ->hasVandalized= true;
                            heroes[botFriendId]  ->isVandalized = true;
                            heroes[botEnemy1Id]  ->isVandalized = true;
                            heroes[botEnemy2Id]  ->isVandalized = true;

                            heroes[botId]      ->vandalizedTileId = getIdFromRowCol(heroes[botId]->row, heroes[botId]->col);
                            heroes[botFriendId]->vandalizedTileId = getIdFromRowCol(heroes[botId]->row, heroes[botId]->col);
                            heroes[botEnemy1Id]->vandalizedTileId = getIdFromRowCol(heroes[botId]->row, heroes[botId]->col);
                            heroes[botEnemy2Id]->vandalizedTileId = getIdFromRowCol(heroes[botId]->row, heroes[botId]->col);

                            heroes[botId]->isReplenished = false;
                            heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;
                            std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);

                            std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                            pthread_cancel ( replenishManager );
                            std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);

                            pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                            goto AFTER_MAGICAL_POWERS;
                        }


                        // implement rest magical powers here
                        switch ( heroes[botId]->whichMagicPower )
                        {
                            case SLOWER:
                                if( arenaChecking( botEnemy1Row, botEnemy1Col ) )
                                {
                                    heroes[botId]->isSlowing    = true;
                                    heroes[botEnemy1Id]->isSlowed = true;

                                    heroes[botId]->isReplenished = false;
                                    heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;
                                    std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);

                                    std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                                    pthread_cancel ( magicManager );
                                    pthread_cancel ( replenishManager );
                                    std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);

                                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                                    pthread_create ( &magicManager, NULL, (DurationManager)slowerMagicDurationManager, &botId);
                                }
                                else if( arenaChecking( botEnemy2Row, botEnemy2Col ) )
                                {
                                    heroes[botId]->isSlowing    = true;
                                    heroes[botEnemy2Id]->isSlowed = true;

                                    heroes[botId]->isReplenished = false;
                                    heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;
                                    std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);

                                    std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                                    pthread_cancel ( magicManager );
                                    pthread_cancel ( replenishManager );
                                    std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);

                                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                                    pthread_create ( &magicManager, NULL, (DurationManager)slowerMagicDurationManager, &botId);
                                }
                                break;
                            case DISABLER:
                                if( arenaChecking( botEnemy1Row, botEnemy1Col ) )
                                {
                                    heroes[botId]->isDisabling    = true;
                                    heroes[botEnemy1Id]->isDisabled = true;

                                    heroes[botId]->isReplenished = false;
                                    heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;
                                    std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);

                                    std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                                    pthread_cancel ( magicManager );
                                    pthread_cancel ( replenishManager );
                                    std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);

                                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                                    pthread_create ( &magicManager, NULL, (DurationManager)disablerMagicDurationManager, &botId);
                                }
                                else if( arenaChecking( botEnemy2Row, botEnemy2Col ) )
                                {
                                    heroes[botId]->isDisabling    = true;
                                    heroes[botEnemy2Id]->isDisabled = true;

                                    heroes[botId]->isReplenished = false;
                                    heroes[botId]->replenishTimeLeft = heroes[botId]->replenishTime;
                                    std::cout << "Bot " << botId << " thread called magic power used\n"; fflush(stdout);

                                    std::cout << "Bot " << botId << " attempting to cancel probably already finished thread\n"; fflush(stdout);
                                    pthread_cancel ( magicManager );
                                    pthread_cancel ( replenishManager );
                                    std::cout << "Bot " << botId << " cancelled probably already finished thread\n"; fflush(stdout);

                                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &botId);
                                    pthread_create ( &magicManager, NULL, (DurationManager)disablerMagicDurationManager, &botId);
                                }
                                break;
                            default:
                                break;
                        }
                        goto AFTER_MAGICAL_POWERS;
                    }
                }

                AFTER_MAGICAL_POWERS:

                // implement items usage here
                // use items if certain conditions are met
                for( int i = 0; i < heroes[botId]->itemBag.size(); i++ )
                {
                    pthread_t attackManager, speedManager, invincibleManager;

                    switch( heroes[botId]->itemBag.at(i).whichItem )
                    {
                        case INVINCIBLE:
                            // if someone is attacking me
                            if( (heroes[botEnemy1Id]->whichState == ATTACKING_ENEMY_1 && (botId%2)==0) ||
                                (heroes[botEnemy1Id]->whichState == ATTACKING_ENEMY_2 && (botId%2)==1) ||
                                (heroes[botEnemy2Id]->whichState == ATTACKING_ENEMY_1 && (botId%2)==0) ||
                                (heroes[botEnemy2Id]->whichState == ATTACKING_ENEMY_2 && (botId%2)==1)  )
                            {
                                heroes[botId]->isInvincible = true;
                                pthread_create ( &invincibleManager, NULL, (DurationManager)invincibleItemDurationManager, &botId);
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            break;
                        case HERO_HEALTH:
                            if( heroes[botId]->current_health <= heroes[botId]->max_health/3 )
                            {
                                heroes[botId]->current_health += heroes[botId]->max_health/2;
                                if( heroes[botId]->current_health > heroes[botId]->max_health )
                                    heroes[botId]->current_health = heroes[botId]->max_health;
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            break;
                        case TEMPLE_HEALTH:
                            if( teams[botTeamId].currentHealth < teams[botTeamId].maxHealth - TEMPLE_HEALTH_RAISE )
                            {
                                teams[botTeamId].currentHealth += TEMPLE_HEALTH_RAISE;
                                if( teams[botTeamId].currentHealth > teams[botTeamId].maxHealth )
                                    teams[botTeamId].currentHealth = teams[botTeamId].maxHealth;
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            break;
                        case INCREASED_ATTACK:
                            if( heroes[botId]->whichState == ATTACKING_ENEMY_1 ||
                                heroes[botId]->whichState == ATTACKING_ENEMY_2 ||
                                heroes[botId]->whichState == ATTACKING_TEMPLE  )
                            {
                                heroes[botId]->attack *= ATTACK_MULTIPLE;
                                pthread_create ( &attackManager, NULL, (DurationManager)attackItemDurationManager, &botId);
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            break;
                        case INCREASED_SPEED:
                            if( heroes[botId]->whichState == MOVING_TO_ATTACK_ENEMY_1 ||
                                heroes[botId]->whichState == MOVING_TO_ATTACK_ENEMY_2 ||
                                heroes[botId]->whichState == MOVING_TO_ATTACK_TEMPLE  ||
                                heroes[botId]->whichState == MOVING_TO_REACH )
                            {
                                if( heroes[botId]->speed == heroes[botId]->normalSpeed )
                                {
                                    heroes[botId]->speed *= SPEED_MULTIPLE;
                                    pthread_create ( &speedManager, NULL, (DurationManager)speedItemDurationManager, &botId);
                                    heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                    i--;
                                }
                            }
                            break;
                        case STUN_ENEMY:
                            if( arenaChecking( botEnemy1Row, botEnemy1Col ) )
                            {
                                heroes[botEnemy1Id]->whichState = STUNNED;
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            else if( arenaChecking( botEnemy2Row, botEnemy2Col ) )
                            {
                                heroes[botEnemy2Id]->whichState = STUNNED;
                                heroes[botId]->itemBag.erase(heroes[botId]->itemBag.begin() + i);
                                i--;
                            }
                            break;
                    }
                }

                // Acquiring the items is independent of STATE apart from DEAD
                // Can acquire maximum of 4 items
                if( heroes[botId]->map[botRow][botCol].isItemPresent && heroes[botId]->itemBag.size() < 4 )
                {
                    heroes[botId]->itemBag.push_back(heroes[botId]->map[botRow][botCol].whichItem);

                    // element is removed from the hero's map
                    heroes[botId]       ->map[heroes[botId]->row][heroes[botId]->col].isItemPresent = false;
                    heroes[botFriendId] ->map[heroes[botId]->row][heroes[botId]->col].isItemPresent = false;

                    for( itemMap::iterator it = teams[botTeamId].itemSet.begin(); it != teams[botTeamId].itemSet.end(); )
                    {
//                        itemMap::iterator tempIt = it;
                        if( it->second->row == botRow && it->second->col == botCol )
                            teams[botTeamId].itemSet.erase(it++);
                        else
                            ++it;
                    }
                }

                // decision to take items or not
                // if yes, which item then
                for( itemMap::iterator it = teams[botTeamId].itemSet.begin(); it != teams[botTeamId].itemSet.end() && heroes[botId]->itemBag.size() < 4 && heroes[botId]->whichState == STANDING; it++ )
                {
                    int itemX = it->second->col - 16;
                    int itemY = 8 - it->second->row;
//                    std::cout << "itemxy botrowcol" << itemX << " " << itemY << " " << botRow << " " << botCol << "\n\n";
                    if( !arenaChecking( botRow, botCol) && heroes[botId]->map[it->second->row][it->second->col].isItemPresent )
                    {
                        heroes[botId]->target_x = itemX;
                        heroes[botId]->target_y = itemY;
                        heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, itemX, itemY);
                        heroes[botId]->whichState = MOVING_TO_REACH;
                        std::cout << "going to fetch item and tiles to move are " << heroes[botId]->tilesToMove << "\n\n"; fflush(stdout);
                        break;
                    }
                }

                // if I am going somewhere, don't make any decision
                if( heroes[botId]->whichState == MOVING_TO_REACH ) goto AFTER_DECISION_MADE;

                // DECISIONS

                // go to respawn area
                if( heroes[botId]->current_health <= 50 )
                {
                    if( botId == 0 ) { heroes[botId]->target_x = -15; heroes[botId]->target_y = -9; }
                    if( botId == 1 ) { heroes[botId]->target_x = -14; heroes[botId]->target_y = -8; }
                    if( botId == 2 ) { heroes[botId]->target_x =  14; heroes[botId]->target_y =  7; }
                    if( botId == 3 ) { heroes[botId]->target_x =  15; heroes[botId]->target_y =  7; }
//                    std::cout << "bot id targetxy are - " << botId << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n\n";
                    heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                    heroes[botId]->whichState = MOVING_TO_REACH;
                }
                // attack enemy temple if this condition follows
                else if( teams[botTeamId].currentHealth > 5000 && teams[botTeamId].currentHealth >= teams[botEnemyTeamId].currentHealth )
                {
                    heroes[botId]->whichState = MOVING_TO_ATTACK_TEMPLE;
                    templeChecking(botEnemyTempleRow, botEnemyTempleCol, botId);
                    heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                }
                // if my temple being attacked, go attack that person
                else if( heroes[botEnemy1Id]->whichState == ATTACKING_TEMPLE  &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_1 &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_2 )
                {
                    // if I can approach him, go attack him
                    if( heroes[botId]->map[botEnemy1Row][botEnemy1Col].isMovable )
                    {
                        heroes[botId]->target_x = heroes[botEnemy1Id]->x;
                        heroes[botId]->target_y = heroes[botEnemy1Id]->y;
                        heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botEnemy1Id]->x, heroes[botEnemy1Id]->y);
                        heroes[botId]->whichState = MOVING_TO_ATTACK_ENEMY_1;
                    }
                }
                else if( heroes[botEnemy2Id]->whichState == ATTACKING_TEMPLE  &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_1 &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_2 )
                {
                    // if I can approach him, go attack him
                    if( heroes[botId]->map[botEnemy2Row][botEnemy2Col].isMovable )
                    {
                        heroes[botId]->target_x = heroes[botEnemy2Id]->x;
                        heroes[botId]->target_y = heroes[botEnemy2Id]->y;
                        heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botEnemy2Id]->x, heroes[botEnemy2Id]->y);
                        heroes[botId]->whichState = MOVING_TO_ATTACK_ENEMY_2;
                    }
                }
                // if no one is attacking my temple, go attack enemy temple
                else if( heroes[botEnemy2Id]->whichState != ATTACKING_TEMPLE  &&
                         heroes[botEnemy1Id]->whichState != ATTACKING_TEMPLE  &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_1 &&
                         heroes[botId]->whichState != ATTACKING_ENEMY_2 )
                {
                    heroes[botId]->whichState = MOVING_TO_ATTACK_TEMPLE;
                    templeChecking(botEnemyTempleRow, botEnemyTempleCol, botId);
                    heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                }

                AFTER_DECISION_MADE:

                switch( heroes[botId]->whichState )
                {
                    case STANDING:

                        break;
                    case MOVING_TO_REACH:
//                        std::cout << "tilesto move - " << heroes[botId]->tilesToMove << "\n";fflush(stdout);
                        if( heroes[botId]->tilesToMove == 0 ) heroes[botId]->whichState = STANDING;
                        break;
                    case MOVING_TO_ATTACK_ENEMY_1:

                        if( heroes[botId]->tilesToMove == heroes[botId]->path.size()/2 && heroes[botId]->tilesToMove > 1 )
                        {
                            heroes[botId]->target_x = floor(0.5+heroes[botEnemy1Id]->x);
                            heroes[botId]->target_y = floor(0.5+heroes[botEnemy1Id]->y);
                            std::cout << "hero " << botId << " called bfs " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
                            heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                        }
                        if( heroes[botId]->tilesToMove == 0 )
                        {
                            heroes[botId]->whichState = ATTACKING_ENEMY_1;
                        }
                        break;
                    case MOVING_TO_ATTACK_ENEMY_2:
                        if( heroes[botId]->tilesToMove == heroes[botId]->path.size()/2 && heroes[botId]->tilesToMove > 1 )
                        {
                            heroes[botId]->target_x = floor(0.5+heroes[botEnemy2Id]->x);
                            heroes[botId]->target_y = floor(0.5+heroes[botEnemy2Id]->y);
                            std::cout << "hero " << botId << " called bfs " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
                            heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                        }
                        if( heroes[botId]->tilesToMove == 0 )
                        {
                            heroes[botId]->whichState = ATTACKING_ENEMY_2;
                        }
                        break;
                    case MOVING_TO_ATTACK_TEMPLE:
                        if( heroes[botId]->tilesToMove == 0 && !heroes[botId]->mapLock )
                        {
                            heroes[botId]->whichState = ATTACKING_TEMPLE;
                        }
                        break;
                    case ATTACKING_TEMPLE:
                        teams[botEnemyTeamId].currentHealth -= heroes[botId]->attack;
           //             std::cout << teams[botEnemyTeamId].currentHealth << "\n";
                        break;
                    case ATTACKING_ENEMY_1:

                        if( heroes[ botEnemy1Id ]->isDead ) { heroes[botId]->whichState = STANDING; break; }
                        if( !heroes[botId]->map[ botEnemy1Row ][ botEnemy1Col ].isMovable ) { heroes[botId]->whichState = STANDING; break; }
                        if( abs( heroes[botId]->row - botEnemy1Row ) > 1 || abs( heroes[botId]->col - botEnemy1Col ) > 1 )
                        {
                            heroes[botId]->whichState = MOVING_TO_ATTACK_ENEMY_1;
                            heroes[botId]->target_x = heroes[botEnemy1Id]->x;
                            heroes[botId]->target_y = heroes[botEnemy1Id]->y;
        //                    std::cout << "myx myy targtx targty   " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
        //                    std::cout << "myrow mycol enrow encol " << heroes[botId]->row << " " << heroes[botId]->col << " " << botEnemy1Row << " " << botEnemy1Col << "\n";

                            std::cout << "hero " << botId << " called bfs " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
                            heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                        }
                        if( !heroes[botEnemy1Id]->isInvincible ) heroes[botEnemy1Id]->current_health -= heroes[botId]->attack;
          //              std::cout << heroes[botEnemy1Id]->current_health << "\n";
                        break;
                    case ATTACKING_ENEMY_2:
                        if( heroes[ botEnemy2Id ]->isDead ) { heroes[botId]->whichState = STANDING; break; }
                        if( !heroes[botId]->map[ botEnemy2Row ][ botEnemy2Col ].isMovable ) { heroes[botId]->whichState = STANDING; break; }
                        if( abs( heroes[botId]->row - botEnemy2Row ) > 1 || abs( heroes[botId]->col - botEnemy2Col ) > 1 )
                        {
                            heroes[botId]->whichState = MOVING_TO_ATTACK_ENEMY_2;
                            heroes[botId]->target_x = heroes[botEnemy2Id]->x;
                            heroes[botId]->target_y = heroes[botEnemy2Id]->y;
            //                    std::cout << "myx myy targtx targty   " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
            //                    std::cout << "myrow mycol enrow encol " << heroes[botId]->row << " " << heroes[botId]->col << " " << botEnemy2Row << " " << botEnemy2Col << "\n";
                            std::cout << "hero " << botId << " called bfs " << heroes[botId]->x << " " << heroes[botId]->y << " " << heroes[botId]->target_x << " " << heroes[botId]->target_y << "\n";
                            heroes[botId]->breadthFirstSearch( heroes[botId]->x, heroes[botId]->y, heroes[botId]->target_x, heroes[botId]->target_y);
                        }
                        if( !heroes[botEnemy2Id]->isInvincible ) heroes[botEnemy2Id]->current_health -= heroes[botId]->attack;
          //              std::cout << heroes[botEnemy2Id]->current_health << "\n";
                        break;
                    }
                break;
        }

        usleep( 100000 );
    }
    return NULL;

}

void* threadFriendHero( void* )
{
    while( true )
    {
        // unlock healths after interval/revive the person if stunned
        if( isServer )
        {
            if( heroes[friendHeroId]->healthLock ) { usleep( LOCK_TIME ); heroes[friendHeroId]->healthLock = false; }
            if( teams[myTeamId].healthLock ) { usleep( LOCK_TIME ); teams[myTeamId].healthLock = false; }
            if( heroes[friendHeroId]->whichState == STUNNED )
            {
                sleep(STUN_TIME);
                heroes[friendHeroId]->whichState = STANDING;
                heroes[friendHeroId]->stateLock = true;
                usleep(LOCK_TIME);
                heroes[friendHeroId]->stateLock = false;
            }
        }

        if( heroes[friendHeroId]->current_health <= 0 && heroes[friendHeroId] && isServer)
        {
            heroes[friendHeroId]->isDead = true;
            heroes[friendHeroId]->whichState = DEAD;
            sleep( 10 );
            //delete( heroes[friendHeroId] );
            //heroes[friendHeroId] = NULL;
            switch(friendHeroId)
            {
                case 0:
                    heroes[0]->respawn( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
                    break;
                case 1:
                    heroes[1]->respawn( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
                    break;
                case 2:
                    heroes[2]->respawn( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
                    break;
                case 3:
                    heroes[3]->respawn( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);
                    break;
            }
        }
        usleep( 100000 );
    }
    return NULL;
}

// for this bot
// enemy 1 is me and enemy 2 is my friend
void* threadEnemyHero1( void* )
{
    static bool pos;

    while( true )
    {
        // unlock healths after interval/ revive the person if stunned
        if( isServer )
        {
            if( heroes[enemyHero1Id]->healthLock ) { usleep( LOCK_TIME ); heroes[enemyHero1Id]->healthLock = false; }
            if( teams[enemyTeamId].healthLock ) { usleep( LOCK_TIME ); teams[enemyTeamId].healthLock = false; }
            if( heroes[enemyHero1Id]->whichState == STUNNED )
            {
                sleep(STUN_TIME);
                heroes[enemyHero1Id]->whichState = STANDING;
                heroes[enemyHero1Id]->stateLock = true;
                usleep(LOCK_TIME);
                heroes[enemyHero1Id]->stateLock = false;

            }
        }

        if( heroes[enemyHero1Id]->current_health <= 0 && heroes[enemyHero1Id] && isServer)
        {
            heroes[enemyHero1Id]->isDead = true;
            heroes[enemyHero1Id]->whichState = DEAD;
            sleep( 10 );
          //  delete( heroes[enemyHero1Id] );
           // heroes[enemyHero1Id] = NULL;
            switch(enemyHero1Id)
            {
                case 0:
                    heroes[0]->respawn( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[0]->row][heroes[0]->col].isMovable = false;
                    break;
                case 1:
                    heroes[1]->respawn( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[1]->row][heroes[1]->col].isMovable = false;
                    break;
                case 2:
                    heroes[2]->respawn( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[2]->row][heroes[2]->col].isMovable = false;
                    break;
                case 3:
                    heroes[3]->respawn( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[3]->row][heroes[3]->col].isMovable = false;
                    break;
            }
        }


    }
    return NULL;
}

void* threadEnemyHero2( void*   )
{
    while( true )
    {
        // unlock healths after interval/ revive the person if stunned
        if( isServer )
        {
            if( heroes[enemyHero2Id]->healthLock ) { usleep( LOCK_TIME ); heroes[enemyHero2Id]->healthLock = false; }
            if( teams[enemyTeamId].healthLock ) { usleep( LOCK_TIME ); teams[enemyTeamId].healthLock = false; }
            if( heroes[enemyHero2Id]->whichState == STUNNED )
            {
                sleep(STUN_TIME);
                heroes[enemyHero2Id]->whichState = STANDING;
                heroes[enemyHero2Id]->stateLock = true;
                usleep(LOCK_TIME);
                heroes[enemyHero2Id]->stateLock = false;

            }
        }

        if( heroes[enemyHero2Id]->current_health <= 0 && heroes[enemyHero2Id] && isServer)
        {
            heroes[enemyHero2Id]->isDead = true;
            heroes[enemyHero2Id]->whichState = DEAD;
            sleep( 10 );
           // delete( heroes[enemyHero2Id] );
           // heroes[enemyHero2Id] = NULL;
            switch(enemyHero2Id)
            {
                case 0:
                    heroes[0]->respawn( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[0]->row][heroes[0]->col].isMovable = false;
                    break;
                case 1:
                    heroes[1]->respawn( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[1]->row][heroes[1]->col].isMovable = false;
                    break;
                case 2:
                    heroes[2]->respawn( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[2]->row][heroes[2]->col].isMovable = false;
                    break;
                case 3:
                    heroes[3]->respawn( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);
                    heroes[myHeroId]->map[heroes[3]->row][heroes[3]->col].isMovable = false;
                    break;
            }
        }

        usleep( 100000 );
    }
    return NULL;
}

void* threadItemManager( void* )
{
    int randomNum, randomRow, randomCol;
    int i;

    static bool firstTime = true;

    while( true )
    {
        for( i = 0; i < 2; i++ )
        {
            // check and create a new item, if necessary
            for( int j = teams[i].itemSet.size(); j < ITEMS_AT_A_TIME; j++ )
            {
                TRY:
                randomNum = rand()%NO_OF_ITEMS;
                randomRow = rand()%18;
                randomCol = rand()%32;

                // heroes have the map
                if( teams[i].itemSet.find(randomNum)  == teams[i].itemSet.end() )
                {
                    if( heroes[i*2]->map[randomRow][randomCol].isMovable && !arenaChecking(randomRow , randomCol))
                    {
                        teams[i].itemSet[randomNum] = new item((itemDesc)randomNum, randomRow, randomCol, itemsTexture[randomNum]);

                        // fill in the entry in the hero's (0,1)/(2,3) map
                        std::cout << "Created new item" << randomNum << "ROWCOL teamid" << randomRow << " " << randomCol << i << "\n";
                    }
                    else{;}
                        //teams[i].itemSet.erase ( teams[i].itemSet.find(randomNum) );
                }
                else
                    goto TRY;
            }

            // check if this is the time to display the items onto the screen
           for( itemMap::const_iterator it = teams[i].itemSet.begin(); it != teams[i].itemSet.end(); it++ )
           {
               if( firstTime )
                   it->second->timeToDisplay = 5;
                if( it->second->timeToDisplay > 0 )
                    it->second->timeToDisplay--;
                else
                {
                    it->second->isPresent = true;
                    heroes[i*2]  ->map[it->second->row][it->second->col].isItemPresent = true;
                    heroes[i*2]  ->map[it->second->row][it->second->col].whichItem     = *teams[i].itemSet[it->first];
                    heroes[i*2+1]->map[it->second->row][it->second->col].isItemPresent = true;
                    heroes[i*2+1]->map[it->second->row][it->second->col].whichItem     = *teams[i].itemSet[it->first];
//                    std::cout << randomRow << " " << randomCol << " " << i*2 << "\n";
                }
           }
        }
        static int m;
        m++;
        usleep(1000000);
        firstTime = false;
        itemsReady = true;
    }
    return NULL;
}

void* attackItemDurationManager(int *a)
{
    std::cout << "SLEEPNG after increasing attack" << *a << "\n";fflush(stdout);
    usleep(1000000*ATTACK_MULTIPLE_TIME);
    heroes[*a]->attack /= ATTACK_MULTIPLE;
    std::cout << "atatck restored" << "\n";fflush(stdout);

    return NULL;
}

void* speedItemDurationManager(int *a)
{
    std::cout << "SLEEPNG after increasing speed" << *a << "\n";fflush(stdout);
    usleep(1000000*SPEED_MULTIPLE_TIME);

    // force stop hero to avoid problems
   // while(heroes[*a]->path.size());
    heroes[*a]->speed /= SPEED_MULTIPLE;
/*
    heroes[*a]->bfsQueue.clear();
    heroes[*a]->visitedSet.clear();
    heroes[*a]->tilesToMove = 0;
    heroes[*a]->path.clear();
    heroes[*a]->path.resize(0, 0);
    heroes[*a]->stepsMoved = 0;
    heroes[*a]->x = floor(0.5+heroes[*a]->x);
    heroes[*a]->y = floor(0.5+heroes[*a]->y);
    heroes[*a]->target_x = heroes[*a]->x;
    heroes[*a]->target_y = heroes[*a]->y;
    heroes[*a]->next_x = heroes[*a]->x;
    heroes[*a]->next_y = heroes[*a]->y;
    heroes[*a]->mapLock = false;*/

    std::cout << "SPeed restored" << "\n";fflush(stdout);
    return NULL;
}

void* invincibleItemDurationManager(int *a)
{
    std::cout << "Sleeeeepng bfor invincible\n";

    usleep(1000000*INVINCIBLE_TIME);
    heroes[*a]->isInvincible = false;

    std::cout << "Sleeeeepng OVER invincible\n";

    return NULL;
}

void* replenishDurationManager(int *a)
{
    std::cout << "Sleeeeepng bfor replenish\n";

    while( !heroes[*a]->isReplenished )
    {
        if( heroes[*a]->replenishTimeLeft < 0 )
            heroes[*a]->isReplenished = true;
        heroes[*a]->replenishTimeLeft -= 0.1;
        usleep(100000);
    }
    heroes[*a]->replenishTimeLeft = 0;

    std::cout << "Sleeeeepng OVERr replenish\n";

    return NULL;
}

void* slowerMagicDurationManager(int *a)
{
    int callerId = *a;
    int callerTeam = callerId/2;
    int callerEnemyTeam = (callerTeam+1)%2;

    int callerFriendId = callerTeam + (callerId+1)%2;
    int callerEnemy1Id = 2*callerEnemyTeam;
    int callerEnemy2Id = 2*callerEnemyTeam + (callerEnemy1Id+1)%2;

    std::cout << "SLEEPNG in slow speed" << *a << "\n";fflush(stdout);

    usleep(1000000*SLOW_TIME);
    heroes[*a]->isSlowing = false;

    // are only true if I modified them
    heroes[callerEnemy1Id]->isSlowed = false;
    heroes[callerEnemy2Id]->isSlowed = false;

    std::cout << "Sow speed restored to fast\n";

    return NULL;
}

void* disablerMagicDurationManager(int *a)
{
    int callerId = *a;
    int callerTeam = callerId/2;
    int callerEnemyTeam = (callerTeam+1)%2;

    int callerFriendId = callerTeam + (callerId+1)%2;
    int callerEnemy1Id = 2*callerEnemyTeam;
    int callerEnemy2Id = 2*callerEnemyTeam + (callerEnemy1Id+1)%2;

    std::cout << "SLEEPNG in disabler" << *a << "\n";fflush(stdout);
    usleep(1000000*DISABLE_TIME);
    heroes[*a]->isDisabling = false;

    std::cout << "SLEEPNG over disabler" << *a << "\n";fflush(stdout);

    // are only true if I modified them
    heroes[callerEnemy1Id]->isDisabled = false;
    heroes[callerEnemy2Id]->isDisabled = false;

    return NULL;
}

void* blackoutMagicDurationManager(int *a)
{
    int callerId = *a;
    int callerTeam = callerId/2;
    int callerEnemyTeam = (callerTeam+1)%2;

    int callerFriendId = callerTeam + (callerId+1)%2;
    int callerEnemy1Id = 2*callerEnemyTeam;
    int callerEnemy2Id = 2*callerEnemyTeam + (callerEnemy1Id+1)%2;

    std::cout << "Sleeping before blackout\n";

    usleep(1000000*BLACKOUT_TIME);
    heroes[*a]->isBlackouting = false;

    std::cout << "Sleeping over blackout\n";

    // are only true if I modified them
    heroes[callerEnemy1Id]->isBlackouted = false;
    heroes[callerEnemy2Id]->isBlackouted = false;

    return NULL;
}
