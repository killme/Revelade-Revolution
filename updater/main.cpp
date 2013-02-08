#include "cube.h"
#include "version.h"

#include <curl/curl.h>

const char *infostring =
"\nRevelade Revolution Updater Tool "RR_VERSION_STRING" - "RR_VERSION_DATE"\n\n"
"About: This tool will download and install updates automatically.\n"
"       It's not yet fully functional and should only be use internally by the\n"
"       game.\n\n"
"Usage: updater [source] [switches]\n\n"
"Switches: -s[source] : sets the source archive to unpack (and download to).\n"
"                       default if a switch is not provided\n"
"          -t[target] : sets target directory to extract to.\n"
"                       set to current working directory by default\n"
"          -u[url]    : sets the URL to download the package from.\n"
"                       if [source] is not set the package is saved to\n"
"                       \"update.zip\"\n"
"          -v         : sets verbose mode on\n"
"          -r         : removes source file after archiving.\n"
"                       always on if [url] is set and [source] isn't\n"
"          -h         : prints this message.\n"
"                       default if no arguments are passed\n";

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
	vprintf(fmt, args);
	putchar('\n');
    va_end(args); 
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
	vprintf(fmt, args);
	putchar('\n');
    va_end(args);
}

bool verbose = false, removesource = false;

void info(const char *fmt, ...)
{
	if (!verbose) return;
    va_list args;
    va_start(args, fmt);
	vprintf(fmt, args);
	putchar('\n');
    va_end(args);
}

void fatal(const char *fmt, ...)
{
	printf("Fatal error: ");
    va_list args;
    va_start(args, fmt);
	vprintf(fmt, args);
	putchar('\n');
    va_end(args);
	exit(EXIT_FAILURE);
}

size_t downloadwritedata(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return fwrite(ptr, size, nmemb, (FILE *)userdata);
}

int dprogress = 0;

int downloadprog(void *a, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int prog = dlnow/dltotal*50.0;
	if (prog > dprogress) putchar('-');
	dprogress = prog;
    return 0;
}

bool download(const char *url, const char *filename)
{
	CURL *handle = curl_easy_init();
	if(!handle) return false;

	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		conoutf("Error opening file \"%s\" for saving", filename);
		return false;
	}

	conoutf("%%0                      %%50                    %%100");
	conoutf("|                        |                       |");

    curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, downloadwritedata);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, downloadprog);
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, NULL);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 120);

	CURLcode ret = curl_easy_perform(handle);

    if (ret != 0)
    {
        remove(filename);
		conoutf("\nCan not download file \"%s\". Connection problems or file does not exist.\nError code: %d", url, ret);
    }

	fclose(file);
	curl_easy_cleanup(handle);

	printf("\n\n");

	return !ret;
}

#if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
void stackdumper(unsigned int type, EXCEPTION_POINTERS *ep)
{
    if(!ep) fatal("unknown type");
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *context = ep->ContextRecord;
    string out, t;
    formatstring(out)("Revelade Revolution Win32 Exception: 0x%x [0x%x]\n\n", er->ExceptionCode, er->ExceptionCode==EXCEPTION_ACCESS_VIOLATION ? er->ExceptionInformation[1] : -1);
    STACKFRAME sf = {{context->Eip, 0, AddrModeFlat}, {}, {context->Ebp, 0, AddrModeFlat}, {context->Esp, 0, AddrModeFlat}, 0};
    SymInitialize(GetCurrentProcess(), NULL, TRUE);

    while(::StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        struct { IMAGEHLP_SYMBOL sym; string n; } si = { { sizeof( IMAGEHLP_SYMBOL ), 0, 0, 0, sizeof(string) } };
        IMAGEHLP_LINE li = { sizeof( IMAGEHLP_LINE ) };
        DWORD off;
        if(SymGetSymFromAddr(GetCurrentProcess(), (DWORD)sf.AddrPC.Offset, &off, &si.sym) && SymGetLineFromAddr(GetCurrentProcess(), (DWORD)sf.AddrPC.Offset, &off, &li))
        {
            char *del = strrchr(li.FileName, '\\');
            formatstring(t)("%s - %s [%d]\n", si.sym.Name, del ? del + 1 : li.FileName, li.LineNumber);
            concatstring(out, t);
        }
    }
    fatal(out);
}
#endif

const int mb4 = 4194304; // 4 MB

