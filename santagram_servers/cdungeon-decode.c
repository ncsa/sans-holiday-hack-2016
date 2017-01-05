/*
Received: from PACIFIC-CARRIER-ANNEX.MIT.EDU by po7.MIT.EDU (5.61/4.7) id AA10214; Fri, 25 Nov 94 19:19:49 EST
Received: from panix.com by MIT.EDU with SMTP
	id AA03556; Fri, 25 Nov 94 19:19:46 EST
Received: by panix.com id AA06757
  (5.67b/IDA-1.5 for jhawk@mit.edu); Fri, 25 Nov 1994 19:19:45 -0500
Date: Fri, 25 Nov 1994 19:19:45 -0500
From: John Hawkinson <jhawk@panix.com>
Message-Id: <199411260019.AA06757@panix.com>
To: jhawk@MIT.EDU
Subject: dungeon

#define EXIT_FAILURE (3141)
#define EXIT_SUCCESS (0)
#define SEEK_SET (0)

/*
From ian@airs.com Sat Sep  4 10:50:38 1993
Received: from relay1.UU.NET by panix.com with SMTP id AA19840
  (5.65c/IDA-1.4.4 for <jhawk@panix.com>); Sat, 4 Sep 1993 10:50:24 -0400
Received: from cygnus.com by relay1.UU.NET with SMTP 
	(5.61/UUNET-internet-primary) id AA19976; Sat, 4 Sep 93 10:50:20 -0400
Received: from tweedledumb.cygnus.com by cygnus.com (4.1/SMI-4.1)
	id AA02163; Sat, 4 Sep 93 07:49:45 PDT
Received: by tweedledumb.cygnus.com (4.1/4.7) id AA26760; Sat, 4 Sep 93 10:49:43 EDT
Message-Id: <9309041449.AA26760@tweedledumb.cygnus.com>
From: ian@airs.com (Ian Lance Taylor)
Subject: Re: Dungeon strings decoder
To: jhawk@panix.com (John Hawkinson)
Date: Sat, 4 Sep 93 10:31:17 -0400
In-Reply-To: <199309022153.AA10453@panix.com>; from "John Hawkinson" at Sep 2, 93 5:53 pm
X-Mailer: ELM [version 2.3 PL11]
Status: RO

> Would you mind mailing me a copy of your program
> to decode the dtext strings for cdungeon?

Here it is.  It's written in ANSI C, but should be easy to convert.
To run it, say something like data -n dtextc.dat -a dtext.asc.  Please
don't ask me any questions about it, as I've already wasted too much
time on it.
-- 
Ian Taylor | ian@airs.com | First to identify quote wins free e-mail message:
``Skeptics might have called that fudging, but IBM, adjusting its wide blue
  tie, pronounced it Computer Science.''

/* data.c -- manipulate data for the dungeon game in various ways
 *
 * This program reads various formats of the dungeon data and writes
 * it out in various formats as well.  It's mostly a hack.  I used it
 * to correct some misspellings and add some history and version
 * information to the game.
 *
 * Ian Lance Taylor 3/8/91 Created
 *
 * This program is copyright (C) Ian Lance Taylor 1991.  You can do
 * anything you like with it, so long as you keep the copyright notice.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#ifdef __AMOS__
#include <amos.h>
#endif

#define TRUE (1)
#define FALSE (0)

#ifndef __GNUC__
#define __inline__
#endif

#ifndef __STDC__
#define const
#endif

/* The library directory */

#ifdef __AMOS__
#define LIBRARY "lib:"
#else
#define LIBRARY "/usr/games/lib/dunlib/"
#endif

/* The default index and text files to use */

#define OLDINDFILE "dindx.dat"
#define OLDTXTFILE "dtext.dat"

#define NEWTXTFILE "dtextc.dat"

/* Routines called by the main routine */

static void
	_uusage(void),
	_ufatal(const char *),
	_ureadascii(void),
	_ureadoldbinary(FILE *, FILE *),
	_ureadnewbinary(FILE *),
	_uwriteascii(void),
	_uwriteoldbinary(const char *, const char *),
	_uwritenewbinary(const char *, char);

/* The main routine */

void main(int argc, char **argv)
{
	char fgotread, fgotwrite;
	char bread = '\0', bwrite = '\0';
	const char *zwriteind = NULL, *zwritetxt = NULL;
	int i;
	FILE *erind = NULL, *ertxt = NULL;

	/* The first set of arguments is the format to read, and
	 * the second set of arguments is the format to write.
	 * The file names are optional.  For the new or old binary
	 * formats we don't open the output files until we have
	 * finished reading the input files (on AMOS this allows us
	 * to determine the size of the random file before creating
	 * it).
	 */

	fgotread = FALSE;
	fgotwrite = FALSE;
	i = 1;
	while (i < argc) {
		char fnamed, fnamed2;
		const char *zind, *ztxt;
		char bchar;

		if (fgotread  &&  fgotwrite)
			_uusage();
		if (argv[i][0] != '-'  ||  argv[i][2] != '\0')
			_uusage();
		fnamed = i + 1 < argc  &&  argv[i + 1][0] != '-';
		fnamed2 = fnamed  &&
			  i + 2 < argc  &&  argv[i + 2][0] != '-';

		bchar = argv[i][1];
		switch (bchar) {
		case 'a':
			if (fnamed) {
				FILE *e;

				if (fgotread)
					e = freopen(argv[i + 1], "w",
						    stdout);
				else
					e = freopen(argv[i + 1], "r",
						    stdin);
				if (e == NULL) {
					perror(argv[i + 1]);
					exit(EXIT_FAILURE);
				}
				i++;
			}
			break;
		case 'b':
		case 'n': /* -n is a synonym for -b */
		case 's': /* -s is -b, but in sequential format */
			if (fnamed) {
				ztxt = argv[i + 1];
				i += 1;
			}
			else {
				if (fgotread)
					ztxt = NEWTXTFILE;
				else
					ztxt = LIBRARY NEWTXTFILE;
			}

			if (fgotread)
				zwritetxt = ztxt;
			else {
#ifdef __AMOS__
				ertxt = fdopen(ropen(ztxt, 0), "rb");
#else
				ertxt = fopen(ztxt, "rb");
#endif
				if (ertxt == NULL) {
					perror(ztxt);
					exit(EXIT_FAILURE);
				}
			}
			break;
		case 'o':
			if (fnamed  &&  ! fnamed2)
				_uusage();
			if (fnamed2) {
				zind = argv[i + 1];
				ztxt = argv[i + 2];
				i += 2;
			}
			else {
				if (fgotread) {
					zind = OLDINDFILE;
					ztxt = OLDTXTFILE;
				}
				else {
					zind = LIBRARY OLDINDFILE;
					ztxt = LIBRARY OLDTXTFILE;
				}
			}

			if (fgotread) {
				zwriteind = zind;
				zwritetxt = ztxt;
			}
			else {
				if ((erind = fopen(zind, "r")) == NULL) {
					perror(zind);
					exit(EXIT_FAILURE);
				}
#ifdef __AMOS__
				ertxt = fdopen(ropen(ztxt, 0), "rb");
#else
				ertxt = fopen(ztxt, "rb");
#endif
				if (ertxt == NULL) {
					perror(ztxt);
					exit(EXIT_FAILURE);
				}
			}
			break;
		default:
			_uusage();
		}

		if (fgotread) {
			bwrite = bchar;
			fgotwrite = TRUE;
		}
		else {
			bread = bchar;
			fgotread = TRUE;
		}

		i++;
	}

	if (! fgotread  ||  ! fgotwrite)
		_uusage();

	/* Read in the input data */

	switch (bread) {
	case 'a':
		_ureadascii();
		break;
	case 'b':
	case 'n':
		_ureadnewbinary(ertxt);
		break;
	case 'o':
		_ureadoldbinary(erind, ertxt);
		break;
	default:
		_ufatal("main: Can't happen");
		break;
	}

	if (erind != NULL)
		(void)fclose(erind);
	if (ertxt != NULL)
		(void)fclose(ertxt);

	/* We don't try to do any error recovery (why bother?) so if
	 * we get here we have succesfully read the data.  Now we
	 * write it out.
	 */

	switch (bwrite) {
	case 'a':
		_uwriteascii();
		break;
	case 'b':
	case 'n':
		_uwritenewbinary(zwritetxt, TRUE);
		break;
	case 's':
		_uwritenewbinary(zwritetxt, FALSE);
		break;
	case 'o':
		_uwriteoldbinary(zwriteind, zwritetxt);
		break;
	default:
		_ufatal("main: Can't happen");
		break;
	}

	exit(EXIT_SUCCESS);
}

/* Print out a usage message and die */

static void _uusage(void)
{
	fprintf(stderr,
		"Copyright (c) Ian Lance Taylor 1991 <ian@airs.com>\n");
	fprintf(stderr,
		"Usage: data -[aobn] [inputfile(s)] -[aobns] [outputfile(s)]\n");
	fprintf(stderr,
		"       -a ASCII (readable) format; one file\n");
	fprintf(stderr,
		"          input default stdin, output default stdout\n");
	fprintf(stderr,
		"       -o old (f77) format; index file, text file\n");
	fprintf(stderr,
		"          input default library files, output default local files\n");
	fprintf(stderr,
		"       -b new compressed binary format; one file\n");
	fprintf(stderr,
		"          input default library file, output default local file\n");
	fprintf(stderr,
		"       -n is a synonym for -b\n");
	fprintf(stderr,
		"       -s creates a sequential file (Alpha Micro only)\n");
	exit(EXIT_FAILURE);
}

/**************************************************************************/

/* Generic subroutines */

/* Print out an error message and die */

static void _ufatal(const char *z)
{
	if (z != NULL)
		fprintf(stderr, "%s\n", z);
	exit(EXIT_FAILURE);
}

/* Copy a string into memory */

static char *_zmalcpy(const char *z)
{
	char *zret;

	if ((zret = malloc(strlen(z) + 1)) == NULL)
		_ufatal("Insufficient memory");
	strcpy(zret, z);
	return zret;
}

/**************************************************************************/

/* The maximum sizes of the various arrays, and also the sizes read in
 * when reading the old data format.
 */

#define ROOMS (200)
#define EXITS (900)
#define OBJECTS (220)
#define ROOM2 (20)
#define CLOCKS (25)
#define VILLS (4)
#define ADVS (4)
#define MESSAGES (1050)
#define MELEEMSGS (137)

/* The data itself */

