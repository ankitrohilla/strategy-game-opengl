#define NO_OF_ITEMS 6
#define ITEMS_AT_A_TIME 4
#define TEMPLE_HEALTH_RAISE 800
#define STUN_TIME 8
#define ATTACK_MULTIPLE 3
#define SPEED_MULTIPLE 3
#define VANDALISM_DAMAGE 500
#define ATTACK_MULTIPLE_TIME 10
#define SPEED_MULTIPLE_TIME 15
#define INVINCIBLE_TIME 10
#define LOCK_TIME 500000
#define SLOW_MAGIC_MULTIPLE 0.2

#define SLOW_TIME 8
#define BLACKOUT_TIME 5
#define DISABLE_TIME 10

#define FPS 50

#define VERY_LOW_SPEED 15
#define LOW_SPEED 20
#define HIGH_SPEED 30
#define VERY_HIGH_SPEED 35

#define VERY_LOW_HEALTH 500
#define LOW_HEALTH 900
#define HIGH_HEALTH 1800
#define VERY_HIGH_HEALTH 2200

#define VERY_LOW_ATTACK 4
#define LOW_ATTACK 6
#define HIGH_ATTACK 12
#define VERY_HIGH_ATTACK 14

bool rightClicked = false, leftClicked = false;
float oglX, oglY, oglRow, oglCol;

int screenWidth, screenHeight;

bool allSet;
bool Exit = false;

bool enteredCheckAllSet = false;

bool itemsReady = false;

bool youLost = false, youWon = false;
int youLostTexture, youWonTexture;

int s; //for initialize server thread

int coordinateToRow( int positionX, int positionY );
int coordinateToCol( int positionX, int positionY );
int coordinateToId( int positionX, int positionY );
int getRowFromId( int id );
int getColFromId( int id );
int getIdFromRowCol( int row, int col);
void showStats( float x, float y, float maxHealth, float CurrentHealth);
int apply_texture(char* file);
void draw_rect_with_tex( float l, float r, float b, float t, int texture);
bool arenaChecking( int row, int col );
bool templeChecking( int row, int col, int id );
void animate( int i );
void setClientMessage();
void clientProcessReply( int heroId );
void serverProcessReceived( int heroId );
void setServerReply( int );

void* threadMyHero(void*);
void* threadFriendHero(void*);
void* threadEnemyHero1(void*);
void* threadEnemyHero2(void*);
void* threadItemManager(void*);
void* threadClient(void*);
void* threadServer(void*);

void* attackItemDurationManager(int *a);
void* speedItemDurationManager(int *a);
void* invincibleItemDurationManager(int *a);
void* replenishDurationManager(int *a);
void* slowerMagicDurationManager(int *a);
void* disablerMagicDurationManager(int *a);
void* blackoutMagicDurationManager(int *a);

// isPlayerChosen[2] says team 1, hero 0 has been chosen and you can't choose it
bool isPlayerChosen[4] = {false};

// isHeroChosen[2] says hero C has been chosen and you can't choose it
bool isHeroChosen[4] = {false};
char heroSelected[4] = {'N','N','N','N'};

int screen = 0;

// no of humans
int players = 1;

bool isServer = false;
bool initializeStarted = false;
bool isGamePlaySet = false;

bool yesBot[4] = {false};

// flag to tell display() to call displayVandalism()
bool displayVandalism = false;

char clientMessage[200];
char serverReply[200];
char clientReceived[200];
char serverReceived[200];

// textures
int waiting1, waiting2, friendPointer, enemyPointer, attackEnhancer, cross;

// placeholders to define temple's location used by bots
int enemyTempleRow, enemyTempleCol;
int myTempleRow, myTempleCol;

int itemsTexture[NO_OF_ITEMS];

int disableTexture , burstDamageTexture , slowerTexture , blackOutTexture ;


float hero0SpawnX = -16, hero0SpawnY = -8;
float hero1SpawnX = -15, hero1SpawnY = -9;
float hero2SpawnX =  15, hero2SpawnY =  7;
float hero3SpawnX =  14, hero3SpawnY =  8;

int hero0Attack, hero0MaxHealth, hero0Speed, heroATextureStandId, hero0replenishTime;
int hero1Attack, hero1MaxHealth, hero1Speed, heroBTextureStandId, hero1replenishTime;
int hero2Attack, hero2MaxHealth, hero2Speed, heroCTextureStandId, hero2replenishTime;
int hero3Attack, hero3MaxHealth, hero3Speed, heroDTextureStandId, hero3replenishTime;

int myHeroId = -1, myTeamId = -1, enemyTeamId;
int friendHeroId;
int enemyHero1Id;
int enemyHero2Id;

char myIp[15], remoteIp[15];
char myPort[7], remotePort[7];

pthread_t threadHero0, threadHero1, threadHero2, threadHero3;
pthread_t clientThreadId, serverThreadId, initializeServerThread , itemManagerThreadId;

