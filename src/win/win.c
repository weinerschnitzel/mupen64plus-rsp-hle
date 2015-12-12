
#include "win.h"
#include "resource.h"
#include "Config.h"
#include "../Rsp.h"


void (*getDllInfo)(PLUGIN_INFO *PluginInfo);
static HMODULE audiohandle = NULL;
char audioname[100];


char AppPath[MAX_PATH];

void ShowMessage(LPSTR lpszMessage) 
{ 
   MessageBox(NULL, lpszMessage, "Info", MB_OK); 
}

void getAppFullPath (char *ret) {
    char drive[_MAX_DRIVE], dirn[_MAX_DIR] ;
	char fname[_MAX_FNAME], ext[_MAX_EXT] ;
	char path_buffer[_MAX_DIR] ;
	
	GetModuleFileName(dll_hInstance, path_buffer, sizeof(path_buffer)); 
    _splitpath(path_buffer, drive, dirn, fname, ext);
    strcpy(ret, drive);
    strcat(ret, dirn);
}

typedef struct _plugins plugins;
struct _plugins
{
    char *file_name;
    char *plugin_name;
    HMODULE handle;
    int type;
    plugins *next;
};
static plugins *liste_plugins = NULL, *current;

void insert_plugin(plugins *p, char *file_name,
		   char *plugin_name, void *handle, int type,int num)
{
    if (p->next)
        insert_plugin(p->next, file_name, plugin_name, handle, type, 
                               (p->type == type) ? num+1 : num);
    else
    {
        p->next = malloc(sizeof(plugins));
        p->next->type = type;
        p->next->handle = handle;
        p->next->file_name = malloc(strlen(file_name)+1);
        strcpy(p->next->file_name, file_name);
        p->next->plugin_name = malloc(strlen(plugin_name)+7);
        sprintf(p->next->plugin_name, "%s", plugin_name);
        p->next->next=NULL;
    }
}

void rewind_plugin()
{
   current = liste_plugins;
}

char *next_plugin()
{
   if (!current->next) return NULL;
   current = current->next;
   return current->plugin_name;
}

int get_plugin_type()
{
   if (!current->next) return -1;
   return current->next->type;
}

void *get_handle(plugins *p, char *name)
{
   if (!p->next) return NULL;
   if (!strcmp(p->next->plugin_name, name))
         return p->next->handle;
   else  
         return get_handle(p->next, name);
}
char* getExtension(char *str)
{
    if (strlen(str) > 3) return str + strlen(str) - 3;
    else return NULL;
}

void (*processAList)();

BOOL APIENTRY
DllMain (
  HINSTANCE hInst     /* Library instance handle. */ ,
  DWORD reason        /* Reason this function is being called. */ ,
  LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
        dll_hInstance = hInst  ;
        getAppFullPath(AppPath);    
        LoadConfig();
        break;

      case DLL_PROCESS_DETACH:
        break;
      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}

INT_PTR CALLBACK ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
			if   (AudioHle==FALSE) {
               CheckDlgButton(hwnd, IDC_ALISTS_INSIDE_RSP, BST_CHECKED);
               break;
			}
			else {
               CheckDlgButton(hwnd, IDC_ALISTS_EMU_DEFINED_PLUGIN, BST_CHECKED);
               break;   
			}
        case WM_CLOSE:
            EndDialog(hwnd, IDOK);  
			break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    SaveConfig();
                    EndDialog(hwnd, IDOK);
					break;
                case IDC_ALISTS_INSIDE_RSP:
                    AudioHle = FALSE;
					break;
                case IDC_ALISTS_EMU_DEFINED_PLUGIN:
                    AudioHle = TRUE;
					break;
            }
			break;
			default:
				return FALSE;
    }
    return TRUE;
}  