static int _ivermaj, _ivermin, _iveredit;
static int _imaxscore, _istars, _imaxendscore;
static int _imaxroom;
static char *_azrdesc1[ROOMS], *_azrdesc2[ROOMS];
static int _airexit[ROOMS], _airaction[ROOMS];
static int _airval[ROOMS], _airflag[ROOMS];
static int _imaxexit, _aiexit[EXITS];
static char *_azexitstring[EXITS];
static int _imaxobject;
static char *_azodesc1[OBJECTS], *_azodesc2[OBJECTS], *_azodesco[OBJECTS];
static int _aioaction[OBJECTS], _aioflag1[OBJECTS];
static int _aioflag2[OBJECTS], _aiofval[OBJECTS], _aiotval[OBJECTS];
static int _aiosize[OBJECTS], _aiocapacity[OBJECTS], _aioroom[OBJECTS];
static int _aioadventurer[OBJECTS], _aiocontainer[OBJECTS];
static char *_azoread[OBJECTS];
static int _imaxroom2, _aioroom2[ROOM2], _airroom2[ROOM2];
static int _imaxclock, _aictick[CLOCKS], _aicaction[CLOCKS];
static char _afcflag[CLOCKS];
static int _imaxvill, _aivillns[VILLS], _aivprob[VILLS], _aivopps[VILLS];
static int _aivbest[VILLS], _aivmelee[VILLS];
static int _imaxadv, _aiaroom[ADVS], _aiascore[ADVS], _aiavehicle[ADVS];
static int _aiaobj[ADVS], _aiaaction[ADVS], _aiastrength[ADVS];
static int _aiaflag[ADVS];
static int _imbase, _imaxmessage;
static char *_azrtext[MESSAGES];

/**************************************************************************/

/* The directions in order of value according to the exit array.  The
 * value in the index array is 0x400 * the index in this array + 1, so for
 * example ne is 0x400 * (1 + 1) = 0x800.  This is needed in a couple
 * of different places.
 */

static const char * const _azdirs[] =
	{ "north", "ne", "east", "se", "south", "sw", "west", "nw", "up",
	  "down", "launch", "land", "enter", "exit", "cross" };

#define DIRS (sizeof _azdirs / sizeof _azdirs[0])

/**************************************************************************/

/* Read the old binary format */

/* Read an integer from the index file */

static int _ireadoldint(FILE *eind)
{
	int i;

	if (fscanf(eind, "%d", &i) != 1) {
		if (feof(eind))
			_ufatal("_ireadoldint: Unexpected EOF");
		else
			_ufatal(strerror(errno));
	}

	return i;
}

/* Read a number of integers from the index file */

static void _ureadoldints(int c, int *pi, FILE *eind)
{
	while (c-- != 0)
		*pi++ = _ireadoldint(eind);
}

/* Read a number of flags from the index file */

static void _ureadoldflags(int c, char *pf, FILE *eind)
{
	while (c-- != 0) {
		int ichar;

		while (isspace(ichar = getc(eind)))
			;
		if (ichar == 'T')
			*pf++ = TRUE;
		else if (ichar == 'F')
			*pf++ = FALSE;
		else
			_ufatal("_ureadoldflags: Read bad character");
	}
}

/* Read a string from the text file.  These are encoded in a fairly
 * simple format.  The text file itself is broken up into 76 byte
 * records.  We read the string into memory, separating lines with
 * newline characters; however, there is no terminating newline.
 */

static char *_zreadoldstring(int i, FILE *etxt)
{
	int inum, irec;
	char *zret;
	int clen;

	if (i == 0)
		return NULL;

	if (i > 0) {
		fprintf(stderr, "_zoldreadstring: Positive value %d\n", i);
		_ufatal(NULL);
	}

	zret = NULL;
	clen = 0;

	irec = - i;
	if (fseek(etxt, (irec - 1) * 76, SEEK_SET) != 0)
		_ufatal("_zoldreadstring: Can't fseek");

	inum = getc(etxt);
	inum += getc(etxt) * 256;
	while (TRUE) {
		char ab[75];
		int i2, ilast, inewnum;
		int cnew;

		if (fread(ab, 1, 74, etxt) != 74)
			_ufatal("_zoldreadstring: Can't fread");
		ilast = -1;
		for (i2 = 0;  i2 < 74;  i2++) {
			ab[i2] = ab[i2] ^ ((irec & 31) + i2 + 1);
			if (ab[i2] != ' ')
				ilast = i2;
		}
		ab[ilast + 1] = '\0';

		cnew = strlen(ab);
		if ((zret = realloc(zret, clen + cnew + 1)) == NULL)
			_ufatal("_zoldreadstring: Insufficient memory");
		strcpy(zret + clen, ab);
		clen += cnew;

		inewnum = getc(etxt);
		inewnum += getc(etxt) * 256;
		if (inum != inewnum)
			return zret;

		/* Turn the null byte into a newline character.  This is
		 * no longer a proper string, and there's no room for
		 * a new null byte; it will be fixed up when the next
		 * string is copied in.
		 */

		zret[clen] = '\n';
		clen++;

		irec++;
	}
}

/* Read information from the old style files.  This format is very simple
 * and easy to read, but it wastes a lot of space.
 */

static void _ureadoldbinary(FILE *eind, FILE *etxt)
{
	register int i;
	int irdesc2;
	int airdesc1[ROOMS];
	int aiodesc1[OBJECTS], aiodesc2[OBJECTS], aiodesco[OBJECTS];
	int aioread[OBJECTS];
	int airtext[MESSAGES];

	/* Read general information */

	_ivermaj = _ireadoldint(eind);
	_ivermin = _ireadoldint(eind);
	_iveredit = _ireadoldint(eind);

	_imaxscore = _ireadoldint(eind);
	_istars = _ireadoldint(eind);
	_imaxendscore = _ireadoldint(eind);

	/* Read the room information */

	_imaxroom = _ireadoldint(eind);
	irdesc2 = _ireadoldint(eind);
	_ureadoldints(ROOMS, airdesc1, eind);
	_ureadoldints(ROOMS, _airexit, eind);
	_ureadoldints(ROOMS, _airaction, eind);
	_ureadoldints(ROOMS, _airval, eind);
	_ureadoldints(ROOMS, _airflag, eind);

	/* Turn the room text file indices into strings */

	for (i = 0;  i < _imaxroom;  i++) {
		_azrdesc1[i] = _zreadoldstring(airdesc1[i], etxt);
		_azrdesc2[i] = _zreadoldstring(irdesc2 - (i + 1), etxt);
	}

	/* Read the exit information */

	_imaxexit = _ireadoldint(eind);
	_ureadoldints(EXITS, _aiexit, eind);

	/* We now must read in all the exit strings, which is unfortunately
	 * complex because of the format of the exit array.
	 */

	for (i = 0;  i < _imaxroom;  i++) {
		int iind;
		unsigned int ival;

		iind = _airexit[i];
		if (iind == 0)
			continue;
		iind--;

		do {
			int itype;

			ival = _aiexit[iind];
			itype = (ival >> 8) & 3;
			if (itype != 0)
				_azexitstring[iind + 1] =
					_zreadoldstring(_aiexit[iind + 1],
							etxt);
			switch (itype) {
			case 0:
				iind += 1;
				break;
			case 1:
				iind += 2;
				break;
			case 2:
			case 3:
				iind += 3;
				break;
			}
		} while ((ival & 0x8000) == 0);
	}

	/* Read all the object information */

	_imaxobject = _ireadoldint(eind);
	_ureadoldints(OBJECTS, aiodesc1, eind);
	_ureadoldints(OBJECTS, aiodesc2, eind);
	_ureadoldints(OBJECTS, aiodesco, eind);
	_ureadoldints(OBJECTS, _aioaction, eind);
	_ureadoldints(OBJECTS, _aioflag1, eind);
	_ureadoldints(OBJECTS, _aioflag2, eind);
	_ureadoldints(OBJECTS, _aiofval, eind);
	_ureadoldints(OBJECTS, _aiotval, eind);
	_ureadoldints(OBJECTS, _aiosize, eind);
	_ureadoldints(OBJECTS, _aiocapacity, eind);
	_ureadoldints(OBJECTS, _aioroom, eind);
	_ureadoldints(OBJECTS, _aioadventurer, eind);
	_ureadoldints(OBJECTS, _aiocontainer, eind);
	_ureadoldints(OBJECTS, aioread, eind);

	/* Turn the object strings from indices into strings */

	for (i = 0;  i < _imaxobject;  i++) {
		_azodesc1[i] = _zreadoldstring(aiodesc1[i], etxt);
		_azodesc2[i] = _zreadoldstring(aiodesc2[i], etxt);
		_azodesco[i] = _zreadoldstring(aiodesco[i], etxt);
		_azoread[i] = _zreadoldstring(aioread[i], etxt);
	}

	/* Read the double room information */

	_imaxroom2 = _ireadoldint(eind);
	_ureadoldints(ROOM2, _aioroom2, eind);
	_ureadoldints(ROOM2, _airroom2, eind);

	/* Read the clock event information */

	_imaxclock = _ireadoldint(eind);
	_ureadoldints(CLOCKS, _aictick, eind);
	_ureadoldints(CLOCKS, _aicaction, eind);
	_ureadoldflags(CLOCKS, _afcflag, eind);

	/* Read the villian information */

	_imaxvill = _ireadoldint(eind);
	_ureadoldints(VILLS, _aivillns, eind);
	_ureadoldints(VILLS, _aivprob, eind);
	_ureadoldints(VILLS, _aivopps, eind);
	_ureadoldints(VILLS, _aivbest, eind);
	_ureadoldints(VILLS, _aivmelee, eind);

	/* Read the adventurer information */

	_imaxadv = _ireadoldint(eind);
	_ureadoldints(ADVS, _aiaroom, eind);
	_ureadoldints(ADVS, _aiascore, eind);
	_ureadoldints(ADVS, _aiavehicle, eind);
	_ureadoldints(ADVS, _aiaobj, eind);
	_ureadoldints(ADVS, _aiaaction, eind);
	_ureadoldints(ADVS, _aiastrength, eind);
	_ureadoldints(ADVS, _aiaflag, eind);

	/* Read the melee message base */

	_imbase = _ireadoldint(eind);

	/* Read the messages (it turns out that the _imaxmessage value
	 * stored in the file is not useful, so we correct it here).
	 */

	_imaxmessage = _ireadoldint(eind);
	_imaxmessage = _imbase + MELEEMSGS;
	_ureadoldints(MESSAGES, airtext, eind);

	/* Convert the messages into strings */

	for (i = 0;  i < _imaxmessage;  i++)
		_azrtext[i] = _zreadoldstring(airtext[i], etxt);

	/* Wasn't that easy? */
}

/**************************************************************************/

/* Write out the old binary format.  The main purpose of this is to
 * check that I can recreate the data I have read in, thus ensuring
 * that I haven't made any mistakes when reading it.  Therefore this
 * code goes to a fair amount of trouble to try to recreate the same
 * format as the original data, which basically means getting the
 * strings into the same place in the text file.
 */

/* Write an integer to the index file */

__inline__ static void _uwriteoldint(int i, FILE *eind)
{
	fprintf(eind, "%6d\n", i <= 32767 ? i : i - 65536);
}

/* Write a bunch of integers to the index file */

static void _uwriteoldints(int c, int *pi, FILE *eind)
{
	while (c-- != 0)
		_uwriteoldint(*pi++, eind);
}

/* Write a bunch of flags to the index file */

static void _uwriteoldflags(int c, char *pf, FILE *eind)
{
	while (c-- != 0)
		fprintf(eind, " %c\n", *pf++ ? 'T' : 'F');
}

/* Write a string to the text file */

static int _iwostrrec;

