/*
400px 	    Maximale Kantenlänge - original
350x250px   clipped		 - big
100x75px    clipped		 - list
90x90px     clipped		 - thumbnail

compile examle: gcc -lssl -o imagestore  imagestore.c 

omaxsize=60000;
bmaxsize=50000;
lmaxsize=10000;
tmaxsize=10000;
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>

void print_html_header(char *titel)
{
   printf("<html><head>\n");
   printf("<title>%s</title>\n",titel);
   printf("</head><body><pre>\n");
}

void print_html_end()
{
   printf("</pre></body></html>\n");
}

void print_header()
{
   printf("Content-Type: text/html\n\n");
}

void print_jpg_header()
{
   printf("Content-Type: image/jpeg\n\n");
}

void print_error(char *reason)
{
   print_header();
   print_html_header("Imagestore\n");

   printf ("reason:%s\n", reason);

   print_html_end();
}

int convert_image(char source[68], int dlresb, int drresb, char res[10], int quali, char destination[68])
{
   char convert[256];
   char jhead[256];
   int destsize;
   FILE *fpdestination;

   	snprintf ( convert, 256, "convert %s -thumbnail %ix -resize 'x%i<' -resize 50%% -gravity center -crop %s+0+0 +repage -format jpg -quality %i %s", source, dlresb, drresb, res, quali, destination );
   	system ( convert );

   	if((fpdestination = fopen(destination, "r"))==NULL) {
         	print_error("Can not read target2fn\n");
         	exit(1);
   	}

   	fseek(fpdestination, 0L, SEEK_END);
   	destsize=ftell(fpdestination);

   	fclose(fpdestination);

   return destsize;
}

void add_image(char source[68], int maxsize, int start, FILE *fppooltmp)
{
   FILE *fpsource;
   char *buffer;
   int end, c, diff;

        if((fpsource = fopen(source, "r"))==NULL) {
                exit(1);
        }

        fseek(fpsource, 0L, SEEK_END);
        end = ftell(fpsource);

   	diff = maxsize - end;

        rewind(fpsource);

	fseek(fppooltmp, start, SEEK_SET);

        buffer = malloc(sizeof(char) * end);

        fread(buffer, sizeof(char), end, fpsource);
        fwrite(buffer, sizeof(char), end, fppooltmp);

        fflush(fppooltmp);
        free(buffer);
        buffer = NULL;

        c = 0;
        while ( diff > 0 ) {
                putc(c,fppooltmp);
                diff = diff-1;
        }

   fclose ( fpsource ); 
}

void show_image(int start, int end, FILE *fppool, int noimg, int maxpoolsize)
{
   int diff, fsizeppool;
   char *buffer;

   diff = end - start;
 
   fseek(fppool, 0L, SEEK_END);
   fsizeppool=ftell(fppool);

   /* test if file is smaller 130000 byte */

   if ( noimg = 0 ) {
        while ( fsizeppool < maxpoolsize ) {
        	sleep(10);
        }
   }

   	fseek(fppool, start, SEEK_SET);

   	print_jpg_header();

   	buffer = malloc(sizeof(char) * diff);

   	fread(buffer, sizeof(char), diff, fppool);
   	fwrite(buffer, sizeof(char), diff, stdout);

   	fflush(fppool);
   	free(buffer);
   	buffer = NULL;

   	fclose ( fppool );

   exit (0);
}


