/*

©Copyright 1984, 1988, 1990, 1993 by Rick Jansen, all rights reserved.

  X S N O W 
        *
  *   *
    *       *
        *
   *

 In 1984 the first Macintosh program I wrote was a computer christmas
 card, which showed a picture of a snowman and falling snow. Later
 a Father Xmas in his sleigh was added. I converted this program to
 an undying desk accessory in 1988 with several additions.
 But, little boys grow up, and when they are forced to a workstation
 they want their thingies there too. So here is Xsnow. For *you*.

 Availability
 ------------
 Xsnow is available directly from me via my WWW page:

   http://www.sara.nl/Rick.Jansen

 Click on the appropriate item and the tar file will be downloaded
 upon your request.

 Xsnow ia also archived at ftp.x.org in the contrib/games directory.


 Credits
 -------
 Xsnow borrows code for drawing on the desktop (the 'root' window as
 they insist on calling it in X) from the infamous xroach program, 
 which is copyright 1991 by J.T. Anderson (jta@locus.com).
 Xroach is available in the 'contrib' directory of 'ftp.x.org' via 
 anonymous ftp.

 For compatibility with virtual window managers Xsnow uses vroot.h
 vroot.h is copyright 1991 by Andreas Stolcke, copyright 1990 by 
 Solbourne Computer Inc. (stolcke@ICSI.Berkeley.EDU)
 The complete distribution of vroot is available in the 'contrib'
 directory of 'ftp.x.org' via anonymous ftp as 'vroot.shar.Z' 

 The big Santa with the nice moving antlers and reins was made by 
 Holger Veit (Holger.Veit@gmd.de).

 The code for wind was done by Eiichi TAZOE (tazoe@yamato.ibm.co.jp,
 tazoe@vnet.ibm.com)
    
 Xsnow is available freely and you may give it to other people as is, 
 but I retain all rights. Therefore it does not classify as 'Public 
 Domain' software.


  To build:
      xmkmf
      make depend
      make
  or:
      cc -o xsnow snow.c -lX11 [-lsocketorwhatever] [-lm] [-l...]

 Happy winter and a merry X-mas!

 Rick Jansen
 (rick@sara.nl)

 Modification history
 03DEC93 version 0
 04DEC93 background setting
 06DEC93 delay in main loop using select
 10DEC93 keep snow on top of windows and at bottom of display
 11DEC93 snow kept sticking to the right of display (1.11)
 11DEC93 updated calcwindowtops
 12DEC93 call calcwindowtops after last exposure event only
 13DEC93 vroot added for virtual window managers
 14DEC93 separate options for snowdepth on windows and on screen
 15DEC93 patch for fvwm from Mike Hollick (hollick@graphics.cis.upenn.edu)
 16DEC93 Really building up snow at bottom of screen
 16DEC93 A choice of Santa's. Thanks to Holger Veit (Holger.Veit@gmd.de)
         for Santa no. 2. Santa 1 was derived from 2 by scaling down.
 20DEC93 Aaaargh! gnu C doesn't like the initialization of the new
         Santa pixmaps. Extra braces do the trick, which is so often the case
 20DEC93 Deleted PaintSnowAtBottom. It's more of a surprise this way.
 21DEC93 In case of negative x coordinates adapt width (CalcWindowTops)
 22DEC93 Rudolf's red nose. Silly. Grmbll.
 22DEC93 Bug with -nokeepsnowonwindows removed
 10JAN94 Improved handling of erasing snow at screen bottom in case of 
         window in or under the snow.
 18FEB94 X-speed really 0 inside snow layers - prevents snowflakes from 
         erasing already fallen snow a bit better
 30OCT94 More efficiency due to GC's in server and not drawing where
         the windows are.
 30OCT94 Eiichi TAZOE's code for wind included in general distribution.
 11NOV94 Bug in XRectInRegion call resolved (santa not appearing gradually
         from behind windows)
 15NOV94 Unsigned longs changed to longs, as strtoul is troublesome on many
         systems (snowDelay)

*/

#define VERSION "Xsnow-1.32, November 15th 1994 by Rick Jansen (rick@sara.nl)"

#ifdef VMS
#include <socket.h>
# if defined(__SOCKET_TYPEDEFS) && !defined(CADDR_T)
#  define CADDR_T
# endif
#endif


#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


/**** V R O O T ****/
/* For vroot.h see the credits at the beginning of Xsnow */
/***#include <X11/vroot.h>   /* if vroot.h installed in /usr/include/X11 ***/
#include "vroot.h"  /* Use this if vroot.h installed in current directory */


#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <limits.h>

#if defined(__STDC__) || defined(VMS)
#include <stdlib.h>
#else
long strtol();
/*double strtod();*/
char *getenv();
#endif

char Copyright[] = "Xsnow\nCopyright 1984, 1988, 1990, 1993 by Rick Jansen, all rights reserved.";

#include "snowmap.h"

/***typedef unsigned long Pixel;***/
typedef int ErrorHandler();

#if !defined(GRAB_SERVER)
#define GRAB_SERVER	0
#endif