static int _iwriteoldstring(const char *z, int inum, FILE *etxt)
{
	int iret;

	if (ftell(etxt) != (_iwostrrec - 1) * 76)
		_ufatal("_iwriteoldstring: Bad file position");

	iret = - _iwostrrec;

	if (inum == 0)
		inum = _iwostrrec;

	do {
		char ab[74];
		char *zset, *zend;
		int ixor;

		putc(inum & 0xff, etxt);
		putc(inum >> 8, etxt);

		ixor = _iwostrrec & 31;

		zset = ab;
		while (*z != '\n'  &&  *z != '\0')
			*zset++ = *z++ ^ (ixor + (zset - ab + 1));

		zend = ab + 74;
		while (zset < zend)
			*zset++ = ' ' ^ (ixor + (zset - ab + 1));

		if (fwrite(ab, 1, 74, etxt) != 74)
			_ufatal("_iwriteoldstring: Bad fwrite");

		_iwostrrec++;
	} while (*z++ != '\0');

	return iret;
}

#ifdef __AMOS__

/* Count how many text file records a string will require. */

static int _cmsgsize(const char *z)
{
	int cret;

	cret = 1;
	while ((z = strchr(z, '\n')) != NULL) {
		cret++;
		z++;
	}

	return cret;
}

#endif

/* Main routine for writing information in the old binary format */

static void _uwriteoldbinary(const char *zind, const char *ztxt)
{
	register int i;
	FILE *eind, *etxt;
	int airtext[MESSAGES];
	int airdesc1[ROOMS], irdesc2;
	int aiodesc1[OBJECTS], aiodesc2[OBJECTS], aiodesco[OBJECTS];
	int aioread[OBJECTS];
#ifdef __AMOS__
	int cmessages;
#endif

	/* Assemble all the messages.  Here is the order and record
	 * numbering (the numbers used in the first two bytes) found
	 * in the original data file:
	 *	general messages (message number)
	 *	melee messages (1-MELEEMSGS)
	 *	long room descriptions (room number)
	 *	short room descriptions (room number)
	 *	long object descriptions (object number)
	 *	short object descriptions (object number)
	 *	initial object descriptions (object number)
	 *	object read strings (object number)
	 *	exit strings (record number in file)
	 */

#ifdef __AMOS__
	/* On AMOS we have to count all the messages so that we know
	 * how big to make the random file.
	 */

	cmessages = 0;
	for (i = 0;  i < _imaxmessage;  i++)
		cmessages += _cmsgsize(_azrtext[i]);

	for (i = 0;  i < _imaxroom;  i++) {
		if (_azrdesc1[i] != NULL)
			cmessages += _cmsgsize(_azrdesc1[i]);
		if (_azrdesc2[i] != NULL)
			cmessages += _cmsgsize(_azrdesc2[i]);
	}

	for (i = 0;  i < _imaxobject;  i++) {
		if (_azodesc1[i] != NULL)
			cmessages += _cmsgsize(_azodesc1[i]);
		if (_azodesc2[i] != NULL)
			cmessages += _cmsgsize(_azodesc2[i]);
		if (_azodesco[i] != NULL)
			cmessages += _cmsgsize(_azodesco[i]);
		if (_azoread[i] != NULL)
			cmessages += _cmsgsize(_azoread[i]);
	}

	for (i = 0;  i < _imaxexit;  i++)
		if (_azexitstring[i] != NULL)
			cmessages += _cmsgsize(_azexitstring[i]);

	etxt = fdopen(rcreat(ztxt, ((cmessages * 76) + 511) / 512, 0), "wb");
#else
	etxt = fopen(ztxt, "wb");
#endif

	if (etxt == NULL) {
		perror(ztxt);
		_ufatal(NULL);
	}

	/* Clear the message text arrays */

	memset(airtext, 0, sizeof airtext);
	memset(airdesc1, 0, sizeof airdesc1);
	memset(aiodesc1, 0, sizeof aiodesc1);
	memset(aiodesc2, 0, sizeof aiodesc2);
	memset(aiodesco, 0, sizeof aiodesco);
	memset(aioread, 0, sizeof aioread);

	/* Write out the messages */

	_iwostrrec = 1;

	for (i = 0;  i < _imbase;  i++)
		airtext[i] = _iwriteoldstring(_azrtext[i], i + 1, etxt);

	for (i = _imbase;  i < _imaxmessage;  i++)
		airtext[i] = _iwriteoldstring(_azrtext[i], i - _imbase + 1,
					      etxt);

	for (i = 0;  i < _imaxroom;  i++) {
		if (_azrdesc1[i] != NULL)
			airdesc1[i] = _iwriteoldstring(_azrdesc1[i],
						       i + 1, etxt);
	}

	/* There is no array of short room names, but instead they
	 * are all offset by the room number from a base (irdesc2).
	 */

	irdesc2 = _iwriteoldstring(_azrdesc2[0], 1, etxt) + 1;
	for (i = 1;  i < _imaxroom;  i++)
		(void)_iwriteoldstring(_azrdesc2[i], i + 1, etxt);

	for (i = 0;  i < _imaxobject;  i++) {
		if (_azodesc1[i] != NULL)
			aiodesc1[i] = _iwriteoldstring(_azodesc1[i],
						       i + 1, etxt);
	}

	for (i = 0;  i < _imaxobject;  i++) {
		if (_azodesc2[i] != NULL)
			aiodesc2[i] = _iwriteoldstring(_azodesc2[i],
						       i + 1, etxt);
	}

 	for (i = 0;  i < _imaxobject;  i++) {
		if (_azodesco[i] != NULL)
			aiodesco[i] = _iwriteoldstring(_azodesco[i],
						       i +  1, etxt);
	}

	for (i = 0;  i < _imaxobject;  i++) {
		if (_azoread[i] != NULL)
			aioread[i] = _iwriteoldstring(_azoread[i],
						      i + 1, etxt);
	}

	for (i = 0;  i < _imaxexit;  i++) {
		if (_azexitstring[i] != NULL)
			_aiexit[i] = _iwriteoldstring(_azexitstring[i],
						      0, etxt);
	}

	if (fclose(etxt) == EOF)
		_ufatal("_uwriteoldbinary: Can't close text file");

	/* Open the index file */

	if ((eind = fopen(zind, "w")) == NULL) {
		perror(zind);
		_ufatal(NULL);
	}

	/* Write general information */

	_uwriteoldint(_ivermaj, eind);
	_uwriteoldint(_ivermin, eind);
	_uwriteoldint(_iveredit, eind);

	_uwriteoldint(_imaxscore, eind);
	_uwriteoldint(_istars, eind);
	_uwriteoldint(_imaxendscore, eind);

	/* Write the room information */

	_uwriteoldint(_imaxroom, eind);
	_uwriteoldint(irdesc2, eind);
	_uwriteoldints(ROOMS, airdesc1, eind);
	_uwriteoldints(ROOMS, _airexit, eind);
	_uwriteoldints(ROOMS, _airaction, eind);
	_uwriteoldints(ROOMS, _airval, eind);
	_uwriteoldints(ROOMS, _airflag, eind);

	/* Write the exit information */

	_uwriteoldint(_imaxexit, eind);
	_uwriteoldints(EXITS, _aiexit, eind);

	/* Write all the object information */

	_uwriteoldint(_imaxobject, eind);
	_uwriteoldints(OBJECTS, aiodesc1, eind);
	_uwriteoldints(OBJECTS, aiodesc2, eind);
	_uwriteoldints(OBJECTS, aiodesco, eind);
	_uwriteoldints(OBJECTS, _aioaction, eind);
	_uwriteoldints(OBJECTS, _aioflag1, eind);
	_uwriteoldints(OBJECTS, _aioflag2, eind);
	_uwriteoldints(OBJECTS, _aiofval, eind);
	_uwriteoldints(OBJECTS, _aiotval, eind);
	_uwriteoldints(OBJECTS, _aiosize, eind);
	_uwriteoldints(OBJECTS, _aiocapacity, eind);
	_uwriteoldints(OBJECTS, _aioroom, eind);
	_uwriteoldints(OBJECTS, _aioadventurer, eind);
	_uwriteoldints(OBJECTS, _aiocontainer, eind);
	_uwriteoldints(OBJECTS, aioread, eind);

	/* Write the double room information */

	_uwriteoldint(_imaxroom2, eind);
	_uwriteoldints(ROOM2, _aioroom2, eind);
	_uwriteoldints(ROOM2, _airroom2, eind);

	/* Write the clock event information */

	_uwriteoldint(_imaxclock, eind);
	_uwriteoldints(CLOCKS, _aictick, eind);
	_uwriteoldints(CLOCKS, _aicaction, eind);
	_uwriteoldflags(CLOCKS, _afcflag, eind);

	/* Write the villian information */

	_uwriteoldint(_imaxvill, eind);
	_uwriteoldints(VILLS, _aivillns, eind);
	_uwriteoldints(VILLS, _aivprob, eind);
	_uwriteoldints(VILLS, _aivopps, eind);
	_uwriteoldints(VILLS, _aivbest, eind);
	_uwriteoldints(VILLS, _aivmelee, eind);

	/* Write the adventurer information */

	_uwriteoldint(_imaxadv, eind);
	_uwriteoldints(ADVS, _aiaroom, eind);
	_uwriteoldints(ADVS, _aiascore, eind);
	_uwriteoldints(ADVS, _aiavehicle, eind);
	_uwriteoldints(ADVS, _aiaobj, eind);
	_uwriteoldints(ADVS, _aiaaction, eind);
	_uwriteoldints(ADVS, _aiastrength, eind);
	_uwriteoldints(ADVS, _aiaflag, eind);

	/* Write the melee message base */

	_uwriteoldint(_imbase, eind);

	/* Write out the messages (this will get _imaxmessage wrong,
	 * since I don't know where the number in the original file
	 * comes from).
	 */

	_uwriteoldint(_imaxmessage, eind);
	_uwriteoldints(MESSAGES, airtext, eind);

	/* That's all */

	if (fclose(eind) == EOF)
		_ufatal("_uwriteoldbinary: Can't close index file");
}

/**************************************************************************/

/* Read in the ASCII format.  We're pretty liberal about what we read,
 * since we ignore a lot of stuff.  All the explanatory text dumped
 * into the file by the writing routines is ignored; only the numbers
 * are significant.  The general format of the file is attribute/value
 * pairs.  The file is always read from stdin.
 */

/* The generic maximum line size we are prepared to read */

#define MAXLINE (100)

/* This structure holds a dispatch table which is used to decide what
 * to do with a line from the file when reading the ASCII format.
 * Each command either invokes another function or sets an integer
 * variable.
 */

struct scmd {
	const char *z;			/* Command */
	void (*pfn)(const char *);	/* Function to call */
	int *pi;			/* Pointer to integer to set */
};

/* Top level file lines */

static void
	_ureadasciiversion(const char *),
	_ureadasciiroom(const char *),
	_ureadasciiobject(const char *),
	_ureadasciidoubleroom(const char *),
	_ureadasciiclock(const char *),
	_ureadasciivillian(const char *),
	_ureadasciiadventurer(const char *),
	_ureadasciimessage(const char *);

