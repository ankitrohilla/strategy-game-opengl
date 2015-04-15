#include <GL/glut.h>
#include "SOIL.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <list>
#include <vector>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include "variables.h"
#include "network_handler.h"
#include "thread_handler.h"

// these functions transforms (-16,-9) to [0][17] and so on
int coordinateToRow( int positionX, int positionY )
{
    return 8.0 - positionY;
}
int coordinateToCol( int positionX, int positionY )
{
    return 16.0 + positionX;
}
int coordinateToId( int positionX, int positionY )
{
    int row = coordinateToRow(positionX, positionY);
    int col = coordinateToCol(positionX, positionY);
    return row * 32.0 + col;
}

int getRowFromId( int id ) { return id / 32 ; }
int getColFromId( int id ) { return id % 32 ; }
int getIdFromRowCol( int row, int col) { return row*32 + col; }

tile::tile()
{
    isMovable = true;
}

tile& tile::operator=(const tile& a)
{
    isMovable = a.isMovable;
    vertexId = a.vertexId;
    vertexX = a.vertexX;
    vertexY = a.vertexY;
    parentId = a.parentId;
    adjacencyList = a.adjacencyList;
}

item& item::operator=(const item& a)
{
    whichItem = a.whichItem;
    isPresent = a.isPresent;
    row = a.row;
    col = a.col;
    timeToDisplay = a.timeToDisplay;
    itemTexture = a.itemTexture;
}



void showStats( float x, float y, float maxValue, float currentValue)
{
    int inc;
    float r, g, b, a;

    if( maxValue > 19000 )
    {
        inc = 2;               // health bar of TEMPLE
        r=0; g=0; b=1; a=0.8;
    }
    else if( maxValue > 100 )
    {
        r=0; g=1; b=0; a=1.0;
        inc = 1;               // health bar of HERO
    }
    else
    {
        r=1.0; g=1.0; b=1.0; a=0.8;
        inc = 1;               // replenish duration bar of hero
    }

    glColor4f( r, g, b, a);
    glRectf( x, y+1, x+inc*currentValue/maxValue, y+1.1);
    glColor4f( 1.0, 0.0, 0.0, a);
    glRectf( x+inc*currentValue/maxValue, y+1, x+inc, y+1.1);

}

item::item(){}

item::item(itemDesc whichItem, int row, int col, int itemTexture )
{
    this->isPresent = false;
    this->whichItem = whichItem;
    this->row = row;
    this->col = col;
    this->timeToDisplay = 10;
    this->itemTexture = itemTexture;
}

// heroes with ID 0, 1, 2 or 3

hero::hero(){}

hero& hero::operator=(const hero &a)
{
    this->id              = a.id;
    this->max_health      = a.max_health;
    this->x               = a.x;
    this->y               = a.y;
    this->attack          = a.attack;
    this->speed           = a.speed;
    this->whichMagicPower = a.whichMagicPower;
    this->textureStandId  = a.textureStandId;
    this->whichBotMode    = a.whichBotMode ;
    this->replenishTime   = a.replenishTime;
    this->isBot           = a.isBot;
}

void hero::respawn( int id, int max_health, float x, float y, int attack, int speed, magicPower whichMagicPower, int textureStandId, botMode whichBotMode, int replenishTime, bool isBot)
{
    this->id              = id;
    this->max_health      = max_health;
    this->x               = x;
    this->y               = y;
    this->attack          = attack;
    this->speed           = speed;
    this->whichMagicPower = whichMagicPower;
    this->textureStandId  = textureStandId;
    this->whichBotMode    = whichBotMode ;
    this->replenishTime   = replenishTime;
    //this->isBot           = isBot;

    path.clear();
    bfsQueue.clear();
    visitedSet.clear();

    gettingAttacked = false;
    mapLock = false;
    isDead = false;
    initialized = true;
    isInvincible = false;
    isReplenished = true;
    isSlowing = false;
    isDisabling = false;
    isBlackouting = false;
    hasVandalized = false;


    this->vandalizedTileId = -1;
    this->current_health = max_health;
    this->normalAttack = attack;
    this->attack = attack;
    this->whichMagicPower = whichMagicPower;
    this->normalSpeed = speed;
    this->speed = speed;
    this->currentItem = (itemDesc)-1;
    this->itemBag.clear();
    this->replenishTime = replenishTime;
    this->replenishTimeLeft = 0;

    stepsMoved = 0;

    this->target_x = x;
    this->target_y = y;

    this->row = coordinateToRow((int)(x),(int)(y));
    this->col = coordinateToCol((int)(x),(int)(y));

    std::cout << "HERO in respawn    " << id << "XYROWCOL" << this->x << " " << this->y << " " << this->row << " " << this->col;

    if( this->id != myHeroId || this->id != friendHeroId )
        heroes[myHeroId]->map[this->row][this->col].isMovable = false;


    this->whichState = STANDING;
/*
    if( id <= 1 )
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
            {
                map[i][j] = bottomMap[i][j];
                //map[i][j].isItemPresent = false;
            }
        }
    }
    else
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
            {
                map[i][j] = topMap[i][j];
                //map[i][j].isItemPresent = false;
            }
        }
    }*/


}

hero::hero( int id, int max_health, float x, float y, int attack, int speed, magicPower whichMagicPower, int textureStandId, botMode whichBotMode, int replenishTime, bool isBot)
{
    if( id <= 1 )
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
            {
                map[i][j] = bottomMap[i][j];
                map[i][j].isItemPresent = false;
            }
        }
    }
    else
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
            {
                map[i][j] = topMap[i][j];
                map[i][j].isItemPresent = false;
            }
        }
    }

    this->textureStandId = textureStandId;

    path.clear();
    bfsQueue.clear();
    visitedSet.clear();

    this->isBot = isBot;

    gettingAttacked = false;
    mapLock = false;
    isDead = false;
    initialized = true;
    isInvincible = false;
    isReplenished = true;
    isSlowing = false;
    isDisabling = false;
    isBlackouting = false;
    hasVandalized = false;


    this->vandalizedTileId = -1;
    this->id = id;
    this->x = x;
    this->y = y;
    this->next_x = x;
    this->next_y = y;
    this->max_health = max_health;
    this->current_health = max_health;
    this->normalAttack = attack;
    this->attack = attack;
    this->whichMagicPower = whichMagicPower;
    this->normalSpeed = speed;
    this->speed = speed;
    this->currentItem = (itemDesc)-1;
    this->itemBag.clear();
    this->replenishTime = replenishTime;
    this->replenishTimeLeft = 0;

    stepsMoved = 0;

    this->target_x = x;
    this->target_y = y;

    this->row = coordinateToRow((int)(x),(int)(y));
    this->col = coordinateToCol((int)(x),(int)(y));

    std::cout << "HERO " << id << "XYROWCOL" << this->x << " " << this->y << " " << this->row << " " << this->col;

    this->whichState = STANDING;
}

void hero::moveFromSourceToDest()
{
    if( mapLock ) return;

    enteredSourcetoDest = true;

    mapLock = true;

    int i;

   /* if(yesBot[id])
    {
        std::cout<<"move from source to dest called by bot "<<id<<"\n";
    fflush(stdout);
}*/


    if (stepsMoved < path.size() * (2000 / speed))
    {
        mouseClickedfirst = true;

        if( !(stepsMoved % (2000 / speed) ))
        {
            if( tilesToMove == 0 )
            {
                std::cout << "Effect of speed change of hero " << id << " probably caused problems, but probabaly solved\n\n";
                bfsQueue.clear();
                visitedSet.clear();
                tilesToMove = 0;
                path.clear();
                path.resize(0, 0);
                stepsMoved = 0;
                x = floor(0.5+x);
                y = floor(0.5+y);
                target_x = x;
                target_y = y;
                next_x = x;
                next_y = y;
                mapLock = false;
                this->whichState = STANDING;
                std::cout << "Now returning after handling tilestomove = 0, probably\n";
                return;
            }

            std::cout << "for player id " << id << " , tiles to move , stepsmoved and path.size() are " << tilesToMove << "  " << stepsMoved << "  " << path.size() << "\n";

            tilesToMove--;
            int nextRow = getRowFromId( path.at( tilesToMove ) );
            int nextCol = getColFromId( path.at( tilesToMove ) );
            next_x = floor(map[nextRow][nextCol].vertexX);
            next_y = floor(map[nextRow][nextCol].vertexY);

            for(i=0;i<4;i++)
            {

                if((fabs(next_x - heroes[i]->x)<0.01 && fabs(next_y - heroes[i]->y)<0.01 ) && i!=this->id )
                {
//     if(this->id==0)                   std::cout << "hero obstacle is " << i << " targetx targety herox heroy " << target_x << " " << target_y << " " << heroes[i]->x << " " << heroes[i]->y << "\n";
                    tilesToMove++;
             //       205
           //         237
         //           269
       //             301
     //               334
   //                 335
 //                   336
//                   337
//                    338

                    map[heroes[0]->row][heroes[0]->col].isMovable = false;
                    map[heroes[1]->row][heroes[1]->col].isMovable = false;
                    map[heroes[2]->row][heroes[2]->col].isMovable = false;
                    map[heroes[3]->row][heroes[3]->col].isMovable = false;

                    map[this->row][this->col].isMovable = true;

//                    std::cout << "hero 2 row col " << heroes[2]->row << "  " << heroes[2]->col << "\n";

                    map[heroes[this->id]->row][heroes[this->id]->col].isMovable = true;

                    stepsMoved  = 0.0;

                    // if next and target are NOT EQUAL
                    if( !(fabs(target_x - next_x) < 0.01 && fabs(target_y - next_y) < 0.01) )
                    {
                    //                  if( map[coordinateToRow(target_x, target_y)][coordinateToCol(target_x,target_y)].isMovable )
                    //              {
                        std::cout << "Recalculating path\n";
                        map[heroes[0]->row][heroes[0]->col].isMovable = false;
                        map[heroes[1]->row][heroes[1]->col].isMovable = false;
                        map[heroes[2]->row][heroes[2]->col].isMovable = false;
                        map[heroes[3]->row][heroes[3]->col].isMovable = false;

                        map[this->row][this->col].isMovable = true;

        //                std::cout << coordinateToId(heroes[2]->x, heroes[2]->y) << map[heroes[3]->row][heroes[3]->col].isMovable;

                        std::cout << "Obstakl hero id - " << i << " and his xy rowcol state - " << heroes[i]->x << " " <<   heroes[i]->y << " " <<   heroes[i]->row << " " <<   heroes[i]->col << " " <<  heroes[i]->whichState << "\n";
                        std::cout << "Player " << id << " called coz next tile is HERO obstacl  , hence emovefrom sorc dest calling BFS START";fflush(stdout);
                        breadthFirstSearch( heroes[id]->x, heroes[id]->y, heroes[id]->target_x, heroes[id]->target_y);

                        std::cout << "my , next , targett ismovables are - " << this->map[row][col].isMovable << " " << this->map[coordinateToRow(this->next_x, this->next_y)][coordinateToCol(this->next_x, this->next_y)].isMovable << " " << this->map[coordinateToRow(this->target_x, this->target_y)][coordinateToCol(this->target_x, this->target_y)].isMovable;

                        std::cout << "Player " << id << " called coz next tile is HERO obstacl  , hence movefrom sorc dest done BFS ";fflush(stdout);

                        if( id <= 1 )
                        {
                            for( int i = 0; i < 18; i++ )
                            {
                                for( int j = 0; j < 32; j++ )
                                    map[i][j] = bottomMap[i][j];
                            }
                        }
                        else
                        {
                            for( int i = 0; i < 18; i++ )
                            {
                                for( int j = 0; j < 32; j++ )
                                    map[i][j] = topMap[i][j];
                            }
                        }

                        map[this->row][this->col].isMovable = true;

                        tilesToMove = path.size();

                        std::cout << "recalclted PATH SIZE and id x y row col nextx nexty botstate are" << path.size() << " " << this->id << this->x<< " " << this->y << " " << this->row<< " " << this->col << " " << this->next_x << " " << this->next_y << " " << this->whichState << "\n" ;
                        for(int i = path.size()-1; i > -1; i--)
                        {
                               std::cout << path.at(i) << "  " << std::endl;
                        }
                    } // for stopping when next and target are equal
                    else
                    {
                        bfsQueue.clear();
                        visitedSet.clear();
                        tilesToMove = 0;
                        path.clear();
                        path.resize(0, 0);
                        stepsMoved = 0;
                        x = floor(0.5+(float)x);
                        y = floor(0.5+(float)y);
                        target_x = x;
                        target_y = y;
                        next_x = x;
                        next_y = y;
                        row = coordinateToRow(next_x,next_y);
                        col = coordinateToCol(next_x,next_y);
                    }
                    mapLock = false;
                    return;
                }
            }
            row = nextRow;
            col = nextCol;
            stepsMoved++;
            moveFromTileToTile();
        }
        else
        {
            stepsMoved++;
            moveFromTileToTile();
        }
    }
    else if(mouseClickedfirst)
    {
       x = next_x;
       y = next_y;

       mouseClickedfirst = false;
    }
    else
    {
        bfsQueue.clear();
        visitedSet.clear();
        tilesToMove = 0;
        path.clear();
        path.resize(0, 0);
//         std::cout << "Path size after .clear() " << path.size();
        stepsMoved = 0;
        x = floor(0.5+x);
        y = floor(0.5+y);
        target_x = x;
        target_y = y;
        next_x = x;
        next_y = y;
        row = coordinateToRow(next_x,next_y);
        col = coordinateToCol(next_x,next_y);
    }
    mapLock = false;
    enteredSourcetoDest = false;
}