Display *display;
int screen;
Window rootWin;
int display_width, display_height;
int center_x, center_y;
GC gc;
char *display_name = NULL;
Pixel black, white;

int done = 0;
int eventBlock = 0;
int errorVal = 0;

int flake;
int SmoothWhirl = 1;
int NoTrees = 0;
int NoSanta = 0;
int Rudolf = 1;
int NoKeepSnow = 0;
int NoKeepSBot = 0;
int NoKeepSWin = 0;
int NoWind = 0;
unsigned int borderWidth = 0;

Snow *snowflakes;
int MaxSnowFlakeHeight = 0;  /* Biggest flake */
int maxSnowflakes = INITIALSNOWFLAKES;
int curSnowflakes = 0;
long snowDelay = 50000; /* useconds */
long SnowTicks = LONG_MAX; /* Decremented each pass thru main loop */
int MaxXStep = MAXXSTEP;
int MaxYStep = MAXYSTEP;
int WhirlFactor = WHIRLFACTOR;
int MaxWinSnowDepth = INITWINSNOWDEPTH;
int MaxScrSnowDepth = INITSCRSNOWDEPTH;

Santa Claus;   

/* The default Santa 0,1,2 */
int SantaSize = 1;
int SantaSpeed = -1;  /* Not initialized yet */

Region SubR;
Region WindowTops = NULL;
Region Windows = NULL;
int ClearX, ClearY;

/* For building up snow at bottom of screen */
Region SnowBottomR = NULL;
XRectangle AddRect;

/* Wind stuff */
int diff=0;
int wind = 0;
int direction=0;
int WindTimer=30;
int current_snow_height;
int geta=0;

/* Forward decls */
void Usage();
void SigHandler();
void InitSnowflake();
void UpdateSnowflake();
void DrawSnowflake();
void EraseSnowflake();
void PaintSnowAtBottom();
void DrawTannenbaum();
void InitSanta();
void DrawSanta();
void EraseSanta();
void UpdateSanta();
void uSsleep();
int CalcWindowTops();
Pixel AllocNamedColor();
void sig_alarm();

/* Colo(u)rs */
char *snowColor = "snow";
char *slcColor = "black";
char *blackColor = "black";
char *redColor = "red";
char *whiteColor = "white";
char *bgColor = "none";
char *trColor = "chartreuse";
/* The author thoroughly recommends a cup of tea */
/* with a dash of green Chartreuse. Jum!         */
char *greenColor = "chartreuse"; 
Pixel redPix;
Pixel whitePix;
Pixel greenPix;
Pixel blackPix;
Pixel snowcPix;
Pixel bgPix;
Pixel trPix;
Pixel slcPix;

/* GC's */
GC SnowGC[SNOWFLAKEMAXTYPE+1];
GC SleighGCs[3];
GC SantaGC,RudolfGC,FurGC;
GC TreesGC;