static struct scmd _astopcmds[] =
	{ { "Version", &_ureadasciiversion, NULL },
	  { "Maximum score", NULL, &_imaxscore },
	  { "Stars", NULL, &_istars },
	  { "Maximum endgame score", NULL, &_imaxendscore },
	  { "Room", &_ureadasciiroom, NULL },
	  { "Object", &_ureadasciiobject, NULL },
	  { "Double room", &_ureadasciidoubleroom, NULL },
	  { "Clock", &_ureadasciiclock, NULL },
	  { "Villian", &_ureadasciivillian, NULL },
	  { "Adventurer", &_ureadasciiadventurer, NULL },
	  { "Melee message base", NULL, &_imbase },
	  { "Message", &_ureadasciimessage, NULL },
	  { NULL, NULL, NULL } };

/* Give a fatal error */

static void _ureadasciifatal(const char *z, const char *zarg)
{
	int i;

	if (z != NULL) {
		fprintf(stderr, "%s", z);
		if (zarg != NULL)
			fprintf(stderr, ": %s", zarg);
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "Next 5 lines:\n");
	for (i = 0;  i < 5;  i++) {
		char ab[MAXLINE];

		if (fgets(ab, sizeof ab, stdin) == NULL) {
			fprintf(stderr, "***EOF***\n");
			break;
		}
		fprintf(stderr, "%s", ab);
	}

	_ufatal(NULL);
}

/* Read a message from stdin.  Each line of the message is indented by
 * a single space.  This reads each line that begins with a space, and
 * catenates them together after removing the initial space.  If there
 * are no lines with an initial space, it returns NULL.  The lines
 * are separated by newlines, but any trailing newline is removed.
 */

static char *_zreadmsg(void)
{
	int inext;
	char *zret;
	int clen;

	zret = NULL;
	clen = 0;
	while ((inext = getchar()) == ' ') {
		char ab[MAXLINE];
		int cnew;

		if (fgets(ab, sizeof ab, stdin) == NULL)
			_ureadasciifatal("Unexpected EOF", NULL);
		cnew = strlen(ab);
		if ((zret = realloc(zret, clen + cnew + 1)) == NULL)
			_ufatal("Insufficient memory");
		strcpy(zret + clen, ab);
		clen += cnew;
	}

	(void)ungetc(inext, stdin);
	if (zret != NULL  &&  zret[clen - 1] == '\n')
		zret[clen - 1] = '\0';

	return zret;
}

/* Dispatch a command */

static void _ureadasciidispatch(const char *z, struct scmd *qcmds)
{
	char *zcolon;
	int clen;

	if ((zcolon = strchr(z, ':')) == NULL)
		_ureadasciifatal("Badly formatted line", z);
	clen = zcolon - z;

	for (;  qcmds->z != NULL;  qcmds++) {
		if (strncmp(z, qcmds->z, clen) != 0)
			continue;

		if (qcmds->pfn != NULL)
			(*qcmds->pfn)(zcolon + 1);
		else if (qcmds->pi != NULL) {
			int i;

			if (*qcmds->pi != 0)
				_ureadasciifatal("Duplicate", z);
			if (sscanf(z + clen + 1, "%d", &i) != 1)
				_ureadasciifatal("No number", z);
			*qcmds->pi = i;
		}
		else
			_ureadasciifatal("_ureadasciidispatch: Can't happen",
					 z);

		return;
	}

	_ureadasciifatal("Unrecognized line", z);
}

/* Clear all the integers pointed to, and then dispatch commands until
 * we reach a blank line.
 */

static void _ureadasciidispatchtoblank(struct scmd *qcmds)
{
	char ab[MAXLINE];
	struct scmd *q;

	for (q = qcmds;  q->z != NULL;  q++)
		if (q->pi != NULL)
			*q->pi = 0;

	while (TRUE) {
		if (fgets(ab, sizeof ab, stdin) == NULL)
			_ureadasciifatal("Unexpected EOF", NULL);
		if (ab[0] == '\n'  ||  ab[0] == '\0')
			return;
		_ureadasciidispatch(ab, qcmds);
	}
}

/* The main function to read the ASCII file */

static void _ureadascii(void)
{
	char ab[MAXLINE];

	/* The way in which we manipulate _imaxexit requires that we
	 * initialize it to 1.
	 */

	_imaxexit = 1;

	while (fgets(ab, sizeof ab, stdin) != NULL) {
		if (ab[0] == '\0'  ||  ab[0] == '\n')
			continue;
		_ureadasciidispatch(ab, _astopcmds);
	}
}

/* Get the version number */

static void _ureadasciiversion(const char *z)
{
	char cver;

	if (_ivermaj != 0  ||  _ivermin != 0  ||  _iveredit != 0)
		_ureadasciifatal("Duplicate version line", z);
	if (sscanf(z, "%1d.%1d%c", &_ivermaj, &_ivermin, &cver) != 3)
		_ureadasciifatal("Bad version line", z);
	_iveredit = cver;
}

/* Get room information */

static int _iroomaction, _iroomval, _iroomflag;

static void _ureadasciiexits(const char *);

static struct scmd _asroomcmds[] =
	{ { "Action", NULL, &_iroomaction },
	  { "Value", NULL, &_iroomval },
	  { "Flags", NULL, &_iroomflag },
	  { "Exits", &_ureadasciiexits, NULL },
	  { NULL, NULL, NULL } };

static void _ureadasciiroom(const char *z)
{
	int i;
	char *zend;
	int clen, iexit;

	/* The first line looks like
	 * "Room: 1: East-West Passage"
	 */	

	i = strtol(z, &zend, 10);
	if (_azrdesc2[i - 1] != NULL)
		_ureadasciifatal("Duplicate room", z);
	if (i < 1  ||  i > ROOMS)
		_ureadasciifatal("Too many rooms", z);
	if (i > _imaxroom)
		_imaxroom = i;
	if (strncmp(zend, ": ", 2) != 0)
		_ureadasciifatal("Bad room line", z);
	_azrdesc2[i - 1] = _zmalcpy(zend + 2);
	clen = strlen(_azrdesc2[i - 1]);
	if (_azrdesc2[i - 1][clen - 1] == '\n')
		_azrdesc2[i - 1][clen - 1] = '\0';

	/* Next comes the long description of the room, indented by
	 * a single space.
	 */

	_azrdesc1[i - 1] = _zreadmsg();

	/* Next come the various specific values, which we dispatch on */

	iexit = _imaxexit;

	_ureadasciidispatchtoblank(_asroomcmds);

	_airaction[i - 1] = _iroomaction;
	_airval[i - 1] = _iroomval;
	_airflag[i - 1] = _iroomflag;
	if (iexit != _imaxexit)
		_airexit[i - 1] = iexit;
	else
		_airexit[i - 1] = 0;
}

/* Process the exit information from a room.  This is stored in a fairly
 * compact format.  Each room holds an index into the exit array.  The
 * exit array is processed as a stream of words.  If the first word is i:
 *	i & 0x8000 is on if this is the last exit
 *	i & 0x7c00 is the direction (as ordered by _azdirs)
 *	i & 0x300 is the type (see below)
 *	i & 0xff is the room number to go to (zero if none)
 * If the type is:
 *	0: no further information
 *	1: not an actual exit; next word is string index to print
 *	2: next word is string index in case of failure, and next + 1 word
 *	   is a pair of bytes.  The high byte is an exit action to run
 *	   (zero if none) and the low byte is a flag to check.
 *	3: next word is string index in case of failure, and next + 1 word
 *	   is a pair of bytes.  The high byte is an exit action to run
 *	   (zero if none) and the low byte is a door object which must
 *	   be open.
 * Any string index may be zero, in which case a default string will be
 * printed.
 *
 * In the ASCII data file, the format is space character direction (a
 * string): type room, followed by optional Action:, Flag:, Door: or
 * String: lines all indented two spaces.  Here are some examples:
 *
 * Exits:
 *  east: 0 83 (Round Room)
 *  south: 2 37 (Deep Ravine)
 *   Action: 1
 *   Flag: 19 (egypt)
 *   String: The coffin will not fit through this passage.
 *  up: 3 36 (Clearing)
 *   Door: 65 (grating)
 *   String: The grating is locked.
 *
 * This format assumes that the exit strings are never more than a
 * single line.  It could be extended fairly easily if necessary.
 *
 * When we load the ASCII data file we set _azexitstring to hold the
 * string value instead of setting the corresponding index in
 * _aiexit.  This lets us simply look through _azexitstring at the end
 * to see where we have to write out a string value, and we can update
 * _aiexit at that point.
 */

static int _iexitaction, _iexitdoor, _iexitflag;
static char *_zexitstring;

static void _ureadasciiexitstring(const char *);

static struct scmd _asexitcmds[] =
	{ { "Action", NULL, &_iexitaction },
	  { "Door", NULL, &_iexitdoor },
	  { "Flag", NULL, &_iexitflag },
	  { "String", &_ureadasciiexitstring, NULL },
	  { NULL, NULL, NULL } };

static void _ureadasciiexits(const char *z)
{
	int ichar;
	int ilast;

	/* We want to read lines that follow that start with a space.
	 * The loop is going to be reading lines to start with a
	 * double space, so the loop assumes that the first character
	 * is a space already.  Start things by going by making sure
	 * of this.
	 */

	if ((ichar = getchar()) != ' ') {
		(void)ungetc(ichar, stdin);
		return;
	}

	while (TRUE) {
		char ab[MAXLINE];
		char abdir[MAXLINE];
		int idir, itype, iroom;

		if (fgets(ab, sizeof ab, stdin) == NULL)
			_ureadasciifatal("Unexpected EOF", NULL);
		if (sscanf(ab, " %[^:]: %d %d", abdir, &itype, &iroom) != 3)
			_ureadasciifatal("Bad exit line", ab);

		for (idir = 0;  idir < DIRS;  idir++)
			if (strcmp(_azdirs[idir], abdir) == 0)
				break;
		if (idir >= DIRS)
			_ureadasciifatal("Bad direction", ab);

		if (itype < 0  ||
		    itype > 3  ||
		    iroom < 0  ||
		    iroom > 255)
			_ureadasciifatal("Bad exit information", ab);

		ilast = _imaxexit - 1;
		_aiexit[ilast] = ((idir + 1) << 10) + (itype << 8) + iroom;

		switch (itype) {
		case 0:
			_imaxexit += 1;
			break;
		case 1:
			_imaxexit += 2;
			break;
		case 2:
		case 3:
			_imaxexit += 3;
			break;
		}

		if (_imaxexit > EXITS)
			_ureadasciifatal("Too many exits", NULL);

		/* Now read lines until we find one that does not begin
		 * with two spaces.  If we find a line that does not
		 * begin with any spaces, we have reached the end of the
		 * exit information.
		 */

		while (TRUE) { 

			/* If this isn't a space, return.  This is
			 * the only exit from this function.
			 */

			if ((ichar = getchar()) != ' ') {
				(void)ungetc(ichar, stdin);
				_aiexit[ilast] |= 0x8000;
				return;
			}

			/* If this isn't a second space, go around the
			 * main loop again to read the next direction.
			 */

			if ((ichar = getchar()) != ' ') {
				(void)ungetc(ichar, stdin);
				break;
			}

			if (fgets(ab, sizeof ab, stdin) == NULL)
				_ureadasciifatal("Unexpected EOF", NULL);

			_iexitaction = 0;
			_iexitdoor = 0;
			_iexitflag = 0;
			_zexitstring = NULL;

			_ureadasciidispatch(ab, _asexitcmds);

			if (_iexitaction != 0) {
				if (itype != 2  &&  itype != 3)
					_ureadasciifatal("Bad action", ab);
				if ((_aiexit[ilast + 2] >> 8) != 0)
					_ureadasciifatal("Duplicate action",
							 ab);
				_aiexit[ilast + 2] += _iexitaction << 8;
			}
			else if (_iexitdoor != 0) {
				if (itype != 3)
					_ureadasciifatal("Bad door", ab);
				if ((_aiexit[ilast + 2] & 0xff) != 0)
					_ureadasciifatal("Duplicate door",
							 ab);
				_aiexit[ilast + 2] += _iexitdoor;
			}
			else if (_iexitflag != 0) {
				if (itype != 2)
					_ureadasciifatal("Bad flag", ab);
				if ((_aiexit[ilast + 2] & 0xff) != 0)
					_ureadasciifatal("Duplicate flag",
							 ab);
				_aiexit[ilast + 2] += _iexitflag;
			}
			else if (_zexitstring != NULL) {
				if (itype == 0)
					_ureadasciifatal("Bad string", ab);
				if (_azexitstring[ilast + 1] != NULL)
					_ureadasciifatal("Duplicate string",
							 ab);
				_azexitstring[ilast + 1] = _zexitstring;
			}
			else
				_ureadasciifatal("_ureadasciiexits: Can't happen",
						 ab);
		}
	}
}

