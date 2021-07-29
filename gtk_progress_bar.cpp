#include <gtk/gtk.h>
#include <glib.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <queue>
#include <thread>
#include <chrono>

class gtkUserData{
    public:
    std::string win_title;
    std::string init_label_text;
    int pBar_init_fraction;
    gtkUserData(std::string win, std::string lbl,int frction):win_title(win),init_label_text(lbl),pBar_init_fraction(frction){

    }
};
GtkApplication *app = nullptr;
static GtkWidget *parentWindow = nullptr;
static GtkWidget* displayLabel = nullptr;
static GtkWidget* progressBar = nullptr;
static GtkWidget* vBox = nullptr;
static std::queue <std::pair<size_t,std::string>> updateQueue;
static std::mutex qMutex;
static bool bStopRequested = false;
std::thread gtkMainLoopThread;

static void RemoveProgressBar(){
    bStopRequested = true; 
    g_application_quit(G_APPLICATION(app));
}

static void InternalProgressBarUpdate(size_t increment, std::string lableText="__DEFAULT_TEXT__"){
    //to be called by ProcessUpdateQueue if there is data in queue
    double curProgress = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progressBar));
    curProgress +=(increment*0.1);
    if(static_cast<int>(curProgress) >= 1){
        curProgress = 0.9;
    }
    std::string curProgress_text = std::to_string(static_cast<int>(curProgress*10))+std::string("%");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar),curProgress);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressBar), curProgress_text.c_str());
    if(lableText != "__DEFAULT_TEXT__"){
        gtk_label_set_text(GTK_LABEL(displayLabel),lableText.c_str());
    }
}

static gboolean UpdateProgressBarCallback(gpointer data){
    if(bStopRequested){
        return false;
    }
    std::unique_lock<std::mutex> qlock(qMutex);
    while(!updateQueue.empty()){
        auto data = updateQueue.front();
        updateQueue.pop();
        InternalProgressBarUpdate(data.first, data.second);
    }
    return true;
}

static void ActivateSimpleGtkProgressBar(GtkApplication *app, gpointer user_data){
    std::string winTtile="Default";
    std::string labelInitText="";
    size_t pBarInitState =0;
    if(user_data){
        gtkUserData* gtk_user_data = (gtkUserData*)user_data;
        std::string winTtile= gtk_user_data->win_title;
        std::string labelInitText = gtk_user_data->init_label_text;
        size_t pBarInitState = gtk_user_data->pBar_init_fraction;
    }

    bStopRequested = false;
    //parentWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    parentWindow = gtk_application_window_new(app);
    gtk_window_set_title (GTK_WINDOW (parentWindow), winTtile.c_str());
    g_signal_connect(parentWindow,"delete-event",G_CALLBACK(RemoveProgressBar),NULL);
    displayLabel =gtk_label_new(labelInitText.c_str());
    progressBar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar),static_cast<double>(pBarInitState)/10);
    g_timeout_add (500, UpdateProgressBarCallback, nullptr);
    vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    gtk_box_pack_start(GTK_BOX(vBox),progressBar,0,0,0);
    gtk_box_pack_start(GTK_BOX(vBox),displayLabel,0,0,0);
    gtk_container_add(GTK_CONTAINER(parentWindow),vBox);
    gtk_widget_show_all(parentWindow);
}


static void UpdateProgressBar(size_t increment, std::string lableText="__DEFAULT_TEXT__"){
    if(bStopRequested){
        return;
    }
    std::unique_lock<std::mutex> qlock(qMutex);
    updateQueue.push(std::make_pair(increment,lableText));
}

static void InitializeProgressBar(int argc, char** argv, std::string win_title, std::string init_label_text, int init_pbar_fraction){
    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    gtkUserData* data =new gtkUserData(win_title,init_label_text,init_pbar_fraction);
	g_signal_connect(app, "activate", G_CALLBACK(ActivateSimpleGtkProgressBar), (gpointer)data);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
}

static void CreateProgressBar(int argc,char** argv,std::string win, std::string lbl, int fraction){
    gtkMainLoopThread = std::thread(&InitializeProgressBar, std::ref(argc),std::ref(argv), win,lbl, fraction);
     
}

static void DestroyProgressBar(){
    RemoveProgressBar();   
    gtkMainLoopThread.join();
}

int main(int argc, char** argv){	
    //CreateProgressBar(argc, argv,"Test", "......", 0);
    gtkMainLoopThread = std::thread(&InitializeProgressBar, std::ref(argc),std::ref(argv), "Test", "......", 0);

 	UpdateProgressBar(1,"Initializing...");
    std::this_thread::sleep_for (std::chrono::seconds(2));
    UpdateProgressBar(2,"In Progress...");
    std::this_thread::sleep_for (std::chrono::seconds(2));
    UpdateProgressBar(2,"In Progress.");
    std::this_thread::sleep_for (std::chrono::seconds(2));
    UpdateProgressBar(2,"In Progress...");
    std::this_thread::sleep_for (std::chrono::seconds(2));
    UpdateProgressBar(2,"Almost there..");
    std::this_thread::sleep_for (std::chrono::seconds(2));
    UpdateProgressBar(1,"Finishing...");
    std::this_thread::sleep_for (std::chrono::seconds(2));

    DestroyProgressBar();
    return 0;
}