#ifdef VMS
int
#else
void
#endif
main(ac, av)
int ac;
char *av[];
{
    XGCValues xgcv;
    int ax;
    char *arg;
    SnowMap *rp;
    TannenbaumMap *tp;
    SleighMap *sp; 
    SantaMap *stp;
    SantaFurMap *stfp;
    XEvent ev;
    int needCalc;
    int i; 

    
    /*
       Process command line options.
    */
    for (ax=1; ax<ac; ax++) {
	arg = av[ax];
	if (strcmp(arg, "-display") == 0) {
	    display_name = av[++ax];
	}
	else if (strcmp(arg, "-sc") == 0) {
	    snowColor = av[++ax];
	}
	else if (strcmp(arg, "-bg") == 0) {
	    bgColor = av[++ax];
	}
	else if (strcmp(arg, "-tc") == 0) {
	    trColor = av[++ax];
	}
	else if (strcmp(arg, "-slc") == 0) {
	    slcColor = av[++ax];
	}
	else if (strcmp(arg, "-delay") == 0) {
	    snowDelay = strtol(av[++ax], (char **)NULL,0);
            snowDelay = snowDelay * 1000;  /* ms to us */
	}
	else if (strcmp(arg, "-snowflakes") == 0) {
	    maxSnowflakes = strtol(av[++ax], (char **)NULL, 0);
	}
        else if (strcmp(arg, "-unsmooth") == 0) {
            SmoothWhirl = 0;
        }
	else if (strcmp(arg, "-whirl") == 0) {
	    WhirlFactor = strtol(av[++ax], (char **)NULL, 0);
	}
	else if (strcmp(arg, "-nowind") == 0) {
	    NoWind = 1;
	}
	else if (strcmp(arg, "-windtimer") == 0) {
	    WindTimer = strtol(av[++ax], (char **)NULL, 0);
	}
	else if (strcmp(arg, "-xspeed") == 0) {
	    MaxXStep = strtol(av[++ax], (char **)NULL, 0);
	}
	else if (strcmp(arg, "-yspeed") == 0) {
	    MaxYStep = strtol(av[++ax], (char **)NULL, 0);
	}
	else if (strcmp(arg, "-wsnowdepth") == 0) {
	    MaxWinSnowDepth = strtol(av[++ax], (char **)NULL, 0);
	}
	else if (strcmp(arg, "-ssnowdepth") == 0) {
	    MaxScrSnowDepth = strtol(av[++ax], (char **)NULL, 0);
	}
        else if (strcmp(arg, "-notrees") == 0) {
            NoTrees = 1;     
        }
        else if (strcmp(arg, "-nosanta") == 0) {
            NoSanta = 1;     
        }
        else if (strcmp(arg, "-santa") == 0) {
	    SantaSize = strtol(av[++ax], (char **)NULL, 0);
        }
        else if (strcmp(arg, "-norudolf") == 0) {
	    Rudolf = 0;
        }
        else if (strcmp(arg, "-santaspeed") == 0) {
	    SantaSpeed = strtol(av[++ax], (char **)NULL, 0);
        }
        else if (strcmp(arg, "-nokeepsnow") == 0) {
            NoKeepSnow = 1;
            NoKeepSWin = 1;
            NoKeepSBot = 1;
        }
        else if (strcmp(arg, "-nokeepsnowonwindows") == 0) {
            NoKeepSWin = 1;
        }
        else if (strcmp(arg, "-nokeepsnowonscreen") == 0) {
            NoKeepSBot = 1;
        }
        else if (strcmp(arg, "-version") == 0) {
            printf("\nThis is %s\n\n", VERSION);
        }
	else {
	    Usage();
	}
    }

    printf("%s\n",VERSION);

    /* Some people... */
    if (maxSnowflakes > MAXSNOWFLAKES) {
      fprintf(stderr,"xx Maximum number of snowflakes is %d\n", MAXSNOWFLAKES);
      exit(1);
    }
    if ((SantaSize < 0) || (SantaSize > MAXSANTA)) {
      fprintf(stderr,"xx Maximum Santa is %d\n",MAXSANTA);
      exit(1);
    }

    /* Eskimo warning */
    if (strstr(snowColor,"yellow") != NULL)
      printf("\nWarning: don't eat yellow snow!\n\n");


    /* Santa speed is different for different size Santa */
    if (SantaSpeed < 0) SantaSpeed = Speed[SantaSize];

    /* Seed random */
    srand((int)time((long *)NULL));

    /*
       Catch some signals so we can erase any visible snowflakes.
    */
    signal(SIGKILL, SigHandler);
    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);
    signal(SIGHUP, SigHandler);

    /* Wind stuff */
    signal(SIGALRM, sig_alarm);
    alarm(WindTimer);


    /* Open X */
    display = XOpenDisplay(display_name);
    if (display == NULL) {
	if (display_name == NULL) display_name = getenv("DISPLAY");
	(void) fprintf(stderr, "%s: cannot connect to X server %s\n", av[0],
	    display_name ? display_name : "(default)");
	exit(1);
    }

    screen = DefaultScreen(display);
    rootWin = RootWindow(display, screen);
    black = BlackPixel(display, screen);
    white = WhitePixel(display, screen);

    display_width = DisplayWidth(display, screen);
    display_height = DisplayHeight(display, screen);
    center_x = display_width / 2;
    center_y = display_height / 2;

    /* No snow at all yet */
    current_snow_height = display_height;

    /* Do not let all of the display snow under */
    if (MaxScrSnowDepth> (display_height-SNOWFREE)) {
      printf("xx Maximum snow depth set to %d\n", display_height-SNOWFREE);
      MaxScrSnowDepth = display_height-SNOWFREE;
    }
    

    /*
        P I X M A P S 
    */

    /* Create the snowflake pixmaps */
    for (flake=0; flake<=SNOWFLAKEMAXTYPE; flake++) {
      rp = &snowPix[flake];
      rp->pixmap = XCreateBitmapFromData(display, rootWin,
            rp->snowBits, rp->width, rp->height);
      /* Whats the biggest flake? (used for erasing kept snow later) */
      if (rp->height > MaxSnowFlakeHeight) MaxSnowFlakeHeight = rp->height;
    }

    /* Allocate structures containing the coordinates and things */
    snowflakes = (Snow *)malloc(sizeof(Snow) * MAXSNOWFLAKES);

    /* Create the tree pixmap */
    tp = &tannenbaumPix[0];
    tp->pixmap = XCreateBitmapFromData(display, rootWin,
            tp->tannenbaumBits, tp->width, tp->height);

    /* Create the Santa pixmaps */
    for (flake=0; flake<=2; flake++) {
      sp = &sleighPix[SantaSize][flake];
      sp->pixmap = XCreateBitmapFromData(display, rootWin,
            sp->sleighBits, sp->width, sp->height);
    }

    /* Pixmaps of the man in the red suit himself */
    stp = &santaPix[SantaSize];
    stp->pixmap = XCreateBitmapFromData(display, rootWin,
            stp->santaBits, stp->width, stp->height);
    /* The fur in his hat */
    stfp = &santaFurPix[SantaSize];
    stfp->pixmap = XCreateBitmapFromData(display, rootWin,
            stfp->santaFurBits, stfp->width, stfp->height);


    
    /* Allocate colors just once */
    redPix =   AllocNamedColor(redColor, black);
    whitePix = AllocNamedColor(whiteColor, white);
    greenPix = AllocNamedColor(greenColor, black);
    blackPix = AllocNamedColor(blackColor, black);
    snowcPix = AllocNamedColor(snowColor, white);   
    trPix = AllocNamedColor(trColor, black);
    slcPix = AllocNamedColor(slcColor, black);


    gc = XCreateGC(display, rootWin, 0L, &xgcv);
    XSetForeground(display, gc, blackPix);
    XSetFillStyle(display, gc, FillStippled);

    /* Set the background color, if specified */
    if(strcmp(bgColor,"none") != 0) {
      bgPix = AllocNamedColor(bgColor, white);

      XSetWindowBackground(display, rootWin, bgPix);
      XClearWindow(display, rootWin);
      XFlush(display);
    }
    
    /* Fill in GCs for snowflakes */
    for (flake=0; flake<=SNOWFLAKEMAXTYPE; flake++) {
      SnowGC[flake] = XCreateGC(display, rootWin, 0L, &xgcv);
      XCopyGC(display,gc,0,SnowGC[flake]);
      XSetStipple(display, SnowGC[flake], snowPix[flake].pixmap);
      XSetForeground(display,SnowGC[flake],snowcPix);
      XSetFillStyle(display, SnowGC[flake], FillStippled);
    }
    
    /* GC for trees */
    TreesGC = XCreateGC(display, rootWin, 0L, &xgcv);
    XCopyGC(display,gc,0,TreesGC);
    XSetStipple(display, TreesGC, tannenbaumPix[0].pixmap);
    XSetForeground(display,TreesGC,trPix);
    XSetFillStyle(display, TreesGC, FillStippled);

    /* GCs for sleighs */
    for (flake=0; flake<=2; flake++) {
      SleighGCs[flake] = XCreateGC(display, rootWin, 0L, &xgcv);
      XCopyGC(display,gc,0,SleighGCs[flake]);
      XSetForeground(display, SleighGCs[flake], slcPix);
      XSetFillStyle(display, SleighGCs[flake], FillStippled);
      XSetStipple(display,SleighGCs[flake],
                  sleighPix[SantaSize][flake].pixmap);
    }

    /* GCs for Santa */
    SantaGC = XCreateGC(display, rootWin, 0L, &xgcv);
    XCopyGC(display,gc,0,SantaGC);
    XSetFillStyle(display, SantaGC, FillStippled);
    XSetForeground(display, SantaGC, redPix);
    XSetStipple(display,SantaGC,
                santaPix[SantaSize].pixmap);

    RudolfGC = XCreateGC(display, rootWin, 0L, &xgcv);
    XCopyGC(display,gc,0,RudolfGC);
    XSetFillStyle(display, RudolfGC, FillSolid);
    XSetForeground(display, RudolfGC, redPix);

    FurGC = XCreateGC(display, rootWin, 0L, &xgcv);
    XCopyGC(display,gc,0,FurGC);
    XSetForeground(display, FurGC, whitePix);
    XSetFillStyle(display, FurGC, FillStippled);
    XSetStipple(display, FurGC, santaFurPix[SantaSize].pixmap);


    /*
     *
     * Here snowing starts
     *
     */

    /* The initial catch region: a little bit below the bottom of the screen */
    AddRect.x = 0;
    AddRect.y = display_height + MaxSnowFlakeHeight/2;
    AddRect.width = display_width - 1;
    AddRect.height = MaxYStep+MaxSnowFlakeHeight;
    SnowBottomR = XCreateRegion();
    XUnionRectWithRegion(&AddRect, SnowBottomR, SnowBottomR);

    /* Initialize all snowflakes */
    for (i=0; i<maxSnowflakes; i++) InitSnowflake(i);

    InitSanta();   

    /* Notify if part of the root window changed */
    XSelectInput(display, rootWin, ExposureMask | SubstructureNotifyMask);

    WindowTops = XCreateRegion();
    Windows = XCreateRegion();

    needCalc = 0;
    /***if (!NoKeepSnow) needCalc = CalcWindowTops();***/
    needCalc = CalcWindowTops();


    /*
     *  M A I N   L O O P
     */
    while (!done) {

      SnowTicks--;   
      if (SnowTicks <= 0) SnowTicks = LONG_MAX;

      /* X event ? */
      /* Handle all the expose events and redo CalcWindowTops after */
      while (XPending(display)) {
        XNextEvent(display, &ev);

        /* No need to do all this computing if we're not keeping the snow */
        if (!NoKeepSnow) {

          switch (ev.type) {

              case Expose:    

                  /* 
                   * Subtract the exposed area from the SnowBottomR 
                   */
                  AddRect.x = ev.xexpose.x;
                  AddRect.y = ev.xexpose.y;
                  AddRect.width = ev.xexpose.width;
                  /* Add MSFH so snow will really build up from scratch */
                  AddRect.height = ev.xexpose.height + MaxSnowFlakeHeight/2;

                  SubR= XCreateRegion();
                  XUnionRectWithRegion(&AddRect, SubR,SubR);
                  
                  /* 
                   * Clear the snow that was on top of the window 
                   */
                  ClearX = ev.xexpose.x - MaxWinSnowDepth - MaxXStep;
                  if (ClearX < 0) ClearX = 0;
                  ClearY = ev.xexpose.y-MaxWinSnowDepth-MaxYStep-borderWidth-1;
                  if (ClearY < 0) ClearY = 0;

                  AddRect.width =  ev.xexpose.width + MaxXStep + 
                                   2*MaxWinSnowDepth + MaxSnowFlakeHeight;
                  AddRect.height = MaxWinSnowDepth + 
                                   MaxYStep+borderWidth+MaxSnowFlakeHeight*2;

                  if (ClearY > (display_height-MaxScrSnowDepth-MaxYStep))  {
                    ClearY = display_height-MaxScrSnowDepth-
                                MaxYStep-MaxSnowFlakeHeight;
                    AddRect.height = MaxScrSnowDepth + MaxYStep + 
                                MaxSnowFlakeHeight;
                  }

                  AddRect.x = ClearX;
                  AddRect.y = ClearY;

                  /* Subtract the erased snow from the SnowBottomR */
                  XUnionRectWithRegion(&AddRect, SubR,SubR);
                  XSubtractRegion(SnowBottomR,SubR, SnowBottomR);
                  XDestroyRegion(SubR);

                  XClearArea(display, rootWin,
                     ClearX,ClearY, 
                     AddRect.width,              
                     AddRect.height,
                     False);

                  needCalc = 1;

                  break;


              case MapNotify:
              case UnmapNotify:
              case ConfigureNotify:
                  needCalc = 1;
                  break;

          } /* switch */

        }  /* if (!NoKeepSnow) */
       
      }  /* while Xpending */

      /* If things have changed while we were away... */
      if (needCalc) needCalc = CalcWindowTops();


      /* 
       *  Update     
       */

      /* Snowflakes */
      for (i=0; i<maxSnowflakes; i++) UpdateSnowflake(i);

      /* Draw dear Andrews */
      if (!NoTrees ) {
        DrawTannenbaum(display_width-150, display_height-500);
        DrawTannenbaum(display_width-100, display_height-200);
        DrawTannenbaum(100, display_height-200);
        DrawTannenbaum(50, display_height-150);
        DrawTannenbaum(center_x, display_height-100);
        DrawTannenbaum(200,400);
      }

      /* Dear Santa */
      if (!NoSanta) {
        UpdateSanta();
      }

      /* Sleep a bit */
      uSsleep(snowDelay);
    }
    
    XDestroyRegion(WindowTops);
    XDestroyRegion(Windows);

    XClearWindow(display, rootWin);

    XCloseDisplay(display);


    exit(0);
}