void hero::moveFromTileToTile()
{
    if( fabs(x - next_x) > 0.0 || fabs(y - next_y) > 0.0)
    {

        if( fabs(x - next_x) < 0.0005*speed ) x = floor(0.5+next_x)  ;
        else if( next_x  > x)     x += 0.0005 * speed;
        else                      x -= 0.0005 * speed;


        if( fabs(y - next_y) < 0.0005*speed )  y = floor(0.5+next_y) ;
        else if( next_y >y )      y += 0.0005 * speed;
        else                      y -= 0.0005 * speed;

    }
    else
    {
//                std::cout << "NEXT X AND Y ARE " << next_x << " " << next_y;
        fflush(stdout);
        x = floor(0.5+next_x);
        y = floor(0.5+next_y);
    }
}

void hero::breadthFirstSearch( int fromX, int fromY, int toX, int toY )
{
    //insideBFS
    // source

//if(this->id == myHeroId)    std::cout << "\nEntered BFS\n";

    mapLock = true;
    stepsMoved  = 0.0;
    int sourceId  = coordinateToId (fromX, fromY);
    int sourceRow = coordinateToRow(fromX, fromY);
    int sourceCol = coordinateToCol(fromX, fromY);
    int targetId = coordinateToId(toX , toY);
    int target_heroId = -1;

    bool sourceNotMovable = false;

    fflush(stdout);

    bfsQueue.clear();
    visitedSet.clear();
    path.clear();
    path.resize( 0, 0);

    // if, by chance, trying to move to obstacle
    if(! map[ coordinateToRow(target_x,target_y) ][ coordinateToCol(target_x,target_y) ].isMovable )
    {
        mapLock = false;
        return;
    }
/*
    if(! map[ coordinateToRow(fromX,fromY) ][ coordinateToCol(fromX,fromY) ].isMovable )
    {
        mapLock = false;
        std::cout<<"came back from bfs due to from \n";
        fflush(stdout);
        return;
    }*/


    if(! map[ coordinateToRow(toX,toY) ][ coordinateToCol(toX ,toY) ].isMovable )
    {
        mapLock = false;
        std::cout<<"came back from bfs due to target \n";
        fflush(stdout);
        return;
    }


    bfsQueue.push_back( sourceId );
    visitedSet[ sourceId ] = true;
    map[sourceRow][sourceCol].parentId = -1;

    int neighbourRow, neighbourCol;
    int neighbourId;

    int currentId  = sourceId ;
    int currentRow = sourceRow;
    int currentCol = sourceCol;

    map[heroes[0]->row][heroes[0]->col].isMovable = false;
    map[heroes[1]->row][heroes[1]->col].isMovable = false;
    map[heroes[2]->row][heroes[2]->col].isMovable = false;
    map[heroes[3]->row][heroes[3]->col].isMovable = false;

    if( coordinateToId(target_x,target_y) == coordinateToId(heroes[0]->x,heroes[0]->y) )
     {   map[heroes[0]->row][heroes[0]->col].isMovable = true; target_heroId =0;}
    if( coordinateToId(target_x,target_y) == coordinateToId(heroes[1]->x,heroes[1]->y) )
       { map[heroes[1]->row][heroes[1]->col].isMovable = true; target_heroId =1;}
    if( coordinateToId(target_x,target_y) == coordinateToId(heroes[2]->x,heroes[2]->y) )
      {  map[heroes[2]->row][heroes[2]->col].isMovable = true;target_heroId =2;}
    if( coordinateToId(target_x,target_y) == coordinateToId(heroes[3]->x,heroes[3]->y) )
     {   map[heroes[3]->row][heroes[3]->col].isMovable = true;target_heroId =3;}

    map[this->row][this->col].isMovable = true;
    if(!map[sourceRow][sourceCol].isMovable)
    {
        sourceNotMovable = true;
        map[sourceRow][sourceCol].isMovable = true;
    }

    while(bfsQueue.size() != 0)
    {
        currentId = bfsQueue.front();
        currentRow = getRowFromId( currentId );
        currentCol = getColFromId( currentId );

        visitedSet[ currentId ] = true;
        bfsQueue.pop_front();

       // if(currentId == targetId)
         //   break;

        for( int i = 0; i < map[currentRow][currentCol].adjacencyList.size(); i++ )
        {
            neighbourId = map[currentRow][currentCol].adjacencyList.at(i);

            neighbourRow = getRowFromId( neighbourId );
            neighbourCol = getColFromId( neighbourId );
//if(this->id==0)            std::cout << "TRgt hero id " << target_heroId;
//if(this->id==0)            std::cout << "ha" << map[12][18].isMovable;
//if(this->id==0)            std::cout << "hero3rowcol " << heroes[3]->row << " " << heroes[3]->col;

            if( visitedSet.find(neighbourId) == visitedSet.end() && (map[neighbourRow][neighbourCol].isMovable) )
            {
//             std::cout << "nbr id row col " << neighbourId << " " << neighbourRow << " " << neighbourCol;
                map[neighbourRow][neighbourCol].parentId = currentId;
                bfsQueue.push_back(neighbourId);
                visitedSet[neighbourId] = true;
            }
        }
    }
 //   std::cout << "Calling pathFinder";


//    std::cout << sourceRow << " " << sourceCol << " " << sourceId << "\n";
//    std::cout << coordinateToRow(toX, toY) << " " << coordinateToCol(toX, toY) << " " << coordinateToId(toX, toY) << "\n";
    fflush(stdout);
//    if(this->id == myHeroId)    std::cout << "Finding path start\n";
    pathFinder(fromX,fromY,toX,toY, target_heroId);
//    if(this->id == myHeroId)    std::cout << "Finding path done\n";

    if( id <= 1 )
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
                map[i][j] = bottomMap[i][j];
        }
    }
    else
    {
        for( int i = 0; i < 18; i++ )
        {
            for( int j = 0; j < 32; j++ )
                map[i][j] = topMap[i][j];
        }
    }

    if(sourceNotMovable)
        map[sourceRow][sourceCol].isMovable = false;

//    map[this->row][this->col].isMovable = false;

    tilesToMove = path.size();

//if(this->id == myHeroId)    std::cout << "Finished BFS";
    fflush(stdout);

    mapLock = false;
}

void hero::pathFinder(int fromX, int fromY, int toX, int toY, int targetHeroId)
{
    int id  = coordinateToId (toX, toY);
    int row = coordinateToRow(toX, toY);
    int col = coordinateToCol(toX, toY);

//    std::cout << "Dest row col id parid are " << row << " " << col << " " << id << " " << map[row][col].parentId;
    if(targetHeroId != -1)
    {
        id = map[row][col].parentId;
        row = getRowFromId(id);
        col = getColFromId(id);
    }
//std::cout << "Dest row col id are " << row << " " << col << " " << id;
    int fromRow = coordinateToRow(fromX, fromY);
    int fromCol = coordinateToCol(fromX, fromY);

//    std::cout << fromRow << " " << fromCol << " " << coordinateToId(fromX, fromY) << "\n";
//    std::cout << coordinateToRow(toX, toY) << " " << coordinateToCol(toX, toY) << " " << coordinateToId(toX, toY) << "\n";
    fflush(stdout);

    while(( row != fromRow || col != fromCol ))
    {
        if(!map[row][col].isMovable)
        {
            bfsQueue.clear();
            visitedSet.clear();
            tilesToMove = 0;
            path.clear();
            path.resize(0, 0);
   //         std::cout << "Path size after .clear() " << path.size();
            stepsMoved = 0;
            x = floor(0.5+x);
            y = floor(0.5+y);
            target_x = x;
            target_y = y;
            next_x = x;
            next_y = y;
            mapLock = false;
            return;
        }
//        std::cout << "my hero id is " << this->id << "Pushing in path row fromRow col fromCol are " << row << " " << fromRow << " " << col << " " << fromCol << "\n" ;
        path.push_back(id);

        id = map[row][col].parentId;
//        std::cout << "parent id is " << id << "\n";
        if(id != -1)
        {
            row = getRowFromId( id );
            col = getColFromId( id );
        }
    }
 //   std::cout << "Path size - " <<path.size() <<"Exiting pathFinder";
//        std::cout << this->id << " suces path find\n";

    fflush(stdout);
}