enum gameMode{ SINGLE_PLAYER, MULTI_PLAYER }whichGameMode;
enum botMode{ AGGRESIVE, DEFENSIVE };
enum itemDesc{
    HERO_HEALTH,
    TEMPLE_HEALTH,
    STUN_ENEMY,             // eliminate enemy capability for a while
    INCREASED_ATTACK,
    INCREASED_SPEED,
    INVINCIBLE             // my hero is invincible for a while
};
enum magicPower{
    SLOWER,
    DISABLER,
    BLACKOUT,
    VANDALIZE,
    E
};

magicPower hero0MagicPower;
magicPower hero1MagicPower;
magicPower hero2MagicPower;
magicPower hero3MagicPower;

enum heroState{
    STANDING,
    MOVING_TO_REACH,
    MOVING_TO_ATTACK_ENEMY_1,
    MOVING_TO_ATTACK_ENEMY_2,
    MOVING_TO_ATTACK_TEMPLE,
    ATTACKING_ENEMY_1,
    ATTACKING_ENEMY_2,
    ATTACKING_TEMPLE,
    DEAD,
    STUNNED
};

struct heroAttributes
{
    char heroName;
    int maxHealth;
    int attackPower;
    int speed;
    magicPower whichMagicPower;
    int replenishTime;
    int heroStandTextureId;
}A,B,C,D;

heroAttributes *myChosenHero = NULL;

heroAttributes *chosenHero[4];

// Stores PNGs i.e. int returned by SOIL_load_OGL_texture();
int mapTexture;

void init();
void display();
void initializeTilesTop();
void initializeTilesBottom();

class item
{
    public:
        itemDesc whichItem;
        bool isPresent;
        int row;
        int col;
        int timeToDisplay;
        int itemTexture;
        item();
        item(itemDesc whichItem, int row, int col, int itemTexture);
        item& operator=(const item& a);
};

class tile
{
    public:
        // info related to obstacles are given in initializeTiles()
        bool isMovable = true;
        int vertexId;
        int parentId;
        int vertexX;
        int vertexY;
        bool isItemPresent = false;
        item whichItem;
        std::vector<int> adjacencyList;

        tile();
        tile& operator=(const tile& a);

}bottomMap[18][32], topMap[18][32];

typedef std::map<int,item*> itemMap;
typedef void*(*DurationManager)(void*);
typedef void*(*BotManager)(void*);

// teams with ID 0 or 1
class team
{
    public:
        int id;
        int maxHealth;
        int currentHealth;
        bool gettingAttacked = false;
        bool healthLock = false;
        itemMap itemSet;
}teams[2];

class hero
{
    public:

        tile map[18][32];

        bool healthLock = false;
        bool stateLock = false;

        bool isBot = false;
        bool enteredSourcetoDest =  false;

        bool gettingAttacked = false;
        bool mapLock = false;
        bool isDead = false;

        std::vector<int> path;
        bool mouseClickedfirst = false;
        bool initialized = false;
        bool isInvincible = false;
        bool isReplenished = true;

        // magical powers in use
        bool isSlowing = false;
        bool isDisabling = false;
        bool isBlackouting = false;
        bool hasVandalized = false;

        // has been affected by magical powers
        bool isSlowed = false;
        bool isDisabled = false;
        bool isBlackouted = false;
        bool isVandalized = false;
        int vandalizedTileId;


        int waitCounter = 0;

        // to maintain a set of vertices that are in ready Queue
        std::list<int> bfsQueue;

        // to maintain a set of vertices that have been visited during the BFS, int be the vertex Id
        std::map<int,bool> visitedSet;

        // used to account for the steps taken to move
        int stepsMoved = 0;
        int tilesToMove = 0;
        int id;
        int max_health;
        int current_health;
        float x, y;
        int target_x, target_y, next_x, next_y;
        float normalAttack, attack;
        int normalSpeed, speed;
        int col;
        int row;
        int textureStandId;
        int textureMoveId;
        float replenishTimeLeft;
        int replenishTime;

        itemDesc currentItem;

        std::vector<item> itemBag;

        int targetHeroId;

        botMode whichBotMode;
        magicPower whichMagicPower;
        heroState whichState;

        hero();
        hero( int id, int max_health, float x, float y, int attack, int speed, magicPower whichMagicPower, int textureStandId, botMode whichBotMode, int replenishTime, bool isBot);
        hero& operator=(const hero& a);
        void respawn( int id, int max_health, float x, float y, int attack, int speed, magicPower whichMagicPower, int textureStandId, botMode whichBotMode, int replenishTime, bool isBot);
        void breadthFirstSearch(int fromX, int fromY, int toX, int toY);
        void pathFinder(int fromX, int fromY, int toX, int toY, int targetHeroId);
        void moveFromSourceToDest();
        void moveFromTileToTile();
}*heroes[4];