#define USEPRT(msg) fprintf(stderr, msg)

void
Usage()
{
    USEPRT("Usage: xsnow [options]\n\n");
    USEPRT("Options:\n");
    USEPRT("       -display     <displayname>\n");
    USEPRT("       -sc          <snowcolor>\n");
    USEPRT("       -tc          <tree color>\n");
    USEPRT("       -bg          <background color>\n");
    USEPRT("       -slc         <sleigh color>\n");
    USEPRT("       -snowflakes  <numsnowflakes>\n");
    USEPRT("       -delay       <delay in milliseconds>\n");
    USEPRT("       -unsmooth\n");
    USEPRT("       -whirl       <whirlfactor>\n");
    USEPRT("       -nowind\n");
    USEPRT("       -windtimer   <Time between windy periods in seconds>\n");
    USEPRT("       -xspeed      <max xspeed snowflakes>\n");
    USEPRT("       -yspeed      <max yspeed snowflakes>\n");
    USEPRT("       -wsnowdepth  <max snow depth on windows>\n");
    USEPRT("       -ssnowdepth  <max snow depth at bottom of display>\n");
    USEPRT("       -notrees\n");
    USEPRT("       -nosanta\n");
    USEPRT("       -norudolf\n");
    USEPRT("       -santa       <santa>\n");
    USEPRT("       -santaspeed  <santa_speed>\n");
    USEPRT("       -nokeepsnow\n");
    USEPRT("       -nokeepsnowonwindows\n");
    USEPRT("       -nokeepsnowonscreen\n");
    USEPRT("       -version\n\n");
    USEPRT("Recommended: xsnow -bg LightSkyBlue3\n");
    
    exit(1);
}

