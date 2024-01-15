
#pragma once

#include "Log.h"

namespace Jahley
{
    class App
    {
     public:
        virtual ~App();

        // Method that runs the application's main loop, handling user input and redrawing the window as necessary.
        void run();

        // Virtual methods that can be overridden by subclasses to handle various events during the application's lifecycle,
        // such as initialization, crashes, input events, and window resizing.
        virtual void update() {}
        virtual void onInit() {}
        virtual void onCrash() {}

        // Methods for handling crashes that occur during the application's execution.
        void preCrash();
        void onFatalError (g3::FatalMessagePtr fatal_message);

     protected:
        App();

        // Boolean value indicating whether the application is currently running.
        bool isRunning = true;

        // Timestamp indicating when the application was started.
        std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();

     private:
        // Boolean value indicating whether the application is windowed or not.
        bool windowApp = false;

        // Callback function that is called in the event of a fatal error.
        FatalErrorCallback errorCallback = nullptr;

        // Callback function that is called before a crash occurs.
        PreCrashCallback preCrashCallback = nullptr;

        // Handler for logging messages generated by the application.
        LogHandler log;

        // Time interval indicating how frequently the application should redraw the window.
        std::chrono::milliseconds refreshWait;

        // Integer value indicating the refresh rate of the application's window, in milliseconds.
        int refresh = 16;

    }; // end class App

    // Implemented by client
    App* CreateApplication();

} // namespace Jahley