int apply_texture(char* file)
{
    return SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
    (
        file,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    //glBindTexture(GL_TEXTURE_2D, grass_texture);
}

void draw_rect_with_tex( float l, float r, float b, float t, int texture)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( l, b,  0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( r, b,  0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( r, t,  0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( l, t,  0.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void init(void)
{
//    screenWidth  = glutGet(GLUT_WINDOW_WIDTH);
//    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    std::cout << "Screen width and height in init first call "<< screenWidth << " " << screenHeight;

    if( screen == 0 )
    {

        chosenHero[0] = NULL;
        chosenHero[1] = NULL;
        chosenHero[2] = NULL;
        chosenHero[3] = NULL;

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_FLAT);
        glutSetCursor(GLUT_CURSOR_FULL_CROSSHAIR);

        itemsTexture[0] = apply_texture("items/heroHealth.png");
        itemsTexture[1] = apply_texture("items/templeHealth.png");
        itemsTexture[2] = apply_texture("items/stunEnemy.png");
        itemsTexture[3] = apply_texture("items/increaseAttack.png");
        itemsTexture[4] = apply_texture("items/increaseSpeed.png");
        itemsTexture[5] = apply_texture("items/invincible.png");

        heroATextureStandId = apply_texture("A.png");
        heroBTextureStandId = apply_texture("B.png");
        heroCTextureStandId = apply_texture("C.png");
        heroDTextureStandId = apply_texture("D.png");

	disableTexture = apply_texture("DISABLE.png");
        burstDamageTexture = apply_texture("BURSTDAMAGE.png");
        slowerTexture = apply_texture("SLOWENEMY.png");
        blackOutTexture = apply_texture("BLACKOUT.png");

        enemyPointer = apply_texture("enemyPointer.png");
        friendPointer = apply_texture("friendPointer.png");
        attackEnhancer = apply_texture("attackEnhance.png");

        youWonTexture  = apply_texture("thumbsUp.png");
        youLostTexture = apply_texture("thumbsDown.png");

        // name, health, attack, speed, power, replenish time

        // very slow, very high health, low attack
        A = { 'A', VERY_HIGH_HEALTH, LOW_ATTACK, VERY_LOW_SPEED, SLOWER   , 20, heroATextureStandId };

        // fast, high health, low attack
        B = { 'B', HIGH_HEALTH, VERY_LOW_ATTACK, HIGH_SPEED, BLACKOUT , 50, heroBTextureStandId };

        // slow, low health, very high attack
        C = { 'C', LOW_HEALTH, VERY_HIGH_ATTACK, LOW_SPEED, DISABLER , 20, heroCTextureStandId };

        // very fast, very low health, high attack
        D = { 'D', VERY_LOW_HEALTH, HIGH_ATTACK, VERY_HIGH_SPEED, VANDALIZE, 18, heroDTextureStandId };

        glutTimerFunc( 5, animate, 5);

        return;
    }

    if(screen == 1)
    {
        if(whichGameMode == MULTI_PLAYER)
        {
            if( isServer )
                pthread_create(&serverThreadId , NULL , threadServer , NULL);
            else
                pthread_create(&clientThreadId , NULL , threadClient , NULL);
        }

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_FLAT);
        glutTimerFunc( 5, animate, 5);
        return;
    }

    // choose heroes
    if( screen == 2 )
    {
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_FLAT);
        glutTimerFunc( 5, animate, 5);
        return;
    }

    if( screen == 3 )
    {
        static int o = 0;
        o++;
        std::cout << "Init screen 3 called "<< o << "th time\n\n";fflush(stdout);

        initializeTilesBottom();
        initializeTilesTop();

        enemyTeamId = (1+myTeamId)%2;

        friendHeroId = 2*myTeamId    + (myHeroId+1)%2;
        enemyHero1Id = 2*enemyTeamId;
        enemyHero2Id = 2*enemyTeamId + (enemyHero1Id+1)%2;

        teams[myTeamId].id = myTeamId;
        teams[enemyTeamId].id = enemyTeamId;

        teams[myTeamId].maxHealth = 20000;
        teams[enemyTeamId].maxHealth = 20000;

        teams[myTeamId].currentHealth = teams[myTeamId].maxHealth;
        teams[enemyTeamId].currentHealth = teams[enemyTeamId].maxHealth;

        for( int m = 0; m < 4; m++ )
        {std::cout << "heroSelected[" << m << "[ is " << heroSelected[m] << "\n";
            if( heroSelected[m] == 'A' )
                chosenHero[m] = &A;

            else if( heroSelected[m] == 'B' )
                chosenHero[m] = &B;

            else if( heroSelected[m] == 'C' )
                chosenHero[m] = &C;

            else if( heroSelected[m] == 'D' )
                chosenHero[m] = &D;
        }

        hero2SpawnY = 7;

        std::cout  << "myteamid is " << myTeamId << " \n\n showing heo spawns\n\n" << hero0SpawnX << hero0SpawnY << "\n\n";
        std::cout  << "\n\n" << hero1SpawnX << hero1SpawnY << "\n\n";;
        std::cout  << "\n\n" << hero2SpawnX << hero2SpawnY << "\n\n";;
        std::cout  << "\n\n" << hero3SpawnX << hero3SpawnY << "\n\n";;

        heroes[0]     = new hero( 0, chosenHero[0]->maxHealth, hero0SpawnX, hero0SpawnY, chosenHero[0]->attackPower, chosenHero[0]->speed, chosenHero[0]->whichMagicPower, chosenHero[0]->heroStandTextureId, AGGRESIVE, chosenHero[0]->replenishTime, false);
        heroes[1]     = new hero( 1, chosenHero[1]->maxHealth, hero1SpawnX, hero1SpawnY, chosenHero[1]->attackPower, chosenHero[1]->speed, chosenHero[1]->whichMagicPower, chosenHero[1]->heroStandTextureId, AGGRESIVE, chosenHero[1]->replenishTime, false);
        heroes[2]     = new hero( 2, chosenHero[2]->maxHealth, hero2SpawnX, hero2SpawnY, chosenHero[2]->attackPower, chosenHero[2]->speed, chosenHero[2]->whichMagicPower, chosenHero[2]->heroStandTextureId, AGGRESIVE, chosenHero[2]->replenishTime, false);
        heroes[3]     = new hero( 3, chosenHero[3]->maxHealth, hero3SpawnX, hero3SpawnY, chosenHero[3]->attackPower, chosenHero[3]->speed, chosenHero[3]->whichMagicPower, chosenHero[3]->heroStandTextureId, AGGRESIVE, chosenHero[3]->replenishTime, false);

//        heroes[1]     = new hero( 1, B.maxHealth, hero1SpawnX, hero1SpawnY, B.attackPower, B.speed, B.whichMagicPower, B.heroStandTextureId, AGGRESIVE, B.replenishTime, false);
//        heroes[2]     = new hero( 2, C.maxHealth, hero2SpawnX, hero2SpawnY, C.attackPower, C.speed, C.whichMagicPower, C.heroStandTextureId, AGGRESIVE, C.replenishTime, false);
//        heroes[3]     = new hero( 3, D.maxHealth, hero3SpawnX, hero3SpawnY, D.attackPower, D.speed, D.whichMagicPower, D.heroStandTextureId, AGGRESIVE, D.replenishTime, false);

        for(int i=0;i<4;i++)
            heroes[i]->initialized = true;
        if(whichGameMode == SINGLE_PLAYER)
            pthread_create ( &threadHero0, NULL, threadMyHero, NULL);

        if( whichGameMode == MULTI_PLAYER )
        {
            if( myHeroId <= 1 )
            {
                if( myHeroId == 0 )
                {
                    friendHeroId = 1;
                    pthread_create ( &threadHero0, NULL, threadMyHero, NULL);
                    pthread_create ( &threadHero1, NULL, threadFriendHero, NULL);
                }
                else
                {
                    friendHeroId = 0;
                    pthread_create ( &threadHero1, NULL, threadMyHero, NULL);
                    pthread_create ( &threadHero0, NULL, threadFriendHero, NULL);
                }
                enemyHero1Id = 2;
                enemyHero2Id = 3;
                pthread_create ( &threadHero2, NULL, threadEnemyHero1, NULL);
                pthread_create ( &threadHero3, NULL, threadEnemyHero2, NULL);
            }
            else
            {
                if( myHeroId == 2 )
                {
                    friendHeroId = 3;
                    pthread_create ( &threadHero2, NULL, threadMyHero, NULL);
                    pthread_create ( &threadHero3, NULL, threadFriendHero, NULL);
                }
                else
                {
                    friendHeroId = 2;
                    pthread_create ( &threadHero3, NULL, threadMyHero, NULL);
                    pthread_create ( &threadHero2, NULL, threadFriendHero, NULL);
                }
                enemyHero1Id = 0;
                enemyHero2Id = 1;
                pthread_create ( &threadHero0, NULL, threadEnemyHero1, NULL);
                pthread_create ( &threadHero1, NULL, threadEnemyHero2, NULL);
            }
        }

        if(whichGameMode == SINGLE_PLAYER || isServer)
            pthread_create ( &itemManagerThreadId, NULL, threadItemManager, NULL);
        isGamePlaySet = true;

        while( whichGameMode == SINGLE_PLAYER && !itemsReady );

        if( whichGameMode == SINGLE_PLAYER )
        {
            pthread_t friendManager, enemy1Manager, enemy2Manager;

            pthread_create ( &friendManager, NULL, (BotManager)threadBotManager, &friendHeroId);
            pthread_create ( &enemy1Manager, NULL, (BotManager)threadBotManager, &enemyHero1Id);
            pthread_create ( &enemy2Manager, NULL, (BotManager)threadBotManager, &enemyHero2Id);
        }

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_FLAT);

        allSet = true;

        glutTimerFunc( 5, animate, 5);

    }
}

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glOrtho (-16.0, 16.0, -9.0, 9.0, 1.5, 20.0);
   glMatrixMode (GL_MODELVIEW);
}

void keyboard( unsigned char BUTTON, int x, int y)
{
    if( screen <= 2 ) return;

    if( BUTTON == 'q' || BUTTON == 'Q' ) exit( 1 );

    if( heroes[myHeroId]->itemBag.size() > BUTTON - 49 )
    {
        // only enum is enough to specify the current item which the hero will use in the
        // next click
        heroes[myHeroId]->currentItem = heroes[myHeroId]->itemBag.at(BUTTON-49).whichItem;
        heroes[myHeroId]->itemBag.erase (heroes[myHeroId]->itemBag.begin()+BUTTON-49);
    }

    // between '1' and '4'
    if( BUTTON >= 49 && BUTTON <= 52 )
    {
        // this item will be processed on mouse click
        if( heroes[myHeroId]->currentItem == STUN_ENEMY )
        {
            glutSetCursor(GLUT_CURSOR_DESTROY);
        }// rest will be processed here
        else
        {
            pthread_t attackManager, speedManager, invincibleManager;
            switch(heroes[myHeroId]->currentItem)
            {
                case HERO_HEALTH:
                    heroes[myHeroId]->healthLock = true;
                    heroes[myHeroId]->current_health += heroes[myHeroId]->max_health/2;
                    if( heroes[myHeroId]->current_health > heroes[myHeroId]->max_health )
                        heroes[myHeroId]->current_health = heroes[myHeroId]->max_health;
                    break;
                case TEMPLE_HEALTH:
                    teams[myTeamId].healthLock = true;
                    teams[myTeamId].currentHealth += TEMPLE_HEALTH_RAISE;
                    if( teams[myTeamId].currentHealth > teams[myTeamId].maxHealth )
                        teams[myTeamId].currentHealth = teams[myTeamId].maxHealth;
                    break;
                case INCREASED_ATTACK:
                    heroes[myHeroId]->attack *= ATTACK_MULTIPLE;
                    pthread_create ( &attackManager, NULL, (DurationManager)attackItemDurationManager, &myHeroId);
                break;
                case INCREASED_SPEED:
                  //  while(heroes[myHeroId]->whichState == MOVING_TO_REACH || heroes[myHeroId]->whichState == MOVING_TO_ATTACK_TEMPLE || heroes[my);

                    if( heroes[myHeroId]->speed == heroes[myHeroId]->normalSpeed )
                    {
                        heroes[myHeroId]->speed *= SPEED_MULTIPLE ;
                        pthread_create ( &speedManager, NULL, (DurationManager)speedItemDurationManager, &myHeroId);
                    }

                break;
                case INVINCIBLE:
                    std::cout << "Invincible used\n\n"; fflush(stdout);
                    heroes[myHeroId]->isInvincible = true;
                    pthread_create ( &invincibleManager, NULL, (DurationManager)invincibleItemDurationManager, &myHeroId);
                    break;
            }
        }
    }
}

bool arenaChecking( int row, int col )
{
    if(( row == 2 && (col >= 9 && col <= 13) ) ||
       ( row == 3 && (col >= 8 && col <= 14) ) ||
       ( row == 4 && (col >= 7 && col <= 14) ) ||
       ( row == 5 && (col >= 7 && col <= 15) ) ||
       ( row == 6 && (col >= 7 && col <= 15) ) ||
       ( row == 7 && (col >= 7 && col <= 14) ) ||
       ( row == 8 && (col >= 8 && col <= 14) ) ||
       ( row == 8 && (col >= 18 && col <= 22)) ||
       ( row == 9 && (col >= 9 && col <= 13) ) ||
       ( row == 9 && (col >= 17 && col <= 23)) ||
       ( row == 10 && (col >= 16 && col <= 23))||
       ( row == 11 && (col >= 16 && col <= 24))||
       ( row == 12 && (col >= 16 && col <= 24))||
       ( row == 13 && (col >= 17 && col <= 23))||
       ( row == 14 && (col >= 17 && col <= 23))||
       ( row == 15 && (col >= 18 && col <= 22) ) )
        return true;
}

// if hero stands where temple checking tells, redirects hero to next available tile
void heroCheckingForTemple0(int *row , int *col)
{
    for(int i = 0 ; i<4 ; i++)
    {
        if( (*row == 4 && *col == 12) )
        {
            *row = 5; *col = 13;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 5 && *col == 13) )
        {
            *row = 6; *col = 13;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 6 && *col == 13) )
        {
            *row = 7; *col = 13;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 7 && *col == 13) )
        {
            *row = 8; *col = 12;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 8 && *col == 12) )
        {
            *row = 8; *col = 11;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 8 && *col == 11) )
        {
            *row = 7; *col = 10;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 7 && *col == 10) )
        {
            *row = 5; *col = 10;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
        else if( (*row == 5 && *col == 10) )
        {
            *row = 4; *col = 12;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple0(row,col);
            else return;

        }
    }
}

// if hero stands where temple checking tells, redirects hero to next available tile
void heroCheckingForTemple1(int *row , int *col)
{
    for(int i = 0 ; i<4 ; i++)
    {
        if( (*row == 9 && *col == 20) )
        {
            *row = 10; *col = 21;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 10 && *col == 21) )
        {
            *row = 11; *col = 22;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 11 && *col == 22) )
        {
            *row = 12; *col = 21;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 12 && *col == 21) )
        {
            *row = 13; *col = 20;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 13 && *col == 20) )
        {
            *row = 13; *col = 19;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 13 && *col == 19) )
        {
            *row = 12; *col = 18;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 12 && *col == 18) )
        {
            *row = 11; *col = 17;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 11 && *col == 17) )
        {
            *row = 10; *col = 18;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
        else if( (*row == 10 && *col == 18) )
        {
            *row = 9; *col = 20;
            if(heroes[i]->row == *row && heroes[i]->col == *col) heroCheckingForTemple1(row,col);
            else return;

        }
    }
}

