
#### Welcome to the guide of this chat application. 

***

<em>First of all, after having compiled the server and the client with the <strong>make file</strong> command (from the project root directory), you have to make sure that in the server directory there is a <strong>listeCommand.txt</strong> file and a <strong>listChannel.txt</strong> file, otherwise the chat won't work. </em>

<em>Thus, as you have understood, you should not move the server executable from its directory. On the other hand, for the client one, you can place it anywhere and on any other computer.</em>

***

## Server usage : 

To launch the server, go to the directory containing the server executable and type <em><strong>./server</em></strong> followed by the port on which you want to launch it (<em><strong>./server 8080</em></strong> for example). 

Then, you arrive on the administrator session which allows you to do four different things:
- receive complaints from users
- send global messages to users (<em><strong>/all CONTENT</em></strong)
- send individual messages to users (<em><strong>/msg USER CONTENT</em></strong)
- eject a user from the server (<em><strong>/kick USER</em></strong)

Finally, if you want to stop it, you just have to quit the administrator session by doing <em><strong>@quit</em></strong> then do a <em><strong>Ctrl-C</em></strong> and write <em><strong>y</em></strong>.

***

## Client usage :

To launch the client, after launching the server, go to the same directory as the client executable and type ./client followed by the ip address of the server and the port number on which the server is launched (./server 127.0.0.1 8080 for example).

Then wait to be accepted by the server and type a user name. Now you can chat freely. By default, when you write a message without a command, it is sent to the channel you are in (public by default). However, all the following commands are available:


- @quit -- close the session
- /help -- command to have the list of possible commands
- /msg USER CONTENT -- command to send a private message to a user
- /list -- show the list of available users
- /listC -- show the list of users in the current channel
- /files -- list files of the server
- /chann -- list of channels available on the server
- /connect CHANNEL -- connect to a certain channel
- /create CHANNEL -- create a new channel
- /delete CHANNEL -- delete an existing channel
- /all CONTENT -- send a message to all users
- /report CONTENT -- send a message to the admin
- &files -- list files of the client
- &up FILENAME -- upload a file on the server
- &dl FILENAME -- download a file from the server

# 