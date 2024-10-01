#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TITLE "pixel editor"
#define WIDTH 640
#define HEIGHT 480
#define ASCII_WIDTH 180
#define ASCII_HEIGHT 130

//sortit standard d'erreur SDL
#define ERROR_SDL(__message__)\
    fprintf(stderr, "file: %s, error at line: %d\n\t%s : %s\n", __FILE__, __LINE__, __message__, SDL_GetError());




int asciiArt(SDL_Surface* image)
{
    if(image == NULL)
        return 0;

    SDL_Surface *imgResized = SDL_CreateRGBSurface(0, ASCII_WIDTH, ASCII_HEIGHT, 32, 0, 0, 0, 0);
    if(imgResized == NULL)
        return 0;

    SDL_BlitScaled(image, NULL, imgResized, NULL);

    char ascii[] = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";
    Uint32* pixel = NULL;
    SDL_Color color;
    Uint8 gray;
    int pos;

    SDL_LockSurface(imgResized);

    for(int i = 0; i < imgResized->h; i++)
    {
        for(int j = 0; j < imgResized->w; j++)
        {
            pixel = (imgResized->pixels + i * imgResized->pitch + j * imgResized->format->BytesPerPixel);
            SDL_GetRGB(*pixel, imgResized->format, &color.r, &color.g, &color.b);
            gray = (color.r + color.g + color.b) / 3;

            pos = gray * (strlen(ascii) - 1) / 255;

            printf("%c", ascii[pos]);
        }
        printf("\n");
        SDL_Delay(10);
    }

    SDL_UnlockSurface(imgResized);

    if(imgResized != NULL)
        SDL_FreeSurface(imgResized);

    return 1;
}





int main(int argc, char* argv[])
{
    int status = EXIT_FAILURE;

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        ERROR_SDL("SDL_Init")
        goto Quit;
    }

    SDL_Surface* image = IMG_Load("onion.jpg");

    if(!asciiArt(image))
        ERROR_SDL("asciiArt")

    status = EXIT_SUCCESS;
    
Quit:
    if(image != NULL)
        SDL_FreeSurface(image);

    SDL_Quit();
    return status;
}