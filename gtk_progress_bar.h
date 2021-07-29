#pragma once

#include <gtk/gtk.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <queue>

namespace SimpleGtkProgressBar{

    class ProgressBarUpdateData{
    public:
        std::queue <std::pair<size_t,std::string>>& updateMsgQueue;
        static std::mutex qMutex;
        GtkWidget* window;
        GtkWidget* label;
        ProgressBarUpdateData(std::queue <std::pair<size_t,std::string>>& msgQueue GtkWidget* win, GtkWidget* lbl):updateMsgQueue(msgQueue),window(win),label(lbl){

        }
    };

    static GtkWidget *parentWindow = nullptr;
    static GtkWidget* displayLabel = nullptr;
    static GtkWidget* progressBar = nullptr;
    static GtkWidget* vBox = nullptr;
    static std::mutex pBarCreateMutex;
    static SimpleGtkProgressBar* pBarInstance = nullptr;
    //SimpleGtkProgressBar(std::string winTtile,std::string labelInitText, size_t pBarInitState);

    //static std::thread pBarThread;
    static std::queue <std::pair<size_t,std::string>> updateQueue;
    static std::mutex qMutex;
    static std::condition_variable qCondVar;
    static bool bStopRequested;
    static gboolean ProcessUpdateQueue();
    static int InternalUpdateQueue(size_t increment, std::string lableText);
    //static void InternalProgressBarUpdate(GtkWidget* widget, gpointer data);  //if need to be passes as callback
    static void InternalProgressBarUpdate(size_t increment, std::string lableText);

    static bool initialized;


    static SimpleGtkProgressBar* GetProgressBar(std::string winTtile,std::string labelInitText, size_t pBarInitState);
    static int ShowProgressBar();
    static int UpdateProgressBar(size_t increment, std::string lableText);
    static int RemoveProgressBar(); 
}