/* Read an exit string.  This just malcpys the string into _zexitstring. */

static void _ureadasciiexitstring(const char *z)
{
	int clen;

	/* This should look like "String: foo", so we have to skip the
	 * leading space.
	 */
	if (*z != ' ')
		_ureadasciifatal("Bad exit string", z);
	_zexitstring = _zmalcpy(z + 1);
	clen = strlen(_zexitstring);
	if (_zexitstring[clen - 1] == '\n')
		_zexitstring[clen - 1] = '\0';
}

/* Get object information */

static int _iobjectaction, _iobjectflag1, _iobjectflag2, _iobjectfval;
static int _iobjecttval, _iobjectsize, _iobjectcapacity, _iobjectroom;
static int _iobjectadventurer, _iobjectcontainer;
static char *_zobjectdesco, *_zobjectread;

static void
	_ureadasciiobjectdesco(const char *),
	_ureadasciiobjectread(const char *);

static struct scmd _asobjectcmds[] =
	{ { "Action", NULL, &_iobjectaction },
	  { "Flag1", NULL, &_iobjectflag1 },
	  { "Flag2", NULL, &_iobjectflag2 },
	  { "Find value", NULL, &_iobjectfval },
	  { "Treasure value", NULL, &_iobjecttval },
	  { "Size", NULL, &_iobjectsize },
	  { "Capacity", NULL, &_iobjectcapacity },
	  { "Room", NULL, &_iobjectroom },
	  { "Adventurer", NULL, &_iobjectadventurer },
	  { "Container", NULL, &_iobjectcontainer },
	  { "Initial", &_ureadasciiobjectdesco, NULL },
	  { "Read", &_ureadasciiobjectread, NULL },
	  { NULL, NULL, NULL } };

static void _ureadasciiobject(const char *z)
{
	int i;
	char *zend;
	int ichar;

	/* The first line looks like 
	 * "Object: 1: brown sack"
	 * but the short description may not exist.
	 */

	i = strtol(z, &zend, 10);
	if (_azodesc2[i - 1] != NULL  ||  _azodesc1[i - 1] != NULL)
		_ureadasciifatal("Duplicate object", z);
	if (i < 1  ||  i > OBJECTS)
		_ureadasciifatal("Too many objects", z);
	if (i > _imaxobject)
		_imaxobject = i;
	if (strncmp(zend, ": ", 2) == 0) {
		int clen;

		_azodesc2[i - 1] = _zmalcpy(zend + 2);
		clen = strlen(_azodesc2[i - 1]);
		if (_azodesc2[i - 1][clen - 1] == '\n')
			_azodesc2[i - 1][clen - 1] = '\0';
	}

	/* If the next line starts with a space, it is the start of the
	 * long description of the room, indented by a single space.
	 */

	ichar = getchar();
	(void)ungetc(ichar, stdin);
	if (ichar == ' ')
		_azodesc1[i - 1] = _zreadmsg();

	_zobjectdesco = NULL;
	_zobjectread = NULL;

	_ureadasciidispatchtoblank(_asobjectcmds);

	_azodesco[i - 1] = _zobjectdesco;
	_aioaction[i - 1] = _iobjectaction;
	_aioflag1[i - 1] = _iobjectflag1;
	_aioflag2[i - 1] = _iobjectflag2;
	_aiofval[i - 1] = _iobjectfval;
	_aiotval[i - 1] = _iobjecttval;
	_aiosize[i - 1] = _iobjectsize;
	_aiocapacity[i - 1] = _iobjectcapacity;
	_aioroom[i - 1] = _iobjectroom;
	_aioadventurer[i - 1] = _iobjectadventurer;
	_aiocontainer[i - 1] = _iobjectcontainer;
	_azoread[i - 1] = _zobjectread;
}

/* Get the initial description of an object */

static void _ureadasciiobjectdesco(const char *z)
{
	if (_zobjectdesco != NULL)
		_ureadasciifatal("Duplicate initial", NULL);
	_zobjectdesco = _zreadmsg();
}

/* Get the read entry of an object */

static void _ureadasciiobjectread(const char *z)
{
	if (_zobjectread != NULL)
		_ureadasciifatal("Duplicate read", NULL);
	_zobjectread = _zreadmsg();
}

/* Read double room information.  This looks like this:
 * "Double room: 63 (window) in 6 (Kitchen)"
 */

static void _ureadasciidoubleroom(const char *z)
{
	char abjunk[MAXLINE];

	if (_imaxroom2 >= ROOM2)
		_ureadasciifatal("Too many double rooms", z);

	if (sscanf(z, "%d %[^-0123456789] %d", &_aioroom2[_imaxroom2],
		   abjunk, &_airroom2[_imaxroom2]) != 3)
		_ureadasciifatal("Bad double room", z);

	_imaxroom2++;
}

/* Read clock information.  This looks like this:
 * "Clock: 1: off: Ticks: 0 Action: 1"
 */

static void _ureadasciiclock(const char *z)
{
	char abon[MAXLINE];
	int i, itick, iaction;

	if (sscanf(z, "%d: %[^:]: Ticks: %d Action: %d", &i, abon, &itick,
		   &iaction) != 4)
		_ureadasciifatal("Bad clock line", z);

	if (i < 1  ||  i > CLOCKS)
		_ureadasciifatal("Bad clock line", z);
	if (i > _imaxclock)
		_imaxclock = i;
	if (strcmp(abon, "on") != 0  &&
	    strcmp(abon, "off") != 0)
		_ureadasciifatal("Bad clock line", z);
	if (_afcflag[i - 1]  ||
	    _aictick[i - 1] != 0  ||
	    _aicaction[i - 1] != 0)
		_ureadasciifatal("Duplicate clock", z);

	_afcflag[i - 1] = strcmp(abon, "on") == 0;
	_aictick[i - 1] = itick;
	_aicaction[i - 1] = iaction;
}

/* Read villian information */

static int _ivillprob, _ivillopps, _ivillbest, _ivillmelee;

static struct scmd _asvillcmds[] =
	{ { "Probability", NULL, &_ivillprob },
	  { "Opponent", NULL, &_ivillopps },
	  { "Best weapon", NULL, &_ivillbest },
	  { "Melee remark", NULL, &_ivillmelee },
	  { NULL, NULL, NULL } };

static void _ureadasciivillian(const char *z)
{
	int i, iobj;

	/* The first line looks like this:
	 * "Villian: 1: 19 (troll)"
	 */

	if (sscanf(z, "%d: %d", &i, &iobj) != 2)
		_ureadasciifatal("Bad villian line", z);
	if (i < 1  ||  i > VILLS)
		_ureadasciifatal("Too many villians", z);
	if (i > _imaxvill)
		_imaxvill = i;
	if (_aivillns[i - 1] != 0)
		_ureadasciifatal("Duplicate villian", z);
	_aivillns[i - 1] = iobj;

	_ureadasciidispatchtoblank(_asvillcmds);

	_aivprob[i - 1] = _ivillprob;
	_aivopps[i - 1] = _ivillopps;
	_aivbest[i - 1] = _ivillbest;
	_aivmelee[i - 1] = _ivillmelee;
}

/* Read adventurer information */

static int _iadvroom, _iadvscore, _iadvvehicle, _iadvaction, _iadvstrength;
static int _iadvflag;

static struct scmd _asadvcmds[] =
	{ { "Room", NULL, &_iadvroom },
	  { "Score", NULL, &_iadvscore },
	  { "Vehicle", NULL, &_iadvvehicle },
	  { "Action", NULL, &_iadvaction },
	  { "Strength", NULL, &_iadvstrength },
	  { "Flag", NULL, &_iadvflag },
	  { NULL, NULL, NULL } };

static void _ureadasciiadventurer(const char *z)
{
	int i, iobj;

	/* The first line looks like this:
	 * "Adventurer: 1: 193 (cretin)
	 */

	if (sscanf(z, "%d: %d", &i, &iobj) != 2)
		_ureadasciifatal("Bad adventurer line", z);
	if (i < 1  ||  i > ADVS)
		_ureadasciifatal("Too many adventurers", z);
	if (i > _imaxadv)
		_imaxadv = i;
	if (_aiaobj[i - 1] != 0)
		_ureadasciifatal("Duplicate adventurer", z);
	_aiaobj[i - 1] = iobj;

	_ureadasciidispatchtoblank(_asadvcmds);

	_aiaroom[i - 1] = _iadvroom;
	_aiascore[i - 1] = _iadvscore;
	_aiavehicle[i - 1] = _iadvvehicle;
	_aiaaction[i - 1] = _iadvaction;
	_aiastrength[i - 1] = _iadvstrength;
	_aiaflag[i - 1] = _iadvflag;
}

/* Read message information */

static void _ureadasciimessage(const char *z)
{
	int i;

	if (sscanf(z, "%d", &i) != 1)
		_ureadasciifatal("Bad message line", z);
	if (i < 1  ||  i > MESSAGES)
		_ureadasciifatal("Too many messages", z);
	if (i > _imaxmessage)
		_imaxmessage = i;
	if (_azrtext[i - 1] != NULL)
		_ureadasciifatal("Duplicate message", z);

	_azrtext[i - 1] = _zreadmsg();
}

/**************************************************************************/

/* Write ASCII information.  This writes out a fairly simple format
 * of attribute/value pairs.  The only complexity is the exit information,
 * which is described under the ASCII read code (the format complexity
 * here is for ease of reading by cheaters).  This also appends a lot
 * of descriptive information in parentheses, which is ignored by the
 * reading code.  Thus, if you want to change some of the data, make
 * sure you change the actual numbers, rather than the descriptive
 * text.
 */

/* The HYPER factor is used to locate objects when one room stands for
 * many rooms, as in the puzzle or the prison cell.
 */

#define HYPER (500)
#define CELL (184)
#define PUZZ (190)