// this function is useful for mouse clicking and called in mouse()
// returns true if the user wishes to go to the temple
// mouse gives the Id of myHeroId
// bots will give their respective Ids
// this function is used by bots to give their BFS appropriate tile to reach enemy temple and attack it
bool templeChecking( int row, int col, int id )
{
    int callerTeamId = id/2;
    int callerEnemyTeamId = (callerTeamId+1)%2;

    if(( row == 4 && col == 11)                ||
       ( row == 5 && (col >= 11 && col <= 12)) ||
       ( row == 6 && (col >= 10 && col <= 12)) ||
       ( row == 7 && (col >= 11 && col <= 12))  )
    {
        if( heroes[id]->row <= 4 && heroes[id]->col > 11  ) {row = 4; col = 12;}
        if( heroes[id]->row <= 5 && heroes[id]->col <= 11 ) {row = 5; col = 10;}
        if( heroes[id]->row == 5 && heroes[id]->col >  11 ) {row = 5; col = 13;}
        if( heroes[id]->row == 6 && heroes[id]->col >  11 ) {row = 6; col = 13;}
        if( heroes[id]->row == 7 && heroes[id]->col >  11 ) {row = 7; col = 13;}
        if( heroes[id]->row == 6 && heroes[id]->col <= 11 ) {row = 7; col = 10;}
        if( heroes[id]->row >= 7 && heroes[id]->col <= 11 ) {row = 7; col = 10;}
        if( heroes[id]->row >  7 && heroes[id]->col == 11 ) {row = 8; col = 11;}
        if( heroes[id]->row >  7 && heroes[id]->col == 12 ) {row = 8; col = 12;}
        if( heroes[id]->row >  7 && heroes[id]->col >= 13 ) {row = 7; col = 13;}

        // i am standing near enemy temple, so i can start attacking instead of
        // moving to attack
        if( heroes[id]->row == row && heroes[id]->col == col && callerTeamId == 1 )
        {
            heroes[id]->whichState = ATTACKING_TEMPLE;
            teams[callerEnemyTeamId].gettingAttacked = true;
            return false;
        }

        for(int i = 0 ; i < 4; i++)
        {
            if(i != id  &&  heroes[i]->row == row && heroes[i]->col == col)
             {
                heroCheckingForTemple0(&row , &col);
                break;
             }
        }

        // set targetx targety which will decide destination for BFS
        heroes[id]->target_x = col - 16.0;
        heroes[id]->target_y = 8 - row;

        heroes[id]->map[row][col].isMovable = true;

        if( callerTeamId == 0 )
            heroes[id]->whichState = MOVING_TO_REACH;
        else
            heroes[id]->whichState = MOVING_TO_ATTACK_TEMPLE;
        return true;
    }

    if(( row == 9 && col == 19)                ||
       ( row == 10 && (col >= 19 && col <= 20))||
       ( row == 11 && (col >= 18 && col <= 21))||
       ( row == 12 && (col >= 19 && col <= 20)) )
    {
        if( heroes[id]->row <= 10 && heroes[id]->col <= 18 ) {row = 10; col = 18;}
        if( heroes[id]->row <= 9  && heroes[id]->col >= 19 ) {row = 9 ; col = 20;}
        if( heroes[id]->row == 10 && heroes[id]->col >= 19 ) {row = 10; col = 21;}
        if( heroes[id]->row == 11 && heroes[id]->col <= 18 ) {row = 11; col = 17;}
        if( heroes[id]->row == 11 && heroes[id]->col >= 19 ) {row = 11; col = 22;}
        if( heroes[id]->row >= 12 && heroes[id]->col <= 18 ) {row = 12; col = 18;}
        if( heroes[id]->row >= 12 && heroes[id]->col == 19 ) {row = 13; col = 19;}
        if( heroes[id]->row >= 12 && heroes[id]->col == 20 ) {row = 13; col = 20;}
        if( heroes[id]->row >= 12 && heroes[id]->col >= 21 ) {row = 12; col = 21;}

        // i am standing near enemy temple, so i can start attacking instead of
        // moving to attack
        if( heroes[id]->row == row && heroes[id]->col == col && callerTeamId == 0)
        {
            heroes[id]->whichState = ATTACKING_TEMPLE;
            teams[callerEnemyTeamId].gettingAttacked = true;
            return false;
        }

        for(int i = 0 ; i < 4; i++)
        {
            if(i != id  &&  heroes[i]->row == row && heroes[i]->col == col)
             {
                heroCheckingForTemple1(&row , &col);
                break;
             }
        }

//        std::cout << "\nrowcol after - " << row << " " << col;

        heroes[id]->target_x = col - 16.0;
        heroes[id]->target_y = 8 - row;

        heroes[id]->map[row][col].isMovable = true;

//        std::cout << "\ni clikcd on tmeple target xy" << heroes[id]->target_x << " " << heroes[id]->target_y; fflush(stdout);

        if( callerTeamId == 1 )
            heroes[id]->whichState = MOVING_TO_REACH;
        else
            heroes[id]->whichState = MOVING_TO_ATTACK_TEMPLE;

        return true;
    }

    return false;
}

void mouse(int button, int state, int x, int y)
{
    oglCol = x / (screenWidth/32.0);
    oglRow = y / (screenHeight/18.0);

    oglX = oglCol - 16.0;
    oglY = 9.0 - oglRow;

    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )  leftClicked = true;
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_UP ) rightClicked = true;

    if( screen == 0 && state == GLUT_UP )
    {
        std::cout<<" x & y are  = "<<x<<"  "<<y<<"\n";fflush(stdout);

        if(x >=screenWidth/4.8 && x <= screenWidth/1.28)
        {
            if( y >= screenHeight/1.52 && y <= screenHeight/1.4 ) // Will join
            {
                strcpy( myIp , get_my_ip());
               // strcpy( myPort , "8000");
    //            strcpy(remoteIp , "10.192.1.47");
          //      strcpy( remotePort , "8000");
                whichGameMode = MULTI_PLAYER;
                isServer = false;
                screen = 1;
                init();
            }
            else if( y >= screenHeight/1.76 && y <= screenHeight/1.61 ) {  // Will host
                strcpy( myIp , get_my_ip());std::cout << "Host  " << screenWidth << "\n\n\n";fflush(stdout);
                strcpy( myPort , "8000");
    //            strcpy(remoteIp , "10.192.1.47");   // Host doesn't need remoteIp
                strcpy( remotePort , "8000");
                whichGameMode = MULTI_PLAYER;
                isServer = true;
                screen = 1;
                init();
            }
            else if( y >= screenHeight/1.34 && y <= screenHeight/1.24 ) exit(1);
            else if( y >= screenHeight/2.11 && y <= screenHeight/1.88 )
            {std::cout << "SP" << "\n\n\n";fflush(stdout);
                whichGameMode = SINGLE_PLAYER;
                screen = 2;
                init();
            }
        }
        return;
    }

    if(screen == 1 && state == GLUT_UP) return;
    if( screen == 2 && state == GLUT_UP )
    {
        glutTimerFunc( 5, animate, 5);
        // area to pick hero A/B/C/D

        if( y > screenHeight/4 && x > screenWidth/5 )
        {
            if( x >= 1*screenWidth/5 && x <= 2*screenWidth/5 && !isHeroChosen[0] && !myChosenHero )
            {
                myChosenHero = &A;
                isHeroChosen[0] = true;
            }
            else if( x >= 2*screenWidth/5 && x <= 3*screenWidth/5 && !isHeroChosen[1] && !myChosenHero )
            {
                myChosenHero = &B;
                isHeroChosen[1] = true;
            }
            else if( x >= 3*screenWidth/5 && x <= 4*screenWidth/5 && !isHeroChosen[2] && !myChosenHero )
            {
                myChosenHero = &C;
                isHeroChosen[2] = true;
            }
            else if( x >= 4*screenWidth/5 && x <= 5*screenWidth/5 && !isHeroChosen[3] && !myChosenHero )
            {
                myChosenHero = &D;
                isHeroChosen[3] = true;
            }
        }
        // area to pick hero 0/1/2/3
        else if( x < screenWidth/5 )
        {
            if     ( y <= 1*screenHeight/4 && !isPlayerChosen[0] && myHeroId == -1 )
            {
                myHeroId = 0;
                myTeamId = myHeroId/2;
                isPlayerChosen[0] = true;
            }
            else if( y <= 2*screenHeight/4 && !isPlayerChosen[1] && myHeroId == -1 )
            {
                myHeroId = 1;
                myTeamId = myHeroId/2;
                isPlayerChosen[1] = true;
            }
            else if( y <= 3*screenHeight/4 && !isPlayerChosen[2] && myHeroId == -1 )
            {
                myHeroId = 2;
                myTeamId = myHeroId/2;
                isPlayerChosen[2] = true;
            }
            else if( y <= 4*screenHeight/4 && !isPlayerChosen[3] && myHeroId == -1 )
            {
                myHeroId = 3;
                myTeamId = myHeroId/2;
                isPlayerChosen[3] = true;
            }
            if( myTeamId == 0 ) mapTexture = apply_texture("bottom_map1.jpg");
            else                mapTexture = apply_texture("top_map1.jpg");
        }

        if( myHeroId != -1 && myChosenHero )
        {
            heroSelected[myHeroId] = myChosenHero->heroName;
            if( whichGameMode == SINGLE_PLAYER )
            {
                setForBots();
                screen = 3;
                init();
            }
        }
    }

    if( screen == 3 && state == GLUT_UP && allSet && !heroes[myHeroId]->isDead )
    {
        if( x > 31*screenWidth/32 && y > 17*screenHeight/18 && button == GLUT_LEFT_BUTTON )
        {
            if(!isServer)
                exit( 1 );
            else
                Exit = true;
        }
        int col = x / (screenWidth/32.0);
        int row = y / (screenHeight/18.0);
//        std::cout << screenWidth << " " << screenHeight << "\n\n";
        if( col > 31 )
            col = 31;

        // mouse function only works for my player

        // move or simple attack
        if( button == GLUT_RIGHT_BUTTON && heroes[myHeroId]->whichState != STUNNED )
        {
            // utilise these two items only by mouse click
            if( heroes[myHeroId]->currentItem == STUN_ENEMY )
            {
                if( heroes[enemyHero1Id]->row == row && heroes[enemyHero1Id]->col == col && heroes[myHeroId]->map[row][col].isMovable )
                {
                    heroes[enemyHero1Id]->whichState = STUNNED;
                    heroes[myHeroId]->currentItem = (itemDesc)-1;
                    glutSetCursor( GLUT_CURSOR_FULL_CROSSHAIR );
                    return;
                }
                else if( heroes[enemyHero2Id]->row == row && heroes[enemyHero2Id]->col == col && heroes[myHeroId]->map[row][col].isMovable  )
                {
                    heroes[enemyHero2Id]->whichState = STUNNED;
                    heroes[myHeroId]->currentItem = (itemDesc)-1;
                    glutSetCursor( GLUT_CURSOR_FULL_CROSSHAIR );
                    return;
                }
                else goto START;
            }

            START:
        //    if( heroes[myHeroId]->isDead ) return;

            if( !heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isMovable )
            {
                std::cout << " PROB";
        //        return;
            }

            int destTileX = heroes[myHeroId]->map[row][col].vertexX;
            int destTileY = heroes[myHeroId]->map[row][col].vertexY;


    //        heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isMovable = false;

            if(( !heroes[myHeroId]->map[row][col].isMovable && !templeChecking(row,col,myHeroId)) || (heroes[myHeroId]->row == row && heroes[myHeroId]->col == col))
                return;

            heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isMovable = true;

            fflush(stdout);

            // check if user right clicked on a temple
            // NOTE : ismovable of temple is set to true
            // NOTE : templeChecking() sets the target of your hero which will be used by BFS below
            // normal move if destination is ground
            if( !templeChecking(row,col,myHeroId) && heroes[myHeroId]->map[row][col].isMovable )
            {
                heroes[myHeroId]->target_x = col - 16.0;
                heroes[myHeroId]->target_y = 8.0 -  row;
                heroes[myHeroId]->whichState = MOVING_TO_REACH;
            }

            if( fabs(destTileX - heroes[enemyHero1Id]->x) < 1 && fabs(destTileY - heroes[enemyHero1Id]->y) < 1  )
                heroes[myHeroId]->whichState = MOVING_TO_ATTACK_ENEMY_1;
            else if( fabs(destTileX - heroes[enemyHero2Id]->x) < 1 && fabs(destTileY - heroes[enemyHero2Id]->y) < 1  )
                heroes[myHeroId]->whichState = MOVING_TO_ATTACK_ENEMY_2;
            //std::cout << "\nrowcol where i clikd - " << row << " " << col;
            //std::cout << "\ni gonna call BFS on target xy and ismovable" << heroes[myHeroId]->target_x << " " << heroes[myHeroId]->target_y << " ismovable is --- " <<  heroes[myHeroId]->map[ coordinateToRow(heroes[myHeroId]->target_x, heroes[myHeroId]->target_y) ][ coordinateToCol(heroes[myHeroId]->target_x, heroes[myHeroId]->target_y) ].isMovable; fflush(stdout);


            if( !heroes[myHeroId]->mapLock )
                heroes[myHeroId]->breadthFirstSearch( heroes[myHeroId]->x, heroes[myHeroId]->y, heroes[myHeroId]->target_x, heroes[myHeroId]->target_y);

        }

        // test and magical power
        if( button == GLUT_LEFT_BUTTON )
        {
            // Implementation of isDisabled
            if( heroes[myHeroId]->isDisabled ) return;
            pthread_t magicManager;

            if( heroes[myHeroId]->isReplenished )
            {
                // if blackout is my power, implementing is straight forward
                if( heroes[myHeroId]->whichMagicPower == BLACKOUT )
                {
                    // send message to server
                    heroes[myHeroId]->isBlackouting    = true;
                    heroes[enemyHero1Id]->isBlackouted = true;
                    heroes[enemyHero2Id]->isBlackouted = true;
                    pthread_create ( &magicManager, NULL, (DurationManager)blackoutMagicDurationManager, &myHeroId);

                    heroes[myHeroId]->isReplenished = false;
                    heroes[myHeroId]->replenishTimeLeft = heroes[myHeroId]->replenishTime;
                    pthread_t replenishManager;
                    std::cout << "thread called magic power used"; fflush(stdout);
                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &myHeroId);
                    return;
                }

                // client will send the tile id to server only once
                // server will send the tile id to each client only once
                if( heroes[myHeroId]->whichMagicPower == VANDALIZE )
                {
                    displayVandalism = true;

                    heroes[myHeroId]    ->hasVandalized= true;
                    heroes[friendHeroId]->isVandalized = true;
                    heroes[enemyHero1Id]->isVandalized = true;
                    heroes[enemyHero2Id]->isVandalized = true;

                    heroes[myHeroId]    ->vandalizedTileId = getIdFromRowCol(heroes[myHeroId]->row, heroes[myHeroId]->col);
                    heroes[friendHeroId]->vandalizedTileId = getIdFromRowCol(heroes[myHeroId]->row, heroes[myHeroId]->col);
                    heroes[enemyHero1Id]->vandalizedTileId = getIdFromRowCol(heroes[myHeroId]->row, heroes[myHeroId]->col);
                    heroes[enemyHero2Id]->vandalizedTileId = getIdFromRowCol(heroes[myHeroId]->row, heroes[myHeroId]->col);

                    heroes[myHeroId]->isReplenished = false;
                    heroes[myHeroId]->replenishTimeLeft = heroes[myHeroId]->replenishTime;
                    pthread_t replenishManager;
                    std::cout << "thread called magic power used"; fflush(stdout);
                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &myHeroId);
                    return;
                }

                // magical power over enemyHero1Id
                if( heroes[enemyHero1Id]->row == row && heroes[enemyHero1Id]->col == col && heroes[myHeroId]->map[row][col].isMovable )
                {
                    // implement magical powers here
                    switch ( heroes[myHeroId]->whichMagicPower )
                    {
                        case SLOWER:
                            // send message to server
                            heroes[myHeroId]->isSlowing    = true;
                            heroes[enemyHero1Id]->isSlowed = true;
                            pthread_create ( &magicManager, NULL, (DurationManager)slowerMagicDurationManager, &myHeroId);
                            break;
                        case DISABLER:
                            // send message to servers
                            heroes[myHeroId]->isDisabling    = true;
                            heroes[enemyHero1Id]->isDisabled = true;
                            pthread_create ( &magicManager, NULL, (DurationManager)disablerMagicDurationManager, &myHeroId);
                            break;
                        default:
                            break;
                    }


                    heroes[myHeroId]->isReplenished = false;
                    heroes[myHeroId]->replenishTimeLeft = heroes[myHeroId]->replenishTime;
                    pthread_t replenishManager;
                    std::cout << "thread called magic power used"; fflush(stdout);
                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &myHeroId);
                    return;
                }// magical power over enemyHero2Id
                else if( heroes[enemyHero2Id]->row == row && heroes[enemyHero2Id]->col == col && heroes[myHeroId]->map[row][col].isMovable )
                {
                    // implement magical powers here
                    switch ( heroes[myHeroId]->whichMagicPower )
                    {
                        case SLOWER:
                            // send message to server
                            heroes[myHeroId]->isSlowing    = true;
                            heroes[enemyHero2Id]->isSlowed = true;
                            pthread_create ( &magicManager, NULL, (DurationManager)slowerMagicDurationManager, &myHeroId);
                            break;
                        case DISABLER:
                            // send message to servers
                            heroes[myHeroId]->isDisabling    = true;
                            heroes[enemyHero2Id]->isDisabled = true;
                            pthread_create ( &magicManager, NULL, (DurationManager)disablerMagicDurationManager, &myHeroId);
                            break;
                        default:
                            break;
                    }

                    heroes[myHeroId]->isReplenished = false;
                    heroes[myHeroId]->replenishTimeLeft = heroes[myHeroId]->replenishTime;
                    pthread_t replenishManager;
                    std::cout << "thread called magic power used"; fflush(stdout);
                    pthread_create ( &replenishManager, NULL, (DurationManager)replenishDurationManager, &myHeroId);
                    return;
                }
            }

            int id  = col + 32*row;
            std::cout << "Mouse X Y are - " << x << " " << y << "\n";
            std::cout << row << " " << col << "     " << bottomMap[row][col].isMovable << std::endl;
            std::cout << "MY ID" << id << std::endl;
            std::cout << "My hero x y is           - " << heroes[myHeroId]->x << " " << heroes[myHeroId]->y << std::endl;
            std::cout << "My hero nextx nexty is   - " << heroes[myHeroId]->next_x << " " << heroes[myHeroId]->next_y << std::endl;
            std::cout << "My hero trgtx trgty is   - " << heroes[myHeroId]->target_x << " " << heroes[myHeroId]->target_y << std::endl;
            std::cout << "My hero rowcol is        - " << heroes[myHeroId]->row << " " << heroes[myHeroId]->col << std::endl;
            std::cout << "My hero maplock          - " << heroes[myHeroId]->mapLock << "\n";
            std::cout << "My hero state            - " << heroes[myHeroId]->whichState << "\n";
        }
    }
}

