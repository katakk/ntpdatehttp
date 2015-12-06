//  
//  
//  
//  https://ntp-a1.nict.go.jp/cgi-bin/time
//  https://ntp-b1.nict.go.jp/cgi-bin/time
//  http://ntp-a1.nict.go.jp/cgi-bin/time
//  http://ntp-b1.nict.go.jp/cgi-bin/time
//  


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/time.h>

char timedata[256];
char* ptimedata;
	int v = 0;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	int tag = 0;
	int read;
	char *p;
	p = buffer;
	for(read = 0; read < nmemb;read++)
	{
		if(p[read] == '\r') continue;
		if(p[read] == '\n') continue;
		if(!tag && p[read] == '<') tag = 1;
		if(!tag) {
			if(!strchr("0123456789.",p[read])) continue;
			*ptimedata++ = p[read];
		}
		if( tag && p[read] == '>') tag = 0;
	}
	return nmemb;
}

int get(const char *url)
{
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		//  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	
	return 0;
}

int ntpdate(const char *url)
{
	int res;
	struct timeval tv_before;
	struct timeval tv_after;
	struct timeval tv_ntp;
	struct timeval tv_diff;
	struct timeval tv_set;

	ptimedata = timedata;

#define TD (25567UL * 24UL * 60UL * 60UL)
	
	gettimeofday(&tv_before, NULL);
	get(url);
	ptimedata = strchr(timedata, '.');
	if(ptimedata) { *ptimedata = '\0'; ptimedata++; }
	tv_ntp.tv_sec = strtoul(timedata, NULL, 10);
	tv_ntp.tv_sec -= TD;
	tv_ntp.tv_usec = strtoul(ptimedata, NULL, 10);
	gettimeofday(&tv_after, NULL);

	timerclear(&tv_diff);
	timersub(&tv_after, &tv_before, &tv_diff);


	timerclear(&tv_set);
	timeradd(&tv_ntp, &tv_diff, &tv_set);

	timerclear(&tv_diff);
	gettimeofday(&tv_after, NULL);
	timersub(&tv_set, &tv_after, &tv_diff);
	if( tv_diff.tv_sec > 600 ) tv_diff.tv_sec = 600;
	if( ((signed)tv_diff.tv_sec) < -600  ) tv_diff.tv_sec = -600;
	timerclear(&tv_set);
	gettimeofday(&tv_after, NULL);
	timeradd(&tv_after, &tv_diff, &tv_set);

	res = settimeofday(&tv_set, NULL);
	if(res) err(res, "%s", ctime(&tv_set.tv_sec));
	if(v) printf(" %32s: %4ld.%06lu : %12lu.%06lu %32s", url, tv_diff.tv_sec, tv_diff.tv_usec, tv_set.tv_sec, tv_set.tv_usec,
	 ctime(&tv_set.tv_sec));
	return res;
}

int main (int argc, char **argv)
{
	int c;
	int i;

	while ((c = getopt (argc, argv, "v")) != -1)
		switch (c)
		{
		case 'v':
			v = 1;
			break;
		}

	for(i =0; i < 2; i++) {
		if(ntpdate("http://ntp-b1.nict.go.jp/cgi-bin/ntp")) break;
		usleep(700000);
		if(ntpdate("http://ntp-a1.nict.go.jp/cgi-bin/ntp")) break;
		usleep(700000);
	}
}

