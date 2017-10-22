#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <stdio.h>

/***************************************/

#include <vector>

#define dolog std::cout<<
#define andthen <<" , "<<
#define please  <<std::endl;
static const int GRID_WIDTH = 7;
static const int GRID_HEIGHT = 6;

enum Tile {
  Empty, Player1, Player2, shinny, qtt_Tiles
};


//static std::vector < std::vector < Tile > > winnerBoard(6+10, std::vector <Tile>(7+10, Empty));


enum RoundState {
  playing, touchDown, end, finished, qtt_roundState
};

enum Player{
    player1, player2
};

void swapPlayers(Player& p){
    if(p == player1) p = player2;
    else p = player1;
}

bool fourHorizontal(const std::vector < std::vector < Tile > >& board, const unsigned int y, const unsigned int x) {

    //dolog x andthen y andthen board[y][x] please
    Tile tileColor = board[y][x];
    int cnt = 1, i;
    for(i = x + 1; i < GRID_WIDTH && board[y][i] == tileColor; i++) cnt++;
    for(i = x - 1; i > 0 && board[y][i] == tileColor; i--) cnt++;

    if(cnt>=4) {
        //while(i+1 < GRID_WIDTH && board[y][++i] == tileColor) winnerBoard[y][i] = shinny;
        return true;
    }
    return false;
}

bool fourVertical(const std::vector < std::vector < Tile > >& board, const unsigned int y, const unsigned int x) {

    Tile tileColor = board[y][x];
    int cnt = 1, i;
    for(i = y + 1; i < GRID_HEIGHT && board[i][x] == tileColor; i++) cnt++;
    for(i = y - 1; i > 0 && board[i][x] == tileColor; i--) cnt++;
    if(cnt>=4) {
        //while(i+1 < GRID_HEIGHT && board[++i][x] == tileColor) winnerBoard[i][x] = shinny;
        return true;
    }
    return false;
}

bool fourDiagonal(const std::vector < std::vector < Tile > >& board, const unsigned int y, const unsigned int x){
    Tile tileColor = board[y][x];
    bool t = false;
    int cnt = 1, i, j;
    for(i = y + 1, j = x - 1; i < GRID_HEIGHT && j > 0 && board[i][j]==tileColor; i++, j--) cnt++;
    for(i = y - 1, j = x + 1; i > 0 && j < GRID_WIDTH && board[i][j]==tileColor; i--, j++) cnt++;
    if(cnt>=4) {
        //while(i+1 < GRID_HEIGHT && j-1 > 0 && board[++i][--j] == tileColor) winnerBoard[i][j] = shinny;
        t = true;
    }
    cnt = 1;
    for(i = y + 1, j = x + 1; i < GRID_HEIGHT && j < GRID_WIDTH && board[i][j]==tileColor; i++, j++) cnt++;
    for(i = y - 1, j = x - 1; i > 0 && j > 0 && board[i][j]==tileColor; i--, j--) cnt++;
    if(cnt>=4) {
        //while(i+1 < GRID_HEIGHT && j+1 < GRID_WIDTH && board[++i][++j] == tileColor) winnerBoard[i][j] = shinny;
        return true;
    }
    return t;
}

bool isEndOfGame(const std::vector < std::vector < Tile > >& board, const unsigned int y, const unsigned int x) {
    //dolog "checking end of game" please
    bool t = false;
    t |= fourHorizontal(board, y, x);
    //dolog t please
    t |= fourVertical(board, y, x);
    //dolog t please
    t |= fourDiagonal(board, y, x);
    //dolog t please
    //dolog "checked" please
    return t;
}

/***************************************/
bool isWhite(sf::Image& image, float px, float py){
    return image.getPixel(px, py) == sf::Color::White;
}

float getAngle(sf::Vector2f &orig, sf::Vector2i &des) {
    return std::atan2(des.y - orig.y, des.x - orig.x)*180/(M_PI);
}

namespace State{
enum state {
  Idle, Speaking, Searching, GroupChat, fadeOut
};
}

float distance(const sf::Vector2f &orig, const sf::Vector2f &des) {
    return std::sqrt(std::pow(std::abs(des.x-orig.x), 2) + std::pow(std::abs(des.y-orig.y), 2));
}

float getAngle(const sf::Vector2f &orig,const sf::Vector2f &des) {
    return std::atan2(des.y - orig.y, des.x - orig.x)*180/(M_PI);
}

int main(){

    /* initialize random seed: */
    srand (time(NULL));


    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), L"quatrenratlla", sf::Style::Resize|sf::Style::Close);


    window.setFramerateLimit(30);


/***************************************************/
    int aux;
    sf::Event event;
    bool waitingTouch = false;
    //sf::CircleShape spoiler(10);

    sf::ConvexShape spoiler(GRID_WIDTH);
        int index = 0;
        spoiler.setPoint(index, sf::Vector2f(-20,0)); ++index;
        spoiler.setPoint(index, sf::Vector2f(-10,0)); ++index;
        spoiler.setPoint(index, sf::Vector2f(-10,-20)); ++index;
        spoiler.setPoint(index, sf::Vector2f(10,-20)); ++index;
        spoiler.setPoint(index, sf::Vector2f(10,0)); ++index;
        spoiler.setPoint(index, sf::Vector2f(20,0)); ++index;
        spoiler.setPoint(index, sf::Vector2f(0,20)); ++index;