// MOST IMPORTANT FUNCTION RESPONSIBLE FOR MOVEMENT OF HEROES
void animate( int i )
{
    if( screen == 0 ) { glutPostRedisplay(); return; }

    if( screen == 1 || screen == 2) { glutTimerFunc(5,animate,5);   glutPostRedisplay(); return; }

    if( screen == 3 && allSet )
    {
        if(heroes[myHeroId]->hasVandalized)
        {
//            std::cout << "i has vandalized " << heroes[myHeroId]->vandalizedTileId << "\n";
//            std::cout << "ROWCOL - " << getRowFromId(heroes[myHeroId]->vandalizedTileId) << " " << getColFromId(heroes[myHeroId]->vandalizedTileId);
        }
        if(heroes[myHeroId]->isVandalized)
        {
//            std::cout << "i am vandalized " << heroes[myHeroId]->vandalizedTileId << "\n";
//            std::cout << "ROWCOL - " << getRowFromId(heroes[myHeroId]->vandalizedTileId) << " " << getColFromId(heroes[myHeroId]->vandalizedTileId);
        }

        if( !heroes[myHeroId]->map[heroes[myHeroId]->row][heroes[myHeroId]->col].isMovable )
        {
            std::cout << " PROBLEM CATCHED hero rowcol" << "  " << heroes[myHeroId]->row << "  " << heroes[myHeroId]->col;fflush(stdout);
        }

        switch( whichGameMode )
        {
            case SINGLE_PLAYER:

                for( int k = 0; k < 4; k++)
                {
                    if( !heroes[k]->mapLock && !heroes[k]->whichState != STUNNED )
                        heroes[k]->moveFromSourceToDest();
                }
                break;
            case MULTI_PLAYER:
                if( heroes[myHeroId]->whichState != STUNNED )
                    heroes[myHeroId]->moveFromSourceToDest();
                break;
        }
        // 1000/FPS is the framerate
        glutTimerFunc( 1000/FPS, animate, 5);
        glutPostRedisplay();
    }
}

int main(int argc, char** argv)
{
    if( argc == 2 )
        strcpy( remoteIp, argv[1] );

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition (0, 0);   
    glutCreateWindow (argv[0]);
    glutFullScreen();
    init (); 
    screenWidth  = glutGet(GLUT_WINDOW_WIDTH);
    screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

    char sms[30] , adjustChar[2];// = NULL;
    memset(sms , 0 , sizeof(sms));
    memset(adjustChar , 0 , sizeof(adjustChar));

    int i = 1;
    adjustChar[0] = i+65; // i+48 for character .. i-48 for int , i+65 for capital letters
    strcpy(sms , "yes");
    strcat(sms,adjustChar);

    if(!strcmp(sms , "yesB"))
        std::cout<<"hiiiiiiiiiiiiiiiii";

    int m = sms[3]-48;
    std::cout<<" sms and m is "<<sms<<"    "<<m<<"\n\n\n";fflush(stdout);
    //std::cout << "Screen width and height after init first call "<< screenWidth << " " << screenHeight;fflush(stdout);

    glutDisplayFunc(display);
    glutTimerFunc( 5, animate, 5);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}