void
SigHandler()
{
  done = 1;
}

/*
   Generate random integer between 0 and maxVal-1.
*/
int
RandInt(maxVal)
int maxVal;
{
	return rand() % maxVal;
}




/*
 * sleep for a number of micro-seconds
 */
void uSsleep(usec) 
unsigned long usec;
{
#ifdef SVR3
    poll((struct poll *)0, (size_t)0, usec/1000);   /* ms resolution */
#else
    struct timeval t;
    t.tv_usec = usec%(unsigned long)1000000;
    t.tv_sec = usec/(unsigned long)1000000;
    select(0, (void *)0, (void *)0, (void *)0, &t);
#endif
}



/*
   Check for topleft point of snowflake in specified rectangle.
*/
int
SnowPtInRect(snx, sny, recx, recy, width, height)
int snx;
int sny;
int recx;
int recy;
int width;
int height;
{
    if (snx < recx) return 0;
    if (snx > (recx + width)) return 0;
    if (sny < recy) return 0;
    if (sny > (recy + height)) return 0;
    
    return 1;
}


/*
   Give birth to a snowflake.
*/
void
InitSnowflake(rx)
int rx;
{
    Snow *r;

    r = &snowflakes[rx];

  if (curSnowflakes < maxSnowflakes) {
    r->whatFlake = RandInt(SNOWFLAKEMAXTYPE+1);

    /* Wind strikes! */
    if (wind) {
      r->intX = RandInt(display_width);
      r->intY =  RandInt(display_height);
    }
    else  {
      r->intX = RandInt(display_width - snowPix[r->whatFlake].width);
      r->intY =  RandInt(display_height/10);
    }

    r->yStep = RandInt(MaxYStep+1)+1;
    r->xStep = RandInt(r->yStep/4+1);
    if (RandInt(1000) > 500) r->xStep = -r->xStep;
    r->active = 1;
    r->visible = 1;
  }
}



