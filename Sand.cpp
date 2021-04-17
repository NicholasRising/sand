#include <cstdlib>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <utility>

struct Point{
    int x,y;
};

const int SCREEN_WIDTH=1000;
const int SCREEN_HEIGHT=1000;
const int SCALE=4;
const int WIDTH=SCREEN_WIDTH/SCALE;
const int HEIGHT=SCREEN_HEIGHT/SCALE;
SDL_Window *window;
SDL_Surface *surface;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Event e;
bool quit,lmbDown,rmbDown,paused;
char material;
int cycles,brushRadius;
uint32_t materialColors[4];

void init();
void close();
void tick(char grid[WIDTH][HEIGHT]);
void sand(char grid[WIDTH][HEIGHT],int x,int y);
void water(char grid[WIDTH][HEIGHT],int x,int y);
void wood(char grid[WIDTH][HEIGHT],int x,int y);

int main(){
    init();
    char grid[WIDTH][HEIGHT]{};
    uint32_t pixels[HEIGHT][WIDTH]; // UpdateTexture takes row by column apparently
    int lastTick=SDL_GetTicks();
    while(!quit){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT){
                quit=true;
            }
            else if(e.type==SDL_MOUSEBUTTONDOWN){
                if(e.button.button==SDL_BUTTON_LEFT){
                    lmbDown=true;
                }
                else if(e.button.button==SDL_BUTTON_RIGHT){
                    rmbDown=true;
                }
            }
            else if(e.type==SDL_MOUSEBUTTONUP){
                if(e.button.button==SDL_BUTTON_LEFT){
                    lmbDown=false;
                }
                else if(e.button.button==SDL_BUTTON_RIGHT){
                    rmbDown=false;
                }
            }
            else if(e.type==SDL_KEYDOWN){
                if(e.key.keysym.sym==SDLK_0){
                    material=0;
                }
                else if(e.key.keysym.sym==SDLK_1){
                    material=1;
                }
                else if(e.key.keysym.sym==SDLK_2){
                    material=2;
                }
                else if(e.key.keysym.sym==SDLK_3){
                    material=3;
                }
                else if(e.key.keysym.sym==SDLK_LSHIFT){
                    if(brushRadius<40){
                        brushRadius+=2;
                    }
                }
                else if(e.key.keysym.sym==SDLK_LCTRL){
                    if(brushRadius>0){
                        brushRadius-=2;
                    }  
                }
                else if(e.key.keysym.sym==SDLK_SPACE){
                    paused=!paused;
                }
            }
        }
        int mouseX,mouseY;
        SDL_GetMouseState(&mouseX,&mouseY);
        mouseX/=SCALE;
        mouseY/=SCALE;
        if(lmbDown){
            for(int x=mouseX-brushRadius;x<mouseX+brushRadius+1;x++){
                for(int y=mouseY-brushRadius;y<mouseY+brushRadius+1;y++){
                    if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT){
                        grid[x][y]=material;
                    }
                }
            }
        }
        for(int x=0;x<WIDTH;x++){
            for(int y=0;y<HEIGHT;y++){
                pixels[y][x]=materialColors[grid[x][y]];
            }
        }
        for(int x=mouseX-brushRadius<0?0:mouseX-brushRadius;x<mouseX+brushRadius+1&&x<WIDTH;x++){
            if(mouseY-brushRadius-1>=0&&mouseY-brushRadius-1<HEIGHT){
                pixels[mouseY-brushRadius-1][x]=0xffffff;
            }
            if(mouseY+brushRadius+1>=0&&mouseY+brushRadius+1<HEIGHT){
                pixels[mouseY+brushRadius+1][x]=0xffffff;
            }
        }
        for(int y=mouseY-brushRadius<0?0:mouseY-brushRadius;y<mouseY+brushRadius+1&&y<HEIGHT;y++){
            if(mouseX-brushRadius-1>=0&&mouseX-brushRadius-1<HEIGHT){
                pixels[y][mouseX-brushRadius-1]=0xffffff;
            }
            if(mouseX+brushRadius+1>=0&&mouseX+brushRadius+1<HEIGHT){
                pixels[y][mouseX+brushRadius+1]=0xffffff;
            }
        }
        if(paused){
            for(int y=2;y<6;y++){
                pixels[y][WIDTH-3]=0xffffff;
                pixels[y][WIDTH-6]=0xffffff;
            }
        }
        SDL_UpdateTexture(texture,NULL,pixels,WIDTH*sizeof(uint32_t));
        SDL_RenderCopy(renderer,texture,NULL,NULL);
        SDL_RenderPresent(renderer);
        if(!rmbDown&&!paused){
            tick(grid);
        }
        else if(cycles%5==0&&!paused){
            tick(grid);
        }
        cycles++;
    }
    close();
    return 0;
}

