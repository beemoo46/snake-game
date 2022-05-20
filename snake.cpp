#include <iostream>
#include <random>
#include <exception>
#include <string>
#include <vector>

struct coordinate
{
    int x;
    int y;
};


struct ll_node
{
    coordinate pos;
    ll_node* previous   = nullptr;
    ll_node* next       = nullptr;
};


struct game_info
{
    std::string player_name;
    uint_fast32_t score = 0; //=length - 2
    uint_fast32_t highest_score = 0;
    //location of food
    coordinate food_location;

    //snake head and tail
    ll_node* snake_head = nullptr;
    ll_node* snake_tail = nullptr;

    char board[10][10] = {{' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
                        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}};

                        //use <>^v to indicate snake body and direction, and # to represent snake head
    //size of board
    int size_x = 10;
    int size_y = 10;

    uint_fast32_t empty_cells = size_x * size_y;
};


//release dynamically assigned memeries for ll_node snake_head
//para: head of the ll_node
bool free_mem(ll_node*& head, bool debug = 0){
    ll_node* wp = head;
    while(head != nullptr){
        wp = head;
        head = head->next;
        if(debug){std::cout << wp << ' ';}

        delete wp;
        wp = nullptr;
    }
    return 1;
}



//for test only
int_fast32_t check_total_empty_cell(const game_info game){
    int_fast32_t count = 0;
    for(int i = 0; i < game.size_x; ++i){
        for (int j = 0; j < game.size_y; ++j){
            if(game.board[i][j] == ' '){++count;}
        }
    }

    return count;
}


//check whether [x,y] is empty
//return 
// 0: out of bound
// 1: empty
// 2: food
// 3: others (eg. snake body)
uint_fast32_t is_empty(const game_info& game, int x, int y){
    if(x<0 || x>=game.size_x || y < 0 || y >= game.size_y){return 0;}
    else if(game.board[x][y] == ' ') return 1;
    else if(game.board[x][y] == '*') return 2;
    
    return 3;
}



//generate new food position
coordinate gen_next_food(const game_info& game){
    std::random_device device;
    std::mt19937 generator(device()); 
    std::uniform_int_distribution<int> distribution(0, game.empty_cells);

    int s;
    int temp[2];
    uint_fast32_t count = 0;

    //gen a number first
    //when empty cell is ard 5 or less, the expected number of choosing an empty cell increases rapidly: 1/P(empty)
    //so another method is used for those cases
    if(game.empty_cells < 5){
        s = distribution(generator);
        for(int i = 0; i < game.size_x; ++i){
            for(int j = 0; j < game.size_y; ++j){
                if(game.board[i][j] == ' '){
                    --s;
                }
                if(s == 0){
                    temp[0] = i;
                    temp[1] = j;
                    break;
                }
            }
            if(s==0){break;}
        }
    }
    else{ 
        while(1){
            //std::cout << s << ' ' ;
            if(count > 10000){
                std::cerr << "Error: taking too many turns to generate next food's position\n";
                throw std::exception();
                return coordinate{-1,-1};
            }

            s = distribution(generator);
            temp[0] = s%game.size_x; //x
            temp[1] = s/game.size_x; //y
            //std::cout << temp[0] << ' ' << temp[1] << '\n';
            ++count;
            if(is_empty(game, temp[0], temp[1]) == 1){std::cout << count << '\n'; break;}
        }
    }

    
    return coordinate{temp[0], temp[1]};
}


//get new food position, update struct info and game board
bool update_food(game_info& game){
    if(game.empty_cells > 0){
        coordinate next = gen_next_food(game);
        game.food_location = next;
        game.board[next.x][next.y] = '*';
        return 1;
    }


    return 0;
}




bool update_snake(game_info& game, char dir){

    coordinate ori_pos = game.snake_head->pos;
    int x = ori_pos.x;
    int y = ori_pos.y;
    char sym = ' ';

    switch (dir){
    case 'w': case 'W':
        x -= 1;
        sym = '^';
        break;
    case 'a': case 'A':
        y -= 1;
        sym = '<';
        break;
    case 's': case 'S':
        x += 1;
        sym = 'v';
        break;
    case 'd': case 'D':
        y += 1;
        sym = '>';
        break;

    default:
        break;
    }

    ll_node* wp = game.snake_head;
    uint_fast32_t temp = is_empty(game, x, y);

    //out of bound
    if(temp == 0){ 
        //game over
        //free_mem(game.snake_head); //need to declare it first
        return 0;
    }
    //empty
    else if(temp == 1){
        ll_node* tp = game.snake_tail;
        game.board[tp->pos.x][tp->pos.y] = ' ';
        game.snake_tail = game.snake_tail->previous;
        game.snake_tail->next = nullptr;
        
        
        tp->previous = nullptr;
        tp->next = game.snake_head;
        game.snake_head->previous = tp;
        game.snake_head = tp;
        tp->pos = coordinate{x,y};
        
        game.board[ori_pos.x][ori_pos.y] = sym;
        game.board[x][y] = '#';


        return 1;
    }
    //food
    else if(temp == 2){
        ++game.score;
        --game.empty_cells;
        if(game.score > game.highest_score){game.highest_score = game.score;}

        ll_node* n_head = new ll_node;
        n_head->pos = game.food_location;
        n_head->next = game.snake_head;
        game.snake_head->previous  = n_head;
        game.board[ori_pos.x][ori_pos.y] = sym;
        game.board[x][y] = '#';
        game.snake_head = n_head;
        //current food location to head

        
        //update_food
        update_food(game);

        return 1;
    }
    //others
    else if (temp == 3){
        return 0;
    }


    return 0;
}


