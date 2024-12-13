// Ritter
// 2024 M. Gerloff

#include <list>
#include "castle.hpp"
#include "assets.hpp"

using namespace blit;

Font font(font4x6);

struct GAME
{
    short state;
    int score;
    short life;
    short level;
    short ani;
    short enemy_timer;
};

struct PLAYER
{
    short sprite;
    short stone;
    short x = 78;
};

struct STONE
{
    Vec2 pos;
    float dy;
    int bonus;
};

struct ENEMY
{
    Vec2 pos;
    Vec2 vel;
    Vec2 target;
    short sprite;
};

struct PARTICLE
{
	Vec2 pos;
	Vec2 vel;
	short alpha;
    float radius;
};

GAME game;
PLAYER p;
STONE s;
ENEMY e;

Timer ani_timer;

static std::list<ENEMY> enemy;
static std::list<STONE> stone;
static std::list<PARTICLE> stoneparticles;
static std::list<PARTICLE> enemyparticles;

void NewStoneParticles(Vec2 pos)
{
	for(short i=0; i<10; i++) 
	{
	    PARTICLE sp;
        float d = rand() %360 * (pi / 180);		
        sp.pos = pos;
        sp.vel = Vec2 (sin(d), cos(d)) * Vec2(.2f, .2f);
        sp.alpha = 50 + rand() %200;
        sp.radius = rand() %40 * .1f;
        stoneparticles.push_back(sp);
	}
}

void UpdateStoneParticles()
{
    for(auto sp = stoneparticles.begin(); sp != stoneparticles.end();) 
	{
        if(sp->alpha < 5 || sp->radius <=0) 
		{
            sp = stoneparticles.erase(sp);
            continue;
        }
        sp->pos += sp->vel;
        sp->alpha -= 3;
        sp->radius-=.1f;
        ++sp;
    }
}

void NewEnemyParticles(Vec2 pos)
{
	for(short i=0; i<15; i++) 
	{
	    PARTICLE ep;
        float d = rand() %360 * (pi / 180);		
        ep.pos = pos;
        ep.vel = Vec2 (sin(d),cos(d)) * Vec2(.2f, .2f);
        ep.alpha = 155 + rand() %100;
        enemyparticles.push_back(ep);
	}
}

void UpdateEnemyParticles()
{
    for(auto ep = enemyparticles.begin(); ep != enemyparticles.end();) 
	{
        if(ep->alpha < 2) 
		{
            ep = stoneparticles.erase(ep);
            continue;
        }
        ep->vel.y += .03f;
        if (ep->pos.y < 107)
            ep->pos += ep->vel;
        ep->alpha -= 2;
        ++ep;
    }
}

void NewEnemy()
{
    if (game.enemy_timer > 0)
    {
        game.enemy_timer--;
    }
    else
    {
        ENEMY e;
        short pos=rand() %5;
        if (pos > (1 + rand() %2))
        {
            e.sprite = 8;
            e.pos = Vec2(160,100);
            e.vel = Vec2(-1,0);
        }
        else
        {
            e.sprite = 4;
            e.pos = Vec2(-6,100);
            e.vel = Vec2(1,0);
        }
        e.target = Vec2((pos * 16 + 45),100);
        enemy.push_back(e);

        game.enemy_timer = 12 + rand() %32;
    }
}

void UpdateEnemy()
{
    for(auto e = enemy.begin(); e != enemy.end();) 
	{
        if (e->pos == e->target)
        {
            if(e->sprite > 0)
            {
                e->sprite = 0;
                e->target = Vec2(e->pos.x,40);
                e->vel = Vec2(0,-1);
            }
            else
            {
                game.life--;
                if (game.life == 0)//game over
                {
                    game.score = 0;
                    game.life = 3;
                }
                e = enemy.erase(e);
                continue;
            }
        }
        e->pos += e->vel;
        ++e;
    }
}

