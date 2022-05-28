
### Welcome to the guide of this chat application. 

***

<em>First of all, after having compiled the server and the client with the <strong>make file</strong> command (from the project root directory), you have to make sure that in the server directory there is a <strong>listeCommand.txt</strong> file and a <strong>listChannel.txt</strong> file, otherwise the chat won't work.</em>

<em>Thus, as you have understood, you should not move the server executable from its directory. 
On the other hand, for the client one, you can place it anywhere and on any other computer.</em>

***

## Server usage : 

To launch the server, go to the directory containing the server executable and type <em><strong>./server</strong></em>followed by the port on which you want to launch it (<em><strong>./server 8080</strong></em> for example). 

Then, you arrive on the administrator session which allows you to do four different things:
- receive complaints from users
- send global messages to users (<em><strong>/all CONTENT</strong></em>)
- send individual messages to users (<em><strong>/msg USER CONTENT</strong></em>)
- eject a user from the server (<em><strong>/kick USER</strong></em>)

Finally, if you want to stop it, you just have to quit the administrator session by doing <em><strong>@quit</strong></em> then do a <em><strong>Ctrl-C</strong></em> and write <em><strong>y</strong></em>.

***

## Client usage :

<em>To launch the client, after launching the server, go to the same directory as the client executable and type <strong>./client</strong> followed by the ip address of the server and the port number on which the server is launched (<strong>./server 127.0.0.1 8080</strong> for example).</em>

Then wait to be accepted by the server and write a user name. 
Now you can chat freely. By default, when you write a message without a command, it is sent to the channel you are in (<em><strong>public</strong></em> by default). 
However, all the following commands are available:
- <em><strong>/help</strong></em> -- command to have the list of possible commands
- <em><strong>/msg USER CONTENT</strong></em> -- command to send a private message to a user
- <em><strong>/list</strong></em> -- show the list of available users
- <em><strong>/listC</strong></em> -- show the list of users in the current channel
- <em><strong>/files</strong></em> -- list files of the server
- <em><strong>/chann</strong></em> -- list of channels available on the server
- <em><strong>/connect CHANNEL</strong></em> -- connect to a certain channel
- <em><strong>/create CHANNEL</strong></em> -- create a new channel
- <em><strong>/delete CHANNEL</strong></em> -- delete an existing channel
- <em><strong>/all CONTENT</strong></em> -- send a message to all users
- <em><strong>/report CONTENT</strong></em> -- send a message to the admin
- <em><strong>&files</strong></em> -- list files of the client
- <em><strong>&up FILENAME</strong></em> -- upload a file on the server
- <em><strong>&dl FILENAME</strong></em> -- download a file from the server

Finally, if you want to leave the chat, just type <em><strong>@quit</strong></em>.

*** 

### Created by Jason, Richard and Thomas, all rights reserved.