void intro(game_info& game){
    std::cout << "The Snake game #<<<<\n\n";
    std::cout << "Enter your player name: ";
    std::cin >> game.player_name;

    std::cout << '\n' << "Welcome, " << game.player_name << "\n\n";
    
}

//initialise game before start
bool initialise(game_info& game){
    //make default value
    //reset board
    for(int i = 0; i < game.size_x; ++i){
        for(int j = 0; j < game.size_y; ++j){
            game.board[i][j] = ' ';
        }
    }
    game.empty_cells = 100;
    game.score = 0;
    game.snake_head = nullptr;
    game.snake_tail = nullptr;
    //gen default snake position
    game.snake_head = new ll_node;
    game.snake_tail = new ll_node;

    game.snake_head->previous = nullptr;
    game.snake_head->next = game.snake_tail;
    game.snake_head->pos = coordinate{4,4};
    game.snake_tail->previous = game.snake_head;
    game.snake_tail->next = nullptr;
    game.snake_tail->pos = coordinate{3,4};
    game.board[4][4] = '#';
    game.board[3][4] = 'v';


    //gen food location
    update_food(game);
    //default player position, default length
    
    //update empty cells count
    game.empty_cells -= 3;



    return 1;
}



void print_board(const game_info& game){
    //std::cout << "food location: " << game.food_location.x << ',' << game.food_location.y << '\n';
    std::cout << "\n\n";
    std::cout << "Score: " << game.score << "\t" << "Empty cells: " << game.empty_cells << '\n';
    std::cout << "Highest score: " << game.highest_score << '\n';
    std::cout << "______________________";
    std::cout << "\tEnter 'b' to exit game\n";
    for(int i = 0; i < game.size_x; ++i){
        std::cout << '|';
        for(int j = 0; j < game.size_y; std::cout << ' ', ++j){
            std::cout << (char)game.board[i][j];
        }
        std::cout << '|' << '\n';
    }

    std::cout << "______________________";
    std::cout << "\n\n";
}


bool restart(game_info& game){
    std::cout << "\nRestart? (Y/N): ";
    char choice;
    std::cin >> choice;

    switch (choice)
    {
        case 'Y': case 'y':{
            initialise(game);
            return 1;
        }
            
        default:
            return 0;
    }
    return 0;
}


bool gameover(game_info& game){
    if(game.score >  game.highest_score){
        game.highest_score = game.score;
    }
    std::cout << "Gameover!! Final Score: " << game.score << " Highest Score: " << game.highest_score << '\n';


    if(!free_mem(game.snake_head)){
        throw std::exception();
    }

    if(restart(game)){
        print_board(game);
        return 0;
    }

    return 1;
}

bool win(const game_info& game){
    if (game.empty_cells == 0){
        std::cout << "\nYou Win !!!\n";
        return 1;
    }
    return 0;
}




game_info game_service(){
    game_info game;
    char move;
    //test
    //std::cout << is_empty(game, 1,2);
    intro(game);
    initialise(game);
    print_board(game);


    //int count = 0;
    while (std::cin >> move)
    {
        if(move == 'w' || move == 'a' || move == 's' || move == 'd'){
            if(update_snake(game, move) == 1){
                print_board(game);
                if(win(game)){break;}
            }
            //end game
            else {
                
                if (gameover(game)){
                    break;
                }

            }
        }
        else{
            if(move == 'b'){break;}
            else if(move == 'c'){game.empty_cells = 1;print_board(game);continue;}
            else{continue;}
        }
        /*
        if(update_food(game)){
            print_board(game);
            std::cout << '\n' << "Empty cells: " << check_empty(game) << '\n';
        }
        */

        //++count;
        //std::cout << "count: " << count << '\n';
    }

    //std::cout << is_empty(game, 0,0);
    if(!free_mem(game.snake_head)){
        throw std::exception();
    }

    return game;
}



void test_food_gen(int time = 100){
    game_info game;
    int count[100] = {};
    int c = 0;
    while (c < time){
        if(update_food(game)){
            //print_board(game);
            ++c;
        }
    }
}

void test_free_mem(std::size_t time = 10){
    ll_node* head = new ll_node;
    ll_node* wp = head;

    for(std::size_t i = 0; i < time; ++i){
        ll_node* b = new ll_node;
        wp->next = b;
        b->previous = wp;
        b->pos = coordinate{0,0};
        wp = b;
    }

    free_mem(head, 1);
}

//bot
bool bot(const game_info& game, std::vector<std::vector<int>> path){

    coordinate h = game.snake_head->pos;
    coordinate f = game.food_location;

    // if diff.x > 0 : w, else: s
    // if diff.y > 0 : a, else: d
    coordinate diff = {h.x - f.x, h.y - f.y};
    coordinate n = {};

    if(diff.x == 0 && diff.y == 0){
        return 1;
    }
    //if(n == isempty(game,n.x,n.y)){return 1;}
    //else{return 0;}

    /*
    if(m == 'w'){

    }
    else if(m =='a'){

    }
    else if(m == 's'){

    }
    else if(m == 'd'){

    }
    */
   return 0;
}


void test(){
    test_food_gen(100);
    test_free_mem(100);
}

int main(){

    game_service();
    //test();
    return 0;
}