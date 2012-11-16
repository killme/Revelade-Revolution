// update
SVAR(dlname, "");
VAR(dlprogress, 1, -1, 0);
VAR(dlsize, 1, 0, 0);
VAR(dlstotal, 1, 0, 0);
VAR(dlspeed, 1, 0, 0);
VAR(dlstatus, 1, 0, 0); // 0 = downloading/no download, 1 = successful, 2 = failed, 3 = cancelled
FILE *dlfile = NULL;
FILE *hdfile = NULL;
string dlfilepath = "";
string downloaddirpath = "";
string hdfilename = "";
bool dlcancel;
CURL *dlhandle = NULL;
ICOMMAND(dlfilepath, "", (), result(dlfilepath));

size_t downloadwritedata(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	if (dlcancel) return 0;
	return fwrite(ptr, size, nmemb, (FILE *)userdata);
}

float lastdlnow = 0;
int lastdltime = 0;

int downloadprog(void *a, double dltotal, double dlnow, double ultotal, double ulnow)
{
    dlprogress = dlnow/dltotal*100.0;
	dlsize = dlnow/0x100000; // bytes -> megabytes
	dlstotal = dltotal/0x100000;
	int ticks = SDL_GetTicks();

	if (ticks >= lastdltime+1000)
	{
		dlspeed = ((dlnow-lastdlnow)/double(ticks-lastdltime)*1.024 + dlspeed)/2.0;
		lastdlnow = dlnow;
		lastdltime = SDL_GetTicks();
	}

    return 0;
}

char *parsehdname(char *header)
{
	char *fnp = strstr(header, "filename");
	if (!fnp) return NULL;
	bool found = false, found2 = false;
	char fchar = '\0', *res = NULL;
	while (*++fnp)
	{
		if (*fnp=='\n' || *fnp=='\r')
		{
			*fnp = '\0';
			if (!fchar || fchar=='\n') found2 = true;
			break;
		}
		if (found)
		{
			if (fchar && *fnp==fchar)
			{
				*fnp = '\0';
				found2 = true;
				break;
			}
			else if (!fchar && *fnp=='"' || *fnp=='\'')
			{
				res = fnp+1;
				fchar = *fnp;
			}
			else if (!fchar && *fnp!=' ')
			{
				res = fnp;
				fchar = '\n';
			}
		}
		else if (*fnp=='=') found = true;
	}
	if (found2) return res;
	return NULL;
}

int downloadfunc(void * data)
{
	CURLcode ret = curl_easy_perform(dlhandle);

    if (ret != 0)   //Connection problems or file doesn't exists
    {
        remove(dlfilepath);
		if (dlcancel)
		{
			dlstatus = 3;
			dlcancel = false;
			showgui("dl_progress");
		}
		else
		{
			dlstatus = 2;
			showgui("dl_progress");
		}
    }
	else
	{
		if (hdfile)
		{
			fseek(hdfile, 0, SEEK_END);
			int hdsize = ftell(hdfile);
			rewind(hdfile);
			char *header = new char[hdsize];
			fread(header, 1, hdsize, hdfile);
			const char *hdfname = parsehdname(header);
			if (hdfname)
			{
				fclose(hdfile);
				hdfile = NULL;
				remove(hdfilename);

				defformatstring(newfilename)("%s%s", downloaddirpath, hdfname);
				if (fileexists(newfilename, "r")) remove(newfilename);
				rename(dlfilepath, newfilename);
				strcpy(dlfilepath, newfilename);
			}
			delete[] header;
		}
		dlstatus = 1;
		showgui("dl_progress");
	}

	dlprogress = -1;
	dlsize = 0;
	dlstotal = 0;
	dlspeed = 0;
	lastdltime = 0;

	curl_easy_cleanup(dlhandle);
	if (dlfile)
	{
		fclose(dlfile);
		dlfile = NULL;
	}
	if (hdfile)
	{
		fclose(hdfile);
		hdfile = NULL;
		remove(hdfilename);
	}

	return 0;
}

void download(const char *url, const char *filename)
{
	if (dlprogress >= 0)
	{
		conoutf("\fecan't start new download while an old one is still in progres. "
				"you must first cancel the active download or wait for it to finish.");
		return;
	}

	dlhandle = curl_easy_init();
	if(!dlhandle) return;

	dlstatus = 0;

	static const char *downloaddir = "downloads" PATHDIVS;
	strcpy(downloaddirpath, findfile(downloaddir, "w"));
	if(!fileexists(downloaddirpath, "w")) createdir(downloaddirpath);

	string murl;
	strcpy(murl, url);
    curl_easy_setopt(dlhandle, CURLOPT_URL, murl);

	if (!filename || filename[0]=='\0')
	{
		char *eurl[1000];
		curl_easy_getinfo(dlhandle, CURLINFO_EFFECTIVE_URL, eurl);
		for (int j = strlen(*eurl), i = j-1; i >= 0; i--)
		{
			if ((*eurl)[i] == '/' || (*eurl)[i] == '\\' || i == 0)
			{
				strncpy(dlname, (i==0)? (*eurl): (*eurl)+i+1, j-i);
				dlname[j-i] = '\0';
				break;
			}
			else if ((*eurl)[i] == '?') j = i-1;
		}
	}
	else strcpy(dlname, filename);

	if (dlname && dlname[0]) formatstring(dlfilepath)("%s%s", downloaddirpath, dlname);
	else formatstring(dlfilepath)("%stemp_%d", downloaddirpath, lastmillis);

	dlfile = fopen(dlfilepath, "wb");
	if (!dlfile)
	{
		conoutf("error opening file \"%s\" for saving", dlfilepath);
		return;
	}

	if (!filename || filename[0]=='\0')
	{
		formatstring(hdfilename)("%s", findfile("header.txt", "w"));
		hdfile = fopen(hdfilename, "w+");
		if (hdfile) curl_easy_setopt(dlhandle, CURLOPT_WRITEHEADER, hdfile);
	}

	curl_easy_setopt(dlhandle, CURLOPT_FOLLOWLOCATION,1);
	curl_easy_setopt(dlhandle, CURLOPT_WRITEFUNCTION, downloadwritedata);
	curl_easy_setopt(dlhandle, CURLOPT_WRITEDATA, dlfile);
	curl_easy_setopt(dlhandle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(dlhandle, CURLOPT_PROGRESSFUNCTION, downloadprog);
	curl_easy_setopt(dlhandle, CURLOPT_PROGRESSDATA, NULL);

	SDL_CreateThread(downloadfunc, dlhandle);
}

COMMAND(download, "ss");
ICOMMAND(stopdownload, "", (), dlcancel = true);