/* The names of the room flags */

#define RFLAGS (16)
static const char * const _azrflag[RFLAGS] =
	{ "", "", "", "", "end", "nowall", "house", "bucket", "mung",
	  "fill", "sacred", "air", "water", "land", "light", "seen" };

/* Flag names */

static const char * const _azflags[] =
	{ "troll", "cages", "bucket", "carouseloff", "carouselfast",
	  "lowtide", "dome", "glacier", "echo", "riddle", "exorcised",
	  "cyclops", "magic", "lightload", "safe", "gnome", "gnomedoor",
	  "mirrormunged", "egypt", "onpole", "blab", "brief", "superbrief",
	  "buoy", "grateunlocked", "gate", "rainbow", "cage", "emptyhanded",
	  "deflated", "glaciermelted", "frobozz", "endgame", "badluck",
	  "theifengrossed", "sings", "mirrorpush", "mirroropen", "pineopen",
	  "mirror1", "mirror2", "inquisition", "follow", "spell",
	  "puzzleout", "puzzlepushed" };

/* Object flags */

#define OFLAGS1 (16)
static const char * const _azoflag1[OFLAGS1] =
	{ "on", "turn", "tool", "flame", "burn", "victim", "light",
	  "container", "drink", "nodescription", "food", "transparent",
	  "door", "take", "read", "visible" };
#define OFLAGS2 (16)
static const char * const _azoflag2[OFLAGS2] =
	{ "search", "vehicle", "touched", "open", "", "try", "stagger",
	  "villian", "fight", "weapon", "actor", "climb", "tie",
	  "sacred", "sleep", "find" };

/* Write out a string, indenting each line a given amount.  Finish
 * with a newline.
 */

static void _uwriteasciistring(const char *z, int cindent)
{
	printf("%*s", cindent, "");

	for (;  *z != '\0';  z++) {
		putchar(*z);
		if (*z == '\n')
			printf("%*s", cindent, "");
	}

	putchar('\n');
}

/* The writing function.  This writes to stdout. */

static void _uwriteascii(void)
{
	int i;

	/* Print general information */

	printf("Version: %1d.%1d%c\n", _ivermaj, _ivermin, _iveredit);

	printf("Maximum score: %d\n", _imaxscore);
	printf("Stars: %d (first generic object)\n", _istars);
	printf("Maximum endgame score: %d\n", _imaxendscore);

	/* Output the room and exit information */

	putchar('\n');
	for (i = 0;  i < _imaxroom;  i++) {

		/* Room number and short description */

		printf("Room: %d: %s\n", i + 1, _azrdesc2[i]);

		/* Long description, indented 1 space */

		if (_azrdesc1[i] != NULL)
			_uwriteasciistring(_azrdesc1[i], 1);

		/* Random information */

		if (_airaction[i] != 0)
			printf("Action: %d\n", _airaction[i]);
		if (_airval[i] != 0)
			printf("Value: %d\n", _airval[i]);
		if (_airflag[i] != 0) {
			int iflag;
			char ffirst;

			printf("Flags: %d (", _airflag[i]);
			ffirst = TRUE;
			for (iflag = 0;  iflag < RFLAGS;  iflag++) {
				if ((_airflag[i] & (1 << iflag)) != 0) {
					if (ffirst)
						ffirst = FALSE;
					else
						putchar(' ');
					printf("%s", _azrflag[iflag]);
				}
			}
			printf(")\n");
		}

		/* Print out the exit information */

		if (_airexit[i] != 0) {
			int iindex;
			unsigned int ival;

			printf("Exits:\n");
			iindex = _airexit[i] - 1;
			do {
				unsigned int itype, iroom, idir, iact;
				const char *zstr;

				ival = _aiexit[iindex] & 0xffff;
				iroom = ival & 0xff;
				itype = ((ival & 0x7fff) >> 8) & 3;
				idir = ((ival & 0x7c00) >> 10) - 1;
				if (idir < 0  ||  idir >= DIRS)
					_ufatal("_uwriteascii: Bad direction");
				printf(" %s: %d %d", _azdirs[idir], itype,
				       iroom);
				if (iroom != 0)
					printf(" (%s)",
					       _azrdesc2[iroom - 1]);
				putchar('\n');

				switch (itype) {
				case 3:
					iact = _aiexit[iindex + 2] & 0xffff;
					if (iact >> 8 != 0)
						printf("  Action: %d\n",
						       iact >> 8);
					iact &= 0xff;
					printf("  Door: %d (%s)\n", iact,
					       _azodesc2[iact - 1]);
					zstr = _azexitstring[iindex + 1];
					if (zstr != NULL)
						printf("  String: %s\n",
						       zstr);
					iindex += 3;
					break;
				case 2:
					iact = _aiexit[iindex + 2] & 0xffff;
					if (iact >> 8 != 0)
						printf("  Action: %d\n",
						       iact >> 8);
					iact &= 0xff;
					printf("  Flag: %d (%s)\n", iact,
					       _azflags[iact - 1]);
					zstr = _azexitstring[iindex + 1];
					if (zstr != NULL)
						printf("  String: %s\n",
						       zstr);
					iindex += 3;
					break;
				case 1:
					zstr = _azexitstring[iindex + 1];
					if (zstr != NULL)
						printf("  String: %s\n",
						       zstr);
					iindex += 2;
					break;
				case 0:
					iindex += 1;
					break;
				}
			} while ((ival & 0x8000) == 0);
		}

		putchar('\n');
	}

	/* Output the object information */

	for (i = 0;  i < _imaxobject;  i++) {

		/* Object number and short description */

		printf("Object: %d", i + 1);
		if (_azodesc2[i] != NULL)
			printf(": %s", _azodesc2[i]);
		putchar('\n');

		/* Long description, indented one space */

		if (_azodesc1[i] != NULL)
			_uwriteasciistring(_azodesc1[i], 1);

		/* Check for initial description */

		if (_azodesco[i] != NULL) {
			printf("Initial:\n");
			_uwriteasciistring(_azodesco[i], 1);
		}

		/* General information */

		if (_aioaction[i] != 0)
			printf("Action: %d\n", _aioaction[i]);
		if (_aioflag1[i] != 0) {
			int iflag;
			char ffirst;

			printf("Flag1: %d (", _aioflag1[i]);
			ffirst = TRUE;
			for (iflag = 0;  iflag < OFLAGS1;  iflag++) {
				if ((_aioflag1[i] & (1 << iflag)) != 0) {
					if (ffirst)
						ffirst = FALSE;
					else
						putchar(' ');
					printf("%s", _azoflag1[iflag]);
				}
			}
			printf(")\n");
		}
		if (_aioflag2[i] != 0) {
			int iflag;
			char ffirst;

			printf("Flag2: %d (", _aioflag2[i]);
			ffirst = TRUE;
			for (iflag = 0;  iflag < OFLAGS2;  iflag++) {
				if ((_aioflag2[i] & (1 << iflag)) != 0) {
					if (ffirst)
						ffirst = FALSE;
					else
						putchar(' ');
					printf("%s", _azoflag2[iflag]);
				}
			}
			printf(")\n");
		}
		if (_aiofval[i] != 0)
			printf("Find value: %d\n", _aiofval[i]);
		if (_aiotval[i] != 0)
			printf("Treasure value: %d\n", _aiotval[i]);
		if (_aiosize[i] != 0)
			printf("Size: %d\n", _aiosize[i]);
		if (_aiocapacity[i] != 0)
			printf("Capacity: %d\n", _aiocapacity[i]);
		if (_aioroom[i] != 0) {
			int iroom;

			iroom = _aioroom[i];
			printf("Room: %d (", iroom);
			if (iroom < HYPER)
				printf("%s", _azrdesc2[iroom - 1]);
			else if (iroom < 10 * HYPER) {
				printf("%s %d", _azrdesc2[CELL - 1],
				       iroom / HYPER);
			}
			else {
				iroom /= HYPER;
				iroom -= 10;
				printf("%s %d,%d", _azrdesc2[PUZZ - 1],
				       (iroom / 8) + 1, (iroom % 8) + 1);
			}
			printf(")\n");
		}
		if (_aioadventurer[i] != 0)
			printf("Adventurer: %d\n", _aioadventurer[i]);
		if (_aiocontainer[i] != 0)
			printf("Container: %d (%s)\n", _aiocontainer[i],
			       _azodesc2[_aiocontainer[i] - 1]);
		if (_azoread[i] != NULL) {
			printf("Read:\n");
			_uwriteasciistring(_azoread[i], 1);
		}

		putchar('\n');
	}

	/* Output the double room information */

	for (i = 0;  i < _imaxroom2;  i++) {
		printf("Double room: %d", _aioroom2[i]);
		if (_azodesc2[_aioroom2[i] - 1] != NULL)
			printf(" (%s)", _azodesc2[_aioroom2[i] - 1]);
		printf(" in %d (%s)\n", _airroom2[i],
		       _azrdesc2[_airroom2[i] - 1]);
	}
	putchar('\n');

	/* Print the clock event information */

	for (i = 0;  i < _imaxclock;  i++)
		printf("Clock: %d: %s: Ticks: %d Action: %d\n", i + 1,
		       _afcflag[i] ? "on" : "off", _aictick[i],
		       _aicaction[i]);
	putchar('\n');

	/* Print the villian information */

	for (i = 0;  i < _imaxvill;  i++) {
		printf("Villian: %d: %d (%s)\n", i + 1, _aivillns[i],
		       _azodesc2[_aivillns[i] - 1]);
		if (_aivprob[i] != 0)
			printf("Probability: %d\n", _aivprob[i]);
		if (_aivopps[i] != 0)
			printf("Opponent: %d\n", _aivopps[i]);
		if (_aivbest[i] != 0)
			printf("Best weapon: %d (%s)\n", _aivbest[i],
			       _azodesc2[_aivbest[i] - 1]);
		printf("Melee remark: %d\n\n", _aivmelee[i]);
	}

	/* Print the adventurer information */

	for (i = 0;  i < _imaxadv;  i++) {
		printf("Adventurer: %d", i + 1);
		if (_aiaobj[i] != 0) {
			printf(": %d", _aiaobj[i]);
			if (_azodesc2[_aiaobj[i] - 1] != NULL)
				printf(" (%s)", _azodesc2[_aiaobj[i] - 1]);
		}
		putchar('\n');

		if (_aiaroom[i] != 0)
			printf("Room: %d (%s)\n", _aiaroom[i],
			       _azrdesc2[_aiaroom[i] - 1]);
		if (_aiascore[i] != 0)
			printf("Score: %d\n", _aiascore[i]);
		if (_aiavehicle[i] != 0)
			printf("Vehicle: %d\n", _aiavehicle[i]);
		if (_aiaaction[i] != 0)
			printf("Action: %d\n", _aiaaction[i]);
		if (_aiastrength[i] != 0)
			printf("Strength: %d\n", _aiastrength[i]);
		if (_aiaflag[i] != 0) {
			printf("Flag: %d", _aiaflag[i]);
			if ((_aiaflag[i] & 32768) != 0)
				printf(" (stagger)");
			putchar('\n');
		}

		putchar('\n');
	}

	printf("Melee message base: %d\n\n", _imbase);

	for (i = 0;  i < _imaxmessage;  i++) {
		printf("Message: %d\n", i + 1);
		_uwriteasciistring(_azrtext[i], 1);
	}
}