void displayHero( int id, int counter )
{
    static float a = 0.0; static bool goingRight = true;
    static int r = 0; static bool goingUp = true;

    if( a <= 0.0 && !goingRight ) { a+= 0.1; goingRight = true ; }
    if( a >= 0.5 &&  goingRight ) { a-= 0.1; goingRight = false; }

    if( a >= 0.0 && a <= 1.0 )
    {
        if( goingRight ) a += 0.01;
        else             a -= 0.01;
    }

    if( !heroes[id]->isDead )
    {
        glColor3f( 1.0, 1.0, 1.0 );
        if( heroes[myHeroId]->whichState == MOVING_TO_ATTACK_ENEMY_1 && enemyHero1Id == id && counter < 20 )
            glColor4f( 1.0, 0.8, 0.8, 0.8 );
        if( heroes[myHeroId]->whichState == MOVING_TO_ATTACK_ENEMY_2 && enemyHero2Id == id && counter < 20  )
            glColor4f( 1.0, 0.8, 0.8, 0.8 );

        if( heroes[myHeroId]->whichState == ATTACKING_ENEMY_1 && enemyHero1Id == id && counter < 20 )
            glColor4f( 1.0, 0.3, 0.5, 0.8 );
        if( heroes[myHeroId]->whichState == ATTACKING_ENEMY_2 && enemyHero2Id == id && counter < 20  )
            glColor4f( 1.0, 0.3, 0.5, 0.8 );


        if( !heroes[myHeroId]->isBlackouted )
        {
            if( heroes[id]->isInvincible )
            {
                glPushMatrix();
                glTranslatef(heroes[id]->x+0.5, heroes[id]->y+0.5, 0);
                glColor4f( 0.9, 0.6, 0.4, a);
                glutSolidSphere( 0.5, 30, 30);
                glPopMatrix();
            }
            // is AttackItem in Use
            if( heroes[id]->attack > heroes[id]->normalAttack )
            {
                glPushMatrix();

                static float s = 0;
                s += 0.008;

                if( s >= 1 ) s = 0;

                glColor4f( 1.0, 1.0, 1.0, 1-s);

                glTranslatef( +heroes[id]->x+0.5, +heroes[id]->y+0.5, 0.0 );
                glScalef( 4*s, 4*s, 4*s);
                glTranslatef( -heroes[id]->x-0.5, -heroes[id]->y-0.5, 0.0 );
                draw_rect_with_tex( heroes[id]->x, heroes[id]->x+1, heroes[id]->y, heroes[id]->y+1, attackEnhancer);
                glPopMatrix();

                glColor4f( 1.0, 1.0, 1.0, 1.0);
            }
            // isSlowed
            if( heroes[id]->speed < heroes[id]->normalSpeed )
            {
                static float b = 0; static bool goingUp = true;
                if( goingUp  ) b += 0.004;
                if( !goingUp ) b -= 0.004;
                if( b >= 1 ) goingUp = false;
                if( b <= 0 ) goingUp = true;

                glColor4f( b, b, b, 1.0-b/2);
            }
            // isDisabled
            if( heroes[id]->isDisabled )
            {
                static float b = 0; static bool goingUp = true;
                if( goingUp  ) { b += 0.05;  glColor3f( 0.0, 1.0, 1.0); }
                if( !goingUp ) { b -= 0.05; }
                if( b >= 1 ) goingUp = false;
                if( b <= 0 ) goingUp = true;
            }
            // isStunned
            if( heroes[id]->whichState == STUNNED )
            {
                static float a = 0.0;
                static bool goingUp = true;
                if( a >= 1.0 ) goingUp = false;
                if( a <= 0.0 ) goingUp = true;
                if( goingUp ) { a += 0.2; }
                if( !goingUp ) { a -= 0.2; }
                glColor4f( a, 0.0, 0.0, a);
            }

            draw_rect_with_tex( heroes[id]->x, heroes[id]->x+1, heroes[id]->y, heroes[id]->y+1, heroes[id]->textureStandId);
            // shows health
            showStats( heroes[id]->x    , heroes[id]->y    , heroes[id]->max_health   , heroes[id]->current_health    );

            glPushMatrix();
            glColor3f( 1.0, 1.0, 1.0);

            if( goingUp ) r++;
            else          r--;

            if( r >= 358 ) goingUp = false;
            if( r <=   2 ) goingUp = true ;

            glTranslatef( 0.0, 0.2*(float)r/360.0, 0.0);
            glTranslatef( +heroes[id]->x+0.5, 0.0, 0.0 );
            glRotatef(r, 0.0, 1.0, 0.0);
            glTranslatef( -heroes[id]->x-0.5, 0.0, 0.0 );

            // pointer
            if( id == enemyHero1Id || id == enemyHero2Id )
            {
                glColor3f( 1.0, 0.5, 0.5);
                draw_rect_with_tex( heroes[id]->x, heroes[id]->x+1, heroes[id]->y+1, heroes[id]->y+2, enemyPointer);
            }
            else
            {
                draw_rect_with_tex( heroes[id]->x, heroes[id]->x+1, heroes[id]->y+1, heroes[id]->y+2, friendPointer);
            }
            glPopMatrix();

            if( id == myHeroId )
                showStats( heroes[id]->x    , heroes[id]->y+0.1, heroes[id]->replenishTime, heroes[id]->replenishTime-heroes[id]->replenishTimeLeft );
        }
        else if( id == myHeroId )
        {
            if( heroes[id]->isInvincible )
            {
                glPushMatrix();
                glTranslatef(heroes[id]->x+0.5, heroes[id]->y+0.5, 0);
                glColor4f( 0.9, 0.6, 0.4, a);
                glutSolidSphere( 0.5, 30, 30);
                glPopMatrix();
            }
            // isSlowed
            if( heroes[id]->speed < heroes[id]->normalSpeed )
            {
                static float b = 0; static bool goingUp = true;
                if( goingUp  ) b += 0.004;
                if( !goingUp ) b -= 0.004;
                if( b >= 1 ) goingUp = false;
                if( b <= 0 ) goingUp = true;

                glColor4f( b, b, b, 1.0-b/2);
            }
            // isDisabled

            draw_rect_with_tex( heroes[id]->x, heroes[id]->x+1, heroes[id]->y, heroes[id]->y+1, heroes[id]->textureStandId);
            // shows health and magic power replenish time
            showStats( heroes[id]->x    , heroes[id]->y    , heroes[id]->max_health   , heroes[id]->current_health    );

            if( id == myHeroId )
                showStats( heroes[id]->x    , heroes[id]->y+0.1, heroes[id]->replenishTime, heroes[id]->replenishTime-heroes[id]->replenishTimeLeft );
        }

    }
}

