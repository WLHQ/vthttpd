# vthttpd
Very Tiny HTTPD for Nintendo 3DS consoles (fork of [3ds-httpd by dimaguy](https://github.com/dimaguy/3ds-httpd))

This homebrew is meant to run as a *dedicated* HTTP daemon via autoboot. As such, if you're launching it via the Homebrew Launcher, the only way to return to the HOME menu is to reboot your console.

vthttpd will shut down both screens when launched to minimize power usage. Logs are stored in sdmc:/vthttpd

The "noservices" version will attempt to kill some services provided by the 3DS' firmware to free extra resources. **This has only been tested on old 3DS consoles and will very likely result in a crash on a n3DS/n2DS.**

(If you are compiling vthttpd yourself and you don't want it to kill said services on launch, comment out terminateUnnecessaryServices() in src/server.c)

This fork includes *many* changes compared to 3ds-httpd (a good bunch of which are shown in action with the included example HTML page), check the changelog below

#### Quick setup guide

1) Download one of the releases and extract it on the root of your SD card
2) The "html" folder *on the root of the SD card* includes an example website: either leave it as-is for a quick test, or replace it with your own files
3) Put your SD card back in your 3DS, power it on, make sure it's connected to a Wi-Fi network and launch vthttpd. *Do not worry if nothing is shown on screen, this is expected behavior: see above*
4) The 3DS should now serve your website: you can connect to it using another device

If something isn't working as expected or you want to check who's connecting to your website, logs can be found in sdmc:/vthttpd

#### Changelog
```
== 19/11/2024 ==
- Initial public release

== 26/06/2023 ==
- The server will now return a fatal error in case it gets disconnected from Wi-Fi
	- Once again, this also automatically reboots the system with my CFW fork, which means the error will be logged and then vftpd will get restarted on boot
- The server will also periodically turn off the backlight
	- This was done as there is a rather rare occurrence in which the backlight will get turned on agan on their own

== 19/06/2023 ==
- The server will now initialize and connect to Wi-Fi manually instead of relying on the 3DS's OS to do so
	- This gives us several added bonuses:
		- The connection should now be more reliable in general, as letting the OS configure it on its own at an early boot stage with several disabled components might be unreliable
		- The Wi-Fi AP config currently gets retrieved from system settings (1st connection slot) but it's now technically possible to add code to allow loading AP config from the SD card
	- failExit now writes fatal errors to the logfile correctly
- Added code to turn off both screens' backlight as soon as the server starts
	- In my specific case, this was handled by my own CFW fork before. However, after rebasing my CFW fork on the latest version of Luma3DS, I've decided it'd be more appropriate to let the application handle this instead

== 18/06/2023 ==
- Implemented a basic, rotating log system
	- The logfile's paths are /vthttpd/latest.log and /vthttpd/prev.log
- General code optimization

== 17/06/2023 ==
- Removed screen init and console-related functions (for my current project the software is meant to run headless on a dedicated 2DS console anyway, with the screen turned off)
	- The function printTop has been stubbed instead of being removed in preparation of a logging system, as it is primarily used to log events on the top screen
	- On the other hand, printBottom has been removed completely as it was only used for the eManual, so it already went unused when that feature got taken out
- Unnecessary background services will now be terminated before the server starts. This includes:
	- mic (Microphone)
	- ir (Infrared)
	- camera (Camera)
	- am (Application manager)
	- hid (Human Interface Device)
	- dlp (Download Play)
	- codec (Codec)
	- news (Notifications)
	- csnd (Sound channels)
	- dsp (Digital Signal Processor / Audio playback)
	- act (Nintendo Account)
	- nfc (Near Field Communications)
	- loader (Process loader)
	- mp (Local 3DS/DS multiplayer)
	- spi (SPI)
	- pdn (PDN register handler)
	- ro (.ro library loader)
		- Some of these services are essential for regular 3DS usage. If you do not mean to use vthttpd on a dedicated console as an HTTP server like I do, you might not want to terminate them
- Moved the custom HTML error page template's path from /html/error.html to /vthttpd/error.html
	- This was changed so that the template error page is not accessible to the end user

== 14/06/2023 ==
- sysinfo handler refactor (it now returns a single json file with all the data when accessing /sysinfo/ instead of several different endpoint)
	- Therefore, sysinfo will now send a Content-Type of application/json instead of text/plain
- Removed DEFAULT_PAGE in favor of errorPage
	- 3DS-HTTPD used DEFAULT_PAGE as a static, hardcoded error 501 page which got served to the client whenever no handlers were able to process the request
		- This has been replaced with a generic HTML page which displays the correct error code and description whenever an error occurs
	- Additionally, a custom template error page can now be loaded from /html/error.html
- Minor code optimization

== 10/06/2023 ==
- MIME type parser refactor (clearer code and fix for a pointer error)
- Added /sysinfo/batteryvoltage to the sysinfo handler (returns the battery's current *raw* voltage level, must be converted to volts in order to be humanly readable)

== 09/06/2023 ==
- Removed the read, write, crypt and system handlers (you *do not* want to expose those to the world wide web for obvious reasons!)
- Default port changed from 8081 to 80
- The sdcard handler has been greatly refactored:
	- It will now serve files from a subdirectory (/html/) instead of the SD card's root
	- Added proper support for serving index.html and favicon.ico from the SD card instead of being hardcoded in the server's executable
		- This effectively supersedes the default and favicon request handlers
	- Added support for serving binary files instead of only plaintext files
	- Added support for the following MIME types:
		- image/x-icon
		- image/png
		- image/bmp
		- image/gif
		- image/webp
		- image/jpeg
		- text/css
		- text/csv
		- text/javascript
		- text/plain
		- application/xml
		- application/octet-stream
	- Please note: support for more MIME types is possible, but this list comprises the only ones currently implemented due to my specific use case. text/plain is currently only used by the sysinfo handler
- Added the sysinfo handler. This is a handler that exposes some useful, non-sensitive information about the system, in particular:
	- /sysinfo/batterylevel returns the system's battery percentage
	- /sysinfo/chargestate returns whether the system's battery is currently charging or not charging (respectively 1/0)
- The server will now wait for the Wi-Fi connection to be up before starting
- When a critical error occurs, the application will now throw a fatal system error instead of quitting (done to aid with my use case, as my cfw fork is set to reboot the system automatically on fatal)
- Removed button inputs, software keboard and eManual (done to aid with my use case, as it's supposed to be a headless http server so no user input is needed)
```