/*
   Move a snowflake by erasing and redraw
*/
void
UpdateSnowflake(rx)
int rx;
{
Snow *snow;

int NewX;
int NewY;
int TouchDown;
int InVisible;
int NoErase;
/***
XRectangle SnowBotClipRect;
int MaxSnowY;
***/
    
  /* move an active snowflake */

  snow = &snowflakes[rx];
  NoErase = 0;  /* Always erase the flakes unless special case */


  /* Activate a new snowflake */
  if (!snow->active) InitSnowflake(rx);


  NewX = snow->intX + snow->xStep;
  NewY = snow->intY + snow->yStep;

  /* If flake disappears offscreen (below bottom) it becomes inactive */
  /* In case of wind also deactivate flakes blown offscreen sideways, */
  /* so they'll reappear immediately. Make sure snow at bottom isn't  */
  /* disrupted.                          */
/***
  if (wind) {
    XClipBox(SnowBottomR, SnowBotClipRect);
    MaxSnowY = SnowBotClipRect.y - MaxSnowFlakeHeight;
    snow->active = SnowPtInRect(NewX, NewY,
                                0, 0, display_width-1, MaxSnowY);
    }
  else
***/
    snow->active = (NewY < display_height);

  /*
   * Screen bottom 
   */
  /* Keep snow that becomes inactive */
  if (!NoKeepSBot) {

    TouchDown = 
       /* Note: if we are too strict and use intXY the snow won't be fluffy */
       /* We really need some flakes just hanging in the air.               */
       /* Correction! We mean, sticking to the *glass*, of course. Ehem.    */
       XRectInRegion(SnowBottomR,
                     NewX,NewY,
                     snowPix[snow->whatFlake].width,
                     snowPix[snow->whatFlake].height);

    /* If the flake already is completely inside the region */
    if (TouchDown == RectangleIn) 
      NoErase = 1;
    else
    if (TouchDown == RectanglePart) {

      NoErase = 1;

      /* Add a bit to the snow bottom region to make snow build up */
      /* Snow isn't built up more than about MaxScrSnowDepth */
      AddRect.y = NewY + snowPix[snow->whatFlake].height - 2;
      /* Limit the building-up */
      if (AddRect.y > (display_height-MaxScrSnowDepth)) {
        AddRect.x = NewX;
        AddRect.height = 2;
        AddRect.width = snowPix[snow->whatFlake].width;               
        XUnionRectWithRegion(&AddRect, SnowBottomR,SnowBottomR);
      }
    }

  }  /* if !NoKeepSBot */


  /*
   * Window tops 
   */
  /* Are we keeping snow on top of the windows? */
  /* Note snow kept on top of a window is still active, so it 
     can reach the bottom of the screen... */
  if (snow->active) {
    if (!NoKeepSWin) 
      NoErase = (NoErase || XPointInRegion(WindowTops,NewX,NewY));
  }

  /* Adjust horizontal speed to mimic whirling */
  if (SmoothWhirl) 
    snow->xStep = snow->xStep + (RandInt(WhirlFactor+1) - WhirlFactor/2);
  else {
    /* Jerkier movement */
    snow->xStep = snow->xStep + RandInt(WhirlFactor);
    if (RandInt(1000) > 500) snow->xStep = -snow->xStep;
  }

  if (!wind) {
    if (snow->xStep > MaxXStep) snow->xStep = MaxXStep;
    if (snow->xStep < -MaxXStep) snow->xStep = -MaxXStep;
  }

  /* Limit X speed inside snowlayers. 's only natural... */
  if (NoErase && !wind) snow->xStep = snow->xStep/2;
  /* xsnow 1.29 method: */
  /***if (NoErase) {
    snow->xStep = 0;
    NewX = snow->intX;
  }***/


  /* Erase, unless something special has happened */
  if (!NoErase) EraseSnowflake(rx);

  /* New coords, unless it's windy */
  snow->intX = NewX;
  snow->intY = NewY;

  if (wind) {
      int       tmp_x;
      tmp_x = abs(snow->xStep);
      if( wind == 1 ){  /* going to stop winding */
          tmp_x = tmp_x + (RandInt(WhirlFactor+1) - WhirlFactor/2);
      }else{            /* WINDY */
          tmp_x = tmp_x + (RandInt(20));
      }
      snow->xStep = tmp_x * direction;

      if (snow->xStep > 50) snow->xStep = 50;
      if (snow->xStep < -50) snow->xStep = -50;

      /* Erase, unless something special has happened */
      if (!NoErase) EraseSnowflake(rx);

      if( wind == 1 ){
          snow->intY = NewY+3;
      }else{
          snow->intY = NewY+10;
      }
  }

  /* If flake covered by windows don't bother drawing it */
  InVisible = XRectInRegion(Windows,
                            NewX,NewY,
                            snowPix[snow->whatFlake].width,
                            snowPix[snow->whatFlake].height);
  snow->visible = (InVisible != RectangleIn);

  /* Draw if still active  and visible*/
  if (snow->active && snow->visible) DrawSnowflake(rx);
 
}
    
