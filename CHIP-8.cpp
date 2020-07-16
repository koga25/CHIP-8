
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include "Opcodes.h"
#include <time.h>
#include "SDL.h"
#include "SDL_image.h"
#include <conio.h>

//CPU
unsigned short opcode;
unsigned char memory[4096];
unsigned char V[16];//indexes
unsigned short I;//index counter
unsigned short pc;//program counter
unsigned char gfx[64*32];//size of screen of graphics
unsigned char delayTimer;
unsigned char soundTimer;
unsigned short stack[16];//stack
unsigned short sp;//stack pointer
unsigned short keys[16];//keypad
const unsigned int FONTSET_START_ADDRESS = 0x50;

bool isLegacyMode = true;


//SDL
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool drawFlag = false;
bool isRunning;



//functions
void setupGraphics();
void loadGame(const char* game);
void initialize();
void emulateCycle();
void drawGraphics();
void handleEvents();
void quitGame();
    //some games use multiple implementations of certain opcodes, legacy mode is the first implementation of it;
    void legacyMode();



unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int main(int argc, char* argv[])
{


    
    setupGraphics();
    //setkeys();

    initialize();
    loadGame("pong");

    while (isRunning == true) {
        //emulate one cycle
        
        handleEvents();
        emulateCycle();
        //pc += 2; -> i need to delete the other instances of this

        //if drawFlag = true, draw on screen
        if (drawFlag) {
            drawGraphics();
        }
            

        SDL_Delay(5);
    }

    quitGame();

    return 0;
}

void setupGraphics() 
{
    //SDL_INIT_EVERYTHING
    if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_EVENTS|SDL_INIT_TIMER) == 0) {
        printf("entering here\n");
        window = SDL_CreateWindow("title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, 0);
        if (window != NULL) 
        {
            printf("window created\n");
        }
        if (renderer != NULL)
        {
            printf("renderer created\n");
        }
        //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        //SDL_RenderSetLogicalSize(renderer, 320, 240);
        SDL_RenderSetLogicalSize(renderer,64, 32);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        isRunning = true;


    }
    else 
    {
        
        isRunning = false;
    } 
    printf("oi");
}

void initialize() 
{
    //starting from instruction 0x0200
    pc = 0x0200;
    //reseting pointers;
    opcode = 0;
    I = 0;
    sp = 0;
    printf("reseting pointers\n");
    //starting random;
    srand(time(NULL));
    printf("starting random\n");
    // Clear display
    for (int x = 0; x<64*32; x++)
    {

        gfx[x] = 0;
        /*for (int y = 0; y < 32; y++)
        {
            gfx[x][y] = 0;
        }*/
    }
    printf("clearing display\n");
    // Clear stack
    printf("clearing stack");
    for (int x = 0; x < 16; x++)
    {
        stack[x] = 0;
    }
    // Clear registers V0-VF
    printf("clearing registers\n");
    for(int x=0; x < 0xF; x++)
    {
        keys[x] = V[x] = 0;
    }
     // Clear memory
    printf("clearing memory\n");
    for (int x = 0; x < 4096; x++)
    {
        memory[x] = 0;
    }

    drawFlag = true;
    //load fontset;
    for (int i = 0; i < 80; ++i)
        memory[FONTSET_START_ADDRESS + i] = chip8_fontset[i];
    printf("loading fontsets\n");

    //reset timers
    printf("reseting timers\n");
    soundTimer = 0;
    delayTimer = 0;
}

void loadGame(const char* game) {
    //opening file in binary form
    FILE* file = fopen("C:\\Users\\xerather\\source\\repos\\NesEmulator\\NesEmulator\\Games\\UFO", "rb");
    if (file == NULL) {
        printf("File not found");
        exit(EXIT_FAILURE);
    }
    else 
    {
        //going to end of file
        fseek(file, 0, SEEK_END);
        //getting size of file
        long bufferSize = ftell(file);
        printf("file size: %d", bufferSize);
        SDL_Delay(2000);
        if (bufferSize > 3584) 
        {
            printf("size of file is too big");
            exit(EXIT_FAILURE);
        }
        //going to beginning of file
        rewind(file);
        //setting the size of char array and reading the binary file in unsigned char form
        char* in = (char*)malloc(sizeof(char)*bufferSize);
        if (in == NULL)
        {
            printf("out of memory\n");
            exit(EXIT_FAILURE);
        }
        //reading file
        size_t result = fread(in, sizeof(unsigned char), bufferSize, file);
        if (result != bufferSize)
        {
            printf("reading error\n");
            SDL_Delay(5000);
        }
        //populating memory
        for (int i = 0; i < bufferSize; i++)
        {
            memory[i + 0x200] = in[i];
        }
        

        fclose(file);
        free(in);
    }
}

