# Mini chat project

Simple client/server chat applications.
Both the server application depend on the Network library, which uses WinSock2.

## Table of contents

* [Preview](https://github.com/torrra/minichat?tab=readme-ov-file#preview)
* [How to build](https://github.com/torrra/minichat?tab=readme-ov-file#how-to-build)
* [How to use](https://github.com/torrra/minichat?tab=readme-ov-file#how-to-use)

## Preview

The client application supports both IPv4 and IPv6 local addresses to connect to a server.

https://github.com/user-attachments/assets/fcce1a15-5799-4fe1-a3fe-ed833bd81e22



<br>

## How to build

This project uses CMake as its build system. To build it you will need:  
* Windows
* Git
* CMake 3.25 or higher
* Visual studio 2022 version 17

<br>

### Step 1 : clone the repo  
  
Open CMD and move open the directory in which you wish to install the repo. You can either press your Win key and type out 'cmd', then use 'cd <target_folder>", or open windows file explorer and right click inside to open in terminal. You can now enter the following command:

```bash
git clone git@github.com:torrra/minichat.git
```
<br>

### Step 2 : generate visual studio solution

Once you have downloaded the repo, you need to open the folder with `cd minichat`, and run CMake with either:

```bash
mkdir build
cd build
cmake ..
```

or

```bash
mkdir build
cmake -B build -S .
```

<br>

### Step 3 : open Visual Studio

After CMake has successfully configured the VS solution, you can open it with your file explorer, or by using 
```
cmake --open .
``` 
or ```cmake --open build``` if you ran the second CMake command shown in the previous step.  
Once Visual Studio has finished starting up, you can select Debug or Release and click on 'Local Windows Debugger' to compile everything. The server application will start automatically.
To launch a client application, right click on the MiniChatClient project in the solution explorer, hover over "Debug" and click on "Start New Instance".

<br>

## How to use

### Connect client to server

	1. start server application
	2. start client appliction
	3. enter server IPv4 or IPv6 address in client console and press enter
	4. type client username and press enter
	5. client app should display a successful connection message,
	   while the server app will display the newly connected client's IP address

<br>

### Disconnect client and shut down server

**Disconnect client without shutting down server:**

	1. Press ESC

**Shut down server and all clients:**

	1. send "/s shutdown" from a client app
	2. server app will exit and terminate connection with all clients
	3. client apps will close when enter is pressed

<br>

### Special commands

	* Rename user:				"/u <username>"
	* Create new channel:			"/c <#channel-name>"
	* Move to an existing channel:		"/m <channel-name>"
	* List other users in channel		"/l"
