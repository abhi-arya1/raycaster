/*
* 
*   Created as an attempt to better understand
*   game engine functionalities at an incredibly
*   raw level. 
*   abhi-arya1, 4/2/23 
*
*   project inspo: https://www.youtube.com/watch?v=xW8skO7MFYw
*   github of inspo: https://github.com/OneLoneCoder/CommandLineFPS/blob/master/CommandLineFPS.cpp
*
*/

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;

#include <stdio.h>
#include <Windows.h>


int nScreenWidth = 120;
int nScreenHeight = 40; 

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0; 
float fDepth = 16.0f; // Based on Map Size 

// note that we are in a radian space. 

int main()
{   
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole); 
    DWORD dwBytesWritten = 0;

    wstring map;
    map += L"################";
    map += L"#..............#";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#......####..###";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";


    auto tp1 = chrono::system_clock::now(); 
    auto tp2 = chrono::system_clock::now();
    
    // Game Loop
    while (1) 
    {   
        // get time diff
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2; 
        float fElapsedTime = elapsedTime.count();


        // Controls ----

        // rotation 
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (2.0f) * fElapsedTime;
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (2.0f) * fElapsedTime;

        // forward/backward 
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            // if wall hit, undo what has just been done. 
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') 
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }

        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }


        for (int x = 0; x < nScreenWidth; x++) 
        {
            // chop up fov range into little bits -- 'potter algorithm'-esque 
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV; 
            float fDistanceToWall = 0; // we go through increments until a collision for this 
            bool bHitWall = false; 
            bool bBoundary = false; // detect ray hitting corners of block 

            float fEyeX = sinf(fRayAngle); // unit vector for the player ray
            float fEyeY = cosf(fRayAngle); 
            

            // raycasting 
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1; // raycasting 
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); 
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;			
                    fDistanceToWall = fDepth;
                }
                else {
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true; 

                        vector<pair<float, float>> p; // dist, dot product of dist
                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }
                        // yikes. 
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });
                    
                        float fBound = 0.0075; 
                        if (acos(p.at(0).second) < fBound) bBoundary = true; 
                        if (acos(p.at(1).second) < fBound) bBoundary = true; 
                        //if (acos(p.at(2).second) < fBound) bBoundary = true; // optional 

                    }
                }
            }

            int nCeil = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeil;


            // shade wall based on unicode characters and distances. 

            short nShade = ' '; // wall shader
            short nShade2 = ' '; // floor shader

            if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	
            else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
            else if (fDistanceToWall < fDepth)				nShade = 0x2591;
            else											nShade = ' ';		
        
            if (bBoundary) nShade = ' ';

            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeil) 
                    screen[y * nScreenWidth + x] = ' '; 
                else if (y > nCeil && y<=nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else // floor shading 
                {
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)		nShade2 = '#';
                    else if (b < 0.5)	nShade2 = 'x';
                    else if (b < 0.75)	nShade2 = '.';
                    else if (b < 0.9)	nShade2 = '-';
                    else				nShade2 = ' ';
                    screen[y * nScreenWidth + x] = nShade2;
                }
            }
        }


        // Basic Stats (Given)
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, 
            (fPlayerA * 180 / 3.141529), 1.0f / fElapsedTime);
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + (nMapWidth - nx - 1)];
            }
        screen[((int)fPlayerY + 1) * nScreenWidth + (int)(nMapWidth - fPlayerX)] = 'P';


        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }

   

    return 0;
}