void emulateCycle() {
    //fetching opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    printf("pc:%X           opcode:%X\n",pc , opcode);
    /*for (int x = 0; x < 16; x++)
    {
        printf("V[%X]:%X     ", x, V[x]);
    }
    printf("\n");
    printf("I:%X          S:%x\n" ,I, stack[0]);*/
    //decoding opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
        //can be 0x0NNN, 0x00E0, 0x00EE
        switch (opcode & 0x000F)
        {
        case 0x0000:
            //clear screen
            for (int x = 0; x < 64*32; x++)
            {
                if (gfx[x] == 1)
                {
                    gfx[x] = 0;
                }
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
            }
            pc += 2;
            break;
        case 0x000E:
            //returns from subroutine
            sp--;
            pc = stack[sp];
            pc += 2;
            break;
        default:
            printf("opcode not found, opcode: [%x]\n", opcode);
            break;
        }
        break;
    case 0x1000:
        //opcode 0x1NNN Jumps to adress NNN
        pc = opcode & 0x0FFF;
        break;
    case 0x2000:
        
        stack[sp] = pc;
        sp++;
        pc = (opcode & 0x0FFF);
        //opcode 0x2NNN calls subroutine at NNN
        break;
    case 0x3000:
        // opcode 3XNN, if (Vx == NN) skips the next instruction (Usually the next instruction is a jump to skip a code block) 
        if ((V[((0x0F00 & opcode) >> 8)] == (0x00FF & opcode))) 
        {
            printf("skipped\n");
            pc += 4;
        }
        else 
        {
            printf("not skipped\n");
            pc += 2;
        }
        break;
    case 0x4000:
        //opcode 4XNN, if (Vx!=NN) skips the next instruction (Usually the next instruction is a jump to skip a code block)
        if ((V[((0x0F00 & opcode) >> 8)] != (0x00FF & opcode)) ) 
        {
            pc += 4;
        }
        else 
        {
            pc += 2;
        }
        break;
    case 0x5000:
        //opcode 5XY0, if (Vx==Vy) skips the next instruction (Usually the next instruction is a jump to skip a code block)
        if ((V[((0x0F00 & opcode) >> 8)]  == (V[((0x00F0 & opcode) >> 4)])))
        {
            pc += 4;
        }
        else 
        {
            pc += 2;
        }
        break;
    case 0x6000:
        //opcode 6XNN, (Vx = NN) sets Vx to NN
        V[((0x0F00 & opcode) >> 8)] = 0x00FF & opcode;
        pc += 2;
        break;
    case 0x7000:
        //opcode 7XNN, (Vx += NN) adds NN to Vx (Carry flag is not changed)
        V[((0x0F00 & opcode) >> 8)] += (0x00FF & opcode);
        pc += 2;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000:
            //opcode 8XY0, (Vx = Vy) sets Vx to Vy;
            V[((0x0F00 & opcode) >> 8)] = V[((0x00F0 & opcode) >> 4)];
            pc += 2;
            break;
        case 0x0001:
            //opcode 8XY1, (Vx = Vx|Vy) sets Vx to Vx or Vy bitwise OR operation 
            V[((0x0F00 & opcode) >> 8)] = V[((0x0F00 & opcode) >> 8)] | V[((0x00F0 & opcode) >> 4)];
            pc += 2;
            break;
        case 0x0002:
            //opcode 8XY2, (Vx = Vx&Vy) sets Vx to Vx and Vy bitwise AND operation
            V[((0x0F00 & opcode) >> 8)] = V[((0x0F00 & opcode) >> 8)] & V[((0x00F0 & opcode) >> 4)];
            pc += 2;
            break;
        case 0x0003:
            //opcode 8XY3, (Vx = Vx^Vy) sets Vx to Vx xor Vy bitwise XOR operation
            V[((0x0F00 & opcode) >> 8)] = V[((0x0F00 & opcode) >> 8)] ^ V[((0x00F0 & opcode) >> 4)];
            pc += 2;
            break;
        case 0x0004:
            //opcode 8XY4, (Vx += Vy) adds Vy to Vx (VF is set to 1 when there's a carry, and to 0 when there isn't.)
            if ((V[((0x0F00 & opcode) >> 8)] + V[((0x00F0 & opcode) >> 4)]) > 255) 
            {
                V[0xF] = 1;
            }
            else 
            {
                V[0xF] = 0;
            }
            V[((0x0F00 & opcode) >> 8)] += (V[((0x00F0 & opcode) >> 4)]);
            pc += 2;
            break;
        case 0x0005:
            //opcode 8XY5, (Vx -= Vy) subtracts Vy from Vx (VF is set to 0 when there's a borrow, and to 1 when there isn't)
            if (V[((0x0F00 & opcode) >> 8)] > V[((0x00F0 & opcode) >> 4)]) 
            {
                V[0xF] = 1;
            }
            else 
            {
                V[0xF] = 0;
            }
            V[((0x0F00 & opcode) >> 8)] -= V[((0x00F0 & opcode) >> 4)];
            pc += 2;
            break;
        case 0x0006:
            //opcode 8XY6, 
            //legacy mode: If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.

            //Non legacy: Store the value of register VY shifted right one bit in register VX
            //Set register VF to the least significant bit prior to the shift
            if (isLegacyMode)
            {
                V[0xF] = 0x01 & V[((0x0F00 & opcode) >> 8)];
                V[((0x0F00 & opcode) >> 8)] >>=1;
            }
            else
            {
                V[0xF] = 0x01 & V[((0x0F00 & opcode) >> 8)];
                V[((0x0F00 & opcode) >> 8)] = V[((0x0000 & opcode) >> 4)] >>= 1;
            }
            pc += 2;
            break;
        case 0x0007:
            //opcode 8XY7, (Vx = Vy-Vx) Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            if (V[((0x00F0 & opcode) >> 4)] > V[((0x0F00 & opcode) >> 8)]) 
            {
                V[0xF] = 1;
            }
            else {
                V[0xF] = 0;
            }
            V[((0x0F00 & opcode) >> 8)] = V[((0x00F0 & opcode) >> 4)] - V[((0x0F00 & opcode) >> 8)];
            pc += 2;
            break;
        case 0x000E:
            //opcode 8XYE, 
            //legacy mode: If the most - significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.

            //non legacy: Store the value of register VY shifted left one bit in register VX
            //Set register VF to the most significant bit prior to the shift
            //0x80 is 10000000 in binary
            if (isLegacyMode)
            {

                V[0xF] = V[((0x0F00 & opcode) >> 8)] >> 7;
                V[((0x0F00 & opcode) >> 8)] <<= 1;
            }
            else
            {
                V[0xF] = V[((0x00F0 & opcode) >> 8)] >> 7;
                V[((0x0F00 & opcode) >> 8)] = V[((0x00F0 & opcode) >> 4)] <<= 1;
            }
            pc += 2;
            break;
        default:
            printf("opcode not found, opcode: [%x]\n", opcode);
            break;
        }
        break;
    case 0x9000:
        //opcode 9XY0, if(Vx!=Vy) Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
        if ((V[((0x0F00 & opcode) >> 8)] != V[((0x00F0) & opcode) >> 4])) 
        {
            pc += 4;
        }
        else {
            pc += 2;
        }
        break;
    case 0xA000:
            I = opcode & 0x0FFF;
            pc += 2;
            break;
    case 0xB000:
        //opcode 0xBNNN (PC = V0+NNN), Jumps to the adress NNN plus V0
        pc = V[0x0] + (0x0FFF & opcode);
        break;
    case 0xC000:
        //opcode 0xCXNN, (Vx=rand()&NN) Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        V[((0x0F00 & opcode) >> 8)] = (rand() % 0xFF) & (0x00FF & opcode);
        pc += 2;
        break;
    case 0xD000:
    {
        //opcode 0xDXYN, draw(Vx,Vy,N) Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of 
        //N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change 
        //after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped 
        //from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
        unsigned short x = V[((0x0F00 & opcode) >> 8)] % 64;
        unsigned short y = V[((0x00F0 & opcode) >> 4)] % 32;
        unsigned short height = (0x000F & opcode);
        unsigned short pixel;
        printf("x:%x,     y:%x,     height%d\n", x, y, height);
        V[0xF] = 0;
        for (int Y=0; Y < height; Y++) 
        {
            pixel = memory[I + Y];
            for (int X = 0; X < 8; X++) 
            {
                //0x80 == 1000000;
                if ((pixel & (0x80 >> X)) != 0 && (((y + Y) * 64) + (x + X)) < 2048)
                {
                    if(gfx[(((y+Y)*64) + (x + X))] == 1)
                    {
                        V[0xF] = 1;
                    }
                    gfx[(((y + Y) * 64) + (x + X))] ^= 1;
                    /*if (gfx[x+X][y+Y] == 1) 
                    {
                        V[0xF] = 1;
                    }
                    gfx[x+X][y+Y] ^= 1;*/
                }
            }
        }
        drawFlag = true;
        pc += 2;
        break;
    }
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            //opcode 0xEX9E, if(key()==Vx) Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
            if (keys[V[((0x0F00 & opcode) >> 8)]] == 0)
            {
                pc += 4;
            }
            else 
            {
                pc += 2;
            }
            break;
        case 0x00A1:
            //opcode 0xEXA1, if(key()!=Vx) Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
            if (keys[V[((0x0F00 & opcode) >> 8)]] != 0)
            {
                pc += 4;
            }
            else
            {
                pc += 2;
            }
            break;
        default:
            printf("opcode not found, opcode: [%x]\n", opcode);
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            //opcode 0xFX07, Vx = get_delay(), Sets VX to the value of the delay timer.
            V[((0x0F00 & opcode) >> 8)] = delayTimer;
            pc += 2;
            break;
        case 0x000A:
        {
            bool keyPressed = false;
            //opcode 0xFX0A, Vx = get_key()	A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
            for (int x = 0; x < 16; x++)
            {
                if (keys[x] == 1)
                {
                    V[((0x0F00 & opcode) >> 8)] = x;
                    keyPressed = true;
                }
            }
            if (!keyPressed)
                return;
            pc += 2;
            break;
        }
        case 0x0015:
            //opcode 0xFX15, delay_timer(Vx) Sets the delay timer to VX.
            delayTimer = V[((0x0F00 & opcode) >> 8)];
            pc += 2;
            break;
        case 0x0018:
            //opcode 0xFX18, sound_timer(Vx) Sets the sound timer to VX.
            soundTimer = V[((0x0F00 & opcode) >> 8)];
            pc += 2;
            break;
        case 0x001E:
            //opcode 0xFX1E, I +=Vx	Adds VX to I. VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to 0 when there isn't.
            if ((I + V[((0x0F00 & opcode) >> 8)]) > 0xFFF) 
            {
                V[0xF] = 1;
            }
            else
            {
                V[0xF] = 0;
            }
            I += V[((0x0F00 & opcode)>> 8)];
            pc += 2;
            break;
        case 0x0029:
            //opcode 0xFX29, I=sprite_addr[Vx] Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            I = FONTSET_START_ADDRESS + (V[((0x0F00 & opcode) >> 8)] * 5);
            pc += 2;
            break;
        case 0x0033:
            //opcode 0xFX33, check wikipedia
            memory[I] = ((V[((0x0F00 & opcode) >> 8)]) / 100)%10;
            memory[I+1] = ((V[((0x0F00 & opcode) >> 8)]) / 10) % 10;
            memory[I+2] = ((V[((0x0F00 & opcode) >> 8)])) % 10;
            pc += 2;
            break;
        case 0x0055:
        {
            //opcode 0XFX55, reg_dump(Vx,&I) Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            int x = 0;
            int lenght = 0x00F00 & opcode;
            while (x <= lenght) 
            {
                memory[I + x] = V[x];
                x++;
            }
            //wikipedia says not to up I but in other's source code they up the I
            if (!isLegacyMode)
            {
                I += lenght + 1;
            }
            pc += 2;
            break;
        }
        case 0x0065: 
        {
            //opcode 0xFX65, reg_load(Vx,&I) Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            int x = 0;
            int lenght = ((0x0F00 & opcode)>>8); 
            while (x <= lenght)
            {
                V[x] = memory[I + x];
                x++;
            }
            //wikipedia says not to up I but in other's source code they up the I
            if (!isLegacyMode)
            {
                I += lenght + 1;
            }
            pc += 2;
            break;
        }
        default:
            printf("opcode not found, opcode: [%x] 542\n", opcode);
            break;
        }
        break;
    default:
        printf("opcode not found, opcode: [%x] 546\n", opcode);
        break;
    }

    if (delayTimer > 0)
        --delayTimer;
    
    if (soundTimer > 0) {
        if (soundTimer == 1)
            printf("Beep\n");
        --soundTimer;
    }

}

