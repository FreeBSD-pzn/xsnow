#include "snow00.xbm"
#include "snow01.xbm"
#include "snow02.xbm"
#include "snow03.xbm"
#include "snow04.xbm"
#include "snow05.xbm"
#include "snow06.xbm"
#define SNOWFLAKEMAXTYPE 6  

#define MAXSNOWFLAKES 1000     /* Max no. of snowflakes */
#define INITIALSNOWFLAKES 100  /* Initial no. of snowflakes */
#define MAXYSTEP 8             /* falling speed max */
#define MAXXSTEP 4             /* drift speed max */
#define WHIRLFACTOR 4
#define INITWINSNOWDEPTH 8
#define INITIALSCRPAINTSNOWDEPTH 8  /* Painted in advance */
#define INITSCRSNOWDEPTH 50    /* Will build up to INITSCRSNOWDEPTH */
#define SNOWFREE 25  /* Must stay snowfree on display :) */

/* Speed for each Santa */
int Speed[] = {1,  /* Santa 0 */
               2,  /* Santa 1 */
               4}; /* Santa 2 */

/* Rudolf's nose (clumsy. Oh well.) */
int NoseX[] = {54,  107,  214};
int NoseY[] = {3,     7,   15};
int NoseXSiz[] = {2, 2, 2};
int NoseYSiz[] = {1, 1, 2};

typedef struct Snow {
    int intX;
    int intY;
    int xStep;    /* drift */
    int yStep;    /* falling speed */
    int active;
    int visible;
    int whatFlake;
} Snow;


typedef struct SnowMap {
	char *snowBits;
	Pixmap pixmap;
	int width;
	int height;
} SnowMap;

SnowMap snowPix[] = {
	{snow00_bits, None, snow00_height, snow00_width},
	{snow01_bits, None, snow01_height, snow01_width},
	{snow02_bits, None, snow02_height, snow02_width},
	{snow03_bits, None, snow03_height, snow03_width},
	{snow04_bits, None, snow04_height, snow04_width},
	{snow05_bits, None, snow05_height, snow05_width},
	{snow06_bits, None, snow06_height, snow06_width}
};


#include "tannenbaum.xbm"

typedef struct TannenbaumMap {
        char *tannenbaumBits;
        Pixmap pixmap;
        int width;
        int height;
} TannenbaumMap;

TannenbaumMap tannenbaumPix[] = {
        {tannenbaum_bits, None, tannenbaum_height, tannenbaum_width}
};


#include "sleigh1_0.xbm"
#include "sleigh1_1.xbm"
#include "sleigh1_2.xbm"
#include "sleigh2_0.xbm"
#include "sleigh2_1.xbm"
#include "sleigh2_2.xbm"
#include "sleigh3_0.xbm"
#include "sleigh3_1.xbm"
#include "sleigh3_2.xbm"

#define MAXSANTA 2     /* 3 sizes of Santa now */
 
typedef struct Santa {
    int x;
    int y;
    int xStep;  
    int yStep; 
    int visible;
    int whatSanta; /* 0-2 index in pix */
} Santa;

typedef struct SleighMap {
        char *sleighBits;
        Pixmap pixmap;
        int width;
        int height;
} SleighMap;

SleighMap sleighPix[][3] = {
    {
	{sleigh1_0_bits, None, sleigh1_0_width , sleigh1_0_height},
	{sleigh1_1_bits, None, sleigh1_1_width , sleigh1_1_height},
	{sleigh1_2_bits, None, sleigh1_2_width , sleigh1_2_height},
    }, {
	{sleigh2_0_bits, None, sleigh2_0_width , sleigh2_0_height},
	{sleigh2_1_bits, None, sleigh2_1_width , sleigh2_1_height},
	{sleigh2_2_bits, None, sleigh2_2_width , sleigh2_2_height},
    }, {
	{sleigh3_0_bits, None, sleigh3_0_width , sleigh3_0_height},
	{sleigh3_1_bits, None, sleigh3_1_width , sleigh3_1_height},
	{sleigh3_2_bits, None, sleigh3_2_width , sleigh3_2_height},
    }
};


#include "santa1.xbm"
#include "santa2.xbm"
#include "santa3.xbm"

typedef struct SantaMap {
        char *santaBits;
        Pixmap pixmap;
        int width;
        int height;
} SantaMap;

SantaMap santaPix[] = {
        {santa1_bits, None, santa1_width, santa1_height},
        {santa2_bits, None, santa2_width, santa2_height},
        {santa3_bits, None, santa3_width, santa3_height},
};



#include "santa1_fur.xbm"
#include "santa2_fur.xbm"
#include "santa3_fur.xbm"

typedef struct SantaFurMap {
        char *santaFurBits;
        Pixmap pixmap;
        int width;
        int height;
} SantaFurMap;

SantaFurMap santaFurPix[] = {
        {santa1_fur_bits, None, santa1_fur_width, santa1_fur_height},
        {santa2_fur_bits, None, santa2_fur_width, santa2_fur_height},
        {santa3_fur_bits, None, santa3_fur_width, santa3_fur_height},
};