int main()
{
   FILE *fppool;
   FILE *fppooltmp;
   FILE *fppooltest;
   FILE *fpbaseimg;
   FILE *fppathtofbimgo;
   FILE *fppathtofbimgb;
   FILE *fppathtofbimgl;
   FILE *fppathtofbimgt;
   FILE *fptarget1;
   FILE *fptarget2;
   FILE *fptarget3;
   FILE *fptarget4;

   char baseimg[40] = "/srv/imagestore/tmp/base-";
   char target1[40] = "/srv/imagestore/tmp/original-";
   char target2[32] = "/srv/imagestore/tmp/big-";
   char target3[32] = "/srv/imagestore/tmp/list-";
   char target4[40] = "/srv/imagestore/tmp/thumbnail-";
   char baseimgfn[76];
   char target1fn[76];
   char target2fn[76];
   char target3fn[76];
   char target4fn[76];
   char rmbaseimg[84];
   char rmtarget1[84];
   char rmtarget2[84];
   char rmtarget3[84];
   char rmtarget4[84];
   char mvtmptoreal[256];
   char basepath[17] = "/srv/imagestore";
   char pathtofbimgo[48] = "/srv/imagestore/fallbackimages/original";
   char pathtofbimgb[48] = "/srv/imagestore/fallbackimages/big";
   char pathtofbimgl[48] = "/srv/imagestore/fallbackimages/list";
   char pathtofbimgt[48] = "/srv/imagestore/fallbackimages/thumbnail";
   char ppooldir[14] = "picturepools";
   char c;
   char res1[10]  = "400x400";
   char res2[10]  = "350x250";
   char res3[10]  = "100x75";
   char res4[10]  = "90x90";
   char filename[128] = "";
   char dir[40] = "";
   char mkdir[48];
   char wget[256];
   char convert1[256];
   char jhead[256];
   char pathtmp[256]; 
   char pathnew[256] = "";
   char path[256] = ""; 
   char ppoolpath[72] = ""; 
   char ppoolpathtmp[80] = ""; 
   char pathhash[33]; 
   char *pathhasht = NULL; 
   char *pathinfo;
   char *type;
   char *next;
   char *nb;
   char *buffer;
   unsigned char *hash = NULL;
   const char delimiter[] = "/";

   int lreso, rreso, dlreso, drreso, dlresb, drresb, dlresl, drresl, dlrest, drrest, i, p, start, end, diff, quali, fsize, fsizebi, fsizemax;

   int omaxsize=60000;
   int ostart=0;
   int oend=omaxsize-1;

   int bmaxsize=50000;
   int bstart=oend+1;
   int bend=bstart+bmaxsize-1;

   int lmaxsize=10000;
   int lstart=bend+1;
   int lend=lstart+lmaxsize-1;

   int tmaxsize=10000;
   int tstart=lend+1;
   int tend=tstart+tmaxsize-1;

   int maxpoolsize=omaxsize+bmaxsize+lmaxsize+tmaxsize-1;

   long lSize;

   pathinfo = getenv("PATH_INFO");

   lreso = 400;
   rreso = 400;
   dlreso = 400*2;
   drreso = 400*2;

   dlresb = 350*2;
   drresb = 250*2;

   dlresl = 100*2;
   drresl = 75*2;

   dlrest = 90*2;
   drrest = 90*2;

   /* using URLs pathinfo to generate the hex directory structure and additional file and path targets */

   snprintf (pathtmp, 256,"%s",pathinfo);

   type = strtok (pathinfo, delimiter);

   next = strtok(pathtmp, "/");
   next = strtok(NULL, "/");

   while(next != NULL){
    	strcat(pathnew, "/");
    	strcat(pathnew, next);
    	snprintf (path, 256,"%s",pathnew);
    	next = strtok(NULL, "/");
   }

   hash = MD5(path, strlen(path), hash);
   for (i=0;i<MD5_DIGEST_LENGTH;i++)
       	snprintf(pathhash+i*2, 33, "%02x",hash[i]);

   pathhash[32] = 0;

   i=3;
   p=0;

   while (i <= 15)
   {
   	snprintf(filename+p*2, 128, "%02x",hash[i]);
   	i++;
   	p++;
   }

   snprintf ( dir, 40, "/srv/imagestore/picturepools/%02x/%02x/%02x/", hash[0], hash[1], hash[2] );
   snprintf ( ppoolpath, 72, "%s%s", dir, filename );
   snprintf ( ppoolpathtmp, 80, "%s%s-%d", dir, filename, getpid());

   snprintf ( baseimgfn, 76, "%s%s-%d.image", baseimg, filename, getpid());
   snprintf ( target1fn, 76, "%s%s-%d.jpg", target1, filename, getpid());
   snprintf ( target2fn, 76, "%s%s-%d.jpg", target2, filename, getpid());
   snprintf ( target3fn, 76, "%s%s-%d.jpg", target3, filename, getpid());
   snprintf ( target4fn, 76, "%s%s-%d.jpg", target4, filename, getpid());

   if((fppooltest = fopen(ppoolpath, "r"))==NULL) {

	/* it fails so download the image from URL */
		
        snprintf ( wget, 256, "wget -q \"http:/%s\" -O %s", path, baseimgfn );
        system ( wget );

  	if((fpbaseimg = fopen(baseimgfn, "r"))==NULL) {
       		print_error("Can not read baseimgfn\n");
   		exit(1);
   	}

      	fseek(fpbaseimg, 0L, SEEK_END);
      	fsizebi=ftell(fpbaseimg);

	fclose ( fpbaseimg );
 
	/* test if download is smaller 500 byte ( assuming that there is no real image smaller then 500 bytes ) */
		
	if ( fsizebi < 500 ) {
	
		/* if, what I have downloaded before, is smaller 500 bytes - stream the placeholder image */

		if(strcmp(type,"original") == 0)
		{
			if((fppool = fopen(pathtofbimgo, "r"))==NULL) {
       				print_error("Can not read pathtofbimgo\n");
      				exit(1);
   			}
      			fseek(fppool, 0L, SEEK_END);
   			end = ftell(fppool);

			show_image(0, end, fppool, 1, maxpoolsize); 
			exit(0); 
		}

		if(strcmp(type,"big") == 0)
		{
			if((fppool = fopen(pathtofbimgb, "r"))==NULL) {
       				print_error("Can not read pathtofbimgb\n");
      				exit(1);
   			}
      			fseek(fppool, 0L, SEEK_END);
               		end = ftell(fppool);
				
			show_image(0, end, fppool, 1, maxpoolsize);
			exit(0); 
        	}

		if(strcmp(type,"list") == 0)
		{
			if((fppool = fopen(pathtofbimgl, "r"))==NULL) {
        			print_error("Can not read pathtofbimgl\n");
      				exit(1);
   			}
      			fseek(fppool, 0L, SEEK_END);
               		end = ftell(fppool);

			show_image(0, end, fppool, 1, maxpoolsize); 
			exit(0); 
        	}

		if(strcmp(type,"thumbnail") == 0)
        	{
			if((fppool = fopen(pathtofbimgt, "r"))==NULL) {
        			print_error("Can not read pathtofbimgt\n");
      				exit(1);
   			}
      			fseek(fppool, 0L, SEEK_END);
                	end = ftell(fppool);

			show_image(0, end, fppool, 1, maxpoolsize);
			exit(0); 

		} else {

			if((fppool = fopen(pathtofbimgo, "r"))==NULL) {
        			print_error("Can not read pathtofbimgo\n");
      				exit(1);
   			}
      			fseek(fppool, 0L, SEEK_END);
   			end = ftell(fppool);

			show_image(0, end, fppool, 1, maxpoolsize);
			exit(0); 

			}

	} else {

		/* if not, create "original" image  (max 60 k) */

        	snprintf ( jhead, 256, "jhead -purejpg %s", baseimgfn );
        	system ( jhead );

		quali = 96;
        	fsize = omaxsize + 1;
        	fsizemax = omaxsize;

		while ( fsize > fsizemax ) {

			snprintf ( convert1, 256, "convert %s -size %ix%i -resize %ix%i +repage -format jpg -quality %i %s", baseimgfn, lreso, rreso, lreso, rreso, quali, target1fn );
                	system ( convert1 );
	
   			if((fptarget1 = fopen(target1fn, "r"))==NULL) {
        			print_error("Can not read target1fn\n");
   				exit(1);
   			}

      			fseek(fptarget1, 0L, SEEK_END);
      			fsize=ftell(fptarget1);

			quali = quali - 4;
			fclose(fptarget1);
		}

		/* create thumbnail image  (10 k) */

		quali = 97;
        	fsize = tmaxsize + 1;
        	fsizemax = tmaxsize;

		while ( fsize > fsizemax ) {

			fsize=convert_image(target1fn, dlrest, drrest, res4, quali, target4fn);
			quali = quali - 2;
		}

		/* create list image  (10 k) */

		quali = 97;
        	fsize = lmaxsize + 1;
        	fsizemax = lmaxsize;

		while ( fsize > fsizemax ) {

			fsize=convert_image(target1fn, dlresl, drresl, res3, quali, target3fn);
			quali = quali - 2;
		}

		/* create big image  (50 k) */

		quali = 97;
        	fsize = bmaxsize + 1;
        	fsizemax = bmaxsize;

		while ( fsize > fsizemax ) {

			fsize=convert_image(target1fn, dlresb, drresb, res2, quali, target2fn);
			quali = quali - 2;
		}


   		snprintf ( mkdir, 48, "mkdir -p %s", dir );
   		system ( mkdir );
	
   		if((fppooltmp = fopen(ppoolpathtmp, "wb"))==NULL) {
       			print_error("Can not read ppoolpathtmp\n");
   			exit(1);
   		}

		/* add image 1 (60 k) */
		add_image(target1fn, omaxsize, ostart, fppooltmp);

        	/* add image 2 (50 k) */
        	add_image(target2fn, bmaxsize, bstart, fppooltmp);

        	/* add image 3 (10 k) */
        	add_image(target3fn, lmaxsize, lstart, fppooltmp);

        	/* add image 4 (10 k) */
        	add_image(target4fn, tmaxsize, tstart, fppooltmp);

  		fclose ( fppooltmp ); 

   		snprintf ( rmbaseimg, 84, "rm -rf %s", baseimgfn );
   		snprintf ( rmtarget1, 84, "rm -rf %s", target1fn );
   		snprintf ( rmtarget2, 84, "rm -rf %s", target2fn );
   		snprintf ( rmtarget3, 84, "rm -rf %s", target3fn );
   		snprintf ( rmtarget4, 84, "rm -rf %s", target4fn );
   		snprintf ( mvtmptoreal, 256, "mv %s %s", ppoolpathtmp, ppoolpath );

   		system ( mvtmptoreal );

   		system ( rmbaseimg ); 
   		system ( rmtarget1 );
   		system ( rmtarget2 );
   		system ( rmtarget3 );
   		system ( rmtarget4 );
	}

   }

   /* stream images by selected type */ 

   if((fppool = fopen(ppoolpath, "r"))==NULL) {
       	print_error("Can not read ppoolpath\n");
	printf ("ppoolpathtmp: %s", ppoolpathtmp);
	printf ("ppoolpath: %s", ppoolpath);
  	exit(1);
   }

   if(strcmp(type,"original") == 0)
   {
	show_image(ostart, oend, fppool, 0, maxpoolsize);
   }

   if(strcmp(type,"big") == 0)
   {
        show_image(bstart, bend, fppool, 0, maxpoolsize);
   }

   if(strcmp(type,"list") == 0)
   {
        show_image(lstart, lend, fppool, 0, maxpoolsize);
   }

   if(strcmp(type,"thumbnail") == 0)
   {
        show_image(tstart, tend, fppool, 0, maxpoolsize);

   } else {

	if((fppool = fopen(pathtofbimgo, "r"))==NULL) {
        	print_error("Can not read pathtofbimgo\n");
      		exit(1);
   	}
   	start = 0;
      	fseek(fppool, 0L, SEEK_END);
   	end = ftell(fppool);

	show_image(start, end, fppool, 1, maxpoolsize);

   }

   fclose ( fppool );
   fclose ( fppooltest );

return 0;

}
