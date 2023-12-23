# Social-Media-App
A social media program that allows the users to communicate with another asynchronously using sockets in C.

Example run of the app:
![image](https://github.com/Dev301203/Social-Media-App/assets/104722753/7aca8145-f45f-40f6-ab09-1346053cc9be)

## Instructions to Run
### Setup Server
In the Makefile, update the port to be set to what port number you want the server to run on.
Then, compile the code using the makefile by running

`make`

You can then start the server by running

`./friend_server`

### Setup Client
Since the client side has not been written yet (future possible update), use netcat to to connect clients to the server by:

`netcat -C localhost PORT`

Replacing localhost and PORT with the full name of the machine and the PORT where the server is running.

Once there is a succesfull connection, you can use the app.

### List of Commands
View a users profile identified with their username
`profile <username>`

List all the users in the server
`list_users`

View a users profile
`profile <username>`

Become friends with a user
`make_friends <username>`

Post a message to some user
`post <username> <message>`

Quit and close connection to the server
`quit`
