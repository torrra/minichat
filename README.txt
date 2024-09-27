Mini chat project

I. Connect client to server

	1. start server application
	2. start client appliction
	3. enter server IPv4 or IPv6 address in client console and press enter
	4. type client username and press enter
	5. client app should display a successful connection message,
	   while the server app will display the newly connected client's IP address


II. Disconnect client and shut down server

Disconnect client without shutting down server:

	1. Press ESC

Shut down server and all clients:

	1. send "/s shutdown" from a client app
	2. server app will exit and terminate connection with all clients
	3. client apps will close when enter is pressed


III. Special commands

	* Rename user:						"/u <username>"
	* Create new channel:				"/c <#channel-name>"
	* Move to an existing channel:		"/m <channel-name>"