//    spoiler.setOrigin(0,0);

    spoiler.setFillColor(sf::Color::Green);

    sf::Vector2f touchPos;
    sf::Vector2i lastTokenPos;
    RoundState state = playing;
    Player currentPlayer = player1;

    std::vector < std::vector < Tile > > matrix (GRID_HEIGHT, std::vector< Tile > (GRID_WIDTH, Empty));

//    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "dos", sf::Style::Close);
/*    while(window.isOpen()){
        window.clear();
        sf::CircleShape c(10);
        c.setPosition(window.getSize().x/2, window.getSize().y/2);
        c.setFillColor(sf::Color::Green);
        window.draw(c);
        window.display();
    }*/

    const float space = std::min(window.getSize().x/GRID_WIDTH, window.getSize().y/GRID_HEIGHT);
    const float leftOffset = (window.getSize().x - space*GRID_WIDTH)*0.5f;

    sf::RectangleShape rectangleDeColor (sf::Vector2f(space*GRID_WIDTH, space*GRID_HEIGHT));
    rectangleDeColor.setFillColor(sf::Color::Blue);
    rectangleDeColor.setPosition(sf::Vector2f(leftOffset, 0.0f));

    window.setFramerateLimit(30);
/***************************************************/
    //GAME LOOP
    while(window.isOpen()){

        //Loop for handling events
        while(window.pollEvent(event)){
            switch (event.type){
                //Close event
                case sf::Event::Closed:
                    window.close();
                    break;
                //KeyPressed event
                case  sf::Event::KeyPressed:
                    //Close key
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                //Default
                default:
                    //Do nothing
                    break;
            }
        }


        if ((state == playing || state == touchDown) && (sf::Touch::isDown(0)||sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
            state = touchDown;
            if(sf::Touch::isDown(0)){
                touchPos = sf::Vector2f(sf::Touch::getPosition(0));
            }
            else{
                touchPos = sf::Vector2f(sf::Mouse::getPosition(window));
            }
            //set arrow position to tocolumn(touchPos.x), 0;
            spoiler.setPosition(touchPos.x, 10);
        }

        if(state == touchDown && !(sf::Touch::isDown(0)||sf::Mouse::isButtonPressed(sf::Mouse::Left))) {

            int column = (GRID_WIDTH)* ((touchPos.x-leftOffset)/(space * GRID_WIDTH));
            // Place in the closes column if click was out of board.
            column = std::max(0, std::min(column, GRID_WIDTH-1));

            std::cout << " column "<< column  << " touchpos " <<touchPos.x << " gridwidth" << GRID_WIDTH << " w size x " << window.getSize().x << std::endl;

            for(uint i = GRID_HEIGHT-1; i >= 0 && state != end; --i){
                if(matrix[i][column] == Empty){
                    lastTokenPos.x = i;
                    lastTokenPos.y = column;
                    if(currentPlayer == player1){
                        matrix[i][column] = Player1;
                    }else {
                        matrix[i][column] = Player2;
                    }
                    state = end;
                }
            }
            if(state == touchDown) state = playing;
        }

        if(state == end){
            std::cout << "end";
            if(isEndOfGame(matrix, lastTokenPos.x, lastTokenPos.y)){
                //dolog "GAME FINISHED YA'LL" please
                waitingTouch = true;
                state = finished;
            }else {
                swapPlayers(currentPlayer);
                state = playing;
            }
            std::cout << "ended";
        }

        if(state == finished){
            if(waitingTouch && (sf::Touch::isDown(0)||sf::Mouse::isButtonPressed(sf::Mouse::Left)) ){
                for(int f = 0; f < matrix.size(); ++f){
                    for(int c = 0; c < matrix[0].size(); ++c){
                        matrix[f][c] = Empty;
                        //winnerBoard[f][c] = Empty;
                    }
                }
                waitingTouch = false;
            }
            if(!waitingTouch && !(sf::Touch::isDown(0)||sf::Mouse::isButtonPressed(sf::Mouse::Left)) ){
                state = playing;
                swapPlayers(currentPlayer);
            }
        }

        window.clear(currentPlayer == player1 ? sf::Color::Yellow : sf::Color::Red);
        window.draw(rectangleDeColor);

        sf::CircleShape circle(space/2.5);
        for(int f = 0; f < matrix.size(); ++f){
            for(int c = 0; c < matrix[0].size(); ++c){
                circle.setPosition(leftOffset + c*space + space*0.1, f*space + space*0.1f);
                if(matrix[f][c] == Player1) circle.setFillColor(sf::Color::Yellow);
                if(matrix[f][c] == Player2) circle.setFillColor(sf::Color::Red);
                if(matrix[f][c] == Empty) circle.setFillColor(sf::Color::Black);
                window.draw(circle);
            }
        }
        window.draw(spoiler);

        window.display();

    }
}