string target = ".", source = "update.zip", url = "";
int targetlen;
char temp[mb4];

bool extractfile(const char *filename)
{
	bool ret = true;
	stream *file = openzipfile(filename, "rb");
	if (!file)
	{
		conoutf("Error opening file \"%s\"", filename);
		ret = false;
	}
	else
	{
		int size = file->size();
		char *data = temp;
		if (size > mb4) data = new char[size];
		file->read(data, size);
		file->close();

		defformatstring(tg)("%s/%s", target, filename);
		static string buf;
		strncpy(buf, target, targetlen);
		int i = targetlen;
		while (tg[i])
		{
			if (tg[i] == '/' || tg[i] == '\\')
			{
				tg[i] = buf[i] = PATHDIV;
				buf[i+1] = '\0';
				if (i > targetlen && !fileexists(buf, "w") && !createdir(buf))
				{
					conoutf("Error creating directory \"%s\"", buf);
					ret = false;
				}
			}
			else buf[i] = tg[i];
			i++;
		}
		buf[i] = '\0';

		file = openfile(buf, "wb");
		if (!file)
		{
			conoutf("Error opening file \"%s\" for writing", buf);
			ret = false;
		}
		else
		{
			file->write(data, size);
			file->close();
		}
		if (data != temp) delete[] data;
	}
	return ret;
}

int main(int argc, char **argv)
{
    //#ifdef WIN32
    ////atexit((void (__cdecl *)(void))_CrtDumpMemoryLeaks);
    //#ifndef _DEBUG
    //#ifndef __GNUC__
    //__try {
    //#endif
    //#endif
    //#endif

	if (argc <= 1)
	{
		conoutf(infostring);
		return EXIT_SUCCESS;
	}

	bool keep = false;
	bool forceremove = false;
    for(int i = 1; i<argc; i++)
    {
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 't': strcpy(target, &argv[i][2]); targetlen = strlen(target); break;
            case 's': strcpy(source, &argv[i][2]); keep = true; break;
			case 'u': strcpy(url, &argv[i][2]); removesource = true; break;
			case 'v': verbose = true; break;
			case 'r': forceremove = true; break;
			case 'h': conoutf(infostring); return EXIT_SUCCESS; break;
        }
		else
		{
			strcpy(source, argv[i]);
			keep = true;
		}
    }
	if (keep) removesource = false;
	if (forceremove) removesource = true;

	conoutf("\nRevelade Revolution Updater Tool %s - %s\n", RR_VERSION_STRING, RR_VERSION_DATE);
	
	if (url[0])
	{
		info("Initialising network module..."); // or "Initializing"
		if(enet_initialize()<0) fatal("Unable to initialise network module");
		atexit(enet_deinitialize);
		enet_time_set(0);

		if (!download(url, source)) fatal("Could not download update package");
		conoutf("Update downloaded successfully. Installing...");
	}
	else conoutf("Installing update...");

	if (!addzip(source)) fatal("Can not read source archive \"%s\"", source);
		
	defformatstring(tg)("%s/", target);
	static string buf;
	int i = 0;
	while (tg[i])
	{
		if (tg[i] == '/' || tg[i] == '\\')
		{
			tg[i] = buf[i] = PATHDIV;
			buf[i+1] = '\0';
			while (tg[i] == '/' || tg[i] == '\\') i++;
			i--;
			if (!fileexists(buf, "w") && !createdir(buf))
				fatal("Can not create target directory \"%s\"", buf);
		}
		else buf[i] = tg[i];
		i++;
	}
	buf[i] = '\0';
	strcpy(target, buf);
	targetlen = i;

	vector<char *> files, failed;
	listallzipfiles(files);
	for (int i = 0; i < files.length(); i++)
	{
		info("Extracting: %s", files[i]);
		if (!extractfile(files[i])) failed.add(files[i]);
	}

	if (failed.length())
	{
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < failed.length(); i++)
			{
				info("Attempting to extract: %s", files[i]);
				if (extractfile(files[i])) failed.remove(i);
			}
		}
	}

	if (removesource)
	{
		removezip(source);
		remove(source);
	}

	conoutf("\nUpdate installation complete!");

    return EXIT_SUCCESS;

    //#if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
    //} __except(stackdumper(0, GetExceptionInformation()), EXCEPTION_CONTINUE_SEARCH) { return 0; }
    //#endif
}