/*
   Draw a snowflake.
*/
void
DrawSnowflake(rx)
int rx;
{
    Snow *snow;

    snow = &snowflakes[rx];

    XSetTSOrigin(display, SnowGC[snow->whatFlake], snow->intX, snow->intY);
    XFillRectangle(display, rootWin, SnowGC[snow->whatFlake],
         snow->intX, snow->intY,
         snowPix[snow->whatFlake].width, snowPix[snow->whatFlake].height);
}


/*
   Erase a snowflake.
*/
void
EraseSnowflake(rx)
int rx;
{
    Snow *snow;

    snow = &snowflakes[rx];
   
    XClearArea(display, rootWin, 
               snow->intX, snow->intY,
               snowPix[snow->whatFlake].width, 
               snowPix[snow->whatFlake].height,
               False);
}


/*
  Paint thick snow at bottom of screen for impaintient people
*/
void
PaintSnowAtBottom(depth)
int depth;
{
int x,y;
Snow *snow;

  for (y=0; y < depth; y++) {
    for (x=0; x<maxSnowflakes; x++) {
      InitSnowflake(x);
      snow = &snowflakes[x];
      snow->intY = display_height - y;
      DrawSnowflake(x);
    }
  }

}


/*
   Draw a tree
*/
void
DrawTannenbaum(x,y)
int x,y;
{
    XSetTSOrigin(display, TreesGC, x,y);
    XFillRectangle(display, rootWin, TreesGC,
         x,y,
         tannenbaumPix[0].width, tannenbaumPix[0].height);
}


/*
   Give birth to a Santa. (What a conception)
*/
void
InitSanta()      
{
  Claus.x = -sleighPix[SantaSize][Claus.whatSanta].width;
  Claus.y = RandInt(display_height / 3)+40;
  Claus.xStep = SantaSpeed;
  Claus.yStep = 1;
  Claus.whatSanta = 0;
  Claus.visible = 1;
}


/*
  Update Santa (How can you update the oldest icon in the world? Oh well. )
*/
void
UpdateSanta()
{
int Visible;

  if (Claus.visible) EraseSanta();

  /* Move forward */
  Claus.x = Claus.x + Claus.xStep;
 
  /* Move down */
  if (RandInt(10) > 3) Claus.y = Claus.y + Claus.yStep; 
  if (Claus.y < 0) Claus.y = 0;
  if (RandInt(100) > 80) Claus.yStep = -Claus.yStep;

  if (Claus.x >= display_width) InitSanta(); 

  /* If Santa entirely covered by windows don't bother drawing him */
  Visible = XRectInRegion(Windows,
                          Claus.x, Claus.y,
                          sleighPix[SantaSize][Claus.whatSanta].width,
                          sleighPix[SantaSize][Claus.whatSanta].height);
  Claus.visible = (Visible != RectangleIn);
  Claus.visible = (Claus.visible || (Claus.x < 0));

  /* Next sleigh */
  Claus.whatSanta++;
  if (Claus.whatSanta > 2) Claus.whatSanta = 0;

  if (Claus.visible) DrawSanta();
}




/*
  Draw Santa
*/
void
DrawSanta()
{
  XSetTSOrigin(display, SleighGCs[Claus.whatSanta], Claus.x,Claus.y);
  XFillRectangle(display, rootWin, SleighGCs[Claus.whatSanta],
         Claus.x,Claus.y,
         sleighPix[SantaSize][Claus.whatSanta].width, 
         sleighPix[SantaSize][Claus.whatSanta].height);

  /* The Man in the Red Suit himself */
  XSetTSOrigin(display, SantaGC, Claus.x,Claus.y);
  XFillRectangle(display, rootWin, SantaGC,
         Claus.x,Claus.y,
         santaPix[SantaSize].width, 
         santaPix[SantaSize].height);

  /* At last.... Rudolf! */
  if (Rudolf) {
    XFillRectangle(display, rootWin, RudolfGC,
                   Claus.x+NoseX[SantaSize], Claus.y+NoseY[SantaSize],
                   NoseXSiz[SantaSize], NoseYSiz[SantaSize]);
  }

  /* The fur in his hat */
  /* Note: the fur in his hat is *imitation* white-seal fur, of course. */
  /* Santa is a big supporter of Greenpeace.                            */
  XSetTSOrigin(display, FurGC, Claus.x,Claus.y);
  XFillRectangle(display, rootWin, FurGC,
         Claus.x,Claus.y,
         santaFurPix[SantaSize].width, 
         santaFurPix[SantaSize].height);

}