void drawGraphics() {
    for (int x=0; x<64; x++)
    {
        for (int y = 0; y < 32; y++)
        {
            if (gfx[x+(y*64)] == 1)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    SDL_RenderPresent(renderer);
    drawFlag = false;
}

void handleEvents() 
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {

        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_1:
                printf("value of key: %d,            key pressed 1\n", keys[1]);
                keys[1] = 1;
                break;
            case SDLK_2:
                printf("value of key: %d,            key pressed 2\n", keys[2]);
                keys[2] = 1;
                break;
            case SDLK_3:
                printf("value of key: %d,            key pressed 3\n", keys[3]);
                keys[3] = 1;
                break;
            case SDLK_4:
                printf("value of key: %d,            key pressed 4\n", keys[12]);
                keys[12] = 1;
                break;
            case SDLK_q:
                printf("value of key: %d,            key pressed q\n", keys[4]);
                keys[4] = 1;
                break;
            case SDLK_w:
                printf("value of key: %d,            key pressed w\n", keys[5]);
                keys[5] = 1;
                break;
            case SDLK_e:
                printf("value of key: %d,            key pressed e\n", keys[6]);
                keys[6] = 1;
                break;
            case SDLK_r:
                printf("value of key: %d,            key pressed r\n", keys[13]);
                keys[13] = 1;
                break;
            case SDLK_a:
                printf("value of key: %d,            key pressed a\n", keys[7]);
                keys[7] = 1;
                break;
            case SDLK_s:
                printf("value of key: %d,            key pressed s\n", keys[8]);
                keys[8] = 1;
                break;
            case SDLK_d:
                printf("value of key: %d,            key pressed d\n", keys[9]);
                keys[9] = 1;
                break;
            case SDLK_f:
                printf("value of key: %d,            key pressed f\n", keys[14]);
                keys[14] = 1;
                break;
            case SDLK_z:
                printf("value of key: %d,            key pressed z\n", keys[10]);
                keys[10] = 1;
                break;
            case SDLK_x:
                printf("value of key: %d,            key pressed x\n", keys[0]);
                keys[0] = 1;
                break;
            case SDLK_c:
                printf("value of key: %d,            key pressed c\n", keys[11]);
                keys[11] = 1;
                break;
            case SDLK_v:
                printf("value of key: %d,            key pressed v\n", keys[15]);
                keys[15] = 1;
                break;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
            case SDLK_1:
                keys[1] = 0;
                printf("key up 1\n");
                break;
            case SDLK_2:
                keys[2] = 0;
                break;
            case SDLK_3:
                keys[3] = 0;
                break;
            case SDLK_4:
                keys[12] = 0;
                break;
            case SDLK_q:
                keys[4] = 0;
                break;
            case SDLK_w:
                keys[5] = 0;
                break;
            case SDLK_e:
                keys[6] = 0;
                break;
            case SDLK_r:
                keys[13] = 0;
                break;
            case SDLK_a:
                keys[7] = 0;
                break;
            case SDLK_s:
                keys[8] = 0;
                break;
            case SDLK_d:
                keys[9] = 0;
                break;
            case SDLK_f:
                keys[14] = 0;
                break;
            case SDLK_z:
                keys[10] = 0;
                break;
            case SDLK_x:
                keys[0] = 0;
                break;
            case SDLK_c:
                keys[11] = 0;
                break;
            case SDLK_v:
                keys[15] = 0;
                break;
            default:
                break;
            }
            break;
        default:
            //printf("event type not found\n");
            break;
        }
    }
    
}

void quitGame()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void legacyMode()
{

}