void init(){
    SDL_Init(SDL_INIT_VIDEO);
    window=SDL_CreateWindow("Sand",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    surface=SDL_GetWindowSurface(window);
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STATIC,WIDTH,HEIGHT);
    quit=false;
    lmbDown=false;
    rmbDown=false;
    paused=false;
    material=1;
    cycles=0;
    brushRadius=4;
    materialColors[0]=0x000000; // Air
    materialColors[1]=0xc2b280; // Sand
    materialColors[2]=0x0f5e9c; // Water
    materialColors[3]=0x855e42; // Wood
    SDL_ShowCursor(SDL_DISABLE);
}

void close(){
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void tick(char grid[WIDTH][HEIGHT]){
    for(int x=0;x<WIDTH;x++){
        for(int y=0;y<HEIGHT;y++){
            if(grid[x][y]==1){
                sand(grid,x,y);
            }
            if(grid[x][y]==2){
                water(grid,x,y);
            }
        }
    }
}

void sand(char grid[WIDTH][HEIGHT],int x,int y){
    if(y<HEIGHT-1){
        if(grid[x][y+1]==0||grid[x][y+1]==2){
            grid[x][y]=grid[x][y+1];
            grid[x][y+1]=1;
        }
        else{
            bool leftOpen=x>0&&(grid[x-1][y+1]==0||grid[x-1][y+1]==2);
            bool rightOpen=x<WIDTH-1&&(grid[x+1][y+1]==0||grid[x+1][y+1]==2);
            if(leftOpen&&!rightOpen){
                grid[x][y]=grid[x-1][y+1];
                grid[x-1][y+1]=1;
            }
            else if(!leftOpen&&rightOpen){
                grid[x][y]=grid[x+1][y+1];
                grid[x+1][y+1]=1;
            }
            else if(leftOpen&&rightOpen){
                if(std::rand()%2){
                    grid[x][y]=grid[x-1][y+1];
                    grid[x-1][y+1]=1;
                }
                else{
                    grid[x][y]=grid[x+1][y+1];
                    grid[x+1][y+1]=1;
                }
            }
        }
    }
}

void water(char grid[WIDTH][HEIGHT],int x,int y){
    if(y<HEIGHT-1){
        if(grid[x][y+1]==0){
            grid[x][y]=grid[x][y+1];
            grid[x][y+1]=2;
        }
        else{
            bool leftOpen=x>0&&grid[x-1][y+1]==0;
            bool rightOpen=x<WIDTH-1&&grid[x+1][y+1]==0;
            if(leftOpen&&!rightOpen){
                grid[x][y]=grid[x-1][y+1];
                grid[x-1][y+1]=2;
            }
            else if(!leftOpen&&rightOpen){
                grid[x][y]=grid[x+1][y+1];
                grid[x+1][y+1]=2;
            }
            else if(leftOpen&&rightOpen){
                if(std::rand()%2){
                    grid[x][y]=grid[x-1][y+1];
                    grid[x-1][y+1]=2;
                }
                else{
                    grid[x][y]=grid[x+1][y+1];
                    grid[x+1][y+1]=2;
                }
            }
            else{
                bool leftOpen=x>0&&grid[x-1][y]==0;
                bool rightOpen=x<WIDTH-1&&grid[x+1][y]==0;
                if(leftOpen&&!rightOpen){
                    grid[x][y]=grid[x-1][y];
                    grid[x-1][y]=2;
                }
                else if(!leftOpen&&rightOpen){
                    grid[x][y]=grid[x+1][y];
                    grid[x+1][y]=2;
                }
                else if(leftOpen&&rightOpen){
                    if(std::rand()%2){
                        grid[x][y]=grid[x-1][y];
                        grid[x-1][y]=2;
                    }
                    else{
                        grid[x][y]=grid[x+1][y];
                        grid[x+1][y]=2;
                    }
                }
            }
        }
    }
    else{
        bool leftOpen=x>0&&grid[x-1][y]==0;
        bool rightOpen=x<WIDTH-1&&grid[x+1][y]==0;
        if(leftOpen&&!rightOpen){
            grid[x][y]=grid[x-1][y];
            grid[x-1][y]=2;
        }
        else if(!leftOpen&&rightOpen){
            grid[x][y]=grid[x+1][y];
            grid[x+1][y]=2;
        }
        else if(leftOpen&&rightOpen){
            if(std::rand()%2){
                grid[x][y]=grid[x-1][y];
                grid[x-1][y]=2;
            }
            else{
                grid[x][y]=grid[x+1][y];
                grid[x+1][y]=2;
            }
        }
    }
}