/**************************************************************************/

/* Read the new binary format */

/* Read an integer from the index file */

__inline__
static int _ireadnewint(FILE *eind)
{
	int i;

	i = getc(eind);
	return (i > 127 ? i - 256 : i) * 256 + getc(eind);
}

/* Read an array of integers from the index file */

static void _ureadnewints(int c, int *pi, FILE *eind)
{
	while (c-- != 0)
		*pi++ = _ireadnewint(eind);
}

/* Read a partial array of integers from the index file.  These are stored
 * as index,value pairs.
 */

static void _ureadnewpartialints(int c, int *pi, FILE *eind)
{
	int i;

	memset(pi, 0, c * sizeof(int));

	while (TRUE) {
		if (c < 255) {
			i = getc(eind);
			if (i == 255)
				return;
		}
		else {
			i = _ireadnewint(eind);
			if (i == -1)
				return;
		}

		if (i >= c)
			_ufatal("_ureadnewpartialints: Bad index");

		pi[i] = _ireadnewint(eind);
	}
}

/* Read an array of flags from the index file */

static void _ureadnewflags(int c, char *pf, FILE *eind)
{
	while (c-- != 0)
		*pf++ = getc(eind);
}

/* Read a string from the text file given an index */

#define STRINGKEY "IanLanceTaylorJr";
#define BLOCKREAD (1024)

static char *_zreadnewstring(int i, FILE *etxt, long ibase)
{
	int ipos, clen, cnew;
	char *zret;
	const char *zkey = STRINGKEY;

	if (i == 0)
		return NULL;

	if (i > 0)
		_ufatal("_zreadnewstring: Positive index");

	ipos = ((- i) - 1) * 8;

	if (fseek(etxt, ipos + ibase, SEEK_SET) == EOF)
		_ufatal("_zreadnewstring: Can't fseek");

	zret = NULL;
	clen = 0;

	do {
		char ab[BLOCKREAD + 1];
		int c;

		c = fread(ab, 1, BLOCKREAD, etxt);
		if (c <= 0)
			_ufatal("_zreadnewstring: Can't fread");
		ab[c] = '\0';

		for (i = 0;  i < c;  i++) {
			ab[i] = ab[i] ^ (zkey[ipos & 0xf] ^ ipos);
			ipos++;
			if (ab[i] == '\0')
				break;
		}

		cnew = strlen(ab);
		if ((zret = realloc(zret, clen + cnew + 1)) == NULL)
			_ufatal("_zreadnewstring: Insufficient memory");
		strcpy(zret + clen, ab);
		clen += cnew;

	} while (cnew == BLOCKREAD);

	return zret;
}

/* Read the new binary format.  This is actually very similar to reading
 * the old binary format, and there probably should not be two different
 * functions at all.
 */

static void _ureadnewbinary(FILE *etxt)
{
	register int i;
	int airdesc1[ROOMS], airdesc2[ROOMS];
	int aiodesc1[OBJECTS], aiodesc2[OBJECTS], aiodesco[OBJECTS];
	int aioread[OBJECTS];
	int airtext[MESSAGES];
	long ibase;

	/* Read general information */

	_ivermaj = _ireadnewint(etxt);
	_ivermin = _ireadnewint(etxt);
	_iveredit = _ireadnewint(etxt);

	_imaxscore = _ireadnewint(etxt);
	_istars = _ireadnewint(etxt);
	_imaxendscore = _ireadnewint(etxt);

	/* Read the room information */

	_imaxroom = _ireadnewint(etxt);
	_ureadnewints(_imaxroom, airdesc1, etxt);
	_ureadnewints(_imaxroom, airdesc2, etxt);
	_ureadnewints(_imaxroom, _airexit, etxt);
	_ureadnewpartialints(_imaxroom, _airaction, etxt);
	_ureadnewpartialints(_imaxroom, _airval, etxt);
	_ureadnewints(_imaxroom, _airflag, etxt);

	/* Read the exit information */

	_imaxexit = _ireadnewint(etxt);
	_ureadnewints(_imaxexit, _aiexit, etxt);

	/* Read all the object information */

	_imaxobject = _ireadnewint(etxt);
	_ureadnewints(_imaxobject, aiodesc1, etxt);
	_ureadnewints(_imaxobject, aiodesc2, etxt);
	_ureadnewpartialints(_imaxobject, aiodesco, etxt);
	_ureadnewpartialints(_imaxobject, _aioaction, etxt);
	_ureadnewints(_imaxobject, _aioflag1, etxt);
	_ureadnewpartialints(_imaxobject, _aioflag2, etxt);
	_ureadnewpartialints(_imaxobject, _aiofval, etxt);
	_ureadnewpartialints(_imaxobject, _aiotval, etxt);
	_ureadnewints(_imaxobject, _aiosize, etxt);
	_ureadnewpartialints(_imaxobject, _aiocapacity, etxt);
	_ureadnewints(_imaxobject, _aioroom, etxt);
	_ureadnewpartialints(_imaxobject, _aioadventurer, etxt);
	_ureadnewpartialints(_imaxobject, _aiocontainer, etxt);
	_ureadnewpartialints(_imaxobject, aioread, etxt);

	/* Read the double room information */

	_imaxroom2 = _ireadnewint(etxt);
	_ureadnewints(_imaxroom2, _aioroom2, etxt);
	_ureadnewints(_imaxroom2, _airroom2, etxt);

	/* Read the clock event information */

	_imaxclock = _ireadnewint(etxt);
	_ureadnewints(_imaxclock, _aictick, etxt);
	_ureadnewints(_imaxclock, _aicaction, etxt);
	_ureadnewflags(_imaxclock, _afcflag, etxt);

	/* Read the villian information */

	_imaxvill = _ireadnewint(etxt);
	_ureadnewints(_imaxvill, _aivillns, etxt);
	_ureadnewpartialints(_imaxvill, _aivprob, etxt);
	_ureadnewpartialints(_imaxvill, _aivopps, etxt);
	_ureadnewints(_imaxvill, _aivbest, etxt);
	_ureadnewints(_imaxvill, _aivmelee, etxt);

	/* Read the adventurer information */

	_imaxadv = _ireadnewint(etxt);
	_ureadnewints(_imaxadv, _aiaroom, etxt);
	_ureadnewpartialints(_imaxadv, _aiascore, etxt);
	_ureadnewpartialints(_imaxadv, _aiavehicle, etxt);
	_ureadnewints(_imaxadv, _aiaobj, etxt);
	_ureadnewints(_imaxadv, _aiaaction, etxt);
	_ureadnewints(_imaxadv, _aiastrength, etxt);
	_ureadnewpartialints(_imaxadv, _aiaflag, etxt);

	/* Read the melee message base */

	_imbase = _ireadnewint(etxt);

	/* Read the messages (it turns out that the _imaxmessage value
	 * stored in the file is not useful, so we correct it here).
	 */

	_imaxmessage = _ireadnewint(etxt);
	_ureadnewints(_imaxmessage, airtext, etxt);

	/* This point in the file is the base of the text strings */

	ibase = ftell(etxt);

	/* Turn the room text file indices into strings */

	for (i = 0;  i < _imaxroom;  i++) {
		_azrdesc1[i] = _zreadnewstring(airdesc1[i], etxt, ibase);
		_azrdesc2[i] = _zreadnewstring(airdesc2[i], etxt, ibase);
	}

	/* We now must read in all the exit strings, which is unfortunately
	 * complex because of the format of the exit array.
	 */

	for (i = 0;  i < _imaxroom;  i++) {
		int iind;
		unsigned int ival;

		iind = _airexit[i];
		if (iind == 0)
			continue;
		iind--;

		do {
			int itype;

			ival = _aiexit[iind];
			itype = (ival >> 8) & 3;
			if (itype != 0)
				_azexitstring[iind + 1] =
					_zreadnewstring(_aiexit[iind + 1],
							etxt, ibase);
			switch (itype) {
			case 0:
				iind += 1;
				break;
			case 1:
				iind += 2;
				break;
			case 2:
			case 3:
				iind += 3;
				break;
			}
		} while ((ival & 0x8000) == 0);
	}

	/* Turn the object strings from indices into strings */

	for (i = 0;  i < _imaxobject;  i++) {
		_azodesc1[i] = _zreadnewstring(aiodesc1[i], etxt, ibase);
		_azodesc2[i] = _zreadnewstring(aiodesc2[i], etxt, ibase);
		_azodesco[i] = _zreadnewstring(aiodesco[i], etxt, ibase);
		_azoread[i] = _zreadnewstring(aioread[i], etxt, ibase);
	}

	/* Convert the messages into strings */

	for (i = 0;  i < _imaxmessage;  i++)
		_azrtext[i] = _zreadnewstring(airtext[i], etxt, ibase);
}

/**************************************************************************/

/* Write the new binary format */

/* Write an integer out to the index file.  We write integers out as
 * two bytes, MSB then LSB.
 */

static void _uwritenewint(int i, FILE *etxt)
{
	if (i < 0)
		i += 65536;

	putc(i >> 8, etxt);
	putc(i & 0xff, etxt);
}

/* Write out an array of integers to the index file */

static void _uwritenewints(int c, int *pi, FILE *etxt)
{
	while (c-- != 0)
		_uwritenewint(*pi++, etxt);
}

/* Write out a partial array of integers to the index file; this writes
 * out index,value pairs, ending with a 255 (or a -1).
 */

static void _uwritenewpartialints(int c, int *pi, FILE *etxt)
{
	register int i;

	for (i = 0;  i < c;  i++) {
		if (pi[i] != 0) {
			if (c < 255)
				putc(i, etxt);
			else
				_uwritenewint(i, etxt);
			_uwritenewint(pi[i], etxt);
		}
	}

	if (c < 255)
		putc(255, etxt);
	else
		_uwritenewint(-1, etxt);
}

#ifdef __AMOS__

/* Count the number of bytes that will be required when a partial array
 * is written out by _uwritenewpartialints.
 */

static int _cpartial(int c, int *pi)
{
	register int i;
	int cret;

	cret = 0;
	for (i = 0;  i < c;  i++) {
		if (pi[i] != 0) {
			if (c < 255)
				cret += 1;
			else
				cret += 2;
			cret += 2;
		}
	}

	if (c < 255)
		cret += 1;
	else
		cret += 2;

	return cret;
}

#endif /* __AMOS__ */

/* Write out an array of flags to the index file */

static void _uwritenewflags(int c, char *pf, FILE *etxt)
{
	while (c-- != 0)
		putc(*pf++, etxt);
}