/*
  Erase Santa
*/
void
EraseSanta()
{
  XClearArea(display, rootWin,
             Claus.x , Claus.y,     
             sleighPix[SantaSize][Claus.whatSanta].width,
             sleighPix[SantaSize][Claus.whatSanta].height,
             False);
}



#if !GRAB_SERVER

int
RoachErrors(dpy, err)
Display *dpy;
XErrorEvent *err;
{
    errorVal = err->error_code;

    return 0;
}

#endif /* GRAB_SERVER */

/*
   Calculate Visible region of tops of windows to catch snowflakes
*/
int
CalcWindowTops()
{
    Window *children;
    unsigned int nChildren;
    Window dummy;
    XWindowAttributes wa;
    int wx;
    XRectangle rect;
    int winX, winY;
    unsigned int winHeight, winWidth;
    unsigned int depth;

    /*
       If we don't grab the server, the XGetWindowAttribute or XGetGeometry
       calls can abort us.  On the other hand, the server grabs can make for
       some annoying delays.
    */
#if GRAB_SERVER
    XGrabServer(display);
#else
    XSetErrorHandler(RoachErrors);
#endif

    /*
       Get children of root.
    */
    XQueryTree(display, rootWin, &dummy, &dummy, &children, &nChildren);

    /*
       For each mapped child, add the rectangle on top of the window
       where the snow could be to the region.
    */
    if (WindowTops) XDestroyRegion(WindowTops);
    if (Windows) XDestroyRegion(Windows);
    WindowTops = XCreateRegion();
    Windows = XCreateRegion();
    for (wx=0; wx<nChildren; wx++) {
        if (XEventsQueued(display, QueuedAlready)) return 1;
        errorVal = 0;
        XGetWindowAttributes(display, children[wx], &wa);
        if (errorVal) continue;
        if (wa.map_state == IsViewable) {
            XGetGeometry(display, children[wx], &dummy, &winX, &winY,
                &winWidth, &winHeight, &borderWidth, &depth);
            if (errorVal) continue;
            rect.x = winX - borderWidth - MaxWinSnowDepth;

            /* Patch from Mike Hollick <hollick@graphics.cis.upenn.edu> */
            /* for fvwm. 14DEC93                                        */
            /* Entirely offscreen? */
            if (((rect.x + winWidth) < 0) || (rect.x > display_width)) continue;

            /* If window partly on screen */
            if (rect.x < 0) {
              winWidth = winWidth + rect.x;
              rect.x = 0;
            }
              
            rect.y = winY - MaxWinSnowDepth - borderWidth;
            if (rect.y < 0) continue;

            rect.width = 
               winWidth + 2*(borderWidth+MaxWinSnowDepth) - MaxSnowFlakeHeight;
            rect.height = MaxWinSnowDepth + borderWidth + MaxSnowFlakeHeight; 
            XUnionRectWithRegion(&rect, WindowTops, WindowTops);

            /* The area of the windows themselves */
            rect.x = winX;
            rect.y = winY;
            rect.width = winWidth + (borderWidth * 2);
            rect.height = winHeight + (borderWidth * 2);
            XUnionRectWithRegion(&rect, Windows, Windows);
        }
    }
    XFree((char *)children);

#if GRAB_SERVER
    XUngrabServer(display);
#else
    XSetErrorHandler((ErrorHandler *)NULL);
#endif


    return 0;
}

 


/*
   Allocate a color by name.
*/
Pixel
AllocNamedColor(colorName, dfltPix)
char *colorName;
Pixel dfltPix;
{
	Pixel pix;
	XColor scrncolor;
	XColor exactcolor;

	if (XAllocNamedColor(display, DefaultColormap(display, screen),
		colorName, &scrncolor, &exactcolor)) {
		pix = scrncolor.pixel;
	}
	else {
		pix = dfltPix;
	}

	return pix;
}


void sig_alarm()
{
    int rand=RandInt(100);
    void (*sig_alarm_ptr)() = sig_alarm;

    /* No wind at all? */
    if (NoWind) return;

    if( rand > 80 ){
        geta -= 1;
    }else if( rand > 55 ){
        geta += 1;
    }

    if( rand > 65 ){
        if( RandInt(10) > 4 ){
            direction = 1;
        }else{
            direction = -1;
        }

        wind = 2;
        signal(SIGALRM, sig_alarm_ptr);

        alarm(abs(RandInt(5))+1);
    }else{
        if( wind == 2 ){
            wind = 1;
            signal(SIGALRM, sig_alarm_ptr);
            alarm(RandInt(3)+1);
        }else{
            wind = 0;
            signal(SIGALRM, sig_alarm_ptr);
            alarm(WindTimer);
        }
    }
}