void display(void)
{

    glClear (GL_COLOR_BUFFER_BIT);
    glLoadIdentity ();
    gluLookAt (0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glColor3f(1,1,1);

    if( screen == 0 )
    {

        int frontTexture = apply_texture("frontpage.jpg");
       /* int sgTexture = apply_texture("images/Singlegame.png");
        int hgTexture = apply_texture("images/Hostgame.png");
        int jgTexture = apply_texture("images/Joingame.png");
        int egTexture = apply_texture("images/Exitgame.png");*/
        cross         = apply_texture("cross.png");

      /*  draw_rect_with_tex(-15, -10, -5, 0, jgTexture);
        draw_rect_with_tex(-2.5, 2.5, -2.5, 2.5, sgTexture);
        draw_rect_with_tex(10, 15, -5, 0, hgTexture);
        draw_rect_with_tex(-1, 1, -8, -6, egTexture);*/

      /*  draw_rect_with_tex(-14, -11,  5, 8, itemsTexture[0]);
        draw_rect_with_tex(-9 , -6 ,  5, 8, itemsTexture[1]);
        draw_rect_with_tex(-4 , -1 ,  5, 8, itemsTexture[2]);
        draw_rect_with_tex( 1 ,  4 ,  5, 8, itemsTexture[3]);
        draw_rect_with_tex( 6 ,  9 ,  5, 8, itemsTexture[4]);
        draw_rect_with_tex( 11, 14 ,  5, 8, itemsTexture[5]);
*/
        draw_rect_with_tex(-16,16,-9,9,frontTexture);

        waiting1 = apply_texture("Waiting1.png");
        waiting2 = apply_texture("Waiting2.png");

        glutSwapBuffers();
    }

    if( screen == 1 )
    {
         static bool goingRight = true;
         static int rotate = 5;
         static float a;
         rotate -= 1;

         if( goingRight  ) a += 0.01;
         if( !goingRight ) a -= 0.01;

         if( a >= 1 ) goingRight = false;
         if( a <= 0 ) goingRight = true;

         glPushMatrix();
         glColor4f( 1.0, 1.0, 1.0, a);
         draw_rect_with_tex( -8, 8 , -15, -5, waiting2);
         glPopMatrix();

         glColor3f( 1.0, 1.0, 1.0);
         glPushMatrix();
         glRotatef( rotate, 0, 0, 1);
         draw_rect_with_tex( -2, 2 , -2, 2, waiting1);
         glPopMatrix();

         glutSwapBuffers();
    }


    if( screen == 2 )
    {
        int player1 = apply_texture("images/Player1.png");
        int player2 = apply_texture("images/Player2.png");
        int player3 = apply_texture("images/Player3.png");
        int player4 = apply_texture("images/Player4.png");

        int stats   = apply_texture("Stats.png");

        if( isPlayerChosen[0] && myHeroId==0)
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if(isPlayerChosen[0])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f( 1.0, 1.0, 1.0, 1.0);
        draw_rect_with_tex( -14, -10.5,  5.0,  8.5, player1);

        if( isPlayerChosen[1] && myHeroId==1)
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if(isPlayerChosen[1])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f( 1.0, 1.0, 1.0, 1.0);
        draw_rect_with_tex( -14, -10.5,  0.5,  4.0, player2);

        if( isPlayerChosen[2] && myHeroId==2)
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if(isPlayerChosen[2])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f( 1.0, 1.0, 1.0, 1.0);
        draw_rect_with_tex( -14, -10.5, -4.0, -0.5, player3);

        if( isPlayerChosen[3] && myHeroId==3)
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if(isPlayerChosen[3])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f( 1.0, 1.0, 1.0, 1.0);
        draw_rect_with_tex( -14, -10.5, -8.5, -5.0, player4);


        glColor3f(1.0,1.0,1.0);

        if( isHeroChosen[0] && myChosenHero && myChosenHero->heroName=='A')
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if( isHeroChosen[0])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f(1.0 , 1.0 , 1.0 , 1.0);
        draw_rect_with_tex(-10, -4, -8 ,  0, A.heroStandTextureId);

        if( isHeroChosen[1] && myChosenHero && myChosenHero->heroName=='B')
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if( isHeroChosen[1])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f(1.0 , 1.0 , 1.0 , 1.0);
        draw_rect_with_tex(-4 , 2 , -8 ,  0, B.heroStandTextureId);

        if( isHeroChosen[2] && myChosenHero && myChosenHero->heroName=='C')
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if( isHeroChosen[2])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f(1.0 , 1.0 , 1.0 , 1.0);
        draw_rect_with_tex( 2 , 8 , -8 ,  0, C.heroStandTextureId);

        if( isHeroChosen[3] && myChosenHero && myChosenHero->heroName=='D')
            glColor4f( 1.0, 1.0, 0.0, 1.0);
        else if( isHeroChosen[3])
            glColor4f( 1.0, 1.0, 1.0, 0.4);
        else
            glColor4f(1.0 , 1.0 , 1.0 , 1.0);
        draw_rect_with_tex( 8 , 14, -8 ,  0, D.heroStandTextureId);

        glColor3f( 1.0, 1.0, 1.0);

        glColor4f( 0.0 , 1.0 , 0.5 , 0.8);
        draw_rect_with_tex(-10, -7, 0, 2, stats);
        glColor4f( 0.0, 1.0, 0.5, 0.8);
        glRectf( -7, 1.8, -7+2*(float)A.maxHealth/VERY_HIGH_HEALTH, 1.9 );
        glRectf( -7, 1.3, -7+2*(float)A.speed/VERY_HIGH_SPEED, 1.4 );
        glRectf( -7, 0.8, -7+2*(float)A.attackPower/VERY_HIGH_ATTACK, 0.9 );
	draw_rect_with_tex(-7 ,-4 , 0.1 , 0.5 , slowerTexture);

        draw_rect_with_tex(-4 , -1, 0, 2, stats);
        glColor4f( 0.0, 1.0, 0.5, 0.8);
        glRectf( -1, 1.8, -1+2*(float)B.maxHealth/VERY_HIGH_HEALTH, 1.9 );
        glRectf( -1, 1.3, -1+2*(float)B.speed/VERY_HIGH_SPEED, 1.4 );
        glRectf( -1, 0.8, -1+2*(float)B.attackPower/VERY_HIGH_ATTACK, 0.9 );
	draw_rect_with_tex(-1 , 2, 0.1 , 0.5 , blackOutTexture);

        draw_rect_with_tex( 2 ,  5, 0, 2, stats);
        glColor4f( 0.0, 1.0, 0.5, 0.8);
        glRectf(  5, 1.8,  5+2*(float)C.maxHealth/VERY_HIGH_HEALTH, 1.9 );
        glRectf(  5, 1.3,  5+2*(float)C.speed/VERY_HIGH_SPEED, 1.4 );
        glRectf(  5, 0.8,  5+2*(float)C.attackPower/VERY_HIGH_ATTACK, 0.9 );
	draw_rect_with_tex(5 , 8 , 0.1 , 0.5 , disableTexture);

        draw_rect_with_tex( 8 , 11, 0, 2, stats);
        glColor4f( 0.0, 1.0, 0.5, 0.8);
        glRectf( 11, 1.8, 11+2*(float)D.maxHealth/VERY_HIGH_HEALTH, 1.9 );
        glRectf( 11, 1.3, 11+2*(float)D.speed/VERY_HIGH_SPEED, 1.4 );
        glRectf( 11, 0.8, 11+2*(float)D.attackPower/VERY_HIGH_ATTACK, 0.9 );
	draw_rect_with_tex(11,14 , 0.1,0.5 , burstDamageTexture);

         glutSwapBuffers();
    }

    if( screen == 3 )
    {

        static int counter1 = 0;
        counter1 = (counter1+1)%50;

        static int counter2 = 0;
        counter2 = (counter2+1)%360;

        // to manage vandalism display
        static int counter3 = 0;

        glColor4f (1.0, 1.0, 1.0, 1.0);

        static float b = 1.0;

        // MAP
        if( heroes[myHeroId]->isBlackouted )
        {
            b -= 0.01;
            glColor4f( 1.0, 1.0, 1.0, b );
            if( b <= 0.0 ) b = 0.0;
        }
        else
        {
            b += 0.01;
            glColor4f( 1.0, 1.0, 1.0, b );
            if( b >= 1.0 ) b = 1.0;
        }

        draw_rect_with_tex( -16.0, 16.0, -9.0, 9.0, mapTexture);

        draw_rect_with_tex( 15.0, 16.0, -9.0, -8.0, cross);

        // HEROES

        if(heroes[myHeroId]->map[heroes[0]->row][heroes[0]->col].isMovable)
            displayHero(0, counter1);
        if(heroes[myHeroId]->map[heroes[1]->row][heroes[1]->col].isMovable)
            displayHero(1, counter1);
        if(heroes[myHeroId]->map[heroes[2]->row][heroes[2]->col].isMovable)
            displayHero(2, counter1);
        if(heroes[myHeroId]->map[heroes[3]->row][heroes[3]->col].isMovable)
            displayHero(3, counter1);

        // DISPLAY MY ACQUIRED ITEMS ( Item Bag )

        for( int i = 0; i < heroes[myHeroId]->itemBag.size(); i++ )
        {
            glColor4f( 1.0, 1.0, 1.0, 1.0);
            draw_rect_with_tex( i-16, i-15, 8, 9, heroes[myHeroId]->itemBag.at(i).itemTexture);
        }

        // TEMPLE HEALTH

        if( !heroes[myHeroId]->isBlackouted )
        {
            showStats( -5, 3, teams[0].maxHealth, teams[0].currentHealth );
            showStats(  3,-2.8, teams[1].maxHealth, teams[1].currentHealth );
        }
        // DISPLAY ITEMS ON MAP

        for( itemMap::iterator it = teams[myTeamId].itemSet.begin(); it != teams[myTeamId].itemSet.end(); it++ )
        {
            glPushMatrix();
            glTranslatef(  (it->second->col-16+0.5), 0, 0 );
            glRotatef( counter2, 0.0, 1.0, 0.0);
            glTranslatef( -(it->second->col-16+0.5), 0, 0 );

            glColor4f( 1.0, 1.0, 1.0, 0.9);

            if( heroes[myHeroId]->isBlackouted )
                glColor4f( 0.6, 0.6, 0.6, 0.2);

            if( it->second->isPresent && it->second->timeToDisplay<=0 )
                draw_rect_with_tex(it->second->col-16, it->second->col-15, 8-it->second->row, 9-it->second->row, it->second->itemTexture);
            glPopMatrix();
        }

        // DISPLAY VANDALISM

        if( (displayVandalism || counter3>0) && counter3 < 100 )
        {
            displayVandalism = false;

            counter3++;

            //finished displaying
            if(counter3 >= 100) counter3 = 0;

            int row = getRowFromId(heroes[myHeroId]->vandalizedTileId);
            int col = getColFromId(heroes[myHeroId]->vandalizedTileId);

            glPushMatrix();
            glTranslatef( col-16 + 0.5, 8-row + 0.5, 0 );
            glColor4f( 1.0, 0.0, 0.0, 0.5 - counter3/150.0);
            glutSolidSphere( 0.04 * counter3, 60, 60 );
            glPopMatrix();

        }

        if( youLost )
        {
            static bool goingUp = true;
            static float a = 0.0;

            if( goingUp  ) a+=0.01;
            if( !goingUp ) a-=0.01;
            if( a >= 0.9 ) goingUp = false;
            if( a <= 0.1 ) goingUp = true;

            glColor4f( 1.0, 1.0, 1.0, a);
            draw_rect_with_tex( -5, 5, -5, 5, youLostTexture);
            teams[enemyTeamId].currentHealth = teams[enemyTeamId].maxHealth;
            teams[myTeamId].currentHealth = 0;
        }
        else if( youWon )
        {
            static bool goingUp = true;
            static float a = 0.0;

            if( goingUp  ) a+=0.01;
            if( !goingUp ) a-=0.01;
            if( a >= 0.9 ) goingUp = false;
            if( a <= 0.1 ) goingUp = true;

            glColor4f( 1.0, 1.0, 1.0, a);
            draw_rect_with_tex( -5, 5, -5, 5, youWonTexture);
            teams[myTeamId].currentHealth = teams[myTeamId].maxHealth;
            teams[enemyTeamId].currentHealth = 0;
        }

        if( leftClicked )
        {
            static float i = 0.001; static bool goingUp = true;
            if(  goingUp ) i+=0.05;
            if( !goingUp ) i-=0.05;
            if( i > 1 ) goingUp = false;

            glPushMatrix();
            glColor4f( 0.0, 0.5+i*0.5, 0.0, i);
            glTranslatef( oglX, oglY, 0);
            glutSolidTorus( 0.01+0.05*i, 0.2+0.1*i, 30, 30);
            glPopMatrix();

            if( i <= 0.0 )
            {
                i = 0.001;
                leftClicked = false;
                goingUp = true;
            }
        }
        if( rightClicked )
        {
            static float i = 0.001; static bool goingUp = true;
            if(  goingUp ) i+=0.05;
            if( !goingUp ) i-=0.05;
            if( i > 1 ) goingUp = false;

            glPushMatrix();
            glColor4f( 0.5+0.05*i, 0.0, 0.0, i);
            glTranslatef( oglX, oglY, 0);
            glutSolidTorus( 0.01+0.05*i, 0.2+0.1*i, 30, 30);
            glPopMatrix();

            if( i <= 0.0 )
            {
                i = 0.001;
                rightClicked = false;
                goingUp = true;
            }
        }

        glutSwapBuffers();
    }

}


void initializeTilesBottom()
{
    for( int i = 0; i < 18; i++ )
    {
        for( int j = 0; j < 32; j++ )
            bottomMap[0][0].isMovable   = true;
    }

    bottomMap[0][0].isMovable   = false;
    bottomMap[0][1].isMovable   = false;
    bottomMap[0][2].isMovable   = false;
    bottomMap[0][3].isMovable   = false;
    bottomMap[0][4].isMovable   = false;
    bottomMap[0][6].isMovable   = false;
    bottomMap[0][7].isMovable   = false;
    bottomMap[0][8].isMovable   = false;
    bottomMap[0][9].isMovable   = false;
    bottomMap[0][18].isMovable  = false;
    bottomMap[0][19].isMovable  = false;
    bottomMap[0][20].isMovable  = false;

    bottomMap[1][0].isMovable   = false;
    bottomMap[1][1].isMovable   = false;
    bottomMap[1][2].isMovable   = false;
    bottomMap[1][3].isMovable   = false;
    bottomMap[1][4].isMovable   = false;
    bottomMap[1][7].isMovable   = false;
    bottomMap[1][8].isMovable   = false;
    bottomMap[1][16].isMovable  = false;
    bottomMap[1][17].isMovable  = false;
    bottomMap[1][18].isMovable  = false;
    bottomMap[1][19].isMovable  = false;
    bottomMap[1][20].isMovable  = false;
    bottomMap[1][21].isMovable  = false;
    bottomMap[1][24].isMovable  = false;

    bottomMap[2][2].isMovable   = false;
    bottomMap[2][3].isMovable   = false;
    bottomMap[2][4].isMovable   = false;
    bottomMap[2][18].isMovable  = false;
    bottomMap[2][19].isMovable  = false;
    bottomMap[2][20].isMovable  = false;
    bottomMap[2][22].isMovable  = false;
    bottomMap[2][23].isMovable  = false;
    bottomMap[2][24].isMovable  = false;
    bottomMap[2][25].isMovable  = false;

    bottomMap[3][3].isMovable   = false;
    bottomMap[3][4].isMovable   = false;
    bottomMap[3][5].isMovable   = false;
    bottomMap[3][6].isMovable   = false;
    bottomMap[3][22].isMovable  = false;
    bottomMap[3][23].isMovable  = false;
    bottomMap[3][24].isMovable  = false;
    bottomMap[3][25].isMovable  = false;
    bottomMap[3][26].isMovable  = false;
    bottomMap[3][27].isMovable  = false;

    bottomMap[4][0].isMovable   = false;
    bottomMap[4][1].isMovable   = false;
    bottomMap[4][5].isMovable   = false;
    bottomMap[4][6].isMovable   = false;
    bottomMap[4][11].isMovable  = false;
    bottomMap[4][23].isMovable  = false;
    bottomMap[4][24].isMovable  = false;
    bottomMap[4][25].isMovable  = false;
    bottomMap[4][26].isMovable  = false;
    bottomMap[4][27].isMovable  = false;

    bottomMap[5][0].isMovable   = false;
    bottomMap[5][1].isMovable   = false;
    bottomMap[5][11].isMovable  = false;
    bottomMap[5][12].isMovable  = false;
    bottomMap[5][23].isMovable  = false;
    bottomMap[5][24].isMovable  = false;
    bottomMap[5][25].isMovable  = false;
    bottomMap[5][26].isMovable  = false;
    bottomMap[5][27].isMovable  = false;
    bottomMap[5][30].isMovable  = false;
    bottomMap[5][31].isMovable  = false;

    bottomMap[6][0].isMovable   = false;
    bottomMap[6][1].isMovable   = false;
    bottomMap[6][4].isMovable   = false;
    bottomMap[6][5].isMovable   = false;
    bottomMap[6][10].isMovable  = false;
    bottomMap[6][11].isMovable  = false;
    bottomMap[6][12].isMovable  = false;
    bottomMap[6][23].isMovable  = false;
    bottomMap[6][29].isMovable  = false;
    bottomMap[6][30].isMovable  = false;
    bottomMap[6][31].isMovable  = false;

    bottomMap[7][0].isMovable   = false;
    bottomMap[7][1].isMovable   = false;
    bottomMap[7][3].isMovable   = false;
    bottomMap[7][4].isMovable   = false;
    bottomMap[7][5].isMovable   = false;
    bottomMap[7][11].isMovable  = false;
    bottomMap[7][12].isMovable  = false;
    bottomMap[7][29].isMovable  = false;
    bottomMap[7][30].isMovable  = false;

    bottomMap[8][0].isMovable   = false;
    bottomMap[8][4].isMovable   = false;
    bottomMap[8][5].isMovable   = false;
    bottomMap[8][6].isMovable   = false;
    bottomMap[8][7].isMovable   = false;
    bottomMap[8][14].isMovable  = false;
    bottomMap[8][15].isMovable  = false;
    bottomMap[8][16].isMovable  = false;
    bottomMap[8][25].isMovable  = false;
    bottomMap[8][26].isMovable  = false;
    bottomMap[8][29].isMovable  = false;
    bottomMap[8][30].isMovable  = false;

    bottomMap[9][4].isMovable   = false;
    bottomMap[9][5].isMovable   = false;
    bottomMap[9][6].isMovable   = false;
    bottomMap[9][7].isMovable   = false;
    bottomMap[9][14].isMovable  = false;
    bottomMap[9][15].isMovable  = false;
    bottomMap[9][16].isMovable  = false;
    bottomMap[9][19].isMovable  = false;
    bottomMap[9][25].isMovable  = false;
    bottomMap[9][30].isMovable  = false;

    bottomMap[10][4].isMovable  = false;
    bottomMap[10][5].isMovable  = false;
    bottomMap[10][6].isMovable  = false;
    bottomMap[10][7].isMovable  = false;
    bottomMap[10][19].isMovable = false;
    bottomMap[10][20].isMovable = false;

    bottomMap[11][4].isMovable  = false;
    bottomMap[11][5].isMovable  = false;
    bottomMap[11][6].isMovable  = false;
    bottomMap[11][7].isMovable  = false;
    bottomMap[11][18].isMovable = false;
    bottomMap[11][19].isMovable = false;
    bottomMap[11][20].isMovable = false;
    bottomMap[11][21].isMovable = false;

    bottomMap[12][5].isMovable  = false;
    bottomMap[12][6].isMovable  = false;
    bottomMap[12][7].isMovable  = false;
    bottomMap[12][9].isMovable  = false;
    bottomMap[12][10].isMovable = false;
    bottomMap[12][11].isMovable = false;
    bottomMap[12][19].isMovable = false;
    bottomMap[12][20].isMovable = false;

    bottomMap[13][5].isMovable  = false;
    bottomMap[13][6].isMovable  = false;
    bottomMap[13][10].isMovable = false;
    bottomMap[13][11].isMovable = false;
    bottomMap[13][24].isMovable = false;
    bottomMap[13][25].isMovable = false;

    bottomMap[14][8].isMovable  = false;
    bottomMap[14][24].isMovable = false;
    bottomMap[14][25].isMovable = false;
    bottomMap[14][26].isMovable = false;
    bottomMap[14][27].isMovable = false;

    bottomMap[15][8].isMovable  = false;
    bottomMap[15][9].isMovable  = false;
    bottomMap[15][26].isMovable = false;
    bottomMap[15][27].isMovable = false;
    bottomMap[15][29].isMovable = false;
    bottomMap[15][28].isMovable = false;
    bottomMap[15][30].isMovable = false;
    bottomMap[15][31].isMovable = false;

    bottomMap[16][8].isMovable  = false;
    bottomMap[16][9].isMovable  = false;
    bottomMap[16][23].isMovable = false;
    bottomMap[16][24].isMovable = false;
    bottomMap[16][25].isMovable = false;
    bottomMap[16][28].isMovable = false;
    bottomMap[16][29].isMovable = false;
    bottomMap[16][30].isMovable = false;
    bottomMap[16][31].isMovable = false;

    bottomMap[17][7].isMovable  = false;
    bottomMap[17][8].isMovable  = false;
    bottomMap[17][9].isMovable  = false;
    bottomMap[17][10].isMovable = false;
    bottomMap[17][23].isMovable = false;
    bottomMap[17][24].isMovable = false;
    bottomMap[17][25].isMovable = false;
    bottomMap[17][28].isMovable = false;
    bottomMap[17][29].isMovable = false;
    bottomMap[17][30].isMovable = false;
    bottomMap[17][31].isMovable = false;


    for( int i = 0; i < 32; i++ )
    {
        for( int j = -(i-16)/2; j < 9; j++ )
        {
            if(!arenaChecking(j+(i-16)/2,i))
            {
                bottomMap[j+(i-16)/2][i].isMovable = false;
            }
        }
    }

    int id;
    for( int i = 0; i < 18; i++ )
    {
        for( int j = 0; j < 32; j++ )
        {
            id = i*32 + j;
            bottomMap[i][j].vertexId = id;
            bottomMap[i][j].vertexX  = j   - 16.0;
            bottomMap[i][j].vertexY  = 8.0 -    i;


            // horizontal/vertical directions included first for a motive, think !

            // north
            if( i > 0 &&            bottomMap[i-1][j].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id - 32 );

            // east
            if( j < 31 &&           bottomMap[i][j+1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id +  1 );

            // south
            if( i < 17 &&           bottomMap[i+1][j].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id + 32 );

            // west
            if( j > 0 &&            bottomMap[i][j-1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id -  1 );

            // north-east
            if( i > 0 && j < 31 &&  bottomMap[i-1][j+1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id - 31 );

            // south-east
            if( i < 17 && j < 31 && bottomMap[i+1][j+1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id + 33 );

            //south-west
            if( i < 17 && j > 0 &&  bottomMap[i+1][j-1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id + 31 );

            // north-west
            if( i > 0 && j > 0 &&   bottomMap[i-1][j-1].isMovable )
                bottomMap[i][j].adjacencyList.push_back( id - 33 );
        }
    }
}

void initializeTilesTop()
{
    for( int i = 0; i < 18; i++ )
    {
        for( int j = 0; j < 32; j++ )
            topMap[0][0].isMovable   = true;
    }

    topMap[0][0].isMovable   = false;
    topMap[0][1].isMovable   = false;
    topMap[0][2].isMovable   = false;
    topMap[0][3].isMovable   = false;
    topMap[0][4].isMovable   = false;
    topMap[0][6].isMovable   = false;
    topMap[0][7].isMovable   = false;
    topMap[0][8].isMovable   = false;
    topMap[0][9].isMovable   = false;
    topMap[0][18].isMovable  = false;
    topMap[0][19].isMovable  = false;
    topMap[0][20].isMovable  = false;

    topMap[1][0].isMovable   = false;
    topMap[1][1].isMovable   = false;
    topMap[1][2].isMovable   = false;
    topMap[1][3].isMovable   = false;
    topMap[1][4].isMovable   = false;
    topMap[1][7].isMovable   = false;
    topMap[1][8].isMovable   = false;
    topMap[1][16].isMovable  = false;
    topMap[1][17].isMovable  = false;
    topMap[1][18].isMovable  = false;
    topMap[1][19].isMovable  = false;
    topMap[1][20].isMovable  = false;
    topMap[1][21].isMovable  = false;
    topMap[1][24].isMovable  = false;

    topMap[2][2].isMovable   = false;
    topMap[2][3].isMovable   = false;
    topMap[2][4].isMovable   = false;
    topMap[2][18].isMovable  = false;
    topMap[2][19].isMovable  = false;
    topMap[2][20].isMovable  = false;
    topMap[2][22].isMovable  = false;
    topMap[2][23].isMovable  = false;
    topMap[2][24].isMovable  = false;
    topMap[2][25].isMovable  = false;

    topMap[3][3].isMovable   = false;
    topMap[3][4].isMovable   = false;
    topMap[3][5].isMovable   = false;
    topMap[3][6].isMovable   = false;
    topMap[3][22].isMovable  = false;
    topMap[3][23].isMovable  = false;
    topMap[3][24].isMovable  = false;
    topMap[3][25].isMovable  = false;
    topMap[3][26].isMovable  = false;
    topMap[3][27].isMovable  = false;

    topMap[4][0].isMovable   = false;
    topMap[4][1].isMovable   = false;
    topMap[4][5].isMovable   = false;
    topMap[4][6].isMovable   = false;
    topMap[4][11].isMovable  = false;
    topMap[4][23].isMovable  = false;
    topMap[4][24].isMovable  = false;
    topMap[4][25].isMovable  = false;
    topMap[4][26].isMovable  = false;
    topMap[4][27].isMovable  = false;

    topMap[5][0].isMovable   = false;
    topMap[5][1].isMovable   = false;
    topMap[5][11].isMovable  = false;
    topMap[5][12].isMovable  = false;
    topMap[5][23].isMovable  = false;
    topMap[5][24].isMovable  = false;
    topMap[5][25].isMovable  = false;
    topMap[5][26].isMovable  = false;
    topMap[5][27].isMovable  = false;
    topMap[5][30].isMovable  = false;
    topMap[5][31].isMovable  = false;

    topMap[6][0].isMovable   = false;
    topMap[6][1].isMovable   = false;
    topMap[6][4].isMovable   = false;
    topMap[6][5].isMovable   = false;
    topMap[6][10].isMovable  = false;
    topMap[6][11].isMovable  = false;
    topMap[6][12].isMovable  = false;
    topMap[6][23].isMovable  = false;
    topMap[6][29].isMovable  = false;
    topMap[6][30].isMovable  = false;
    topMap[6][31].isMovable  = false;

    topMap[7][0].isMovable   = false;
    topMap[7][1].isMovable   = false;
    topMap[7][3].isMovable   = false;
    topMap[7][4].isMovable   = false;
    topMap[7][5].isMovable   = false;
    topMap[7][11].isMovable  = false;
    topMap[7][12].isMovable  = false;
    topMap[7][29].isMovable  = false;
    topMap[7][30].isMovable  = false;

    topMap[8][0].isMovable   = false;
    topMap[8][4].isMovable   = false;
    topMap[8][5].isMovable   = false;
    topMap[8][6].isMovable   = false;
    topMap[8][7].isMovable   = false;
    topMap[8][14].isMovable  = false;
    topMap[8][15].isMovable  = false;
    topMap[8][16].isMovable  = false;
    topMap[8][25].isMovable  = false;
    topMap[8][26].isMovable  = false;
    topMap[8][29].isMovable  = false;
    topMap[8][30].isMovable  = false;

    topMap[9][4].isMovable   = false;
    topMap[9][5].isMovable   = false;
    topMap[9][6].isMovable   = false;
    topMap[9][7].isMovable   = false;
    topMap[9][14].isMovable  = false;
    topMap[9][15].isMovable  = false;
    topMap[9][16].isMovable  = false;
    topMap[9][19].isMovable  = false;
    topMap[9][25].isMovable  = false;
    topMap[9][30].isMovable  = false;

    topMap[10][4].isMovable  = false;
    topMap[10][5].isMovable  = false;
    topMap[10][6].isMovable  = false;
    topMap[10][7].isMovable  = false;
    topMap[10][19].isMovable = false;
    topMap[10][20].isMovable = false;

    topMap[11][4].isMovable  = false;
    topMap[11][5].isMovable  = false;
    topMap[11][6].isMovable  = false;
    topMap[11][7].isMovable  = false;
    topMap[11][18].isMovable = false;
    topMap[11][19].isMovable = false;
    topMap[11][20].isMovable = false;
    topMap[11][21].isMovable = false;

    topMap[12][5].isMovable  = false;
    topMap[12][6].isMovable  = false;
    topMap[12][7].isMovable  = false;
    topMap[12][9].isMovable  = false;
    topMap[12][10].isMovable = false;
    topMap[12][11].isMovable = false;
    topMap[12][19].isMovable = false;
    topMap[12][20].isMovable = false;

    topMap[13][5].isMovable  = false;
    topMap[13][6].isMovable  = false;
    topMap[13][10].isMovable = false;
    topMap[13][11].isMovable = false;
    topMap[13][24].isMovable = false;
    topMap[13][25].isMovable = false;

    topMap[14][8].isMovable  = false;
    topMap[14][24].isMovable = false;
    topMap[14][25].isMovable = false;
    topMap[14][26].isMovable = false;
    topMap[14][27].isMovable = false;

    topMap[15][8].isMovable  = false;
    topMap[15][9].isMovable  = false;
    topMap[15][26].isMovable = false;
    topMap[15][27].isMovable = false;
    topMap[15][28].isMovable = false;
    topMap[15][30].isMovable = false;
    topMap[15][31].isMovable = false;

    topMap[16][8].isMovable  = false;
    topMap[16][9].isMovable  = false;
    topMap[16][23].isMovable = false;
    topMap[16][24].isMovable = false;
    topMap[16][25].isMovable = false;
    topMap[16][28].isMovable = false;
    topMap[16][29].isMovable = false;
    topMap[16][30].isMovable = false;
    topMap[16][31].isMovable = false;

    topMap[17][7].isMovable  = false;
    topMap[17][8].isMovable  = false;
    topMap[17][9].isMovable  = false;
    topMap[17][10].isMovable = false;
    topMap[17][23].isMovable = false;
    topMap[17][24].isMovable = false;
    topMap[17][25].isMovable = false;
    topMap[17][28].isMovable = false;
    topMap[17][29].isMovable = false;
    topMap[17][30].isMovable = false;
    topMap[17][31].isMovable = false;


    for( int i = -9; i < 9; i++ )
    {
        for( int j = -16; j < -i * 16 / 9 - 1; j++ )
        {
            if( !arenaChecking(coordinateToRow(j,i),coordinateToCol(j,i)) )
                topMap[coordinateToRow(j,i)][coordinateToCol(j,i)].isMovable = false;
        }
    }

    int id;
    for( int i = 0; i < 18; i++ )
    {
        for( int j = 0; j < 32; j++ )
        {
            id = i*32 + j;
            topMap[i][j].vertexId = id;
            topMap[i][j].vertexX  = j   - 16.0;
            topMap[i][j].vertexY  = 8.0 -    i;


            // horizontal/vertical directions included first for a motive, think !

            // north
            if( i > 0 &&            topMap[i-1][j].isMovable )
                topMap[i][j].adjacencyList.push_back( id - 32 );

            // east
            if( j < 31 &&           topMap[i][j+1].isMovable )
                topMap[i][j].adjacencyList.push_back( id +  1 );

            // south
            if( i < 17 &&           topMap[i+1][j].isMovable )
                topMap[i][j].adjacencyList.push_back( id + 32 );

            // west
            if( j > 0 &&            topMap[i][j-1].isMovable )
                topMap[i][j].adjacencyList.push_back( id -  1 );

            // north-east
            if( i > 0 && j < 31 &&  topMap[i-1][j+1].isMovable )
                topMap[i][j].adjacencyList.push_back( id - 31 );

            // south-east
            if( i < 17 && j < 31 && topMap[i+1][j+1].isMovable )
                topMap[i][j].adjacencyList.push_back( id + 33 );

            //south-west
            if( i < 17 && j > 0 &&  topMap[i+1][j-1].isMovable )
                topMap[i][j].adjacencyList.push_back( id + 31 );

            // north-west
            if( i > 0 && j > 0 &&   topMap[i-1][j-1].isMovable )
                topMap[i][j].adjacencyList.push_back( id - 33 );
        }
    }
}