/* We build a random file to hold text just as before.  However, rather
 * than requiring 76 bytes for each line of output, we store the
 * strings as a stream.  We pad each string to a multiple of eight bytes.
 * When dungeon wants to print a message, if the number is positive we
 * look it up in the rtext array to get a negative number.  Given
 * i < 0, go to byte ((- i) - 1) * 8 of the file.  The string starts
 * there and continues until it reaches a null character.  This allows
 * us a total of 32767 * 8 == 262136 bytes of text information, which
 * which is much more than enough (note that the indices are two byte
 * numbers).  In fact, it's barely more than twice enough, but we don't
 * pad to a multiple of four because that would screw anybody who tried
 * to add more than a few K of characters (such as a translation to
 * Japanese).
 * 
 * The strings are also encrypted very simply using my name (plus a
 * couple of characters) and the file position as a key.  This is just
 * to prevent simplistic cheating while leaving the database world
 * readable.
 *
 * We use a hash table when creating the strings to avoid creating
 * any duplicates.  This requires us to maintain indices for the short
 * room descriptions; the old format stored different strings for each
 * short room description, although many of them were the same.
 */

#define HASHSIZE (257)

struct shash {
	struct shash *qnexthash;
	struct shash *qnextstring;
	const char *z;
	int iindex;
};

static struct shash *_qwnstrings;
static struct shash *_qwnlast;
static int _iwnstrindex;

static int _iwritenewstring(const char *z)
{
	static struct shash *_aqhash[HASHSIZE];
	unsigned short ihash;
	const char *zhash;
	struct shash **pq;

	ihash = 0;
	for (zhash = z;  *zhash != '\0';  zhash++)
		ihash = ihash << 2 + *zhash;

	for (pq = &_aqhash[ihash % HASHSIZE];
	     *pq != NULL;
	     pq = &(*pq)->qnexthash)
		if (strcmp((*pq)->z, z) == 0)
			return (*pq)->iindex;

	if ((*pq = malloc(sizeof(struct shash))) == NULL)
		_ufatal("_iwritenewstring: Insufficient memory");

	(*pq)->qnexthash = NULL;
	(*pq)->qnextstring = NULL;
	(*pq)->z = z;
	(*pq)->iindex = - ((_iwnstrindex / 8) + 1);

	if (_qwnlast == NULL)
		_qwnstrings = *pq;
	else
		_qwnlast->qnextstring = *pq;

	_qwnlast = *pq;

	_iwnstrindex += strlen(z) + 1;
	_iwnstrindex += 7;
	_iwnstrindex -= _iwnstrindex % 8;

	return (*pq)->iindex;
}

/* Actually write out all the new strings from the hash table (we do
 * this at the end so that we know the total size of the file to create
 * on AMOS).
 */

static void _uwritenewstrings(FILE *etxt)
{
	struct shash *q;
	long ibase;
	int ipos;
	const char *zkey = STRINGKEY;

	ibase = ftell(etxt);

	ipos = 0;
	for (q = _qwnstrings;  q != NULL;  q = q->qnextstring) {
		const char *z;

		if (ftell(etxt) - ibase != ((- q->iindex) - 1) * 8)
			_ufatal("_uwritenewstrings: Bad file position");

		for (z = q->z;  *z != '\0';  z++) {
			putc(*z ^ (zkey[ipos & 0xf] ^ ipos), etxt);
			ipos++;
		}
		putc(zkey[ipos & 0xf] ^ ipos, etxt);
		ipos++;

		while ((ipos % 8) != 0) {
			putc(ipos, etxt);
			ipos++;
		}
	}

	if (ftell(etxt) - ibase != ipos)
		_ufatal("_uwritenewstrings: Bad end file position");
}

/* Write out the data in the new binary format */

static void _uwritenewbinary(const char *ztxt, char frandom)
{
	register int i;
	FILE *etxt;
	int airtext[MESSAGES];
	int airdesc1[ROOMS], airdesc2[ROOMS];
	int aiodesc1[OBJECTS], aiodesc2[OBJECTS], aiodesco[OBJECTS];
	int aioread[OBJECTS];
#ifdef __AMOS__
	int c;
#endif

	/* Clear the message text arrays */

	memset(airtext, 0, sizeof airtext);
	memset(airdesc1, 0, sizeof airdesc1);
	memset(airdesc2, 0, sizeof airdesc2);
	memset(aiodesc1, 0, sizeof aiodesc1);
	memset(aiodesc2, 0, sizeof aiodesc2);
	memset(aiodesco, 0, sizeof aiodesco);
	memset(aioread, 0, sizeof aioread);

	/* Determine the message numbers */

	for (i = 0;  i < _imaxmessage;  i++)
		airtext[i] = _iwritenewstring(_azrtext[i]);

	for (i = 0;  i < _imaxroom;  i++) {
		if (_azrdesc1[i] != NULL)
			airdesc1[i] = _iwritenewstring(_azrdesc1[i]);
		if (_azrdesc2[i] != NULL)
			airdesc2[i] = _iwritenewstring(_azrdesc2[i]);
	}

	for (i = 0;  i < _imaxobject;  i++) {
		if (_azodesc1[i] != NULL)
			aiodesc1[i] = _iwritenewstring(_azodesc1[i]);
		if (_azodesc2[i] != NULL)
			aiodesc2[i] = _iwritenewstring(_azodesc2[i]);
		if (_azodesco[i] != NULL)
			aiodesco[i] = _iwritenewstring(_azodesco[i]);
		if (_azoread[i] != NULL)
			aioread[i] = _iwritenewstring(_azoread[i]);
	}

	for (i = 0;  i < _imaxexit;  i++) {
		if (_azexitstring[i] != NULL)
			_aiexit[i] = _iwritenewstring(_azexitstring[i]);
	}

#ifdef __AMOS__
	/* On AMOS, determine how much room the general information will
	 * require so that we know the size to create the random file.
	 */

	if (! frandom)
		etxt = fopen(ztxt, "wb");
	else {
		c = 2 * 7;
		c += 2 * (_imaxroom * 4);
		c += _cpartial(_imaxroom, _airaction);
		c += _cpartial(_imaxroom, _airval);
		c += 2;
		c += 2 * _imaxexit;
		c += 2;
		c += 2 * (_imaxobject *  5);
		c += _cpartial(_imaxobject, aiodesco);
		c += _cpartial(_imaxobject, _aioaction);
		c += _cpartial(_imaxobject, _aioflag2);
		c += _cpartial(_imaxobject, _aiofval);
		c += _cpartial(_imaxobject, _aiotval);
		c += _cpartial(_imaxobject, _aiocapacity);
		c += _cpartial(_imaxobject, _aioadventurer);
		c += _cpartial(_imaxobject, _aiocontainer);
		c += _cpartial(_imaxobject, aioread);
		c += 2;
		c += 2 * (_imaxroom2 * 2);
		c += 2;
		c += 2 * (_imaxclock * 2);
		c += _imaxclock;
		c += 2;
		c += 2 * (_imaxvill * 3);
		c += _cpartial(_imaxvill, _aivprob);
		c += _cpartial(_imaxvill, _aivopps);
		c += 2;
		c += 2 * (_imaxadv * 4);
		c += _cpartial(_imaxadv, _aiascore);
		c += _cpartial(_imaxadv, _aiavehicle);
		c += _cpartial(_imaxadv, _aiaflag);
		c += 2;
		c += 2;
		c += 2 * _imaxmessage;

		c += _iwnstrindex;

		etxt = fdopen(rcreat(ztxt, (c + 511) / 512, 0), "wb");
	}

#else /* ! __AMOS__ */

	etxt = fopen(ztxt, "wb");

#endif /* ! __AMOS__ */

	if (etxt == NULL) {
		perror(ztxt);
		_ufatal(NULL);
	}

	/* Write general information */

	_uwritenewint(_ivermaj, etxt);
	_uwritenewint(_ivermin, etxt);
	_uwritenewint(_iveredit, etxt);

	_uwritenewint(_imaxscore, etxt);
	_uwritenewint(_istars, etxt);
	_uwritenewint(_imaxendscore, etxt);

	/* Write the room information */

	_uwritenewint(_imaxroom, etxt);
	_uwritenewints(_imaxroom, airdesc1, etxt);
	_uwritenewints(_imaxroom, airdesc2, etxt);
	_uwritenewints(_imaxroom, _airexit, etxt);
	_uwritenewpartialints(_imaxroom, _airaction, etxt);
	_uwritenewpartialints(_imaxroom, _airval, etxt);
	_uwritenewints(_imaxroom, _airflag, etxt);

	/* Write the exit information */

	_uwritenewint(_imaxexit, etxt);
	_uwritenewints(_imaxexit, _aiexit, etxt);

	/* Write all the object information */

	_uwritenewint(_imaxobject, etxt);
	_uwritenewints(_imaxobject, aiodesc1, etxt);
	_uwritenewints(_imaxobject, aiodesc2, etxt);
	_uwritenewpartialints(_imaxobject, aiodesco, etxt);
	_uwritenewpartialints(_imaxobject, _aioaction, etxt);
	_uwritenewints(_imaxobject, _aioflag1, etxt);
	_uwritenewpartialints(_imaxobject, _aioflag2, etxt);
	_uwritenewpartialints(_imaxobject, _aiofval, etxt);
	_uwritenewpartialints(_imaxobject, _aiotval, etxt);
	_uwritenewints(_imaxobject, _aiosize, etxt);
	_uwritenewpartialints(_imaxobject, _aiocapacity, etxt);
	_uwritenewints(_imaxobject, _aioroom, etxt);
	_uwritenewpartialints(_imaxobject, _aioadventurer, etxt);
	_uwritenewpartialints(_imaxobject, _aiocontainer, etxt);
	_uwritenewpartialints(_imaxobject, aioread, etxt);

	/* Write the double room information */

	_uwritenewint(_imaxroom2, etxt);
	_uwritenewints(_imaxroom2, _aioroom2, etxt);
	_uwritenewints(_imaxroom2, _airroom2, etxt);

	/* Write the clock event information */

	_uwritenewint(_imaxclock, etxt);
	_uwritenewints(_imaxclock, _aictick, etxt);
	_uwritenewints(_imaxclock, _aicaction, etxt);
	_uwritenewflags(_imaxclock, _afcflag, etxt);

	/* Write the villian information */

	_uwritenewint(_imaxvill, etxt);
	_uwritenewints(_imaxvill, _aivillns, etxt);
	_uwritenewpartialints(_imaxvill, _aivprob, etxt);
	_uwritenewpartialints(_imaxvill, _aivopps, etxt);
	_uwritenewints(_imaxvill, _aivbest, etxt);
	_uwritenewints(_imaxvill, _aivmelee, etxt);

	/* Write the adventurer information */

	_uwritenewint(_imaxadv, etxt);
	_uwritenewints(_imaxadv, _aiaroom, etxt);
	_uwritenewpartialints(_imaxadv, _aiascore, etxt);
	_uwritenewpartialints(_imaxadv, _aiavehicle, etxt);
	_uwritenewints(_imaxadv, _aiaobj, etxt);
	_uwritenewints(_imaxadv, _aiaaction, etxt);
	_uwritenewints(_imaxadv, _aiastrength, etxt);
	_uwritenewpartialints(_imaxadv, _aiaflag, etxt);

	/* Write the melee message base */

	_uwritenewint(_imbase, etxt);

	/* Write out the messages */

	_uwritenewint(_imaxmessage, etxt);
	_uwritenewints(_imaxmessage, airtext, etxt);

	/* Write out all the text strings */

	_uwritenewstrings(etxt);

	/* That's all */

	if (fclose(etxt) == EOF)
		_ufatal("_uwriteoldbinary: Can't close index file");
}