void UpdateStone()
{
    for(auto s = stone.begin(); s != stone.end();) 
	{
        if (s->pos.y > 103)
        {
            NewStoneParticles(s->pos + Vec2(1,1));
            s = stone.erase(s);
            continue;
        }
        for(auto e = enemy.begin(); e != enemy.end();)
        {
            if (s->pos.x < e->pos.x+5 && s->pos.x+3 > e->pos.x && s->pos.y+3 > e->pos.y)
            {
                s->bonus++;
                game.score+=s->bonus;
                NewEnemyParticles(e->pos + Vec2(3,3));
                e = enemy.erase(e);
                continue;
            }
            ++e;
        }
        s->dy+=.03f;
        s->pos.y+=s->dy;
        ++s;
    }
}

void Ani(Timer &t)
{
    game.ani++;
    if (game.ani > 3)
        game.ani = 0;
    NewEnemy();
    UpdateEnemy();
}

void Control()
{
    if (buttons & Button::DPAD_LEFT && p.sprite != 2)
    {
        if (p.x > 40)
        {
            p.sprite = 1;
            p.x--;
        }
        else if (p.stone == 0)
            p.stone = 3;
    }
    else if (buttons & Button::DPAD_RIGHT && p.sprite != 1)
    {
        if (p.x < 116)
        {
            p.sprite = 2;
            p.x++;
        }
        else if (p.stone == 0)
            p.stone = 3;
    }
    else if (buttons & Button::A && p.stone == 3)
    {
        p.stone = 0;
        STONE s;
        s.dy = .1f;
        s.pos = Vec2(p.x,34);
        s.bonus = 0;
        stone.push_back(s);
    }
    else
    {
        p.sprite = 0;
    }
}

// init()
void init() 
{
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(sprites);

    NewEnemy();
    ani_timer.init(Ani, 75, -1);
    ani_timer.start();

    game.score = 0;
    game.life = 3;
}

// render()
void render(uint32_t time) 
{
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.mask = nullptr;
    screen.alpha = 255;

    screen.stretch_blit(screen.sprites,Rect(0, 8, 8, 7), Rect(0, 8, 160, 98));

    if (p.stone == 3)
        screen.sprite(6, Point(p.x,33));
    screen.sprite(p.sprite + p.stone, Point(p.x,36));

    for (short i=0; i<159; i+=8)
        screen.sprite(20, Point(i, 106));
    short burg[9][13]{
        {22, 0,19,21,19,21,19,21,19,21,19, 0,22},
        {18,18,17,18,17,18,17,18,17,18,17,18,18},
        {23,18,17,18,17,18,17,18,17,18,17,18,24},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0},
        { 0,18,17,18,17,18,17,18,17,18,17,18, 0}};
    for (short y=0;y<9;y++)
    {    
        for (short x=0;x<13;x++)
        {
            if (burg[y][x]>0)
                screen.sprite(burg[y][x], Point(x * 8 + 28, y * 8 + 35));
        }
    }        
        
    for(auto &e : enemy)
        screen.sprite(32 + e.sprite + game.ani, Point(e.pos.x, e.pos.y));

    for(auto &s : stone)
        screen.sprite(6, Point(s.pos.x, s.pos.y));

    screen.pen = Pen(255,0,0);
    for(auto &ep : enemyparticles)
	{
		screen.alpha = ep.alpha;
        screen.pixel(Point(ep.pos.x, ep.pos.y));
	}

    screen.pen = Pen(255,255,255);
    for(auto &sp : stoneparticles)
	{
		screen.alpha = sp.alpha;
        screen.circle(Point(sp.pos.x, sp.pos.y),sp.radius);
	}
	screen.alpha = 255;

//    if (game.state == 0)
    for (short i=0; i<game.life; i++)
        screen.sprite(7, Point(150 - (i * 6),11));

    std::string score_txt ("000000");
    std::string score (std::to_string(game.score) + "0");
    score_txt.erase(0, score.size());
    screen.text(score_txt + score, font, Point(80, 11), true, TextAlign::top_center); 
}

// update()
void update(uint32_t time) 
{
    if (game.state == 0) // game
    {
        Control();
        UpdateStone();
        UpdateStoneParticles();
        UpdateEnemyParticles();
    }
}
